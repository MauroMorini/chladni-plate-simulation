# Python tools (`chladni-viz`)

Turns the results of the C++ simulation into pictures. The package is managed with
[uv](https://docs.astral.sh/uv/): `pyproject.toml` lists the dependencies (numpy, matplotlib) and
`uv.lock` pins their exact versions. You never need to install anything by hand: every `uv run ...`
first makes sure the environment in `.venv/` is up to date.

All commands below are run from this `python/` folder. The simulation results it reads live in
`../cpp/build/output/`, and the plots are saved there as well.

## The main use: the first N eigenmodes in one PDF

First compute the eigenmodes with the C++ program (see the
[main README](../README.md#quick-start-the-first-n-chladni-figures-as-a-pdf)), e.g.

```bash
(cd ../cpp/build && ./chladni ../parameters/eigenmodes_free.prm)
```

Then plot them:

```bash
uv run chladni-plot ../cpp/build/output/eigenmodes_free/mode_*_top.csv \
    --align-pairs --first 24 --save ../cpp/build/output/first24_free.pdf
```

What happens, step by step:

1. **Read** all given `*_top.csv` files (the shell expands `mode_*_top.csv` to every mode file).
2. **Sort** them by the frequency in their file name.
3. `--align-pairs`: **straighten pairs.** Because the plate is square, many modes come in pairs with
   the same frequency, a pattern and its 90° rotation. Any mix of the two is also a valid mode, and the
   eigensolver returns an arbitrary, often skewed one. The two modes are replaced by the combinations
   that are symmetric / antisymmetric under the mirror x → −x: parallel (wavy) lines, as on the real plate.
4. `--symmetric-only` (optional): **keep only the strongest patterns,** those unchanged by a 90°
   rotation and by mirroring. Only these are excited by a perfectly centered shaker. Since only about
   1 in 6 modes passes, compute about 6 N modes if you want N of them.
5. `--first N`: **keep the N lowest** and number them 1..N.
6. **Draw** one panel per mode in a roughly square grid and `--save` it. The file ending selects the
   format (`.pdf`, `.png`, `.svg`). Without `--save` a window opens instead.

Reading a panel:

| What you see | Meaning |
|--------------|---------|
| bright | the plate is at rest: the sand collects here |
| dark | the plate vibrates strongly: the sand is thrown away |
| orange lines | nodal lines u_z = 0, the Chladni figure |
| title `k: f Hz` | k-th lowest mode, frequency f. Equal frequencies = a pair. |

Typical variations:

```bash
O=../cpp/build/output

# only the patterns a centered shaker excites, first 9
uv run chladni-plot $O/eigenmodes_free/mode_*_top.csv --symmetric-only --first 9 --save $O/first9_symmetric.pdf

# the clamped plate (after running a parameter file with Center support = clamped, see the main README)
uv run chladni-plot $O/eigenmodes_clamped/mode_*_top.csv --align-pairs --first 24 --save $O/first24_clamped.pdf

# 4 panels per row, as PNG
uv run chladni-plot $O/eigenmodes_free/mode_*_top.csv --align-pairs --first 16 --columns 4 --save $O/first16.png

# just look at it in a window
uv run chladni-plot $O/eigenmodes_free/mode_*_top.csv --align-pairs --first 12
```

## All commands and options

| Command | What it does |
|---------|--------------|
| `uv run chladni-plot FILES...` | 2D top view of one or more `*_top.csv` files in one figure, sorted by frequency. |
| `uv run chladni-3d FILE` | Rotatable 3D view of one `*_top.csv` file, deformed by u_z (exaggerated). `--field uz` colors up (red) / down (blue), `--field stress` colors by stress. `--animate` lets the plate oscillate. `--save` writes a `.png` (or a `.gif` with `--animate`). |

`chladni-plot` options:

| Option | Effect |
|--------|--------|
| `--first N` | keep the N lowest frequencies (after the filters), numbered 1..N |
| `--align-pairs` | show pairs of modes with equal frequency as parallel lines (only with `--field uz`) |
| `--symmetric-only` | keep only modes unchanged by a 90° rotation and by mirroring |
| `--save FILE` | write to FILE (`.pdf`, `.png`, `.svg`, ...) instead of opening a window |
| `--columns C` | panels per row (default: roughly square) |
| `--field uz` / `--field stress` | color by \|u_z\| (default) or by the von Mises stress |
| `--no-nodal-lines` | do not draw the orange u_z = 0 lines |

`uv run chladni-plot --help` prints the same information.

### What the files contain

The `*_top.csv` files hold, for the top face of the plate on a regular grid (columns `x,y,uz,von_mises`):

- `uz`: the vertical displacement, the component that throws the sand around.
- `von_mises`: the von Mises stress sqrt(3/2 s:s), where s is the deviatoric part of the stress tensor
  sigma = lambda tr(eps) I + 2 mu eps. It is a single number that says how strongly the metal is loaded.

Eigenmodes have no physical amplitude, so both fields are shown relative to their maximum. For the stress,
the colors are scaled to the 99th percentile and clipped above it. The clamped bolt causes a stress peak
(a singularity at the edge of the clamp) that would otherwise use up the whole color range.

## Development

```bash
uv sync          # create .venv and install everything (uv run also does this automatically)
uv run pytest    # run the tests
```

```
src/chladni_viz/io.py            reading the C++ output, sorting files by frequency
src/chladni_viz/symmetry.py      symmetry test (--symmetric-only) and pair alignment (--align-pairs)
src/chladni_viz/plot_pattern.py  chladni-plot command
src/chladni_viz/view3d.py        chladni-3d command
tests/                           pytest tests
```

Planned: sand-particle animation, a resonance-curve plot of `sweep.csv`, and an interactive app with a
frequency slider based on modal superposition.
