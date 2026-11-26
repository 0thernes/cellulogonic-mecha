/* SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2026 Alexander Stephen Donahue
 *
 * Automated Integration & Security Test Suite for web/server.mjs (CELLULOGONIC MECHA v1.6.0)
 * Verifies /healthz, static CSP/security headers, P9 GET /api/state, POST /api/cmd,
 * CSRF origin/host/fetch-site/content-type blocking, 405 legacy GET block,
 * odd-hex & array rejection, 413 payload limit, and path-traversal denial.
 */
import assert from "node:assert/strict";
import http from "node:http";
import path from "node:path";
import { spawn } from "node:child_process";
import { fileURLToPath } from "node:url";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const ROOT = path.resolve(__dirname, "..");

function httpReq(port, method, reqPath, headers = {}, body = null) {
  return new Promise((resolve, reject) => {
    const req = http.request(
      {
        host: "127.0.0.1",
        port,
        method,
        path: reqPath,
        headers: { Host: `127.0.0.1:${port}`, ...headers }
      },
      (res) => {
        const chunks = [];
        res.on("data", (c) => chunks.push(c));
        res.on("end", () => {
          resolve({
            status: res.statusCode,
            headers: res.headers,
            body: Buffer.concat(chunks).toString("utf8")
          });
        });
      }
    );
    req.on("error", reject);
    if (body !== null) req.write(body);
    req.end();
  });
}

async function main() {
  const srv = spawn(process.execPath, [path.join(ROOT, "web", "server.mjs")], {
    env: { ...process.env, PORT: "0" },
    stdio: ["ignore", "pipe", "pipe"]
  });

  let stderrBuf = "";
  srv.stderr.on("data", (d) => { stderrBuf += String(d); });

  const port = await new Promise((resolve, reject) => {
    let buf = "";
    const timer = setTimeout(() => reject(new Error(`Server failed to bind within 10s. stderr=${stderrBuf}`)), 10000);
    srv.stdout.on("data", (chunk) => {
      buf += String(chunk);
      const m = buf.match(/http:\/\/127\.0\.0\.1:(\d+)/);
      if (m) {
        clearTimeout(timer);
        resolve(Number(m[1]));
      }
    });
    srv.on("exit", (code) => {
      clearTimeout(timer);
      reject(new Error(`Server exited prematurely with code ${code}. stderr=${stderrBuf}`));
    });
  });

  try {
    /* 1. Health check endpoint */
    const hz = await httpReq(port, "GET", "/healthz");
    assert.equal(hz.status, 200);
    const hzJson = JSON.parse(hz.body);
    assert.equal(hzJson.status, "ok");
    assert.equal(hzJson.version, "1.6.0");
    assert.equal(hzJson.worker_alive, true);

    /* 2. Static index.html + strict security headers (zero unsafe-inline!) */
    const idx = await httpReq(port, "GET", "/");
    assert.equal(idx.status, 200);
    assert.match(idx.body, /CELLULOGONIC MECHA/);
    assert.equal(idx.headers["x-content-type-options"], "nosniff");
    assert.equal(idx.headers["x-frame-options"], "DENY");
    assert.equal(idx.headers["referrer-policy"], "no-referrer");
    assert.equal(idx.headers["cross-origin-resource-policy"], "same-origin");
    assert.ok(!idx.headers["content-security-policy"].includes("unsafe-inline"), "CSP must not contain unsafe-inline");

    /* 3. Read-only P9 GET /api/state */
    const st1 = await httpReq(port, "GET", "/api/state");
    assert.equal(st1.status, 200);
    const s1 = JSON.parse(st1.body);
    assert.equal(s1.telemetry.mass_res, 0);
    assert.equal(s1.telemetry.energy_res, 0);
    assert.equal(s1.telemetry.second_law, 0);
    const ep0 = s1.telemetry.epoch;

    /* 4. Valid POST /api/cmd step + plume grid dimensions */
    const st2 = await httpReq(
      port,
      "POST",
      "/api/cmd",
      {
        "Content-Type": "application/json",
        Origin: `http://127.0.0.1:${port}`,
        "Sec-Fetch-Site": "same-origin"
      },
      JSON.stringify({ op: "step", n: 2 })
    );
    assert.equal(st2.status, 200);
    const s2 = JSON.parse(st2.body);
    assert.equal(s2.telemetry.epoch, ep0 + 2);
    assert.equal(s2.telemetry.mass_res, 0);
    assert.equal(s2.telemetry.energy_res, 0);
    assert.equal(s2.grid.plume_w, 16);
    assert.equal(s2.grid.plume_h, 16);
    assert.equal(s2.grid.plumes.length, s2.grid.plume_w * s2.grid.plume_h);

    /* 5. Legacy CSRF-prone GET /api/step -> 405 */
    const leg = await httpReq(port, "GET", "/api/step");
    assert.equal(leg.status, 405);

    /* 6. Cross-origin CSRF attempt -> 403 */
    const csrfOrigin = await httpReq(
      port,
      "POST",
      "/api/cmd",
      { "Content-Type": "application/json", Origin: "https://evil.example" },
      JSON.stringify({ op: "step", n: 1 })
    );
    assert.equal(csrfOrigin.status, 403);

    /* 7. DNS-rebinding Host spoofing -> 403 (both API and static!) */
    const badHostApi = await httpReq(port, "GET", "/api/state", { Host: `evil.example:${port}` });
    assert.equal(badHostApi.status, 403);
    const badHostStatic = await httpReq(port, "GET", "/", { Host: `127.0.0.1:${port + 1}` });
    assert.equal(badHostStatic.status, 403);

    /* 8. Wrong Content-Type on POST -> 403 */
    const badCt = await httpReq(port, "POST", "/api/cmd", { "Content-Type": "text/plain" }, "{}");
    assert.equal(badCt.status, 403);

    /* 9. JSON array payload -> 400 */
    const arrBody = await httpReq(port, "POST", "/api/cmd", { "Content-Type": "application/json" }, "[1,2,3]");
    assert.equal(arrBody.status, 400);

    /* 10. Odd-length AUTOPHAGE DNA hex -> 400; even-length 64 hex -> 200 */
    const oddHex = await httpReq(
      port, "POST", "/api/cmd", { "Content-Type": "application/json" },
      JSON.stringify({ op: "autophage_dna", hex: "a".repeat(65) })
    );
    assert.equal(oddHex.status, 400);
    const goodHex = await httpReq(
      port, "POST", "/api/cmd", { "Content-Type": "application/json" },
      JSON.stringify({ op: "autophage_dna", hex: "ab".repeat(32) })
    );
    assert.equal(goodHex.status, 200);

    /* 11. Oversized payload (> 64 KiB) -> 413 */
    const bigBody = JSON.stringify({ op: "state", pad: "x".repeat(70000) });
    const tooBig = await httpReq(port, "POST", "/api/cmd", { "Content-Type": "application/json" }, bigBody);
    assert.equal(tooBig.status, 413);

    /* 12. Static allowlist & path traversal -> 404 */
    const trav = await httpReq(port, "GET", "/../README.md");
    assert.equal(trav.status, 404);

    /* 13. Clean 204 for /favicon.ico with strict security headers (F-38) */
    const fav = await httpReq(port, "GET", "/favicon.ico");
    assert.equal(fav.status, 204);
    assert.equal(fav.headers["x-content-type-options"], "nosniff");

    console.log(`PASS: All 13 Web Observatory HTTP, CSP, CSRF, and Engine-Bridge tests passed (port ${port})!`);
  } finally {
    srv.kill("SIGTERM");
  }
}

main().catch((err) => {
  console.error("FAIL:", err);
  process.exit(1);
});
