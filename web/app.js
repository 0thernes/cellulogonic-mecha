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

let playing = false;
let timer = null;
let lastData = null;
let pendingTun = 0;

function getRestraintsCsv() {
  const v = Number($("rng-rest").value);
  if (v <= 0) return "";
  return Array(13).fill(v).join(",");
}

async function fetchAndRender() {
  const epochs = Number($("inp-epoch").value);
  const seed = Number($("inp-seed").value);
  const regime = Number($("sel-regime").value);
  const solvent = Number($("sel-solvent").value);
  const restraints = getRestraintsCsv();

  $("vp-status").textContent = "Computing native C11 AVX2 step...";
  const qs = new URLSearchParams({
    epochs: String(epochs),
    size: "32",
    seed: String(seed),
    regime: String(regime),
    solvent: String(solvent),
    tun: String(pendingTun)
  });
  if (restraints) qs.set("restraints", restraints);

  try {
    const resp = await fetch(`/api/snapshot?${qs.toString()}`);
    const data = await resp.json();
    if (data.error) throw new Error(data.error);
    lastData = data;
    pendingTun = 0;
    renderTelemetry(data.telemetry);
    renderMicroscope(data.grid, $("sel-layer").value);
    $("vp-status").textContent = `t=${data.telemetry.epoch} | ${data.telemetry.alive} alive | 0-bit exact`;
  } catch (err) {
    $("vp-status").textContent = `Error: ${err.message}`;
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
  $("k-stage").textContent = `Stage ${t.stage} / 6`;
  $("k-met").textContent = `${t.met} / 7`;
  $("k-alive").textContent = String(t.alive);
  $("k-split").textContent = `P:${t.prok} E:${t.euk} S2:${t.sec} Tun:${t.tuns}`;
  $("k-orgs").textContent = String(t.orgs);
  $("k-org-split").textContent = `Mito:${t.mito} Chloro:${t.chlo} Self:${t.selfish_org}`;
  $("k-lane").textContent = t.mean_euk_vol > 0 ? `${t.mean_euk_vol.toFixed(0)} / ${t.mean_prok_vol.toFixed(0)}` : `${t.mean_prok_vol.toFixed(0)} (Prok)`;
  $("k-tis").textContent = String(t.multicellular);
  $("k-maxt").textContent = `Max Cluster: ${t.max_tissue}`;
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

  const evRows = [
    ["CMC Abiogenesis Vesicles", t.abiogenesis],
    ["Symmetric Fissions (Q ≤ 1/2)", t.sym_fiss],
    ["Asymmetric Buddings (1/2 < Q < 1)", t.asym_bud],
    ["Osmotic Lyses", t.lyses],
    ["Endosymbiotic Captures (Sec)", `${t.endo_cap} (${t.sec_cap})`],
    ["Endosymbiotic Gene Transfers (EGT)", t.egt],
    ["Mitophagy Policing", t.mitophagy],
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

function renderMicroscope(grid, layer) {
  const canvas = $("microscope");
  const ctx = canvas.getContext("2d");
  const W = grid.w, H = grid.h;
  const cw = canvas.width / W;
  const ch = canvas.height / H;

  ctx.fillStyle = "#04070d";
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  // 1. Background spatial substrate S & oxidant X field
  for (let i = 0; i < grid.cells.length; i++) {
    const c = grid.cells[i];
    const x = i % W, y = Math.floor(i / W);
    const sVal = Math.min(1, c[8] / 220);
    const xVal = Math.min(1, c[9] / 180);
    ctx.fillStyle = `rgb(${Math.floor(8 + sVal * 20)}, ${Math.floor(14 + xVal * 28)}, ${Math.floor(24 + sVal * 32)})`;
    ctx.fillRect(x * cw, y * ch, cw, ch);
  }

  // 2. Bioelectric gap-junction tissue bonds
  ctx.strokeStyle = "rgba(86, 240, 159, 0.55)";
  ctx.lineWidth = 2;
  for (let i = 0; i < grid.cells.length; i++) {
    const c = grid.cells[i];
    if (c[0] === 0 || c[3] === 0) continue;
    const x = i % W, y = Math.floor(i / W);
    const cx = (x + 0.5) * cw, cy = (y + 0.5) * ch;
    const bm = c[3];
    if ((bm & 4) && x + 1 < W) {
      ctx.beginPath(); ctx.moveTo(cx, cy); ctx.lineTo(cx + cw, cy); ctx.stroke();
    }
    if ((bm & 2) && y + 1 < H) {
      ctx.beginPath(); ctx.moveTo(cx, cy); ctx.lineTo(cx, cy + ch); ctx.stroke();
    }
  }

  // 3. Living cells, bilayers & internalized nested organelles
  const LIPID_COLORS = ["#38e8c6", "#60b8ff", "#ff9f43", "#e8ff59"];
  for (let i = 0; i < grid.cells.length; i++) {
    const c = grid.cells[i];
    const state = c[0];
    if (state === 0) continue;
    const orgs = c[1], sec = c[2], lipClass = c[4], vmem = c[5], vol = c[6], lip = c[7];
    const x = i % W, y = Math.floor(i / W);
    const cx = (x + 0.5) * cw, cy = (y + 0.5) * ch;
    const r = Math.max(3.5, Math.min(cw * 0.48, (cw * 0.24) + Math.sqrt(Math.max(1, vol)) * 0.35));

    let fill = "#38e8c6";
    let stroke = "#a8fff0";
    if (layer === "composite") {
      if (state === 3) { fill = "#7be3ff"; stroke = "#ffffff"; }
      else if (state === 2) { fill = "#496988"; stroke = "#9ec3e6"; }
      else if (sec > 0) { fill = "#b67cff"; stroke = "#f2ddff"; }
      else if (orgs > 0) { fill = "#f5b942"; stroke = "#fff0c2"; }
    } else if (layer === "vmem") {
      const t = Math.max(0, Math.min(1, (vmem + 90) / 130));
      const R = Math.floor(40 + t * 215);
      const G = Math.floor(70 + (1 - Math.abs(t - 0.5) * 2) * 140);
      const B = Math.floor(245 - t * 195);
      fill = `rgb(${R},${G},${B})`;
      stroke = "#ffffff";
    } else if (layer === "strain") {
      const Q = lip > 0 ? (100 * vol * vol) / (lip * lip * lip) : 1.2;
      fill = Q <= 0.5 ? "#38e8c6" : (Q <= 0.9 ? "#f5b942" : "#ff5d73");
      stroke = "#ffffff";
    } else if (layer === "solvent") {
      fill = LIPID_COLORS[lipClass & 3];
      stroke = lipClass === grid.solvent ? "#ffffff" : "#ff5d73";
    }

    ctx.beginPath();
    ctx.arc(cx, cy, r, 0, Math.PI * 2);
    ctx.fillStyle = fill;
    ctx.globalAlpha = state === 2 ? 0.55 : 0.85;
    ctx.fill();
    ctx.globalAlpha = 1.0;
    ctx.lineWidth = sec > 0 ? 2.0 : 1.2;
    ctx.strokeStyle = stroke;
    ctx.stroke();

    // Render internalized organelle dots inside eukaryotes!
    if (layer === "composite" && orgs > 0 && state === 1) {
      const shown = Math.min(6, orgs);
      for (let k = 0; k < shown; k++) {
        const ang = (k / shown) * Math.PI * 2 + i * 0.3;
        const ox = cx + Math.cos(ang) * (r * 0.45);
        const oy = cy + Math.sin(ang) * (r * 0.45);
        ctx.beginPath();
        ctx.arc(ox, oy, 1.7, 0, Math.PI * 2);
        ctx.fillStyle = (k % 2 === 0) ? "#101828" : "#00ffd5";
        ctx.fill();
      }
    }
  }
}

$("sel-regime").addEventListener("change", fetchAndRender);
$("sel-solvent").addEventListener("change", fetchAndRender);
$("sel-layer").addEventListener("change", () => lastData && renderMicroscope(lastData.grid, $("sel-layer").value));
$("inp-seed").addEventListener("change", fetchAndRender);
$("inp-epoch").addEventListener("change", fetchAndRender);
$("rng-rest").addEventListener("input", () => { $("rest-lbl").textContent = `${$("rng-rest").value}%`; });
$("rng-rest").addEventListener("change", fetchAndRender);

$("btn-step").addEventListener("click", () => {
  $("inp-epoch").value = String(Math.min(400, Number($("inp-epoch").value) + 12));
  fetchAndRender();
});
$("btn-reset").addEventListener("click", () => {
  $("inp-epoch").value = "0";
  fetchAndRender();
});
$("btn-tun").addEventListener("click", () => { pendingTun = 1; fetchAndRender(); });
$("btn-shock").addEventListener("click", () => { pendingTun = 2; fetchAndRender(); });
$("btn-r0").addEventListener("click", () => { $("rng-rest").value = "0"; $("rest-lbl").textContent = "0%"; fetchAndRender(); });
$("btn-r50").addEventListener("click", () => { $("rng-rest").value = "50"; $("rest-lbl").textContent = "50%"; fetchAndRender(); });
$("btn-r100").addEventListener("click", () => { $("rng-rest").value = "100"; $("rest-lbl").textContent = "100%"; fetchAndRender(); });

$("btn-play").addEventListener("click", () => {
  playing = !playing;
  $("btn-play").textContent = playing ? "⏸ Pause" : "▶ Play Live";
  if (playing) {
    timer = setInterval(() => {
      const next = Number($("inp-epoch").value) + 6;
      $("inp-epoch").value = String(next > 360 ? 0 : next);
      fetchAndRender();
    }, 550);
  } else {
    clearInterval(timer);
  }
});

fetchAndRender();
