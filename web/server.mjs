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

const MIME = {
  ".html": "text/html; charset=utf-8",
  ".js": "application/javascript; charset=utf-8",
  ".css": "text/css; charset=utf-8",
  ".json": "application/json; charset=utf-8",
  ".svg": "image/svg+xml"
};

/* Persistent Stateful Native C11 Streaming Engine Worker (Sub-3ms Live 3D Steps!) */
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
    if (job) job.resolve(line);
  });
  worker.stderr.on("data", (d) => console.error("[engine stderr]", String(d)));
  worker.on("exit", (code) => {
    while (pending.length > 0) {
      pending.shift().reject(new Error(`Engine exited with code ${code}`));
    }
    worker = null;
  });
}

function sendEngineCmd(cmd) {
  if (!worker) startWorker();
  return new Promise((resolve, reject) => {
    pending.push({ resolve, reject });
    worker.stdin.write(cmd + "\n");
  });
}

function clampInt(v, def, min, max) {
  const n = Number.parseInt(v, 10);
  if (!Number.isFinite(n)) return def;
  return Math.max(min, Math.min(max, n));
}

startWorker();
// Warm up initial rich 3D Embryogenesis specimen immediately on server launch
sendEngineCmd("init 5 0 5000 0 36").catch((e) => console.error("Warmup error:", e));

const server = http.createServer(async (req, res) => {
  const u = new URL(req.url, `http://127.0.0.1:${PORT}`);

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
    if (op === "tun")       cmd = "tun";
    if (op === "shock")     cmd = "shock";
    if (op === "rehydrate") cmd = "rehydrate";
    if (op === "plume")     cmd = `plume ${clampInt(u.searchParams.get("x"), 16, 0, 31)} ${clampInt(u.searchParams.get("y"), 16, 0, 31)}`;
    if (op === "autophage") cmd = `autophage ${clampInt(u.searchParams.get("seed"), 0, 0, 99999999)}`;
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

  let rel = u.pathname === "/" ? "index.html" : u.pathname.replace(/^\/+/, "");
  if (rel.includes("..")) { res.writeHead(403); res.end("Forbidden"); return; }
  const fp = path.join(__dirname, rel);
  fs.readFile(fp, (err, data) => {
    if (err) { res.writeHead(404); res.end("Not found"); return; }
    const ext = path.extname(fp);
    res.writeHead(200, { "Content-Type": MIME[ext] || "application/octet-stream" });
    res.end(data);
  });
});

server.listen(PORT, "127.0.0.1", () => {
  console.log(`CELLULOGONIC MECHA v1.2 3D Embryogenesis & Cellular Observatory running at http://127.0.0.1:${PORT}`);
  console.log(`Native 3D off-lattice engine binary: ${BIN}`);
});
