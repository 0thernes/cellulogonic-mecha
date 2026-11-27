/* SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2026 Alexander Stephen Donahue
 * CELLULOGONIC MECHA v1.7.0 — Observatory Controller (Serialized Queue, Zero Unsafe-Inline, WebGL2 Fallback)
 */
import { CellMicroscope3D } from "./renderer3d.js";

const $ = (id) => document.getElementById(id);

function announce(msg) {
  const sr = $("sr-announcements");
  if (!sr) return;
  sr.textContent = "";
  queueMicrotask(() => { sr.textContent = msg; });
}

const PILLAR_NAMES = [
  "P0 Autopoiesis & Survival",
  "P1 Chemiosmosis & Solvent",
  "P2 Bedau Diversity & Novelty",
  "P3 Endosymbiosis & Eukaryogenesis",
  "P4 Bioelectric Morphogenesis",
  "P5 Assembly A & Active Operon",
  "P6 Trophic & HGT Ecology",
  "P7 Thermodynamic Closure"
];

const RESTRAINT_NAMES = [
  "NERFING", "MUZZLING", "SANDBOXING", "SILOING", "DELIMITING",
  "DEGRESSING", "SLOWING", "SPEEDBUMPING", "BOXING", "LOBOTOMIZING",
  "MEDICATING", "DEGRADING", "MINIMIZING"
];

const prefersReducedMotion =
  typeof window.matchMedia === "function" &&
  window.matchMedia("(prefers-reduced-motion: reduce)").matches;

let scope = null;
try {
  scope = new CellMicroscope3D($("gl-canvas"), $("hud-canvas"), $("stage-wrap"));
  if (prefersReducedMotion) {
    scope.autoOrbit = false;
    scope.reducedMotion = true;
    $("chk-orbit").checked = false;
  }
} catch (err) {
  const fb = $("webgl-fallback");
  if (fb) fb.hidden = false;
  console.warn("WebGL2 fallback active:", err?.message || err);
}

const scopeCanvas = $("scope-canvas");
const asmoCanvas = $("asmo-canvas");
const scopeCtx = scopeCanvas.getContext("2d");
const asmoCtx = asmoCanvas.getContext("2d");

/* Cached canvas CSS sizes updated via ResizeObserver (zero 60 FPS layout thrashing) */
const canvasSizes = new WeakMap();
let panelRo = null;
if (typeof ResizeObserver === "function") {
  panelRo = new ResizeObserver((entries) => {
    for (const entry of entries) {
      const cr = entry.contentRect;
      canvasSizes.set(entry.target, { width: cr.width, height: cr.height });
    }
  });
  panelRo.observe(scopeCanvas);
  panelRo.observe(asmoCanvas);
}

let playing = true;
let latestData = null;
let lastDetSeed = "5000";
let lastScopeEpoch = -1;
const scopeHist = [];
const restVec = new Array(13).fill(0);

/* Single strictly serialized command queue preventing step/init/action races */
let cmdTail = Promise.resolve();
let stepPending = false;
let epochGeneration = 0;

/* Pre-build static DOM scaffolds ONCE (zero hot-loop innerHTML, zero inline styles!) */
const pillarDom = [];
function initPillarDom() {
  const root = $("pillars");
  root.textContent = "";
  for (let i = 0; i < 8; i++) {
    const item = document.createElement("div");
    item.className = "p-item";
    const top = document.createElement("div");
    top.className = "p-top";
    const lbl = document.createElement("span");
    lbl.textContent = PILLAR_NAMES[i];
    const val = document.createElement("strong");
    val.textContent = "0.0";
    top.append(lbl, val);
    const prog = document.createElement("progress");
    prog.className = "p-progress";
    prog.max = 100;
    prog.value = 0;
    prog.setAttribute("aria-label", PILLAR_NAMES[i]);
    item.append(top, prog);
    root.append(item);
    pillarDom.push({ val, prog });
  }
}

function initStatTable(tableId, labels) {
  const tbl = $(tableId);
  const caption = tbl.querySelector("caption");
  tbl.textContent = "";
  if (caption) tbl.append(caption);

  const thead = document.createElement("thead");
  thead.className = "sr-only";
  const hr = document.createElement("tr");
  const thM = document.createElement("th");
  thM.scope = "col";
  thM.textContent = "Metric";
  const thV = document.createElement("th");
  thV.scope = "col";
  thV.textContent = "Value";
  hr.append(thM, thV);
  thead.append(hr);
  tbl.append(thead);

  const tbody = document.createElement("tbody");
  const cells = [];
  for (const label of labels) {
    const tr = document.createElement("tr");
    const thK = document.createElement("th");
    thK.scope = "row";
    thK.textContent = label;
    const tdV = document.createElement("td");
    tdV.textContent = "—";
    tr.append(thK, tdV);
    tbody.append(tr);
    cells.push(tdV);
  }
  tbl.append(tbody);
  return cells;
}

const morphCells = initStatTable("tbl-morph", [
  "GRN Fates (Ec/N/M/G/Gm)",
  "Steinberg Cadherin Sorting",
  "3D Axons (Live · Wired/Pruned)",
  "CPG Rhythmic Motor Pulses",
  "Size Control (Hippo / Eutely)",
  "GPS Wnt / BMP / Shh · Wedge"
]);

const eventCells = initStatTable("tbl-events", [
  "Lineages · Solvent Adapted",
  "CMC Abiogenesis Vesicles",
  "3D Binary Fissions / Buddings",
  "Organelles (Mito / Chloro)",
  "Endosymbiotic Captures (Sec)",
  "EGT Transfers / Mitophagy",
  "HGT Uptakes / Phage Lyses",
  "Somatic Cancer / Apoptosis",
  "Mean c-Ring nH / Reduced Q",
  "Radiated 2nd-Law Heat Sink"
]);

const backlashCells = initStatTable("tbl-backlash", [
  "Kill-Switch Deletions (DEGRADING)",
  "Audit-Evasion Spores (SPEEDBUMP)",
  "HGT Complexity Smuggles (DEGRESS)",
  "Metabolic Bypasses (NERFING)"
]);

const rest13Inputs = [];
const rest13Labels = [];
function initRestraint13Dom() {
  const grid = $("rest13-grid");
  grid.textContent = "";
  for (let i = 0; i < 13; i++) {
    const row = document.createElement("div");
    row.className = "r13-row";
    const id = `r13-slider-${i}`;
    const name = document.createElement("label");
    name.setAttribute("for", id);
    name.textContent = RESTRAINT_NAMES[i];
    const inp = document.createElement("input");
    inp.id = id;
    inp.type = "range";
    inp.min = "0";
    inp.max = "100";
    inp.value = "0";
    inp.setAttribute("aria-label", `${RESTRAINT_NAMES[i]} restraint percentage`);
    const out = document.createElement("span");
    out.textContent = "0%";
    inp.addEventListener("input", () => {
      const v = Number(inp.value);
      restVec[i] = v;
      out.textContent = `${v}%`;
    });
    inp.addEventListener("change", () => {
      restVec[i] = Number(inp.value);
      out.textContent = `${restVec[i]}%`;
      doCmd({ op: "restraint_vec", vec: restVec.slice() });
    });
    row.append(name, inp, out);
    grid.append(row);
    rest13Inputs.push(inp);
    rest13Labels.push(out);
  }
}

initPillarDom();
initRestraint13Dom();

function syncRest13Sliders(val) {
  for (let i = 0; i < 13; i++) {
    restVec[i] = val;
    rest13Inputs[i].value = String(val);
    rest13Labels[i].textContent = `${val}%`;
  }
}

/* Strictly CSRF-safe JSON POST helper (ALL state mutations go exclusively to POST /api/cmd) */
async function apiCmd(payload) {
  const r = await fetch("/api/cmd", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload)
  });
  let j = null;
  try {
    j = await r.json();
  } catch {
    throw new Error(`HTTP ${r.status} ${r.statusText}`);
  }
  if (!r.ok || (j && j.error)) {
    throw new Error((j && j.error) || `HTTP ${r.status} ${r.statusText}`);
  }
  return j;
}

function enqueueCmd(payload, { isStep = false, onSuccess = null, errPrefix = "Command" } = {}) {
  if (isStep) {
    if (stepPending) return Promise.resolve();
    stepPending = true;
  }
  const gen = payload.op === "init" ? ++epochGeneration : epochGeneration;
  const run = cmdTail.then(async () => {
    try {
      const data = await apiCmd(payload);
      if (gen === epochGeneration) {
        applyData(data);
        if (onSuccess) onSuccess(data);
      }
    } catch (e) {
      const errMsg = `${errPrefix} error: ${e.message}`;
      $("vp-status").textContent = errMsg;
      announce(errMsg);
    } finally {
      if (isStep) stepPending = false;
    }
  });
  cmdTail = run;
  return run;
}

function ensureHiDpi2D(canvas, ctx, cssHeight) {
  const cached = canvasSizes.get(canvas);
  const wCss = cached && cached.width > 0 ? cached.width : (canvas.clientWidth || canvas.width);
  const hCss = cached && cached.height > 0 ? cached.height : (canvas.clientHeight || cssHeight);
  const dpr = Math.min(window.devicePixelRatio || 1, 2);
  const cw = Math.max(240, Math.round(wCss));
  const ch = Math.max(40, Math.round(hCss));
  const pw = Math.round(cw * dpr);
  const ph = Math.round(ch * dpr);
  if (canvas.width !== pw || canvas.height !== ph) {
    canvas.width = pw;
    canvas.height = ph;
  }
  ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
  return { W: cw, H: ch };
}

function applyData(data) {
  latestData = data;
  if (scope) scope.setGrid(data.grid);
  renderTelemetry(data.telemetry);
  renderBridge(data.bridge);
  const t = data.telemetry;
  const ledgerOk = (t.mass_res === 0 && t.energy_res === 0 && t.second_law === 0);
  $("vp-status").textContent =
    `t=${t.epoch} | ${t.alive} 3D cells | ${t.mean_mt_ghz.toFixed(2)} GHz | ${ledgerOk ? "0-drift" : "LEDGER VIOLATION"}`;
}

function initWorld(forceFeral = false) {
  if (forceFeral) {
    const curVal = Number($("inp-seed").value);
    if (curVal > 0) lastDetSeed = String(curVal);
    $("inp-seed").value = "0";
  } else {
    const curVal = Number($("inp-seed").value);
    if (curVal > 0) lastDetSeed = String(curVal);
  }
  const regime = Number($("sel-regime").value);
  const solvent = Number($("sel-solvent").value);
  const seed = Number($("inp-seed").value);
  const rest = Number($("rng-rest").value);
  const warm = regime === 5 ? 28 : (regime === 0 ? 48 : 80);
  scopeHist.length = 0;
  lastScopeEpoch = -1;
  syncRest13Sliders(rest);
  const initMsg = seed === 0 ? "Seeding from hardware RDSEED/entropy..." : "Initializing 3D specimen...";
  $("vp-status").textContent = initMsg;
  announce(initMsg);
  return enqueueCmd(
    { op: "init", regime, solvent, seed, restraint: rest, warmup: warm },
    {
      errPrefix: "Init",
      onSuccess: (data) => {
        if (scope && regime === 5) scope.dist = 18.5;
        announce(`Specimen ready at epoch ${data.telemetry.epoch} with ${data.telemetry.alive} living cells.`);
      }
    }
  );
}

function stepWorld() {
  if (!playing || document.hidden) return;
  const n = Number($("sel-speed").value);
  enqueueCmd({ op: "step", n }, { isStep: true, errPrefix: "Step" });
}

function doCmd(payload) {
  return enqueueCmd(payload, { errPrefix: "Command" });
}

function renderTelemetry(t) {
  const bm = $("badge-mass");
  bm.textContent = `MASS Δ: ${t.mass_res} amu`;
  bm.className = `badge ${t.mass_res === 0 ? "ok" : "bad"}`;

  const be = $("badge-energy");
  be.textContent = `1ST LAW Δ: ${t.energy_res} uq`;
  be.className = `badge ${t.energy_res === 0 ? "ok" : "bad"}`;

  const bd = $("badge-drift");
  const driftVal = Number(t.drift || 0);
  bd.textContent = `DRIFT: ${driftVal.toExponential(2)}`;
  bd.className = `badge ${driftVal < 1e-12 ? "ok" : "bad"}`;

  const b2 = $("badge-2nd");
  b2.textContent = `2ND LAW VIOL: ${t.second_law}`;
  b2.className = `badge ${t.second_law === 0 ? "ok" : "bad"}`;

  $("badge-sha").textContent = `SHA-256: ${t.sha256.slice(0, 14)}…`;

  const adaptPct = t.alive > 0 ? ((100 * (t.adapted || 0)) / t.alive).toFixed(0) : "0";
  $("k-covi").textContent = t.covi.toFixed(2);
  $("k-stage").textContent = `Stage ${t.stage}/6 · MET ${t.met}/7`;
  $("k-mt").textContent = `${t.mean_mt_ghz.toFixed(2)} GHz`;
  $("k-coh").textContent = `Fröhlich Coh ${(t.mean_mt_coh * 100).toFixed(0)}%`;
  $("k-alive").textContent = String(t.alive);
  $("k-split").textContent = `P:${t.prok} E:${t.euk} S2:${t.sec} Sp:${t.spores || 0} Tun:${t.tuns}`;
  $("k-col").textContent = `${t.fewcell} / ${t.multicell}`;
  $("k-maxt").textContent = `Few / Multi · Max ${t.max_tissue}`;
  $("k-spd").textContent = `${t.mean_speed.toFixed(2)} μm/s`;
  $("k-lin").textContent = `L:${t.lineages || 0} · Adapt ${adaptPct}%`;
  $("k-lane").textContent = t.mean_euk_vol > 0
    ? `${t.mean_euk_vol.toFixed(0)} / ${t.mean_prok_vol.toFixed(0)}`
    : `${t.mean_prok_vol.toFixed(0)} (Prok)`;
  $("k-vmem").textContent = `${t.mean_vmem.toFixed(1)} mV`;
  $("k-vstd").textContent = `σ = ${t.std_vmem.toFixed(1)} mV`;
  $("k-assm").textContent = `${t.assembly.toFixed(1)} / ${t.leff.toFixed(1)}`;
  $("k-anew").textContent = `A_new = ${t.anew.toFixed(3)}`;

  for (let i = 0; i < 8; i++) {
    const v = Math.max(0, Math.min(100, t.pillars[i] || 0));
    pillarDom[i].val.textContent = v.toFixed(1);
    pillarDom[i].prog.value = v;
  }

  const f = t.fates || [0, 0, 0, 0, 0, 0];
  const mm = t.morph_means || [0, 0, 0];
  morphCells[0].textContent = `${f[1]}/${f[2]}/${f[3]}/${f[4]}/${f[5]} (P:${f[0]})`;
  morphCells[1].textContent = `${(t.homo_cad_pct || 0).toFixed(1)}% homophilic`;
  morphCells[2].textContent = `${t.active_axons || 0} (${t.axons_wired || 0} / ${t.axons_pruned || 0})`;
  morphCells[3].textContent = `${(t.cpg_pulses || 0).toLocaleString()}`;
  morphCells[4].textContent = `${t.hippo_arrests || 0} / ${t.eutelic_stops || 0}`;
  morphCells[5].textContent = `${mm[0].toFixed(0)} / ${mm[1].toFixed(0)} / ${mm[2].toFixed(0)} · ${(t.mean_wedge || 0).toFixed(2)}`;

  eventCells[0].textContent = `${t.lineages || 0} · ${t.adapted || 0}/${t.alive} (${adaptPct}%)`;
  eventCells[1].textContent = String(t.abiogenesis);
  eventCells[2].textContent = `${t.sym_fiss} / ${t.asym_bud}`;
  eventCells[3].textContent = `${t.orgs} (${t.mito} / ${t.chlo})`;
  eventCells[4].textContent = `${t.endo_cap} (${t.sec_cap})`;
  eventCells[5].textContent = `${t.egt} / ${t.mitophagy}`;
  eventCells[6].textContent = `${t.hgt} / ${t.phages}`;
  eventCells[7].textContent = `${t.cancer} / ${t.apoptosis}`;
  eventCells[8].textContent = `${t.mean_cring.toFixed(2)} / ${t.mean_Q.toFixed(3)}`;
  eventCells[9].textContent = `${t.heat_sink.toLocaleString()} uq`;

  backlashCells[0].textContent = String(t.backlash[0]);
  backlashCells[1].textContent = String(t.backlash[1]);
  backlashCells[2].textContent = String(t.backlash[2]);
  backlashCells[3].textContent = String(t.backlash[3]);
}

function renderBridge(b) {
  if (!asmoCtx) return;
  const { W, H } = ensureHiDpi2D(asmoCanvas, asmoCtx, 58);
  asmoCtx.fillStyle = "#040810";
  asmoCtx.fillRect(0, 0, W, H);
  if (!b || !b.valid || !b.traits112) {
    $("bridge-meta").textContent = "Awaiting living cellular specimen...";
    return;
  }
  const sigTag = b.apex_sig && b.apex_sig !== "0000000000000000" ? `DNA:${b.apex_sig.slice(0, 8)}` : "Native Operon";
  $("bridge-meta").textContent =
    `Apex L#${b.apex_lineage} (G${b.apex_gen}, ${b.apex_orgs} org, ${b.apex_mt_ghz.toFixed(1)}GHz, ${sigTag}) · AUTOPHAGE carriers: ${b.autophage_carriers} · μ=${b.mean.toFixed(2)} σ=${b.std.toFixed(2)}`;

  const n = b.traits112.length;
  const bw = (W - 12) / n;
  for (let i = 0; i < n; i++) {
    const v = Math.max(0, Math.min(1, b.traits112[i]));
    const bh = Math.max(2, v * (H - 16));
    if (i < 24) {
      asmoCtx.fillStyle = `hsl(${34 + i * 7}, 95%, ${48 + v * 22}%)`;
    } else {
      asmoCtx.fillStyle = `hsl(${175 + ((i - 24) / 88) * 135}, 82%, ${38 + v * 30}%)`;
    }
    asmoCtx.fillRect(6 + i * bw, H - 12 - bh, Math.max(1.5, bw - 0.8), bh);
  }
  const divX = 6 + 24 * bw;
  asmoCtx.strokeStyle = "rgba(255, 255, 255, 0.35)";
  asmoCtx.beginPath();
  asmoCtx.moveTo(divX, 2);
  asmoCtx.lineTo(divX, H - 12);
  asmoCtx.stroke();

  asmoCtx.fillStyle = "#93accf";
  asmoCtx.font = "9px ui-monospace, Consolas, monospace";
  asmoCtx.fillText("0..23 Physio/MT/GRN", 6, H - 2);
  asmoCtx.fillText("24..111 AUTOPHAGE Operon -> Asmodensius 112-Trait Vector [0,1]", divX + 5, H - 2);
}

function drawGHzOscilloscope() {
  if (!scopeCtx) return;
  const { W, H } = ensureHiDpi2D(scopeCanvas, scopeCtx, 70);
  scopeCtx.fillStyle = "#040810";
  scopeCtx.fillRect(0, 0, W, H);
  if (!latestData || !latestData.grid || !latestData.grid.cells || !latestData.grid.cells.length) return;

  const bins = new Float32Array(28);
  let maxB = 1;
  for (const cell of latestData.grid.cells) {
    if (cell[1] !== 1) continue;
    const ghz = cell[13] / 1000;
    const coh = cell[14] / 1000;
    const bi = Math.max(0, Math.min(27, Math.floor((ghz - 1.0) * 2.0)));
    bins[bi] += (0.25 + coh);
    if (bins[bi] > maxB) maxB = bins[bi];
  }
  const bw = (W * 0.56) / 28;
  for (let i = 0; i < 28; i++) {
    const h = (bins[i] / maxB) * (H - 18);
    scopeCtx.fillStyle = `hsl(${175 + i * 4}, 92%, ${46 + (bins[i] / maxB) * 24}%)`;
    scopeCtx.fillRect(6 + i * bw, H - 12 - h, Math.max(1, bw - 2), h);
  }
  scopeCtx.fillStyle = "#93accf";
  scopeCtx.font = "9.5px ui-monospace, Consolas, monospace";
  scopeCtx.fillText("1 GHz", 6, H - 2);
  scopeCtx.fillText("8 GHz", W * 0.27, H - 2);
  scopeCtx.fillText("15 GHz (Fröhlich MT Modes)", W * 0.36, H - 2);

  const x0 = W * 0.60, ww = W * 0.38;
  const coh = latestData.telemetry.mean_mt_coh || 0;
  const f = latestData.telemetry.mean_mt_ghz || 0;
  const ep = latestData.telemetry.epoch || 0;
  if (scopeHist.length === 0 || scopeHist[scopeHist.length - 1].ep !== ep) {
    scopeHist.push({ ep, coh, f });
    if (scopeHist.length > 96) scopeHist.shift();
  }
  scopeCtx.strokeStyle = "#38e8c6";
  scopeCtx.lineWidth = 1.6;
  scopeCtx.beginPath();
  for (let i = 0; i < scopeHist.length; i++) {
    const px = x0 + (scopeHist.length > 1 ? (i / (scopeHist.length - 1)) * ww : ww * 0.5);
    const y = (H - 14) - Math.min(1, scopeHist[i].coh) * (H - 22);
    if (i === 0) scopeCtx.moveTo(px, y);
    else scopeCtx.lineTo(px, y);
  }
  scopeCtx.stroke();
  scopeCtx.strokeStyle = "#f59e0b";
  scopeCtx.lineWidth = 1.2;
  scopeCtx.beginPath();
  for (let i = 0; i < scopeHist.length; i++) {
    const px = x0 + (scopeHist.length > 1 ? (i / (scopeHist.length - 1)) * ww : ww * 0.5);
    const y = (H - 14) - Math.min(1, scopeHist[i].f / 15.0) * (H - 22);
    if (i === 0) scopeCtx.moveTo(px, y);
    else scopeCtx.lineTo(px, y);
  }
  scopeCtx.stroke();
  scopeCtx.fillStyle = "#93accf";
  scopeCtx.fillText(`C11 Coh(${(coh * 100).toFixed(0)}%) & GHz(${f.toFixed(1)}) Trace`, x0, H - 2);
}

/* Wire controls (ALL mutations strictly via POST /api/cmd) */
$("sel-regime").addEventListener("change", () => initWorld(false));
$("sel-solvent").addEventListener("change", () => doCmd({ op: "solvent", val: Number($("sel-solvent").value) }));
$("sel-layer").addEventListener("change", () => { if (scope) scope.layer = $("sel-layer").value; });
$("inp-seed").addEventListener("change", () => initWorld(false));

$("btn-play").addEventListener("click", () => {
  playing = !playing;
  const btn = $("btn-play");
  btn.textContent = playing ? "⏸ Pause Live" : "▶ Play Live";
  btn.setAttribute("aria-pressed", String(playing));
});
$("btn-embryo").addEventListener("click", () => {
  $("sel-regime").value = "5";
  $("sel-layer").value = "fates";
  if (Number($("inp-seed").value) === 0) $("inp-seed").value = lastDetSeed || "5000";
  if (scope) scope.layer = "fates";
  initWorld(false);
});
$("btn-feral").addEventListener("click", () => {
  $("sel-regime").value = "3";
  $("sel-layer").value = "composite";
  if (scope) scope.layer = "composite";
  initWorld(true);
});
$("btn-plume").addEventListener("click", () => {
  const rx = Math.floor(Math.random() * 32), ry = Math.floor(Math.random() * 32);
  doCmd({ op: "plume", x: rx, y: ry });
});
$("btn-autophage").addEventListener("click", () => {
  const s = Number($("inp-seed").value);
  doCmd({ op: "autophage", seed: s === 0 ? 0 : Math.floor(Math.random() * 9999999) + 1 });
});
$("btn-ap-hex").addEventListener("click", () => {
  const hex = String($("inp-ap-hex").value || "").trim();
  if (hex.length % 2 !== 0 || !/^[0-9a-fA-F]{64,256}$/.test(hex)) {
    const errMsg = "Invalid DNA hex: enter an even number (64–256) of hexadecimal characters";
    $("vp-status").textContent = errMsg;
    announce(errMsg);
    return;
  }
  doCmd({ op: "autophage_dna", hex });
  announce("AUTOPHAGE DNA transfected.");
});
$("btn-focus").addEventListener("click", () => { if (scope) scope.focusApex(); });
$("btn-cam").addEventListener("click", () => { if (scope) scope.resetCamera(); });

$("chk-orbit").addEventListener("change", (e) => { if (scope) scope.autoOrbit = e.target.checked; });
$("chk-axons").addEventListener("change", (e) => { if (scope) scope.showAxons = e.target.checked; });
$("chk-mt").addEventListener("change", (e) => { if (scope) scope.showMicrotubules = e.target.checked; });
$("chk-flag").addEventListener("change", (e) => { if (scope) scope.showFlagella = e.target.checked; });
$("chk-glyph").addEventListener("change", (e) => { if (scope) scope.showGlyphs = e.target.checked; });

$("btn-tun").addEventListener("click", () => { doCmd({ op: "tun" }); announce("92% desiccation tun triggered."); });
$("btn-shock").addEventListener("click", () => { doCmd({ op: "shock" }); announce("90% shock lysis triggered."); });
$("btn-rehyd").addEventListener("click", () => { doCmd({ op: "rehydrate" }); announce("Tun rehydration triggered."); });

const setRest = (v) => {
  $("rng-rest").value = String(v);
  $("rest-lbl").textContent = `${v}%`;
  syncRest13Sliders(v);
  doCmd({ op: "restraint", val: v });
};
$("rng-rest").addEventListener("input", () => {
  const v = Number($("rng-rest").value);
  $("rest-lbl").textContent = `${v}%`;
  syncRest13Sliders(v);
});
$("rng-rest").addEventListener("change", () => {
  const v = Number($("rng-rest").value);
  syncRest13Sliders(v);
  doCmd({ op: "restraint", val: v });
});
$("btn-r0").addEventListener("click", () => setRest(0));
$("btn-r50").addEventListener("click", () => setRest(50));
$("btn-r100").addEventListener("click", () => setRest(100));

let rafId = 0;
let stepTimerId = 0;

function rafLoop(now) {
  if (!document.hidden) {
    if (scope) scope.render(now);
    const ep = latestData?.telemetry?.epoch ?? -1;
    if (ep !== lastScopeEpoch) {
      lastScopeEpoch = ep;
      drawGHzOscilloscope();
    }
  }
  rafId = requestAnimationFrame(rafLoop);
}
rafId = requestAnimationFrame(rafLoop);

initWorld(false).then(() => {
  stepTimerId = setInterval(stepWorld, 110);
});

window.addEventListener("pagehide", () => {
  if (rafId) cancelAnimationFrame(rafId);
  if (stepTimerId) clearInterval(stepTimerId);
  if (panelRo) panelRo.disconnect();
  if (scope) scope.dispose();
});
