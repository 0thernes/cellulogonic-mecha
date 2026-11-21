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

/* Persistent Stateful Native C11 Streaming Engine Worker (Deadlock-Proof FIFO + Timeouts!) */
let worker = null;
let rl = null;
let pending = [];

function startWorker() {
  if (worker) {
    try { worker.kill(); } catch {}
  }
  worker = spawn(BIN, ["stream"], { stdio: ["pipe", "pipe", "pipe"] });
  rl = readline.createInterface({ input: worker.stdout });
  rl.on("line", (line) => {
    const job = pending.shift();
    if (job) {
      clearTimeout(job.timer);
      job.resolve(line);
    }
  });
  worker.stderr.on("data", (d) => console.error("[engine stderr]", String(d)));
  worker.on("exit", (code) => {
    while (pending.length > 0) {
      const job = pending.shift();
      clearTimeout(job.timer);
      job.reject(new Error(`Engine exited with code ${code}`));
    }
    worker = null;
  });
}

function sendEngineCmd(cmd) {
  if (!worker || !worker.stdin.writable) startWorker();
  return new Promise((resolve, reject) => {
    const job = { resolve, reject, timer: null };
    job.timer = setTimeout(() => {
      const idx = pending.indexOf(job);
      if (idx !== -1) pending.splice(idx, 1);
      reject(new Error("Engine command timeout (15s)"));
      try { if (worker) worker.kill(); } catch {}
    }, 15000);
    pending.push(job);
    worker.stdin.write(cmd + "\n");
  });
}

function clampInt(v, def, min, max) {
  const n = Number.parseInt(v, 10);
  if (!Number.isFinite(n)) return def;
  return Math.max(min, Math.min(max, n));
}

function isLoopbackRequest(req) {
  const host = String(req.headers.host || "");
  if (!/^(127\.0\.0\.1|localhost)(:\d+)?$/i.test(host)) return false;
  const origin = req.headers.origin;
  if (origin && !/^https?:\/\/(127\.0\.0\.1|localhost)(:\d+)?$/i.test(String(origin))) return false;
  return true;
}

function readJsonBody(req) {
  return new Promise((resolve, reject) => {
    let body = "";
    req.on("data", (c) => {
      body += c;
      if (body.length > 65536) reject(new Error("Payload too large"));
    });
    req.on("end", () => {
      try { resolve(JSON.parse(body || "{}")); } catch (e) { reject(e); }
    });
    req.on("error", reject);
  });
}

startWorker();
sendEngineCmd("init 5 0 5000 0 36").catch((e) => console.error("Warmup error:", e));

const server = http.createServer(async (req, res) => {
  const u = new URL(req.url, `http://127.0.0.1:${PORT}`);

  if (u.pathname.startsWith("/api/") && !isLoopbackRequest(req)) {
    res.writeHead(403, { "Content-Type": "application/json" });
    res.end(JSON.stringify({ error: "Forbidden host/origin" }));
    return;
  }

  /* Unified POST /api/cmd endpoint (supports full 13-vec restraints & real 128B AUTOPHAGE DNA!) */
  if (req.method === "POST" && u.pathname === "/api/cmd") {
    try {
      const msg = await readJsonBody(req);
      let cmd = null;
      if (msg.op === "restraint_vec" && Array.isArray(msg.vec) && msg.vec.length === 13) {
        const vals = msg.vec.map((x) => clampInt(x, 0, 0, 100));
        cmd = `restraint_vec ${vals.join(" ")}`;
      } else if (msg.op === "autophage_dna" && typeof msg.hex === "string" && /^[0-9a-fA-F]{64,256}$/.test(msg.hex)) {
        cmd = `autophage_dna ${msg.hex}`;
      } else if (msg.op === "step") {
        cmd = `step ${clampInt(msg.n, 1, 1, 30)}`;
      } else if (msg.op === "solvent") {
        cmd = `solvent ${clampInt(msg.val, 0, 0, 3)}`;
      } else if (msg.op === "restraint") {
        cmd = `restraint ${clampInt(msg.val, 0, 0, 100)}`;
      } else if (msg.op === "tun" || msg.op === "shock" || msg.op === "rehydrate") {
        cmd = msg.op;
      } else if (msg.op === "plume") {
        cmd = `plume ${clampInt(msg.x, 16, 0, 31)} ${clampInt(msg.y, 16, 0, 31)}`;
      } else if (msg.op === "init") {
        const reg = clampInt(msg.regime, 5, 0, 5);
        const solv = clampInt(msg.solvent, 0, 0, 3);
        const seed = clampInt(msg.seed, 5000, 0, 99999999);
        const rest = clampInt(msg.restraint, 0, 0, 100);
        const warm = clampInt(msg.warmup, reg === 5 ? 36 : 72, 0, 300);
        cmd = `init ${reg} ${solv} ${seed} ${rest} ${warm}`;
      }
      if (!cmd) {
        res.writeHead(400, { "Content-Type": "application/json" });
        res.end(JSON.stringify({ error: "Invalid POST /api/cmd payload" }));
        return;
      }
      const jsonStr = await sendEngineCmd(cmd);
      res.writeHead(200, { "Content-Type": "application/json", "Cache-Control": "no-store" });
      res.end(jsonStr);
    } catch (e) {
      res.writeHead(500, { "Content-Type": "application/json" });
      res.end(JSON.stringify({ error: String(e.message || e) }));
    }
    return;
  }

  if (u.pathname === "/api/init") {
    const regime = clampInt(u.searchParams.get("regime"), 5, 0, 5);
    const solvent = clampInt(u.searchParams.get("solvent"), 0, 0, 3);
    const seed = clampInt(u.searchParams.get("seed"), 5000, 0, 99999999);
    const rest = clampInt(u.searchParams.get("restraint"), 0, 0, 100);
    const warm = clampInt(u.searchParams.get("warmup"), regime === 5 ? 36 : (regime === 0 ? 72 : 90), 0, 300);
    try {
      const jsonStr = await sendEngineCmd(`init ${regime} ${solvent} ${seed} ${rest} ${warm}`);
      res.writeHead(200, { "Content-Type": "application/json", "Cache-Control": "no-store" });
      res.end(jsonStr);
    } catch (e) {
      res.writeHead(500, { "Content-Type": "application/json" });
      res.end(JSON.stringify({ error: String(e.message || e) }));
    }
    return;
  }

  if (u.pathname === "/api/step") {
    const n = clampInt(u.searchParams.get("n"), 1, 1, 30);
    try {
      const jsonStr = await sendEngineCmd(`step ${n}`);
      res.writeHead(200, { "Content-Type": "application/json", "Cache-Control": "no-store" });
      res.end(jsonStr);
    } catch (e) {
      res.writeHead(500, { "Content-Type": "application/json" });
      res.end(JSON.stringify({ error: String(e.message || e) }));
    }
    return;
  }

  if (u.pathname === "/api/action") {
    const op = u.searchParams.get("op") || "";
    let cmd = null;
    if (op === "solvent")   cmd = `solvent ${clampInt(u.searchParams.get("val"), 0, 0, 3)}`;
    if (op === "restraint") cmd = `restraint ${clampInt(u.searchParams.get("val"), 0, 0, 100)}`;
    if (op === "restraint_vec") {
      const parts = String(u.searchParams.get("vec") || "").split(",").map((x) => clampInt(x, 0, 0, 100));
      if (parts.length === 13) cmd = `restraint_vec ${parts.join(" ")}`;
    }
    if (op === "tun")       cmd = "tun";
    if (op === "shock")     cmd = "shock";
    if (op === "rehydrate") cmd = "rehydrate";
    if (op === "plume")     cmd = `plume ${clampInt(u.searchParams.get("x"), 16, 0, 31)} ${clampInt(u.searchParams.get("y"), 16, 0, 31)}`;
    if (op === "autophage") cmd = `autophage ${clampInt(u.searchParams.get("seed"), 0, 0, 99999999)}`;
    if (op === "autophage_dna") {
      const hex = String(u.searchParams.get("hex") || "");
      if (/^[0-9a-fA-F]{64,256}$/.test(hex)) cmd = `autophage_dna ${hex}`;
    }
    if (!cmd) {
      res.writeHead(400, { "Content-Type": "application/json" });
      res.end(JSON.stringify({ error: "Invalid action" }));
      return;
    }
    try {
      const jsonStr = await sendEngineCmd(cmd);
      res.writeHead(200, { "Content-Type": "application/json", "Cache-Control": "no-store" });
      res.end(jsonStr);
    } catch (e) {
      res.writeHead(500, { "Content-Type": "application/json" });
      res.end(JSON.stringify({ error: String(e.message || e) }));
    }
    return;
  }

  /* Strictly serve ONLY allowlisted static UI files (blocks GET /server.mjs & path traversal!) */
  const rel = u.pathname === "/" ? "index.html" : u.pathname.replace(/^\/+/, "");
  if (!ALLOWED_STATIC.has(rel)) { res.writeHead(404); res.end("Not found"); return; }
  const fp = path.resolve(__dirname, rel);
  if (!fp.startsWith(__dirname + path.sep)) { res.writeHead(403); res.end("Forbidden"); return; }
  fs.readFile(fp, (err, data) => {
    if (err) { res.writeHead(404); res.end("Not found"); return; }
    const ext = path.extname(fp);
    res.writeHead(200, { "Content-Type": MIME[ext] || "application/octet-stream" });
    res.end(data);
  });
});

server.listen(PORT, "127.0.0.1", () => {
  console.log(`CELLULOGONIC MECHA v1.2.1 3D Embryogenesis & Cellular Observatory running at http://127.0.0.1:${PORT}`);
  console.log(`Native 3D off-lattice engine binary: ${BIN}`);
});

function shutdown() {
  try { if (worker && worker.stdin.writable) worker.stdin.write("quit\n"); } catch {}
  try { if (worker) worker.kill(); } catch {}
  server.close(() => process.exit(0));
}
process.on("SIGINT", shutdown);
process.on("SIGTERM", shutdown);
