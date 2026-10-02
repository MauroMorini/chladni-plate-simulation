# chladni-plate-simulation

Simulation of the [Chladni plate](https://en.wikipedia.org/wiki/Ernst_Chladni#Chladni_figures)
experiment with [deal.II](https://www.dealii.org). A thin square metal plate is held by a bolt at its
center and shaken up and down. At certain frequencies sand on the plate gathers along the lines that
do not move, the *nodal lines*, and forms the famous Chladni figures.

```
cpp/            C++ / deal.II simulation code (main part)
python/         Python visualization package (uv, numpy + matplotlib)
presentation/   Slides (planned)
docs/           Additional guides (building deal.II from source)
```

## Physics

The plate is modeled as a 3D linear elastic body Ω = [-L/2, L/2]² × [0, h]:

- **Material:** isotropic, with Young's modulus E, Poisson ratio ν and density ρ. The Lamé parameters are
  λ = Eν / ((1+ν)(1−2ν)) and μ = E / (2(1+ν)).
- **Boundary conditions:** every outer face is traction free (zero Neumann).
- **The bolt:** the plate is fixed through a small cylinder of radius r around the vertical center axis. We don't
  fix just a single point: a point Dirichlet condition is not well defined for H¹ functions in 3D, so the result
  would depend on the mesh.

After finite element discretization we get the stiffness matrix K and mass matrix M:

    K_ij = ∫ 2μ ε(φ_i):ε(φ_j) + λ div φ_i div φ_j dx
    M_ij = ∫ ρ φ_i·φ_j dx

The program has three modes:

| Mode         | What it solves | Output |
|--------------|----------------|--------|
| `eigenmodes` | K u = ω² M u with u = 0 on the bolt. Returns the N modes whose frequency f = ω/2π is closest to `Frequency` (SLEPc shift-and-invert around σ = (2π f)²). | `mode_XX_<f>Hz.vtu`, `mode_XX_<f>Hz_top.csv`, `eigenfrequencies.csv` |
| `forced`     | The experiment: the bolt is shaken as A cos(ωt), and we solve for the steady amplitude field û with (K − ω² M) û = 0 and û = (0, 0, A) on the bolt. This has a solution for every frequency that is not exactly an eigenfrequency. | `forced_<f>Hz.vtu`, `forced_<f>Hz_top.csv` |
| `sweep`      | `forced` for many frequencies. It records the response amplitude G(f) = RMS(û_z) / A = ‖û_z‖_L² / (A \|Ω\|^½). G = 1 means the plate moves as much as the bolt, and the peaks of G are the resonances. | `sweep.csv` |

Near an eigenfrequency the forced response looks like the corresponding eigenmode. That is why a
Chladni figure appears when you slowly turn the frequency knob. A shaker at the center can only
excite modes that the center actually pushes, so some eigenmodes (e.g. ones that are antisymmetric in x)
have no peak in the sweep.

## Requirements

deal.II ≥ 9.8 configured **with PETSc and SLEPc** (needed for the eigenmode solver).

The easiest way to get it is the official deal.II Docker image, which already contains PETSc and SLEPc.
deal.II 9.8 is not released yet, so use the `master` image. From the repository root:

```bash
docker run --rm -it -v "$PWD":/home/dealii/chladni -w /home/dealii/chladni dealii/dealii:master-noble
```

This opens a shell with deal.II installed and the repository mounted, so you can run the commands
from [Build, test, run](#build-test-run) directly. Output files are written into your local checkout.

To install deal.II natively instead, see [docs/building-dealii.md](docs/building-dealii.md).

## Build, test, run

```bash
cd cpp
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release   # add -DDEAL_II_DIR=... if deal.II is not in /usr/local/deal.II
cmake --build build -j
ctest --test-dir build --output-on-failure
cd build && ./chladni ../parameters/default.prm
```

The results go to the `Output directory` (default `output/`), together with a copy of the
parameter file that was used.

### Viewing the Chladni figure

The quickest way is the Python package in `python/`, which is managed with [uv](https://docs.astral.sh/uv/).
It reads the `*_top.csv` files, which hold u_z on the top face as a regular grid:

```bash
cd python
uv run chladni-plot ../cpp/build/output/mode_*_top.csv
```

Bright regions are at rest (that's where the sand goes), dark regions vibrate, and the orange lines
are the nodal lines u_z = 0. `--field stress` shows a von Mises stress heatmap instead, and
`uv run chladni-3d <file> --animate` shows a rotatable, oscillating 3D view. See `python/README.md`
for more.

For the full 3D fields, open a `.vtu` file in ParaView, color by `abs_uz` (= |u_z|) or `von_mises` (von Mises stress) and look at the plate from the top (+z).
The dark lines, where |u_z| ≈ 0, are the nodal lines, which is where the sand would collect. The
`displacement` field can be used with the *Warp By Vector* filter to see the plate deform.

## Parameters

All parameters live in a deal.II `.prm` file (see `cpp/parameters/default.prm`). It is plain text
(`set Key = value` inside `subsection ... end`), so scripts can easily generate it.

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
| | Frequency | 500 | target (eigenmodes) or drive (forced) frequency [Hz] |
| | Number of eigenmodes | 6 | how many modes around `Frequency` |
| | Drive amplitude | 1e-6 | amplitude A of the shaken bolt [m] |
| | Sweep min/max frequency | 10 / 1000 | frequency range of the sweep [Hz] |
| | Sweep steps | 200 | number of frequencies in the sweep |
| Output | Output directory | output | where results are written |

## Code structure (`cpp/`)

| File | Content |
|------|---------|
| `include/chladni/parameters.h` | `Parameters`: reads the `.prm` file and computes the Lamé constants |
| `include/chladni/plate.h` | `Plate`: mesh, Q_p³ finite elements, assembly of K and M, bolt DoFs |
| `include/chladni/eigenmode_solver.h` | `EigenmodeSolver`: clamped bolt, SLEPc Krylov–Schur + shift-and-invert (MUMPS) |
| `include/chladni/forced_response_solver.h` | `ForcedResponseSolver`: shaken bolt, UMFPACK solve per frequency, G(f), sweep |
| `include/chladni/output.h` | `.vtu` output (with \|u_z\| and von Mises stress), top-face grid (u_z, stress) and `.csv` tables |
| `include/chladni/stress.h` | Hooke's law σ = λ tr(ε) I + 2μ ε and the von Mises stress |
| `app/main.cc` | reads the parameters and runs the selected mode |
| `tests/` | CTest executables (see below) |

## Tests

| Test | Checks |
|------|--------|
| `test_parameters` | `.prm` parsing, defaults, Lamé constants, invalid input rejected |
| `test_assembly` | K annihilates all 6 rigid body motions, M integrates the total mass ρL²h |
| `test_stress` | von Mises stress for uniaxial tension (= σ_xx) and pure shear (= √3 τ), no stress for a rigid rotation |
| `test_bolt_dofs` | bolt DoFs lie inside the bolt radius, expected count, tiny bolt falls back to the center axis |
| `test_eigenmodes` | SLEPc frequencies match a dense LAPACK solve (lowest and around a target), square symmetry gives equal-frequency pairs, modes vanish on the bolt |
| `test_forced_response` | G(f → 0) = 1 (rigid motion), response grows without bound towards an excited eigenfrequency, sweep bookkeeping |
