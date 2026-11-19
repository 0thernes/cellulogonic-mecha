import http from "node:http";
import fs from "node:fs";
import path from "node:path";
import { execFile } from "node:child_process";
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

function sanitizeInt(v, def, min, max) {
  const n = Number.parseInt(v, 10);
  if (!Number.isFinite(n)) return def;
  return Math.max(min, Math.min(max, n));
}

function sanitizeRestraints(raw) {
  if (!raw) return "";
  const parts = String(raw).split(",").slice(0, 13).map((x) => sanitizeInt(x, 0, 0, 100));
  return parts.join(",");
}

const server = http.createServer((req, res) => {
  const u = new URL(req.url, `http://127.0.0.1:${PORT}`);

  if (u.pathname === "/api/snapshot") {
    const epochs = sanitizeInt(u.searchParams.get("epochs"), 80, 0, 400);
    const size = sanitizeInt(u.searchParams.get("size"), 32, 16, 48);
    const seed = sanitizeInt(u.searchParams.get("seed"), 5000, 1, 9999999);
    const regime = sanitizeInt(u.searchParams.get("regime"), 0, 0, 4);
    const solvent = sanitizeInt(u.searchParams.get("solvent"), 0, 0, 3);
    const tun = sanitizeInt(u.searchParams.get("tun"), 0, 0, 2);
    const restraints = sanitizeRestraints(u.searchParams.get("restraints"));

    const args = [
      "json",
      "--epochs", String(epochs),
      "--size", String(size),
      "--seed", String(seed),
      "--regime", String(regime),
      "--solvent", String(solvent),
      "--tun", String(tun)
    ];
    if (restraints) args.push("--restraints", restraints);

    execFile(BIN, args, { maxBuffer: 8 * 1024 * 1024, timeout: 15000 }, (err, stdout, stderr) => {
      if (err) {
        res.writeHead(500, { "Content-Type": "application/json" });
        res.end(JSON.stringify({ error: String(err.message || err), stderr }));
        return;
      }
      res.writeHead(200, { "Content-Type": "application/json", "Cache-Control": "no-store" });
      res.end(stdout);
    });
    return;
  }

  let rel = u.pathname === "/" ? "index.html" : u.pathname.replace(/^\/+/, "");
  if (rel.includes("..")) {
    res.writeHead(403);
    res.end("Forbidden");
    return;
  }
  const fp = path.join(__dirname, rel);
  fs.readFile(fp, (err, data) => {
    if (err) {
      res.writeHead(404);
      res.end("Not found");
      return;
    }
    const ext = path.extname(fp);
    res.writeHead(200, { "Content-Type": MIME[ext] || "application/octet-stream" });
    res.end(data);
  });
});

server.listen(PORT, "127.0.0.1", () => {
  console.log(`CELLULOGONIC MECHA v1.0 Cellular Observatory running at http://127.0.0.1:${PORT}`);
  console.log(`Native engine binary: ${BIN}`);
});
