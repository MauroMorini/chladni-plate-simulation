# Python tools (`chladni-viz`)

Visualization of the simulation results. The package is managed with [uv](https://docs.astral.sh/uv/):
`pyproject.toml` lists the dependencies (numpy, matplotlib) and `uv.lock` pins their exact versions.

```bash
cd python
uv sync                 # create .venv and install everything (uv run also does this automatically)
uv run pytest           # run the tests
```

| Command | What it does |
|---------|--------------|
| `uv run chladni-plot FILES...` | 2D top view of one or more `*_top.csv` files. `--field uz` (default): bright = at rest (sand), dark = vibrating. `--field stress`: heatmap of the von Mises stress. Orange = nodal lines u_z = 0. |
| `uv run chladni-3d FILE` | Rotatable 3D view of the top face, deformed by u_z (exaggerated). `--field uz` colors up (red) / down (blue), `--field stress` colors by stress. `--animate` lets the plate oscillate. |

Both commands take `--save FILE` to write a `.png` (or a `.gif` for `chladni-3d --animate`) instead of
opening a window.

```bash
uv run chladni-plot ../cpp/build/output/mode_*_top.csv                   # all modes in one figure
uv run chladni-plot ../cpp/build/output/mode_*_top.csv --field stress
uv run chladni-3d ../cpp/build/output/mode_03_506.1Hz_top.csv --animate
```

### What is shown

The `*_top.csv` files contain, for the top face of the plate on a regular grid:

- `uz`: the vertical displacement, the component that throws the sand around.
- `von_mises`: the von Mises stress sqrt(3/2 s:s), where s is the deviatoric part of the stress tensor
  sigma = lambda tr(eps) I + 2 mu eps. It is a single number that says how strongly the metal is loaded.

Eigenmodes have no physical amplitude, so both fields are shown relative to their maximum. For the stress,
the colors are scaled to the 99th percentile and clipped above it. The clamped bolt causes a stress peak
(a singularity at the edge of the clamp) that would otherwise use up the whole color range.

## Layout

```
src/chladni_viz/io.py            reading the C++ output (load_top_surface, ...)
src/chladni_viz/plot_pattern.py  chladni-plot command
src/chladni_viz/view3d.py        chladni-3d command
tests/                           pytest tests
```

Planned: sand-particle animation, a resonance-curve plot of `sweep.csv`, and an interactive app with a
frequency slider based on modal superposition.
