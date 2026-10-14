# chladni-plate-simulation

Simulation of the [Chladni plate](https://en.wikipedia.org/wiki/Ernst_Chladni#Chladni_figures)
experiment with [deal.II](https://www.dealii.org). A thin square metal plate is held by a bolt at its
center and shaken up and down. At certain frequencies sand on the plate gathers along the lines that
do not move, the *nodal lines*, and forms the famous Chladni figures.

```
cpp/              C++ / deal.II simulation code: computes the vibrations of the plate
cpp/parameters/   all parameter files (.prm)
cpp/build/        the only place where the C++ code is built and run; results go to cpp/build/output/
python/           Python package (uv, numpy + matplotlib): turns the results into pictures
presentation/     slides; they take their simulation results and plots from cpp/build/output/
site/             public website (slides PDF, animations, game), see below
game/             Chladni game in the browser (plain JavaScript), from github.com/inversecrime/chladni
```

## Quick start: the first N Chladni figures as a PDF

The whole pipeline is three steps: **build** the C++ program once, **compute** the eigenmodes, and
**plot** them. The result is a single PDF with one panel per mode, numbered and sorted by
frequency (for the default free 24 cm aluminium plate: 1: 56.8 Hz, 2: 82.9 Hz, 3: 105.1 Hz, ...).

```bash
# all three steps, starting in the repository root
cd cpp
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j    # 1. build
cd build && ./chladni ../parameters/eigenmodes_free.prm                     # 2. compute
cd ../../python && uv run chladni-plot ../cpp/build/output/eigenmodes_free/mode_*_top.csv \
    --align-pairs --first 24 --save ../cpp/build/output/first24_free.pdf     # 3. plot
```

The steps in detail:

### Step 0: requirements (once)

- deal.II ≥ 9.8 configured with PETSc and SLEPc (see [Requirements](#requirements) below).
- [uv](https://docs.astral.sh/uv/) for the Python part. It installs numpy and matplotlib by itself
  the first time you run a `uv run ...` command.

### Step 1: build the simulation (once, and after every change to the C++ code)

```bash
cd cpp
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release   # add -DDEAL_II_DIR=... if deal.II is not in /usr/local/deal.II
cmake --build build -j
ctest --test-dir build                           # optional: all tests should pass
```

This creates the program `cpp/build/chladni`.

### Step 2: compute the eigenmodes

The program reads everything from a parameter file. All parameter files live in `cpp/parameters/`.
A ready-made one for the lowest 60 modes of the free plate is `eigenmodes_free.prm`. Always run the
program from `cpp/build`:

```bash
cd cpp/build
./chladni ../parameters/eigenmodes_free.prm      # takes about 15 s
```

It writes one `mode_XX_<f>Hz_top.csv` per mode (plus `.vtu` files for ParaView) to
`cpp/build/output/eigenmodes_free/`: the `Output directory` of the parameter file, relative to
`cpp/build`.

| Parameter file | Content |
|----------------|---------|
| `default.prm` | 6 clamped modes around 500 Hz, commented template with all entries |
| `eigenmodes_free.prm` | lowest 60 modes of the free plate → `output/eigenmodes_free/` |
| `eigenmodes.prm` | lowest 40 modes of the clamped plate, used for the slides → `output/eigenmodes/` |

The entries that matter for this pipeline (in `subsection Simulation`):

| Entry | Value for "the first N modes" | Why |
|-------|-------------------------------|-----|
| `Mode` | `eigenmodes` | compute vibration modes (not a forced vibration) |
| `Frequency` | `0` | the solver returns the modes **closest** to this frequency, so 0 gives the lowest ones |
| `Number of eigenmodes` | at least N, e.g. 60 | how many modes to compute. With `--symmetric-only` (see below) only about 1 in 6 is kept, so then use about 6 N. |
| `Center support` | `free` or `clamped` | how the shaker holds the plate, see [Physics](#physics). `free` matches the typical Chladni figures. |

To change the plate itself (other size, thickness or material), edit `subsection Geometry` and
`subsection Material`. Changing E, ρ or h scales all frequencies by the same factor, while the order
and shape of the patterns stay the same.

To keep several runs side by side, copy the file and change `Output directory`, e.g.

```bash
cd cpp/parameters
cp eigenmodes_free.prm eigenmodes_clamped.prm
# in eigenmodes_clamped.prm: set Center support = clamped
#                            set Output directory = output/eigenmodes_clamped
cd ../build && ./chladni ../parameters/eigenmodes_clamped.prm
```

### Step 3: plot the first N modes into a PDF

```bash
cd python
uv run chladni-plot ../cpp/build/output/eigenmodes_free/mode_*_top.csv \
    --align-pairs --first 24 --save ../cpp/build/output/first24_free.pdf
```

Saving the plot next to the results in `cpp/build/output/` keeps everything in one place; the
slides reference their images from there.

| Option | Effect |
|--------|--------|
| `--first N` | keep the N lowest frequencies and number the panels 1..N |
| `--save FILE` | write the figure to FILE. The ending picks the format: `.pdf`, `.png`, `.svg`. Without `--save` a window opens. |
| `--align-pairs` | **recommended.** Many modes come in pairs with the same frequency (a pattern and its 90° rotation). The solver returns an arbitrary, skewed mix of the two. This option shows them as the parallel (wavy) lines you see on the real plate. |
| `--symmetric-only` | keep only the modes that a perfectly centered shaker excites (patterns unchanged by a 90° rotation and by mirroring). These are the strongest, easiest to find patterns. A real setup also shows the others, more weakly, because the bolt rocks a little and nothing is perfectly symmetric. |
| `--columns C` | number of panels per row (default: roughly square) |
| `--field stress` | color by the von Mises stress instead of the displacement (not together with `--align-pairs`) |

How to read the figure:

- **Bright** = the plate is at rest. This is where the sand collects.
- **Dark** = the plate vibrates strongly. The sand is thrown away from here.
- **Orange lines** = the nodal lines u_z = 0, i.e. the Chladni figure itself.
- The title is the mode number and its frequency. Two panels with the same frequency are a pair:
  on the real plate you get one of them, or a mix, depending on small imperfections.

More plotting options (3D view, animation) are in [`python/README.md`](python/README.md).

## Physics

The plate is modeled as a 3D linear elastic body Ω = [-L/2, L/2]² × [0, h]:

- **Material:** isotropic, with Young's modulus E, Poisson ratio ν and density ρ. The Lamé parameters are
  λ = Eν / ((1+ν)(1−2ν)) and μ = E / (2(1+ν)).
- **Boundary conditions:** every outer face is traction free (zero Neumann).
- **The bolt:** the shaker acts on the plate through a small cylinder of radius r around the vertical center axis.
  We don't use just a single point: a point Dirichlet condition is not well defined for H¹ functions in 3D, so the
  result would depend on the mesh.
- **Center support** (how the shaker holds the bolt):
  - `clamped` (displacement drive): the shaker dictates the bolt's motion. Right for a heavy, stiff shaker.
  - `free` (force drive): the shaker only pushes on the bolt and the plate is otherwise free. Right for a shaker
    that is light compared to the plate.

  A real shaker is somewhere in between. Mathematically, `clamped` prescribes u on the bolt and leaves the
  force there unknown; `free` prescribes the force and leaves u unknown. The resonances are therefore the
  eigenfrequencies of the clamped plate and of the free plate, respectively.

After finite element discretization we get the stiffness matrix K and mass matrix M:

    K_ij = ∫ 2μ ε(φ_i):ε(φ_j) + λ div φ_i div φ_j dx
    M_ij = ∫ ρ φ_i·φ_j dx

### Eigenmodes and what the shaker does

An eigenmode is a vibration the plate performs on its own, K φ = ω² M φ, with the shaker "switched
off": for `clamped` the shaker holds the bolt still (u = 0 there), for `free` it does nothing at all.
The drive does not enter the eigenproblem. It decides **which** eigenmodes show up and how strongly.
Writing the forced vibration in terms of the eigenmodes φ_k (scaled to φ_kᵀ M φ_k = 1):

    û = Σ_k φ_k (φ_kᵀ f) / (ω_k² − ω²)

- The denominator gives the **resonances**: near ω = ω_k mode k dominates and its pattern appears.
- The numerator φ_kᵀ f is the **coupling** of the drive to mode k. A perfectly centered vertical push
  only couples to modes that look the same after a 90° rotation and after mirroring (about 1 in 6).
  For all others φ_kᵀ f = 0. A real setup is not perfect (the bolt rocks a little, the plate is not
  exactly square), so the other modes appear too, but more weakly and only very close to their frequency.

Because the plate is square, many eigenmodes come in **pairs** with the same frequency. Every
combination of the two is also an eigenmode; the imperfections of the real plate pick one, typically
parallel lines. `chladni-plot --align-pairs` shows exactly these.

### The three program modes

| Mode         | What it solves | Output |
|--------------|----------------|--------|
| `eigenmodes` | K u = ω² M u, with u = 0 on the bolt (`clamped`) or without any condition (`free`, the 6 rigid body motions with f = 0 are left out). Returns the N modes whose frequency f = ω/2π is closest to `Frequency` (SLEPc shift-and-invert around σ = (2π f)²). | `mode_XX_<f>Hz.vtu`, `mode_XX_<f>Hz_top.csv`, `eigenfrequencies.csv` |
| `forced`     | The experiment: the shaker oscillates as cos(ωt) and we solve for the steady amplitude field û. `clamped`: (K − ω² M) û = 0 and û = (0, 0, A) on the bolt. `free`: (K − ω² M) û = f, where f pushes the bolt upwards with total force F. This has a solution for every frequency that is not exactly an eigenfrequency (and, for `free`, f > 0). | `forced_<f>Hz.vtu`, `forced_<f>Hz_top.csv` |
| `sweep`      | `forced` for many frequencies. It records the response amplitude G(f) = RMS(û_z) / A (`clamped`, dimensionless) or RMS(û_z) / F (`free`, in m/N), with RMS(û_z) = ‖û_z‖_L² / \|Ω\|^½. The peaks of G are the resonances. | `sweep.csv` |

Every run also copies its parameter file into the output directory, so each result can be reproduced.

## Requirements

deal.II ≥ 9.8 configured **with PETSc and SLEPc** (needed for the eigenmode solver). On Ubuntu 24.04:

```bash
sudo apt install petsc-dev slepc-dev
cd <deal.II build dir>
cmake -DDEAL_II_WITH_PETSC=ON -DPETSC_DIR=/usr/lib/petsc \
      -DDEAL_II_WITH_SLEPC=ON -DSLEPC_DIR=/usr/lib/slepc .
make -j 24 && sudo make install
```

## Other ways to look at the results

- **Single mode in 3D:** `uv run chladni-3d <file>_top.csv --animate` (from `python/`) shows a
  rotatable, oscillating plate. See [`python/README.md`](python/README.md).
- **ParaView:** open a `.vtu` file, color by `abs_uz` (= |u_z|) or `von_mises` and look from the top
  (+z). Dark lines (|u_z| ≈ 0) are the nodal lines. The *Warp By Vector* filter with `displacement`
  shows the plate deform.
- **Forced vibration / resonance curve:** set `Mode = forced` (one frequency) or `Mode = sweep`
  (resonance curve in `sweep.csv`) in the parameter file. The `forced_<f>Hz_top.csv` files can be
  plotted with `chladni-plot` like the modes.

## Parameters

All parameters live in a deal.II `.prm` file (see `cpp/parameters/default.prm`). It is plain text
(`set Key = value` inside `subsection ... end`), so scripts can easily generate it. Entries that are
left out keep their default.

| Section | Entry | Default | Meaning |
|---------|-------|---------|---------|
| Geometry | Side length | 0.24 | edge length L [m] |
| | Thickness | 0.001 | thickness h [m] |
| | Bolt radius | 0.005 | radius r of the central bolt [m] |
| Material | Young's modulus | 70e9 | E [Pa] (aluminium) |
| | Poisson ratio | 0.33 | ν [-] |
| | Density | 2700 | ρ [kg/m³] |
| Discretization | Cells per side | 32 | cells along x and y (even) |
| | Cells through thickness | 1 | cells along z |
| | Polynomial degree | 2 | degree of the Q_p elements (≥ 2, Q1 locks for thin plates) |
| Simulation | Mode | eigenmodes | `eigenmodes`, `forced` or `sweep` |
| | Center support | clamped | `clamped` (displacement drive) or `free` (force drive) |
| | Frequency | 500 | target (eigenmodes) or drive (forced) frequency [Hz] |
| | Number of eigenmodes | 6 | how many modes around `Frequency` |
| | Drive amplitude | 1e-6 | amplitude A of the shaken bolt [m] (`clamped`) |
| | Drive force | 0.01 | force amplitude F on the bolt [N] (`free`) |
| | Sweep min/max frequency | 10 / 1000 | frequency range of the sweep [Hz] |
| | Sweep steps | 200 | number of frequencies in the sweep |
| Output | Output directory | output | where results are written, relative to `cpp/build` (where `chladni` is run) |

## Website (`site/`)

A static website (German) for the audience of the talk: the HTML animations from
`presentation/animations/`, the slides as a PDF download and the Chladni game from `game/` (plain
JavaScript, no build step; served under `spiel/`). It is hosted on GitHub Pages at
<https://mauromorini.github.io/chladni-plate-simulation/>: the workflow
`.github/workflows/pages.yml` builds and deploys it on every push to `main` (or manually from the
Actions tab). Other branches are not deployed; preview them locally:

```bash
bash site/build.sh                         # assembles the site into _site/ (ignored by git)
python3 -m http.server -d _site 8000       # local preview at http://localhost:8000
```

`build.sh` only copies files: the animations, the game, `presentation/slides.pdf` and the pictures. The slides are **not** compiled in CI (no Typst, no Inter font there), so run
`typst compile slides.typ` in `presentation/` and commit `slides.pdf` before pushing slide changes.

One-time setup: the repository must be public (GitHub Pages is not free for private repositories)
and under Settings → Pages the source must be set to "GitHub Actions".

## Code structure (`cpp/`)

| File | Content |
|------|---------|
| `include/chladni/parameters.h` | `Parameters`: reads the `.prm` file and computes the Lamé constants |
| `include/chladni/plate.h` | `Plate`: mesh, Q_p³ finite elements, assembly of K and M, bolt DoFs |
| `include/chladni/eigenmode_solver.h` | `EigenmodeSolver`: clamped or free plate, SLEPc Krylov–Schur + shift-and-invert (MUMPS) |
| `include/chladni/forced_response_solver.h` | `ForcedResponseSolver`: bolt shaken (clamped) or pushed (free), UMFPACK solve per frequency, G(f), sweep |
| `include/chladni/output.h` | `.vtu` output (with \|u_z\| and von Mises stress), top-face grid (u_z, stress) and `.csv` tables |
| `include/chladni/stress.h` | Hooke's law σ = λ tr(ε) I + 2μ ε and the von Mises stress |
| `app/main.cc` | reads the parameters and runs the selected mode |
| `tests/` | CTest executables (see below) |

## Tests

Run them with `ctest --test-dir cpp/build --output-on-failure` (C++) and `cd python && uv run pytest` (Python).

| Test | Checks |
|------|--------|
| `test_parameters` | `.prm` parsing (incl. center support and drive force), defaults, Lamé constants, invalid input rejected |
| `test_assembly` | K annihilates all 6 rigid body motions, M integrates the total mass ρL²h |
| `test_stress` | von Mises stress for uniaxial tension (= σ_xx) and pure shear (= √3 τ), no stress for a rigid rotation |
| `test_bolt_dofs` | bolt DoFs lie inside the bolt radius, expected count, tiny bolt falls back to the center axis |
| `test_eigenmodes` | SLEPc frequencies match a dense LAPACK solve (lowest and around a target) for the clamped and the free plate, the free plate's 6 rigid body motions are left out, square symmetry gives equal-frequency pairs, modes vanish on the clamped bolt |
| `test_forced_response` | clamped: G(f → 0) = 1 (rigid motion); free: G(f → 0) = 1 / (m ω²) (the force accelerates the whole plate); both: response grows without bound towards an excited eigenfrequency; sweep bookkeeping |
