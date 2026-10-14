// Lebende Chladni-Platte für die Startseite: Sandkörner werden dort herumgeschüttelt, wo die Platte
// stark schwingt, und bleiben auf den Knotenlinien liegen. Als Schwingungsform dient die klassische
// Näherung für die quadratische Platte
//   u(x, y) = cos(nπx) cos(mπy) ± cos(mπx) cos(nπy),   x, y ∈ [0, 1],
// alle paar Sekunden wechselt (n, m, ±) und der Sand sucht sich die neuen Knotenlinien.

const MODES = [[2, 3, 1], [1, 2, -1], [1, 3, -1], [3, 4, -1], [1, 4, 1], [2, 5, -1], [3, 5, 1], [1, 5, -1]];
const GRAINS = 7000;
const MODE_SECONDS = 7;
const SHAKE = 0.06;    // maximale Verschiebung pro Schritt (in Plattenbreiten) bei |u| = 1
const SAND = "#A5D7D2";

const canvas = document.getElementById("plate");
const ctx = canvas.getContext("2d");
const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches;

const xs = new Float32Array(GRAINS), ys = new Float32Array(GRAINS);
for (let i = 0; i < GRAINS; i++) { xs[i] = Math.random(); ys[i] = Math.random(); }

let mode = 0, modeStart = performance.now();

function u(x, y, [n, m, sign]) {
  const px = Math.PI * x, py = Math.PI * y;
  return Math.cos(n * px) * Math.cos(m * py) + sign * Math.cos(m * px) * Math.cos(n * py);
}

function shake(steps) {
  const current = MODES[mode];
  for (let s = 0; s < steps; s++) {
    for (let i = 0; i < GRAINS; i++) {
      const a = SHAKE * Math.abs(u(xs[i], ys[i], current));
      // an den Rändern zurückspiegeln, damit kein Sand verloren geht
      let x = xs[i] + (Math.random() - 0.5) * a, y = ys[i] + (Math.random() - 0.5) * a;
      xs[i] = x < 0 ? -x : x > 1 ? 2 - x : x;
      ys[i] = y < 0 ? -y : y > 1 ? 2 - y : y;
    }
  }
}

function resize() {
  const size = canvas.clientWidth * devicePixelRatio;
  canvas.width = canvas.height = size;
}

function draw() {
  const size = canvas.width, r = Math.max(1, size / 380);
  ctx.clearRect(0, 0, size, size);
  ctx.fillStyle = SAND;
  for (let i = 0; i < GRAINS; i++) ctx.fillRect(xs[i] * size, ys[i] * size, r, r);
}

function frame(now) {
  if (now - modeStart > MODE_SECONDS * 1000) { mode = (mode + 1) % MODES.length; modeStart = now; }
  shake(2);
  draw();
  requestAnimationFrame(frame);
}

resize();
window.addEventListener("resize", () => { resize(); draw(); });
if (reducedMotion) { shake(400); draw(); }   // ohne Animation: fertiges Muster zeigen
else requestAnimationFrame(frame);
