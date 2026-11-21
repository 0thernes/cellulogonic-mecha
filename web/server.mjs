import http from "node:http";
import fs from "node:fs";
import path from "node:path";
import { spawn } from "node:child_process";
import readline from "node:readline";
import { fileURLToPath } from "node:url";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const ROOT = path.resolve(__dirname, "..");
const PORT = Number(process.env.PORT || 8788);
const BIN = process.platform === "win32"
  ? path.join(ROOT, "bin", "cellulogonic.exe")
  : path.join(ROOT, "bin", "cellulogonic_linux");

const ALLOWED_STATIC = new Set(["index.html", "app.js", "renderer3d.js", "styles.css"]);
const MIME = {
  ".html": "text/html; charset=utf-8",
  ".js": "application/javascript; charset=utf-8",
  ".css": "text/css; charset=utf-8",
  ".json": "application/json; charset=utf-8",
  ".svg": "image/svg+xml"
};

const SECURITY_HEADERS = {
  "X-Content-Type-Options": "nosniff",
  "Content-Security-Policy":
    "default-src 'self'; script-src 'self'; style-src 'self' 'unsafe-inline'; img-src 'self' data:; connect-src 'self'; frame-ancestors 'none'; base-uri 'none'; form-action 'none'"
};

const MAX_BODY_BYTES = 65536;
const MAX_PENDING_JOBS = 16;

/* Persistent Stateful Native C11 Streaming Engine Worker (Race-Free & Backpressure-Bounded FIFO!) */
let worker = null;
let rl = null;
let pending = [];

function rejectPendingJobs(reason) {
  while (pending.length > 0) {
    const job = pending.shift();
    clearTimeout(job.timer);
    job.reject(new Error(reason));
  }
}

function startWorker() {
  const prevW = worker;
  const prevR = rl;
  worker = null;
  rl = null;
  if (prevR) {
    try { prevR.close(); } catch {}
  }
  if (prevW) {
    try { prevW.kill(); } catch {}
  }
  rejectPendingJobs("Engine worker restarted");

  const w = spawn(BIN, ["stream"], { stdio: ["pipe", "pipe", "pipe"] });
  const r = readline.createInterface({ input: w.stdout });
  worker = w;
  rl = r;

  r.on("line", (line) => {
    if (worker !== w) return;
    const job = pending.shift();
    if (job) {
      clearTimeout(job.timer);
      job.resolve(line);
    }
  });
  w.stderr.on("data", (d) => {
    if (worker !== w) return;
    console.error("[engine stderr]", String(d));
  });
  w.on("error", (err) => {
    if (worker !== w) return;
    try { r.close(); } catch {}
    worker = null;
    rl = null;
    rejectPendingJobs(`Engine spawn error: ${err.message}`);
  });
  w.on("exit", (code) => {
    if (worker !== w) return;
    try { r.close(); } catch {}
    worker = null;
    rl = null;
    rejectPendingJobs(`Engine exited with code ${code}`);
  });
}

function sendEngineCmd(cmd) {
  if (pending.length >= MAX_PENDING_JOBS) {
    const err = new Error("Engine command queue saturated (max 16 in-flight)");
    err.status = 429;
    return Promise.reject(err);
  }
  if (!worker || !worker.stdin || !worker.stdin.writable) startWorker();
  const w = worker;
  return new Promise((resolve, reject) => {
    const job = { resolve, reject, timer: null };
    job.timer = setTimeout(() => {
      const idx = pending.indexOf(job);
      if (idx !== -1) pending.splice(idx, 1);
      const err = new Error("Engine command timeout (15s)");
      err.status = 504;
      reject(err);
      if (worker === w) {
        try { w.kill(); } catch {}
      }
    }, 15000);
    pending.push(job);
    w.stdin.write(cmd + "\n");
  });
}

function clampInt(v, def, min, max) {
  const n = Number.parseInt(v, 10);
  if (!Number.isFinite(n)) return def;
  return Math.max(min, Math.min(max, n));
}

function isLoopbackRequest(req) {
  const host = String(req.headers.host || "");
  if (!/^(127\.0\.0\.1|localhost|\[::1\])(:\d+)?$/i.test(host)) return false;

  const origin = req.headers.origin;
  if (origin && !/^https?:\/\/(127\.0\.0\.1|localhost|\[::1\])(:\d+)?$/i.test(String(origin))) {
    return false;
  }

  const fetchSite = req.headers["sec-fetch-site"];
  if (fetchSite !== undefined) {
    const s = String(fetchSite).toLowerCase();
    if (s !== "same-origin" && s !== "none") return false;
  }

  if (req.method === "POST") {
    const ct = String(req.headers["content-type"] || "");
    if (!/^application\/json(\s*;.*)?$/i.test(ct)) return false;
  }
  return true;
}

function readJsonBody(req) {
  return new Promise((resolve, reject) => {
    const chunks = [];
    let totalBytes = 0;
    let settled = false;

    const fail = (err) => {
      if (settled) return;
      settled = true;
      reject(err);
    };

    req.on("data", (chunk) => {
      if (settled) return;
      totalBytes += chunk.length;
      if (totalBytes > MAX_BODY_BYTES) {
        const err = new Error("Payload too large");
        err.status = 413;
        req.pause();
        fail(err);
        return;
      }
      chunks.push(chunk);
    });
    req.on("end", () => {
      if (settled) return;
      settled = true;
      try {
        const raw = Buffer.concat(chunks).toString("utf8");
        resolve(JSON.parse(raw || "{}"));
      } catch {
        const err = new Error("Malformed JSON body");
        err.status = 400;
        reject(err);
      }
    });
    req.on("error", (err) => fail(err));
  });
}

function sendJson(res, status, body, extraHeaders = {}) {
  res.writeHead(status, {
    ...SECURITY_HEADERS,
    "Content-Type": "application/json; charset=utf-8",
    "Cache-Control": "no-store",
    ...extraHeaders
  });
  res.end(typeof body === "string" ? body : JSON.stringify(body));
}

startWorker();
sendEngineCmd("init 5 0 5000 0 36").catch((e) => console.error("Warmup error:", e));

const server = http.createServer(async (req, res) => {
  let u;
  try {
    u = new URL(req.url || "/", `http://127.0.0.1:${PORT}`);
  } catch {
    sendJson(res, 400, { error: "Malformed request URL" });
    return;
  }

  if (u.pathname.startsWith("/api/")) {
    if (!isLoopbackRequest(req)) {
      sendJson(res, 403, { error: "Forbidden host, origin, fetch-site, or content-type" });
      return;
    }

    /* Strictly block legacy CSRF-prone mutating GET endpoints */
    if (u.pathname === "/api/init" || u.pathname === "/api/step" || u.pathname === "/api/action") {
      sendJson(res, 405, { error: "Method not allowed; use POST /api/cmd with JSON body" }, { Allow: "POST" });
      return;
    }

    /* Strictly read-only P9 observer snapshot */
    if (u.pathname === "/api/state") {
      if (req.method !== "GET") {
        sendJson(res, 405, { error: "Method not allowed; use GET /api/state" }, { Allow: "GET" });
        return;
      }
      try {
        const jsonStr = await sendEngineCmd("state");
        sendJson(res, 200, jsonStr);
      } catch (e) {
        sendJson(res, e.status || 500, { error: String(e.message || e) });
      }
      return;
    }

    /* Unified POST /api/cmd endpoint (ALL engine mutations go exclusively through here!) */
    if (u.pathname === "/api/cmd") {
      if (req.method !== "POST") {
        sendJson(res, 405, { error: "Method not allowed; use POST /api/cmd" }, { Allow: "POST" });
        return;
      }
      try {
        const msg = await readJsonBody(req);
        if (!msg || typeof msg !== "object") {
          sendJson(res, 400, { error: "Invalid JSON object payload" });
          return;
        }
        let cmd = null;
        if (msg.op === "state") {
          cmd = "state";
        } else if (msg.op === "init") {
          const reg = clampInt(msg.regime, 5, 0, 5);
          const solv = clampInt(msg.solvent, 0, 0, 3);
          const seed = clampInt(msg.seed, 5000, 0, 99999999);
          const rest = clampInt(msg.restraint, 0, 0, 100);
          const warm = clampInt(msg.warmup, reg === 5 ? 36 : (reg === 0 ? 72 : 90), 0, 300);
          cmd = `init ${reg} ${solv} ${seed} ${rest} ${warm}`;
        } else if (msg.op === "step") {
          cmd = `step ${clampInt(msg.n, 1, 1, 30)}`;
        } else if (msg.op === "solvent") {
          cmd = `solvent ${clampInt(msg.val, 0, 0, 3)}`;
        } else if (msg.op === "restraint") {
          cmd = `restraint ${clampInt(msg.val, 0, 0, 100)}`;
        } else if (msg.op === "restraint_vec" && Array.isArray(msg.vec) && msg.vec.length === 13) {
          const vals = msg.vec.map((x) => clampInt(x, 0, 0, 100));
          cmd = `restraint_vec ${vals.join(" ")}`;
        } else if (msg.op === "tun" || msg.op === "shock" || msg.op === "rehydrate") {
          cmd = msg.op;
        } else if (msg.op === "plume") {
          cmd = `plume ${clampInt(msg.x, 16, 0, 31)} ${clampInt(msg.y, 16, 0, 31)}`;
        } else if (msg.op === "autophage") {
          cmd = `autophage ${clampInt(msg.seed, 0, 0, 99999999)}`;
        } else if (msg.op === "autophage_dna" && typeof msg.hex === "string" && /^[0-9a-fA-F]{64,256}$/.test(msg.hex)) {
          cmd = `autophage_dna ${msg.hex}`;
        }

        if (!cmd) {
          sendJson(res, 400, { error: "Invalid POST /api/cmd payload" });
          return;
        }
        const jsonStr = await sendEngineCmd(cmd);
        sendJson(res, 200, jsonStr);
      } catch (e) {
        const status = e.status || 500;
        if (status === 413) {
          sendJson(res, 413, { error: "Payload too large (max 64 KiB)" }, { Connection: "close" });
          res.on("finish", () => { try { req.socket.destroy(); } catch {} });
          return;
        }
        sendJson(res, status, { error: String(e.message || e) });
      }
      return;
    }

    sendJson(res, 404, { error: "Unknown API endpoint" });
    return;
  }

  /* Strictly serve ONLY allowlisted static UI files via GET/HEAD */
  if (req.method !== "GET" && req.method !== "HEAD") {
    res.writeHead(405, { ...SECURITY_HEADERS, Allow: "GET, HEAD" });
    res.end("Method not allowed");
    return;
  }

  const rel = u.pathname === "/" ? "index.html" : u.pathname.replace(/^\/+/, "");
  if (!ALLOWED_STATIC.has(rel)) {
    res.writeHead(404, SECURITY_HEADERS);
    res.end("Not found");
    return;
  }
  const fp = path.resolve(__dirname, rel);
  if (!fp.startsWith(__dirname + path.sep)) {
    res.writeHead(403, SECURITY_HEADERS);
    res.end("Forbidden");
    return;
  }
  fs.readFile(fp, (err, data) => {
    if (err) {
      res.writeHead(404, SECURITY_HEADERS);
      res.end("Not found");
      return;
    }
    const ext = path.extname(fp);
    res.writeHead(200, {
      ...SECURITY_HEADERS,
      "Content-Type": MIME[ext] || "application/octet-stream"
    });
    res.end(req.method === "HEAD" ? undefined : data);
  });
});

server.listen(PORT, "127.0.0.1", () => {
  console.log(`CELLULOGONIC MECHA v1.3.0 3D Embryogenesis & Cellular Observatory running at http://127.0.0.1:${PORT}`);
  console.log(`Native 3D off-lattice engine binary: ${BIN}`);
});

function shutdown() {
  const w = worker;
  const r = rl;
  worker = null;
  rl = null;
  try { if (r) r.close(); } catch {}
  try { if (w && w.stdin && w.stdin.writable) w.stdin.write("quit\n"); } catch {}
  try { if (w) w.kill(); } catch {}
  server.close(() => process.exit(0));
}
process.on("SIGINT", shutdown);
process.on("SIGTERM", shutdown);
