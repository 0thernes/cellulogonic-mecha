import { CellMicroscope3D } from "./renderer3d.js";

const $ = (id) => document.getElementById(id);

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

const scope = new CellMicroscope3D($("gl-canvas"), $("hud-canvas"));
const scopeCtx = $("scope-canvas").getContext("2d");
const asmoCtx = $("asmo-canvas").getContext("2d");

let playing = true;
let inflight = false;
let latestData = null;

async function apiJson(url) {
  const r = await fetch(url);
  const j = await r.json();
  if (j.error) throw new Error(j.error);
  return j;
}

function applyData(data) {
  latestData = data;
  scope.setGrid(data.grid);
  renderTelemetry(data.telemetry);
  renderBridge(data.bridge);
  $("vp-status").textContent =
    `t=${data.telemetry.epoch} | ${data.telemetry.alive} 3D cells | ${data.telemetry.mean_mt_ghz.toFixed(2)} GHz | 0-drift`;
}

async function initWorld(forceFeral = false) {
  if (forceFeral) $("inp-seed").value = "0";
  const regime = Number($("sel-regime").value);
  const solvent = Number($("sel-solvent").value);
  const seed = Number($("inp-seed").value);
  const rest = Number($("rng-rest").value);
  const warm = regime === 5 ? 28 : (regime === 0 ? 48 : 80);
  $("vp-status").textContent = seed === 0 ? "Seeding from Intel RDSEED thermal entropy..." : "Initializing 3D specimen...";
  try {
    const data = await apiJson(`/api/init?regime=${regime}&solvent=${solvent}&seed=${seed}&restraint=${rest}&warmup=${warm}`);
    applyData(data);
    if (regime === 5) {
      scope.dist = 18.5; // Zoom in smoothly onto the growing 3D clonal embryo!
    }
  } catch (e) {
    $("vp-status").textContent = `Init error: ${e.message}`;
  }
}

async function stepWorld() {
  if (!playing || inflight) return;
  inflight = true;
  try {
    const n = Number($("sel-speed").value);
    const data = await apiJson(`/api/step?n=${n}`);
    applyData(data);
  } catch (e) {
    $("vp-status").textContent = `Step error: ${e.message}`;
  } finally {
    inflight = false;
  }
}

async function doAction(op, params = {}) {
  const qs = new URLSearchParams({ op, ...params });
  try {
    const data = await apiJson(`/api/action?${qs.toString()}`);
    applyData(data);
  } catch (e) {
    $("vp-status").textContent = `Action error: ${e.message}`;
  }
}

function renderTelemetry(t) {
  const bm = $("badge-mass");
  bm.textContent = `MASS Δ: ${t.mass_res} amu`;
  bm.className = `badge ${t.mass_res === 0 ? "ok" : "bad"}`;

  const be = $("badge-energy");
  be.textContent = `1ST LAW Δ: ${t.energy_res} uq`;
  be.className = `badge ${t.energy_res === 0 ? "ok" : "bad"}`;

  const b2 = $("badge-2nd");
  b2.textContent = `2ND LAW VIOL: ${t.second_law}`;
  b2.className = `badge ${t.second_law === 0 ? "ok" : "bad"}`;

  $("badge-sha").textContent = `SHA-256: ${t.sha256.slice(0, 14)}…`;

  $("k-covi").textContent = t.covi.toFixed(2);
  $("k-stage").textContent = `Stage ${t.stage}/6 · MET ${t.met}/7`;
  $("k-mt").textContent = `${t.mean_mt_ghz.toFixed(2)} GHz`;
  $("k-coh").textContent = `Fröhlich Coh ${(t.mean_mt_coh * 100).toFixed(0)}%`;
  $("k-alive").textContent = String(t.alive);
  $("k-split").textContent = `P:${t.prok} E:${t.euk} S2:${t.sec} Tun:${t.tuns}`;
  $("k-col").textContent = `${t.fewcell} / ${t.multicell}`;
  $("k-maxt").textContent = `Few / Multi · Max ${t.max_tissue}`;
  $("k-spd").textContent = `${t.mean_speed.toFixed(2)} μm/s`;
  $("k-lane").textContent = t.mean_euk_vol > 0
    ? `${t.mean_euk_vol.toFixed(0)} / ${t.mean_prok_vol.toFixed(0)}`
    : `${t.mean_prok_vol.toFixed(0)} (Prok)`;
  $("k-vmem").textContent = `${t.mean_vmem.toFixed(1)} mV`;
  $("k-vstd").textContent = `σ = ${t.std_vmem.toFixed(1)} mV`;
  $("k-assm").textContent = `${t.assembly.toFixed(1)} / ${t.leff.toFixed(1)}`;
  $("k-anew").textContent = `A_new = ${t.anew.toFixed(3)}`;

  $("pillars").innerHTML = t.pillars.map((v, i) => `
    <div class="p-item">
      <div class="p-top"><span>${PILLAR_NAMES[i]}</span><strong>${v.toFixed(1)}</strong></div>
      <div class="p-bar"><div class="p-fill" style="width:${Math.min(100, v)}%"></div></div>
    </div>
  `).join("");

  const f = t.fates || [0,0,0,0,0,0];
  const mm = t.morph_means || [0,0,0];
  const morphRows = [
    ["GRN Fates (Ec/N/M/G)", `${f[1]} / ${f[2]} / ${f[3]} / ${f[4]} (P:${f[0]})`],
    ["Steinberg Cadherin Sorting", `${(t.homo_cad_pct || 0).toFixed(1)}% homophilic`],
    ["3D Axons (Live · Wired/Pruned)", `${t.active_axons || 0} (${t.axons_wired || 0} / ${t.axons_pruned || 0})`],
    ["CPG Rhythmic Motor Pulses", `${(t.cpg_pulses || 0).toLocaleString()}`],
    ["Size Control (Hippo / Eutely)", `${t.hippo_arrests || 0} / ${t.eutelic_stops || 0}`],
    ["GPS Wnt / BMP / Shh · Wedge", `${mm[0].toFixed(0)} / ${mm[1].toFixed(0)} / ${mm[2].toFixed(0)} · ${(t.mean_wedge || 0).toFixed(2)}`]
  ];
  $("tbl-morph").innerHTML = morphRows.map(([k, v]) => `<tr><td>${k}</td><td>${v}</td></tr>`).join("");

  const evRows = [
    ["CMC Abiogenesis Vesicles", t.abiogenesis],
    ["3D Binary Fissions / Buddings", `${t.sym_fiss} / ${t.asym_bud}`],
    ["Organelles (Mito / Chloro)", `${t.orgs} (${t.mito} / ${t.chlo})`],
    ["Endosymbiotic Captures (Sec)", `${t.endo_cap} (${t.sec_cap})`],
    ["EGT Transfers / Mitophagy", `${t.egt} / ${t.mitophagy}`],
    ["HGT Uptakes / Phage Lyses", `${t.hgt} / ${t.phages}`],
    ["Somatic Cancer / Apoptosis", `${t.cancer} / ${t.apoptosis}`],
    ["Mean c-Ring nH / Reduced Q", `${t.mean_cring.toFixed(2)} / ${t.mean_Q.toFixed(3)}`],
    ["Radiated 2nd-Law Heat Sink", `${t.heat_sink.toLocaleString()} uq`]
  ];
  $("tbl-events").innerHTML = evRows.map(([k, v]) => `<tr><td>${k}</td><td>${v}</td></tr>`).join("");

  const blRows = [
    ["Kill-Switch Deletions (DEGRADING)", t.backlash[0]],
    ["Audit-Evasion Spores (SPEEDBUMP)", t.backlash[1]],
    ["HGT Complexity Smuggles (DEGRESS)", t.backlash[2]],
    ["Metabolic Bypasses (NERFING)", t.backlash[3]]
  ];
  $("tbl-backlash").innerHTML = blRows.map(([k, v]) => `<tr><td>${k}</td><td>${v}</td></tr>`).join("");
}

function renderBridge(b) {
  const c = $("asmo-canvas");
  const W = c.width, H = c.height;
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
    // Traits 0..15: Direct cellular bioenergetics/morphology (Gold-Cyan); Traits 16..111: Operon 96D (Emerald-Violet)
    if (i < 16) {
      asmoCtx.fillStyle = `hsl(${38 + i * 10}, 95%, ${48 + v * 22}%)`;
    } else {
      asmoCtx.fillStyle = `hsl(${170 + ((i - 16) / 96) * 135}, 82%, ${38 + v * 30}%)`;
    }
    asmoCtx.fillRect(6 + i * bw, H - 12 - bh, Math.max(1.5, bw - 0.8), bh);
  }
  // Divider between 16 direct cellular observables and 96 operon-projected traits
  const divX = 6 + 16 * bw;
  asmoCtx.strokeStyle = "rgba(255, 255, 255, 0.35)";
  asmoCtx.beginPath();
  asmoCtx.moveTo(divX, 2);
  asmoCtx.lineTo(divX, H - 12);
  asmoCtx.stroke();

  asmoCtx.fillStyle = "#7f97ba";
  asmoCtx.font = "9px ui-monospace, Consolas, monospace";
  asmoCtx.fillText("0..15 Physio/V_m/Org", 6, H - 2);
  asmoCtx.fillText("16..111 AUTOPHAGE Operon -> Asmodensius 112-Trait Vector [0,1]", divX + 5, H - 2);
}

function drawGHzOscilloscope(nowMs) {
  const c = $("scope-canvas");
  const W = c.width, H = c.height;
  scopeCtx.fillStyle = "#040810";
  scopeCtx.fillRect(0, 0, W, H);
  if (!latestData || !latestData.grid.cells.length) return;

  // Left half: live population GHz microtubule eigenfrequency spectrum (1..15 GHz)
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
    scopeCtx.fillRect(6 + i * bw, H - 12 - h, bw - 2, h);
  }
  scopeCtx.fillStyle = "#7f97ba";
  scopeCtx.font = "9.5px ui-monospace, Consolas, monospace";
  scopeCtx.fillText("1 GHz", 6, H - 2);
  scopeCtx.fillText("8 GHz", W * 0.27, H - 2);
  scopeCtx.fillText("15 GHz (Fröhlich MT Modes)", W * 0.36, H - 2);

  // Right half: REAL rolling C11 telemetry history (mean GHz & Fröhlich coherence — zero synthetic sine waves!)
  const x0 = W * 0.60, ww = W * 0.38;
  const coh = latestData.telemetry.mean_mt_coh || 0;
  const f = latestData.telemetry.mean_mt_ghz || 0;
  const ep = latestData.telemetry.epoch || 0;
  if (!window._cmScopeHist) window._cmScopeHist = [];
  const hist = window._cmScopeHist;
  if (hist.length === 0 || hist[hist.length - 1].ep !== ep) {
    hist.push({ ep, coh, f });
    if (hist.length > 96) hist.shift();
  }
  scopeCtx.strokeStyle = "#38e8c6";
  scopeCtx.lineWidth = 1.6;
  scopeCtx.beginPath();
  for (let i = 0; i < hist.length; i++) {
    const px = x0 + (hist.length > 1 ? (i / (hist.length - 1)) * ww : ww * 0.5);
    const y = (H - 14) - Math.min(1, hist[i].coh) * (H - 22);
    if (i === 0) scopeCtx.moveTo(px, y);
    else scopeCtx.lineTo(px, y);
  }
  scopeCtx.stroke();
  scopeCtx.strokeStyle = "#f59e0b";
  scopeCtx.lineWidth = 1.2;
  scopeCtx.beginPath();
  for (let i = 0; i < hist.length; i++) {
    const px = x0 + (hist.length > 1 ? (i / (hist.length - 1)) * ww : ww * 0.5);
    const y = (H - 14) - Math.min(1, hist[i].f / 15.0) * (H - 22);
    if (i === 0) scopeCtx.moveTo(px, y);
    else scopeCtx.lineTo(px, y);
  }
  scopeCtx.stroke();
  scopeCtx.fillStyle = "#7f97ba";
  scopeCtx.fillText(`C11 Coh(${(coh*100).toFixed(0)}%) & GHz(${f.toFixed(1)}) Trace`, x0, H - 2);
}

// Wire controls
$("sel-regime").addEventListener("change", () => initWorld(false));
$("sel-solvent").addEventListener("change", () => doAction("solvent", { val: $("sel-solvent").value }));
$("sel-layer").addEventListener("change", () => { scope.layer = $("sel-layer").value; });
$("inp-seed").addEventListener("change", () => initWorld(false));

$("btn-play").addEventListener("click", () => {
  playing = !playing;
  $("btn-play").textContent = playing ? "⏸ Pause Live" : "▶ Play Live";
});
$("btn-embryo").addEventListener("click", () => {
  $("sel-regime").value = "5";
  $("sel-layer").value = "fates";
  scope.layer = "fates";
  initWorld(false);
});
$("btn-feral").addEventListener("click", () => {
  $("sel-regime").value = "3";
  $("sel-layer").value = "composite";
  scope.layer = "composite";
  initWorld(true);
});
$("btn-plume").addEventListener("click", () => {
  const rx = Math.floor(Math.random() * 32), ry = Math.floor(Math.random() * 32);
  doAction("plume", { x: rx, y: ry });
});
$("btn-autophage").addEventListener("click", () => {
  const s = Number($("inp-seed").value);
  // If seed == 0, uses live Intel RDSEED hardware entropy for feral AUTOPHAGE 256B plasmid!
  doAction("autophage", { seed: s === 0 ? 0 : Math.floor(Math.random() * 9999999) + 1 });
});
$("btn-focus").addEventListener("click", () => scope.focusApex());
$("btn-cam").addEventListener("click", () => scope.resetCamera());

$("chk-orbit").addEventListener("change", (e) => { scope.autoOrbit = e.target.checked; });
$("chk-axons").addEventListener("change", (e) => { scope.showAxons = e.target.checked; });
$("chk-mt").addEventListener("change", (e) => { scope.showMicrotubules = e.target.checked; });
$("chk-flag").addEventListener("change", (e) => { scope.showFlagella = e.target.checked; });
$("chk-glyph").addEventListener("change", (e) => { scope.showGlyphs = e.target.checked; });

$("btn-tun").addEventListener("click", () => doAction("tun"));
$("btn-shock").addEventListener("click", () => doAction("shock"));
$("btn-rehyd").addEventListener("click", () => doAction("rehydrate"));

const setRest = (v) => {
  $("rng-rest").value = String(v);
  $("rest-lbl").textContent = `${v}%`;
  doAction("restraint", { val: v });
};
$("rng-rest").addEventListener("input", () => { $("rest-lbl").textContent = `${$("rng-rest").value}%`; });
$("rng-rest").addEventListener("change", () => doAction("restraint", { val: $("rng-rest").value }));
$("btn-r0").addEventListener("click", () => setRest(0));
$("btn-r50").addEventListener("click", () => setRest(50));
$("btn-r100").addEventListener("click", () => setRest(100));

// Start smooth 60 FPS WebGL2 render loop + continuous live C11 simulation stepping (~10 Hz!)
function rafLoop(now) {
  scope.render(now);
  drawGHzOscilloscope(now);
  requestAnimationFrame(rafLoop);
}
requestAnimationFrame(rafLoop);

initWorld(false).then(() => {
  setInterval(stepWorld, 110);
});
