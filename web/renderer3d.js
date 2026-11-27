/* SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2026 Alexander Stephen Donahue
 * ============================================================================
 * CELLULOGONIC MECHA v1.7.0 — Zero-Dependency Bare-Metal WebGL2 3D Organic
 * Deformable Cellular Microscopy Engine (GLSL ES 3.00)
 * ============================================================================ */

function mat4Perspective(fovy, aspect, near, far) {
  const f = 1.0 / Math.tan(fovy / 2);
  const nf = 1 / (near - far);
  return new Float32Array([
    f / aspect, 0, 0, 0,
    0, f, 0, 0,
    0, 0, (far + near) * nf, -1,
    0, 0, 2 * far * near * nf, 0
  ]);
}

function mat4LookAt(eye, center, up) {
  let zx = eye[0] - center[0], zy = eye[1] - center[1], zz = eye[2] - center[2];
  let zl = Math.hypot(zx, zy, zz) || 1;
  zx /= zl; zy /= zl; zz /= zl;
  let xx = up[1] * zz - up[2] * zy;
  let xy = up[2] * zx - up[0] * zz;
  let xz = up[0] * zy - up[1] * zx;
  let xl = Math.hypot(xx, xy, xz) || 1;
  xx /= xl; xy /= xl; xz /= xl;
  let yx = zy * xz - zz * xy;
  let yy = zz * xx - zx * xz;
  let yz = zx * xy - zy * xx;
  return new Float32Array([
    xx, yx, zx, 0,
    xy, yy, zy, 0,
    xz, yz, zz, 0,
    -(xx * eye[0] + xy * eye[1] + xz * eye[2]),
    -(yx * eye[0] + yy * eye[1] + yz * eye[2]),
    -(zx * eye[0] + zy * eye[1] + zz * eye[2]),
    1
  ]);
}

function mat4Mul(a, b) {
  const out = new Float32Array(16);
  for (let c = 0; c < 4; c++) {
    for (let r = 0; r < 4; r++) {
      out[c * 4 + r] =
        a[0 * 4 + r] * b[c * 4 + 0] +
        a[1 * 4 + r] * b[c * 4 + 1] +
        a[2 * 4 + r] * b[c * 4 + 2] +
        a[3 * 4 + r] * b[c * 4 + 3];
    }
  }
  return out;
}

/* Build subdivided unit icosphere mesh (depth 2 -> 320 smooth triangles) */
function buildIcosphere(subdiv = 2) {
  const t = (1 + Math.sqrt(5)) / 2;
  const verts = [
    [-1, t, 0], [1, t, 0], [-1, -t, 0], [1, -t, 0],
    [0, -1, t], [0, 1, t], [0, -1, -t], [0, 1, -t],
    [t, 0, -1], [t, 0, 1], [-t, 0, -1], [-t, 0, 1]
  ].map((v) => { const l = Math.hypot(...v); return [v[0] / l, v[1] / l, v[2] / l]; });

  let faces = [
    [0,11,5],[0,5,1],[0,1,7],[0,7,10],[0,10,11],
    [1,5,9],[5,11,4],[11,10,2],[10,7,6],[7,1,8],
    [3,9,4],[3,4,2],[3,2,6],[3,6,8],[3,8,9],
    [4,9,5],[2,4,11],[6,2,10],[8,6,7],[9,8,1]
  ];

  const cache = new Map();
  const mid = (a, b) => {
    const k = a < b ? `${a}_${b}` : `${b}_${a}`;
    if (cache.has(k)) return cache.get(k);
    const va = verts[a], vb = verts[b];
    const m = [va[0] + vb[0], va[1] + vb[1], va[2] + vb[2]];
    const l = Math.hypot(...m) || 1;
    const idx = verts.length;
    verts.push([m[0] / l, m[1] / l, m[2] / l]);
    cache.set(k, idx);
    return idx;
  };

  for (let s = 0; s < subdiv; s++) {
    const nf = [];
    for (const [a, b, c] of faces) {
      const ab = mid(a, b), bc = mid(b, c), ca = mid(c, a);
      nf.push([a, ab, ca], [b, bc, ab], [c, ca, bc], [ab, bc, ca]);
    }
    faces = nf;
  }

  const pos = new Float32Array(faces.length * 9);
  let ptr = 0;
  for (const [a, b, c] of faces) {
    for (const idx of [a, b, c]) {
      pos[ptr++] = verts[idx][0];
      pos[ptr++] = verts[idx][1];
      pos[ptr++] = verts[idx][2];
    }
  }
  return { pos, count: faces.length * 3 };
}

function compileShader(gl, type, src) {
  const sh = gl.createShader(type);
  gl.shaderSource(sh, src);
  gl.compileShader(sh);
  if (!gl.getShaderParameter(sh, gl.COMPILE_STATUS)) {
    const log = gl.getShaderInfoLog(sh);
    gl.deleteShader(sh);
    throw new Error("GLSL compile error: " + log);
  }
  return sh;
}

function linkProgram(gl, vsSrc, fsSrc) {
  const vs = compileShader(gl, gl.VERTEX_SHADER, vsSrc);
  const fs = compileShader(gl, gl.FRAGMENT_SHADER, fsSrc);
  const p = gl.createProgram();
  gl.attachShader(p, vs);
  gl.attachShader(p, fs);
  gl.linkProgram(p);
  gl.deleteShader(vs);
  gl.deleteShader(fs);
  if (!gl.getProgramParameter(p, gl.LINK_STATUS)) {
    const log = gl.getProgramInfoLog(p);
    gl.deleteProgram(p);
    throw new Error("GLSL link error: " + log);
  }
  return p;
}

const CELL_VS = `#version 300 es
precision highp float;

layout(location=0) in vec3 a_meshPos;
layout(location=1) in vec4 a_posRad;
layout(location=2) in vec4 a_headElong;
layout(location=3) in vec4 a_dyn;
layout(location=4) in vec4 a_colorCore;
layout(location=5) in vec4 a_colorRim;

uniform mat4 u_viewProj;
uniform vec3 u_camPos;
uniform float u_time;

out vec3 v_normal;
out vec3 v_viewDir;
out vec3 v_coreCol;
out vec3 v_rimCol;
out float v_coherence;
out float v_mtRipple;
out float v_stateFlag;
out float v_morphSeed;

vec3 safeNorm(vec3 v, vec3 fb) {
  float l = length(v);
  return l > 1e-5 ? (v / l) : fb;
}

void main() {
  vec3 p = a_meshPos;
  vec3 fwd = safeNorm(a_headElong.xyz, vec3(1.0, 0.0, 0.0));
  vec3 upRef = abs(fwd.y) < 0.9 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
  vec3 right = safeNorm(cross(upRef, fwd), vec3(0.0, 0.0, 1.0));
  vec3 up = cross(fwd, right);

  float elong = a_headElong.w;
  float ruffle = a_dyn.x;
  float bPhase = a_dyn.y;
  float bAmp = a_dyn.z;
  float mtGhz = a_dyn.w;
  float mSeed = a_colorRim.a;
  float stateFlag = floor(a_colorCore.a);
  float coh = fract(a_colorCore.a);

  float breathTempo = 1.6 + mSeed * 3.4 + coh * 2.2;
  float breathWave = sin(u_time * breathTempo + bPhase) * bAmp;

  vec3 q = p;
  vec3 nLocal = p;
  if (stateFlag > 2.5) {
    float facet = 0.14 * abs(sin(p.x * 5.0 + mSeed * 6.0) * cos(p.y * 5.0) * sin(p.z * 5.0));
    q *= (0.86 + facet);
    nLocal = safeNorm(p + vec3(0.18 * cos(p.x * 5.0 + mSeed * 6.0), -0.18 * sin(p.y * 5.0), 0.18 * cos(p.z * 5.0)), p);
    v_mtRipple = 0.0;
  } else {
    float sx = 1.0, sy = 1.0, sz = 1.0;
    vec2 nxy = p.xy;
    if (elong < 0.88) {
      float r2 = q.x * q.x + q.y * q.y;
      sz = max(0.22, elong * (0.48 + 0.55 * r2));
      sx = 1.16 + (0.88 - elong) * 0.4;
      sy = sx;
      q.z *= sz;
      q.xy *= sx;
    } else if (elong > 1.28) {
      float stretch = elong;
      float pinch = 1.0 - smoothstep(1.38, 2.05, elong) * 0.36 * exp(-6.5 * q.z * q.z);
      float twist = (mSeed > 0.55 ? 1.8 : 0.0) * q.z;
      float ct = cos(twist + u_time * 1.4), st = sin(twist + u_time * 1.4);
      vec2 tw = vec2(q.x * ct - q.y * st, q.x * st + q.y * ct);
      sx = max(0.25, pinch / sqrt(stretch * 0.85));
      sy = sx;
      sz = stretch * 0.88;
      q.xy = tw * sx;
      q.z *= sz;
      nxy = vec2(p.x * ct - p.y * st, p.x * st + p.y * ct);
    }

    float l1 = 2.0 + floor(mSeed * 4.0);
    float l2 = 3.0 + floor(fract(mSeed * 7.3) * 3.0);
    float lobe = sin(p.x * l1 + u_time * (1.2 + mSeed) + bPhase) *
                 cos(p.y * l2 - u_time * 1.1) *
                 sin(p.z * (l1 + 1.0) + bPhase * 0.5);
    float disp = 1.0 + breathWave + ruffle * 0.22 * lobe;

    float mtWave = sin(p.z * (4.0 + mtGhz * 0.9) - u_time * (4.0 + mtGhz * 0.65) + bPhase) *
                   cos((p.x + p.y) * 5.0 + u_time * 3.0);
    disp += coh * 0.045 * mtWave;
    q *= disp;
    nLocal = safeNorm(vec3(nxy.x / sx, nxy.y / sy, p.z / sz), p);
    v_mtRipple = 0.5 + 0.5 * mtWave;
  }

  vec3 worldOffset = (right * q.x + up * q.y + fwd * q.z) * a_posRad.w;
  vec3 worldPos = a_posRad.xyz + worldOffset;

  v_normal = safeNorm(right * nLocal.x + up * nLocal.y + fwd * nLocal.z, vec3(0.0, 1.0, 0.0));
  v_viewDir = safeNorm(u_camPos - worldPos, vec3(0.0, 0.0, 1.0));
  v_coreCol = a_colorCore.rgb;
  v_rimCol = a_colorRim.rgb;
  v_coherence = coh;
  v_stateFlag = stateFlag;
  v_morphSeed = mSeed;
  gl_Position = u_viewProj * vec4(worldPos, 1.0);
}
`;

const CELL_FS = `#version 300 es
precision highp float;

in vec3 v_normal;
in vec3 v_viewDir;
in vec3 v_coreCol;
in vec3 v_rimCol;
in float v_coherence;
in float v_mtRipple;
in float v_stateFlag;
in float v_morphSeed;

out vec4 fragColor;

vec3 safeNorm(vec3 v, vec3 fb) {
  float l = length(v);
  return l > 1e-5 ? (v / l) : fb;
}

void main() {
  vec3 N = safeNorm(v_normal, vec3(0.0, 1.0, 0.0));
  vec3 V = safeNorm(v_viewDir, vec3(0.0, 0.0, 1.0));
  vec3 L1 = normalize(vec3(0.55, 0.80, 0.45));
  vec3 L2 = normalize(vec3(-0.65, -0.35, 0.65));

  float ndv = clamp(dot(N, V), 0.0, 1.0);
  float fresnel = pow(1.0 - ndv, 2.35);
  float diff1 = max(dot(N, L1), 0.0);
  float diff2 = max(dot(N, L2), 0.0) * 0.45;
  float spec = pow(max(dot(reflect(-L1, N), V), 0.0), 28.0);

  vec3 col = v_coreCol * (0.34 + 0.66 * (diff1 + diff2));
  col += v_rimCol * fresnel * (1.25 + 0.55 * v_coherence * v_mtRipple);
  col += vec3(0.85, 0.96, 1.0) * spec * (v_stateFlag > 2.5 ? 0.95 : 0.42);

  float alpha = v_stateFlag > 2.5 ? 0.78 : (0.24 + 0.62 * fresnel + 0.12 * spec);
  if (v_stateFlag > 1.5 && v_stateFlag < 2.5) alpha *= 0.65;
  fragColor = vec4(col, clamp(alpha, 0.16, 0.92));
}
`;

const LINE_VS = `#version 300 es
precision highp float;
layout(location=0) in vec3 a_pos;
layout(location=1) in vec4 a_col;
uniform mat4 u_viewProj;
out vec4 v_col;
void main() {
  v_col = a_col;
  gl_Position = u_viewProj * vec4(a_pos, 1.0);
}
`;

const LINE_FS = `#version 300 es
precision highp float;
in vec4 v_col;
out vec4 fragColor;
void main() { fragColor = v_col; }
`;

const POINT_VS = `#version 300 es
precision highp float;
layout(location=0) in vec4 a_posSize;
layout(location=1) in vec4 a_col;
uniform mat4 u_viewProj;
uniform float u_scale;
out vec4 v_col;
void main() {
  v_col = a_col;
  vec4 clip = u_viewProj * vec4(a_posSize.xyz, 1.0);
  gl_Position = clip;
  gl_PointSize = clamp((a_posSize.w * u_scale) / max(clip.w, 1.0), 1.5, 42.0);
}
`;

const POINT_FS = `#version 300 es
precision highp float;
in vec4 v_col;
out vec4 fragColor;
void main() {
  vec2 c = gl_PointCoord * 2.0 - 1.0;
  float r2 = dot(c, c);
  if (r2 > 1.0) discard;
  float soft = pow(1.0 - r2, 1.8);
  fragColor = vec4(v_col.rgb, v_col.a * soft);
}
`;

function hsvToRgb(h, s, v) {
  h = ((h % 1) + 1) % 1;
  const i = Math.floor(h * 6);
  const f = h * 6 - i;
  const p = v * (1 - s);
  const q = v * (1 - f * s);
  const t = v * (1 - (1 - f) * s);
  switch (i % 6) {
    case 0: return [v, t, p];
    case 1: return [q, v, p];
    case 2: return [p, v, t];
    case 3: return [p, q, v];
    case 4: return [t, p, v];
    default: return [v, p, q];
  }
}

/* 20 strictly unique biochemical syntax glyphs (no duplicates) */
const BIO_GLYPHS = ["◈","⟁","Ψ","Ω","λ","Φ","⌬","⏣","⧖","⟐","☍","⧗","⊗","⊕","⟠","⟴","✦","❖","⬡","⎊"];

export class CellMicroscope3D {
  constructor(glCanvas, hudCanvas, stageWrap = null) {
    this.glCanvas = glCanvas;
    this.hudCanvas = hudCanvas;
    this.stageWrap = stageWrap || glCanvas.parentElement;
    this.hudCtx = hudCanvas.getContext("2d");

    const gl = glCanvas.getContext("webgl2", { antialias: true, alpha: false, preserveDrawingBuffer: false });
    if (!gl) throw new Error("WebGL2 required for 3D Cellular Microscopy Observatory");
    this.gl = gl;
    this.contextLost = false;
    this.lastNowMs = null;

    this.sphere = buildIcosphere(2);
    this.maxCells = 4096;
    this.instData = new Float32Array(this.maxCells * 20);
    this.lineData = new Float32Array(260000);
    this.ptData = new Float32Array(80000);

    this._initGlResources();

    this._onContextLost = (e) => {
      e.preventDefault();
      this.contextLost = true;
    };
    this._onContextRestored = () => {
      try {
        this._initGlResources();
        this.contextLost = false;
      } catch (err) {
        console.error("Failed to restore WebGL2 context:", err);
        this.contextLost = true;
      }
    };
    glCanvas.addEventListener("webglcontextlost", this._onContextLost);
    glCanvas.addEventListener("webglcontextrestored", this._onContextRestored);

    /* Cached CSS dimensions updated via ResizeObserver (avoids 60 FPS getBoundingClientRect) */
    this.cssWidth = glCanvas.clientWidth || 640;
    this.cssHeight = glCanvas.clientHeight || 480;
    this._ro = null;
    if (typeof ResizeObserver === "function") {
      this._ro = new ResizeObserver((entries) => {
        for (const entry of entries) {
          this.cssWidth = Math.max(300, entry.contentRect.width);
          this.cssHeight = Math.max(300, entry.contentRect.height);
        }
      });
      this._ro.observe(glCanvas);
    }

    this.yaw = 0.65;
    this.pitch = 0.36;
    this.dist = 36.0;
    this.target = [0, 0, 0];
    this.autoOrbit = true;
    this.reducedMotion = false;
    this.layer = "fates";
    this.showGlyphs = true;
    this.showMicrotubules = true;
    this.showFlagella = true;
    this.showAxons = true;

    this.motes = [];
    for (let i = 0; i < 360; i++) {
      this.motes.push([
        (Math.random() - 0.5) * 32,
        (Math.random() - 0.5) * 32,
        (Math.random() - 0.5) * 32,
        0.5 + Math.random() * 1.4,
        Math.random() * Math.PI * 2
      ]);
    }

    this.smoothPos = new Map();
    this.grid = null;
    this.lastViewProj = null;

    this._initInputControls();
  }

  _initGlResources() {
    const gl = this.gl;
    this.cellProg  = linkProgram(gl, CELL_VS, CELL_FS);
    this.lineProg  = linkProgram(gl, LINE_VS, LINE_FS);
    this.pointProg = linkProgram(gl, POINT_VS, POINT_FS);
    this.uLocs = {
      ptVP:    gl.getUniformLocation(this.pointProg, "u_viewProj"),
      ptScale: gl.getUniformLocation(this.pointProg, "u_scale"),
      lnVP:    gl.getUniformLocation(this.lineProg, "u_viewProj"),
      clVP:    gl.getUniformLocation(this.cellProg, "u_viewProj"),
      clCam:   gl.getUniformLocation(this.cellProg, "u_camPos"),
      clTime:  gl.getUniformLocation(this.cellProg, "u_time")
    };

    this.cellVao = gl.createVertexArray();
    gl.bindVertexArray(this.cellVao);
    this.meshVbo = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.meshVbo);
    gl.bufferData(gl.ARRAY_BUFFER, this.sphere.pos, gl.STATIC_DRAW);
    gl.enableVertexAttribArray(0);
    gl.vertexAttribPointer(0, 3, gl.FLOAT, false, 0, 0);

    this.instVbo = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.instVbo);
    gl.bufferData(gl.ARRAY_BUFFER, this.instData.byteLength, gl.DYNAMIC_DRAW);
    for (let loc = 1; loc <= 5; loc++) {
      gl.enableVertexAttribArray(loc);
      gl.vertexAttribPointer(loc, 4, gl.FLOAT, false, 80, (loc - 1) * 16);
      gl.vertexAttribDivisor(loc, 1);
    }

    this.lineVao = gl.createVertexArray();
    gl.bindVertexArray(this.lineVao);
    this.lineVbo = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.lineVbo);
    gl.bufferData(gl.ARRAY_BUFFER, this.lineData.byteLength, gl.DYNAMIC_DRAW);
    gl.enableVertexAttribArray(0);
    gl.vertexAttribPointer(0, 3, gl.FLOAT, false, 28, 0);
    gl.enableVertexAttribArray(1);
    gl.vertexAttribPointer(1, 4, gl.FLOAT, false, 28, 12);

    this.ptVao = gl.createVertexArray();
    gl.bindVertexArray(this.ptVao);
    this.ptVbo = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.ptVbo);
    gl.bufferData(gl.ARRAY_BUFFER, this.ptData.byteLength, gl.DYNAMIC_DRAW);
    gl.enableVertexAttribArray(0);
    gl.vertexAttribPointer(0, 4, gl.FLOAT, false, 32, 0);
    gl.enableVertexAttribArray(1);
    gl.vertexAttribPointer(1, 4, gl.FLOAT, false, 32, 16);

    gl.bindVertexArray(null);
  }

  _initInputControls() {
    let dragging = false, panMode = false, lx = 0, ly = 0;
    this._onContextMenu = (e) => e.preventDefault();
    this._onMouseDown = (e) => {
      dragging = true;
      panMode = (e.button === 2 || e.shiftKey);
      lx = e.clientX; ly = e.clientY;
    };
    this._onMouseUp = () => { dragging = false; };
    this._onMouseMove = (e) => {
      if (!dragging) return;
      const dx = e.clientX - lx, dy = e.clientY - ly;
      lx = e.clientX; ly = e.clientY;
      if (panMode) {
        const s = this.dist * 0.0018;
        this.target[0] -= Math.cos(this.yaw) * dx * s;
        this.target[2] += Math.sin(this.yaw) * dx * s;
        this.target[1] += dy * s;
      } else {
        this.yaw -= dx * 0.0065;
        this.pitch = Math.max(-1.42, Math.min(1.42, this.pitch + dy * 0.0065));
      }
    };
    this._onWheel = (e) => {
      e.preventDefault();
      this.dist = Math.max(5.5, Math.min(72.0, this.dist * (1 + e.deltaY * 0.001)));
    };
    this._onKeyDown = (e) => {
      if (e.key === "ArrowLeft") { this.yaw -= 0.12; e.preventDefault(); }
      else if (e.key === "ArrowRight") { this.yaw += 0.12; e.preventDefault(); }
      else if (e.key === "ArrowUp") { this.pitch = Math.min(1.42, this.pitch + 0.10); e.preventDefault(); }
      else if (e.key === "ArrowDown") { this.pitch = Math.max(-1.42, this.pitch - 0.10); e.preventDefault(); }
      else if (e.key === "+" || e.key === "=") { this.dist = Math.max(5.5, this.dist * 0.88); e.preventDefault(); }
      else if (e.key === "-" || e.key === "_") { this.dist = Math.min(72.0, this.dist * 1.12); e.preventDefault(); }
      else if (e.key === "r" || e.key === "R") { this.resetCamera(); e.preventDefault(); }
    };

    this.glCanvas.addEventListener("contextmenu", this._onContextMenu);
    this.glCanvas.addEventListener("mousedown", this._onMouseDown);
    window.addEventListener("mouseup", this._onMouseUp);
    window.addEventListener("mousemove", this._onMouseMove);
    this.glCanvas.addEventListener("wheel", this._onWheel, { passive: false });
    if (this.stageWrap) this.stageWrap.addEventListener("keydown", this._onKeyDown);
  }

  dispose() {
    this.glCanvas.removeEventListener("webglcontextlost", this._onContextLost);
    this.glCanvas.removeEventListener("webglcontextrestored", this._onContextRestored);
    this.glCanvas.removeEventListener("contextmenu", this._onContextMenu);
    this.glCanvas.removeEventListener("mousedown", this._onMouseDown);
    window.removeEventListener("mouseup", this._onMouseUp);
    window.removeEventListener("mousemove", this._onMouseMove);
    this.glCanvas.removeEventListener("wheel", this._onWheel);
    if (this.stageWrap) this.stageWrap.removeEventListener("keydown", this._onKeyDown);
    if (this._ro) this._ro.disconnect();
    const gl = this.gl;
    if (gl && !this.contextLost) {
      for (const p of [this.cellProg, this.lineProg, this.pointProg]) if (p) gl.deleteProgram(p);
      for (const b of [this.meshVbo, this.instVbo, this.lineVbo, this.ptVbo]) if (b) gl.deleteBuffer(b);
      for (const v of [this.cellVao, this.lineVao, this.ptVao]) if (v) gl.deleteVertexArray(v);
    }
  }

  setGrid(grid) {
    this.grid = grid;
    if (!grid || !Array.isArray(grid.cells)) return;
    const spanX = grid.span_x || grid.span || 32000;
    const spanY = grid.span_y || grid.span || 32000;
    const spanZ = grid.span_z || grid.span || 32000;
    const scaleX = 32.0 / spanX;
    const scaleY = 32.0 / spanY;
    const scaleZ = 32.0 / spanZ;
    const aliveSet = new Set();
    for (const c of grid.cells) {
      const id = c[0];
      const lineage = c[20] || 0;
      aliveSet.add(id);
      const tx = c[2] * scaleX - 16.0;
      const ty = c[4] * scaleZ - 16.0;
      const tz = c[3] * scaleY - 16.0;
      const cur = this.smoothPos.get(id);
      if (!cur || cur.lineage !== lineage) {
        this.smoothPos.set(id, { pos: [tx, ty, tz], lineage });
      }
    }
    for (const k of this.smoothPos.keys()) {
      if (!aliveSet.has(k)) this.smoothPos.delete(k);
    }
  }

  focusApex() {
    if (!this.grid || !this.grid.cells || !this.grid.cells.length) return;
    let best = this.grid.cells[0], bestScore = -1;
    for (const c of this.grid.cells) {
      const pack = c[15];
      const orgs = (pack & 15) + ((pack >>> 4) & 15);
      const score = c[8] + orgs * 400 + ((pack >>> 8) & 1) * 900;
      if (score > bestScore) { bestScore = score; best = c; }
    }
    const entry = this.smoothPos.get(best[0]);
    if (entry) {
      this.target = [entry.pos[0], entry.pos[1], entry.pos[2]];
      this.dist = 9.5;
    }
  }

  resetCamera() {
    this.yaw = 0.65; this.pitch = 0.36; this.dist = 36.0; this.target = [0, 0, 0];
  }

  render(nowMs) {
    if (this.contextLost) return;
    const gl = this.gl;
    const t = this.reducedMotion ? 0.0 : nowMs * 0.001;
    const dt = this.lastNowMs === null ? (1 / 60) : Math.min(0.1, Math.max(0.001, (nowMs - this.lastNowMs) * 0.001));
    this.lastNowMs = nowMs;

    const dpr = Math.min(window.devicePixelRatio || 1, 2);
    const rw = Math.max(300, Math.floor(this.cssWidth * dpr));
    const rh = Math.max(300, Math.floor(this.cssHeight * dpr));
    if (this.glCanvas.width !== rw || this.glCanvas.height !== rh) {
      this.glCanvas.width = rw; this.glCanvas.height = rh;
      this.hudCanvas.width = rw; this.hudCanvas.height = rh;
    }

    /* Frame-rate independent auto-orbit (~0.096 rad/sec regardless of 60/120/240 Hz!) */
    if (this.autoOrbit) this.yaw += 0.096 * dt;

    const camX = this.target[0] + this.dist * Math.cos(this.pitch) * Math.sin(this.yaw);
    const camY = this.target[1] + this.dist * Math.sin(this.pitch);
    const camZ = this.target[2] + this.dist * Math.cos(this.pitch) * Math.cos(this.yaw);
    const eye = [camX, camY, camZ];
    const proj = mat4Perspective(0.82, rw / rh, 0.2, 220.0);
    const view = mat4LookAt(eye, this.target, [0, 1, 0]);
    const vp = mat4Mul(proj, view);
    this.lastViewProj = vp;

    gl.viewport(0, 0, rw, rh);
    gl.clearColor(0.014, 0.024, 0.044, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.enable(gl.DEPTH_TEST);
    gl.enable(gl.BLEND);

    if (!this.grid || !Array.isArray(this.grid.cells)) return;

    const spanX = this.grid.span_x || this.grid.span || 32000;
    const spanY = this.grid.span_y || this.grid.span || 32000;
    const spanZ = this.grid.span_z || this.grid.span || 32000;
    const scaleX = 32.0 / spanX;
    const scaleY = 32.0 / spanY;
    const scaleZ = 32.0 / spanZ;
    const lerpAlpha = 1.0 - Math.exp(-10.5 * dt);

    const idMap = new Map();
    for (const c of this.grid.cells) {
      const id = c[0];
      const lineage = c[20] || 0;
      const tx = c[2] * scaleX - 16.0;
      const ty = c[4] * scaleZ - 16.0;
      const tz = c[3] * scaleY - 16.0;
      let entry = this.smoothPos.get(id);
      if (!entry || entry.lineage !== lineage) {
        entry = { pos: [tx, ty, tz], lineage };
        this.smoothPos.set(id, entry);
      } else {
        const sp = entry.pos;
        const dx = tx - sp[0], dy = ty - sp[1], dz = tz - sp[2];
        if (dx * dx + dy * dy + dz * dz > 64.0) {
          sp[0] = tx; sp[1] = ty; sp[2] = tz;
        } else {
          sp[0] += dx * lerpAlpha;
          sp[1] += dy * lerpAlpha;
          sp[2] += dz * lerpAlpha;
        }
      }
      idMap.set(id, { c, pos: entry.pos });
    }

    let ptPtr = 0;
    const pushPt = (x, y, z, sz, r, g, b, a) => {
      if (ptPtr + 8 > this.ptData.length) return;
      this.ptData[ptPtr++] = x; this.ptData[ptPtr++] = y; this.ptData[ptPtr++] = z; this.ptData[ptPtr++] = sz;
      this.ptData[ptPtr++] = r; this.ptData[ptPtr++] = g; this.ptData[ptPtr++] = b; this.ptData[ptPtr++] = a;
    };

    for (const m of this.motes) {
      const mx = m[0] + Math.sin(t * 0.35 + m[4]) * 0.6;
      const my = m[1] + Math.cos(t * 0.28 + m[4] * 1.3) * 0.5;
      const mz = m[2] + Math.sin(t * 0.31 - m[4]) * 0.6;
      pushPt(mx, my, mz, m[3] * 1.4, 0.35, 0.65, 0.85, 0.22);
    }

    if (this.grid.plumes && this.grid.plumes.length > 0) {
      const gw = Math.max(1, this.grid.plume_w || Math.round(Math.sqrt(this.grid.plumes.length)));
      const gh = Math.max(1, this.grid.plume_h || Math.ceil(this.grid.plumes.length / gw));
      const cellSpanX = 32.0 / gw;
      const cellSpanZ = 32.0 / gh;
      for (let i = 0; i < this.grid.plumes.length; i++) {
        const pl = this.grid.plumes[i];
        const px = ((i % gw) + 0.5) * cellSpanX - 16.0;
        const pz = (Math.floor(i / gw) + 0.5) * cellSpanZ - 16.0;
        const wntN = Math.min(1, (pl[3] || 0) / 140);
        const bmpN = Math.min(1, (pl[4] || 0) / 140);
        const shhN = Math.min(1, (pl[5] || 0) / 140);
        if (this.layer === "morphogens" || (wntN + bmpN + shhN > 0.25)) {
          if (wntN + bmpN + shhN > 0.08) {
            const boost = this.layer === "morphogens" ? 0.26 : 0.11;
            pushPt(px, Math.sin(t * 0.45 + i * 0.7) * 5.0, pz, 26.0 + (wntN + bmpN + shhN) * 18.0,
                   0.12 + wntN * 0.88, 0.12 + bmpN * 0.85, 0.14 + shhN * 0.86, boost);
          }
        } else {
          const sNorm = Math.min(1, pl[0] / 210);
          const xNorm = Math.min(1, pl[1] / 170);
          const qNorm = Math.min(1, pl[2] / 60);
          if (sNorm > 0.35 || qNorm > 0.12) {
            pushPt(px, -11.0 + Math.sin(t * 0.5 + i) * 3.5, pz, 24.0 + sNorm * 22.0,
                   0.06 + qNorm * 0.35, 0.18 + xNorm * 0.30, 0.28 + sNorm * 0.38, 0.08 + qNorm * 0.09);
          }
        }
      }
    }

    let lnPtr = 0;
    const pushSeg = (ax, ay, az, bx, by, bz, r, g, b, a) => {
      if (lnPtr + 14 > this.lineData.length) return;
      this.lineData[lnPtr++] = ax; this.lineData[lnPtr++] = ay; this.lineData[lnPtr++] = az;
      this.lineData[lnPtr++] = r;  this.lineData[lnPtr++] = g;  this.lineData[lnPtr++] = b;  this.lineData[lnPtr++] = a;
      this.lineData[lnPtr++] = bx; this.lineData[lnPtr++] = by; this.lineData[lnPtr++] = bz;
      this.lineData[lnPtr++] = r;  this.lineData[lnPtr++] = g;  this.lineData[lnPtr++] = b;  this.lineData[lnPtr++] = a;
    };

    for (let ring = 0; ring < 2; ring++) {
      const segs = 64, R = 17.8;
      for (let s = 0; s < segs; s++) {
        const a0 = (s / segs) * Math.PI * 2, a1 = ((s + 1) / segs) * Math.PI * 2;
        if (ring === 0) pushSeg(Math.cos(a0)*R, 0, Math.sin(a0)*R, Math.cos(a1)*R, 0, Math.sin(a1)*R, 0.16, 0.34, 0.52, 0.18);
        else pushSeg(Math.cos(a0)*R, Math.sin(a0)*R, 0, Math.cos(a1)*R, Math.sin(a1)*R, 0, 0.16, 0.34, 0.52, 0.12);
      }
    }

    if (this.grid.bonds) {
      for (const bItem of this.grid.bonds) {
        const ia = bItem[0], ib = bItem[1], homo = bItem[2] !== 0;
        const A = idMap.get(ia), B = idMap.get(ib);
        if (!A || !B) continue;
        const dx = B.pos[0] - A.pos[0], dy = B.pos[1] - A.pos[1], dz = B.pos[2] - A.pos[2];
        if (dx*dx + dy*dy + dz*dz > 64.0) continue;
        const pulse = 0.5 + 0.5 * Math.sin(t * 6.5 + (ia + ib) * 0.7);
        const mx = (A.pos[0] + B.pos[0]) * 0.5 + Math.sin(t * 2.5 + ia) * 0.14;
        const my = (A.pos[1] + B.pos[1]) * 0.5 + Math.cos(t * 2.2 + ib) * 0.14;
        const mz = (A.pos[2] + B.pos[2]) * 0.5;
        const cr = homo ? (0.20 + pulse * 0.45) : 0.96;
        const cg = homo ? 0.95 : 0.44;
        const cb = homo ? 0.76 : 0.52;
        pushSeg(A.pos[0], A.pos[1], A.pos[2], mx, my, mz, cr, cg, cb, homo ? 0.72 : 0.45);
        pushSeg(mx, my, mz, B.pos[0], B.pos[1], B.pos[2], cr, cg, cb, homo ? 0.72 : 0.45);
      }
    }

    if (this.showAxons && this.grid.axons) {
      for (const [ia, ib, wt, firing] of this.grid.axons) {
        const A = idMap.get(ia), B = idMap.get(ib);
        if (!A || !B) continue;
        const dx = B.pos[0] - A.pos[0], dy = B.pos[1] - A.pos[1], dz = B.pos[2] - A.pos[2];
        if (dx*dx + dy*dy + dz*dz > 196.0) continue;
        const wNorm = Math.min(1, wt / 160);
        const arcY = 0.65 + 0.25 * Math.sin(t * 3.0 + ia);
        const mx = (A.pos[0] + B.pos[0]) * 0.5;
        const my = (A.pos[1] + B.pos[1]) * 0.5 + arcY;
        const mz = (A.pos[2] + B.pos[2]) * 0.5;
        const ar = firing ? 1.0 : (0.78 + 0.2 * wNorm);
        const ag = firing ? 0.92 : (0.38 + 0.3 * wNorm);
        const ab = firing ? 0.35 : 0.98;
        const aa = firing ? 0.98 : (0.42 + 0.45 * wNorm);
        pushSeg(A.pos[0], A.pos[1], A.pos[2], mx, my, mz, ar, ag, ab, aa);
        pushSeg(mx, my, mz, B.pos[0], B.pos[1], B.pos[2], ar, ag, ab, aa);
        if (firing) {
          const u = (t * 2.6 + ia * 0.19) % 1.0;
          const inv = 1.0 - u;
          const sx = inv*inv*A.pos[0] + 2*inv*u*mx + u*u*B.pos[0];
          const sy = inv*inv*A.pos[1] + 2*inv*u*my + u*u*B.pos[1];
          const sz = inv*inv*A.pos[2] + 2*inv*u*mz + u*u*B.pos[2];
          pushPt(sx, sy, sz, 19.0, 1.0, 0.94, 0.28, 0.98);
        }
      }
    }

    let instCount = 0;
    const LIPID_HUES = [0.47, 0.56, 0.08, 0.18];
    const FATE_CORE = [
      [0.86, 0.92, 1.00],
      [0.18, 0.84, 1.00],
      [0.72, 0.34, 1.00],
      [1.00, 0.34, 0.42],
      [0.14, 0.94, 0.62],
      [1.00, 0.82, 0.24]
    ];

    for (const c of this.grid.cells) {
      if (instCount >= this.maxCells) break;
      const id = c[0];
      const state = c[1];
      const sp = idMap.get(id).pos;
      const hx = c[5] / 1024.0, hy = c[7] / 1024.0, hz = c[6] / 1024.0;
      const rad = Math.max(0.24, Math.min(1.85, c[8] * scaleX * 1.15));
      const elong = c[9] / 1000.0;
      const ruffle = c[10] / 1000.0;
      const bPhase = c[11] * 0.001;
      const bAmp = c[12] / 1000.0;
      const mtGhz = c[13] / 1000.0;
      const mtCoh = Math.min(0.995, c[14] / 1000.0);
      const pack = c[15] >>> 0;
      const mito = pack & 15;
      const chlo = (pack >>> 4) & 15;
      const sec = (pack >>> 8) & 1;
      const lipClass = (pack >>> 9) & 3;
      const mtFils = Math.max(2, (pack >>> 11) & 31);
      const flagella = c[16] / 255.0;
      const vmem = c[17];
      const vol = c[18];
      const lip = c[19];
      const lineage = c[20] || 1;
      const morphSig = (c[21] || 0) >>> 0;
      const grnPack = (c[22] || 0) >>> 0;
      const fate = grnPack & 7;
      const cpgFiring = (grnPack >>> 8) & 1;
      const apicalWedge = ((grnPack >>> 9) & 63) / 63.0;
      const morphSeed = ((morphSig * 2654435761) >>> 0) / 4294967296.0;

      let coreRgb, rimRgb;
      if (this.layer === "fates" || this.layer === "morphogens") {
        if (state === 3) {
          coreRgb = [0.42, 0.88, 1.0]; rimRgb = [0.95, 1.0, 1.0];
        } else if (cpgFiring) {
          coreRgb = [1.0, 0.94, 0.42]; rimRgb = [1.0, 1.0, 1.0];
        } else {
          const fc = FATE_CORE[fate] || FATE_CORE[0];
          coreRgb = [fc[0], fc[1], fc[2]];
          rimRgb  = apicalWedge > 0.35 ? [0.25, 1.0, 0.82] : [Math.min(1, fc[0] + 0.25), Math.min(1, fc[1] + 0.25), Math.min(1, fc[2] + 0.25)];
        }
      } else if (this.layer === "composite" || this.layer === "glyphs") {
        if (state === 3) {
          coreRgb = [0.42, 0.88, 1.0]; rimRgb = [0.95, 1.0, 1.0];
        } else {
          let hue = ((lineage * 0.6180339887) + ((morphSig & 255) / 255.0) * 0.35) % 1.0;
          if (chlo > mito) hue = (0.34 + (hue * 0.14)) % 1.0;
          else if (sec > 0) hue = (0.76 + (hue * 0.12)) % 1.0;
          else if (mito > 0) hue = (0.08 + (hue * 0.10)) % 1.0;
          const sat = state === 2 ? 0.35 : (0.68 + 0.28 * (((morphSig >>> 8) & 255) / 255.0));
          const val = state === 2 ? 0.55 : (0.78 + 0.20 * mtCoh);
          coreRgb = hsvToRgb(hue, sat, val);
          rimRgb  = hsvToRgb((hue + 0.14 + 0.20 * (((morphSig >>> 16) & 255) / 255.0)) % 1.0, 0.92, 1.0);
        }
      } else if (this.layer === "vmem") {
        const vn = Math.max(0, Math.min(1, (vmem + 90) / 130));
        coreRgb = [0.15 + vn * 0.85, 0.25 + (1 - Math.abs(vn - 0.5) * 2) * 0.65, 1.0 - vn * 0.82];
        rimRgb  = [1.0, 0.85, 0.95];
      } else if (this.layer === "strain") {
        const Q = lip > 0 ? (100 * vol * vol) / (lip * lip * lip) : 1.2;
        coreRgb = Q <= 0.5 ? [0.18, 0.95, 0.78] : (Q <= 0.9 ? [0.98, 0.72, 0.22] : [1.0, 0.25, 0.42]);
        rimRgb  = [1.0, 1.0, 1.0];
      } else {
        coreRgb = hsvToRgb(LIPID_HUES[lipClass], 0.85, 0.92);
        rimRgb  = lipClass === this.grid.solvent ? [0.9, 1.0, 1.0] : [1.0, 0.22, 0.35];
      }

      const off = instCount * 20;
      this.instData[off + 0]  = sp[0]; this.instData[off + 1]  = sp[1]; this.instData[off + 2]  = sp[2]; this.instData[off + 3]  = rad;
      this.instData[off + 4]  = hx;    this.instData[off + 5]  = hy;    this.instData[off + 6]  = hz;    this.instData[off + 7]  = elong;
      this.instData[off + 8]  = ruffle; this.instData[off + 9] = bPhase; this.instData[off + 10] = bAmp; this.instData[off + 11] = mtGhz;
      this.instData[off + 12] = coreRgb[0]; this.instData[off + 13] = coreRgb[1]; this.instData[off + 14] = coreRgb[2];
      this.instData[off + 15] = state + mtCoh;
      this.instData[off + 16] = rimRgb[0]; this.instData[off + 17] = rimRgb[1]; this.instData[off + 18] = rimRgb[2]; this.instData[off + 19] = morphSeed;
      instCount++;

      const totalOrg = mito + chlo;
      if (state === 1 && totalOrg > 0) {
        for (let k = 0; k < Math.min(8, totalOrg); k++) {
          const isChlo = k >= mito;
          const orbR = rad * (0.34 + 0.24 * ((k * 0.37) % 1));
          const a1 = t * (0.9 + mtGhz * 0.08) + k * 2.1 + bPhase;
          const a2 = t * 0.65 + k * 1.4;
          const ox = sp[0] + Math.cos(a1) * Math.cos(a2) * orbR;
          const oy = sp[1] + Math.sin(a2) * orbR;
          const oz = sp[2] + Math.sin(a1) * Math.cos(a2) * orbR;
          if (sec && k === 0) pushPt(ox, oy, oz, rad * 18.0, 0.85, 0.45, 1.0, 0.98);
          else if (isChlo)    pushPt(ox, oy, oz, rad * 13.5, 0.15, 1.0, 0.62, 0.95);
          else                pushPt(ox, oy, oz, rad * 13.5, 1.0, 0.78, 0.18, 0.95);
        }
      }

      if (this.showMicrotubules && state === 1 && mtCoh > 0.04) {
        const rays = Math.min(totalOrg > 0 ? 8 : 3, mtFils);
        for (let k = 0; k < rays; k++) {
          const th = (k / rays) * Math.PI * 2 + morphSeed * 6.28 + t * 0.18;
          const ph = Math.acos(1 - 2 * ((k + 0.5) / rays));
          const dx = Math.sin(ph) * Math.cos(th) * rad * 0.84;
          const dy = Math.cos(ph) * rad * 0.84;
          const dz = Math.sin(ph) * Math.sin(th) * rad * 0.84;
          const wave = 0.45 + 0.55 * Math.sin(t * (6.0 + mtGhz * 1.3) + k * 1.7 + bPhase);
          const br = 0.35 + 0.65 * mtCoh * wave;
          pushSeg(sp[0], sp[1], sp[2], sp[0] + dx, sp[1] + dy, sp[2] + dz,
                  0.45 * br, 0.95 * br, 1.0 * br, 0.38 + 0.45 * mtCoh);
        }
      }

      if (this.showFlagella && state === 1 && flagella > 0.18) {
        const hl = Math.hypot(hx, hy, hz) || 1;
        const fx = -hx / hl, fy = -hy / hl, fz = -hz / hl;
        const ux = Math.abs(fy) < 0.9 ? 0 : 1, uy = Math.abs(fy) < 0.9 ? 1 : 0, uz = 0;
        let rx = fy * uz - fz * uy, ry = fz * ux - fx * uz, rz = fx * uy - fy * ux;
        const rl = Math.hypot(rx, ry, rz) || 1;
        rx /= rl; ry /= rl; rz /= rl;
        const vx = fy * rz - fz * ry, vy = fz * rx - fx * rz, vz = fx * ry - fy * rx;
        const tailLen = rad * (1.5 + flagella * 1.9);
        const whipFreq = 8.0 + flagella * 14.0;
        let px0 = sp[0] + fx * rad * 0.8, py0 = sp[1] + fy * rad * 0.8, pz0 = sp[2] + fz * rad * 0.8;
        const segs = 5;
        for (let s = 1; s <= segs; s++) {
          const u = s / segs;
          const ang = u * 5.2 - t * whipFreq + bPhase;
          const amp = rad * 0.34 * u * flagella;
          const px1 = sp[0] + fx * (rad * 0.8 + tailLen * u) + (rx * Math.cos(ang) + vx * Math.sin(ang)) * amp;
          const py1 = sp[1] + fy * (rad * 0.8 + tailLen * u) + (ry * Math.cos(ang) + vy * Math.sin(ang)) * amp;
          const pz1 = sp[2] + fz * (rad * 0.8 + tailLen * u) + (rz * Math.cos(ang) + vz * Math.sin(ang)) * amp;
          pushSeg(px0, py0, pz0, px1, py1, pz1, rimRgb[0], rimRgb[1], rimRgb[2], 0.55 * (1 - u * 0.65));
          px0 = px1; py0 = py1; pz0 = pz1;
        }
      }
    }

    gl.depthMask(false);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE);
    gl.useProgram(this.pointProg);
    gl.uniformMatrix4fv(this.uLocs.ptVP, false, vp);
    gl.uniform1f(this.uLocs.ptScale, rh * 0.55);
    gl.bindVertexArray(this.ptVao);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.ptVbo);
    gl.bufferSubData(gl.ARRAY_BUFFER, 0, this.ptData.subarray(0, ptPtr));
    gl.drawArrays(gl.POINTS, 0, ptPtr / 8);

    gl.useProgram(this.lineProg);
    gl.uniformMatrix4fv(this.uLocs.lnVP, false, vp);
    gl.bindVertexArray(this.lineVao);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.lineVbo);
    gl.bufferSubData(gl.ARRAY_BUFFER, 0, this.lineData.subarray(0, lnPtr));
    gl.drawArrays(gl.LINES, 0, lnPtr / 7);

    gl.enable(gl.CULL_FACE);
    gl.cullFace(gl.BACK);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
    gl.useProgram(this.cellProg);
    gl.uniformMatrix4fv(this.uLocs.clVP, false, vp);
    gl.uniform3fv(this.uLocs.clCam, eye);
    gl.uniform1f(this.uLocs.clTime, t);
    gl.bindVertexArray(this.cellVao);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.instVbo);
    gl.bufferSubData(gl.ARRAY_BUFFER, 0, this.instData.subarray(0, instCount * 20));
    gl.drawArraysInstanced(gl.TRIANGLES, 0, this.sphere.count, instCount);
    gl.disable(gl.CULL_FACE);

    gl.depthMask(true);
    gl.bindVertexArray(null);

    this._renderGlyphHud(rw, rh, vp, idMap, dpr);
  }

  _renderGlyphHud(rw, rh, vp, idMap, dpr) {
    const ctx = this.hudCtx;
    if (!ctx) return;
    ctx.clearRect(0, 0, rw, rh);
    if (!this.showGlyphs && this.layer !== "glyphs") return;
    if (!this.grid || !this.grid.cells.length) return;

    ctx.font = `${Math.round(10.5 * dpr)}px ui-monospace, Consolas, monospace`;

    const orgCount = (pack) => (pack & 15) + ((pack >>> 4) & 15);
    const ranked = [...this.grid.cells]
      .filter((c) => c[1] !== 0)
      .sort((a, b) => (b[8] + orgCount(b[15]) * 400) - (a[8] + orgCount(a[15]) * 400))
      .slice(0, this.layer === "glyphs" ? 36 : 12);

    for (const c of ranked) {
      const entry = idMap.get(c[0]);
      if (!entry) continue;
      const [x, y, z] = entry.pos;
      const cx = vp[0]*x + vp[4]*y + vp[8]*z + vp[12];
      const cy = vp[1]*x + vp[5]*y + vp[9]*z + vp[13];
      const cw = vp[3]*x + vp[7]*y + vp[11]*z + vp[15];
      if (cw <= 0.5) continue;
      const sx = (cx / cw * 0.5 + 0.5) * rw;
      const sy = (1.0 - (cy / cw * 0.5 + 0.5)) * rh;
      if (sx < 10 || sx > rw - 120 || sy < 14 || sy > rh - 14) continue;

      const sig = (c[21] || 0) >>> 0;
      const g0 = BIO_GLYPHS[((c[20] || 0) >>> 0) % BIO_GLYPHS.length];
      const g1 = BIO_GLYPHS[(sig >>> 4) % BIO_GLYPHS.length];
      const g2 = BIO_GLYPHS[(sig >>> 10) % BIO_GLYPHS.length];
      const ghz = (c[13] / 1000).toFixed(1);
      const diam = ((c[8] * 2) / 1000).toFixed(2);
      const tag = `${g0}${g1}${g2} ${ghz}GHz · ${diam}μm · ${c[17]}mV`;

      ctx.fillStyle = "rgba(7, 13, 24, 0.58)";
      const tw = ctx.measureText(tag).width + 10 * dpr;
      ctx.fillRect(sx + 8 * dpr, sy - 10 * dpr, tw, 15 * dpr);
      ctx.fillStyle = c[1] === 3 ? "#8cebff" : (orgCount(c[15]) > 0 ? "#ffd46b" : "#67f7dc");
      ctx.fillText(tag, sx + 13 * dpr, sy + 1 * dpr);
    }
  }
}
