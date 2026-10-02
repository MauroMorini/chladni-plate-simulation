# Building deal.II from source

This guide builds deal.II with everything the Chladni simulation needs (PETSc, SLEPc, UMFPACK, LAPACK).
If you only want to run the simulation, the Docker image described in the main [README](../README.md#requirements)
is much faster. Build from source if you want a native install or a debug build of deal.II.

The commands below are for Ubuntu 24.04. Other distributions need the same packages under their own names.

## 1. Install the dependencies

```bash
sudo apt update
sudo apt install build-essential cmake git \
     libopenmpi-dev openmpi-bin \
     liblapack-dev libblas-dev \
     libsuitesparse-dev \
     libboost-all-dev libtbb-dev zlib1g-dev \
     petsc-dev slepc-dev libmumps-dev
```

| Package | Why it is needed |
|---------|------------------|
| `build-essential`, `cmake`, `git` | compiler and build system |
| `libopenmpi-dev`, `openmpi-bin` | MPI, required by PETSc and SLEPc |
| `liblapack-dev`, `libblas-dev` | dense linear algebra (`LAPACKFullMatrix`, used in the tests) |
| `libsuitesparse-dev` | UMFPACK, the direct solver for the forced response |
| `libboost-all-dev`, `libtbb-dev`, `zlib1g-dev` | general deal.II dependencies (`.vtu` output needs zlib) |
| `petsc-dev`, `libmumps-dev` | PETSc matrices/vectors and the MUMPS direct solver used for shift-and-invert |
| `slepc-dev` | SLEPc, the eigenvalue solver |

The Ubuntu packages install PETSc to `/usr/lib/petsc` and SLEPc to `/usr/lib/slepc`.

## 2. Get the source

The project needs deal.II ≥ 9.8. Until 9.8 is released, use the `master` branch:

```bash
mkdir -p ~/tools && cd ~/tools
git clone --depth 1 https://github.com/dealii/dealii.git dealii
```

## 3. Configure

```bash
cd ~/tools/dealii
cmake -S . -B build \
      -DCMAKE_BUILD_TYPE=DebugRelease \
      -DCMAKE_INSTALL_PREFIX=/usr/local/deal.II \
      -DDEAL_II_WITH_MPI=ON \
      -DDEAL_II_WITH_PETSC=ON -DPETSC_DIR=/usr/lib/petsc \
      -DDEAL_II_WITH_SLEPC=ON -DSLEPC_DIR=/usr/lib/slepc \
      -DDEAL_II_WITH_UMFPACK=ON \
      -DDEAL_II_WITH_LAPACK=ON \
      -DDEAL_II_COMPONENT_EXAMPLES=OFF
```

`DebugRelease` builds both the debug library (with assertions, good for development) and the optimized
one. Use `Release` to halve the build time. `/usr/local/deal.II` is the default location where the
project's `CMakeLists.txt` looks for deal.II.

At the end of the output, CMake prints a summary. Check that it contains

```
DEAL_II_WITH_PETSC set up with external dependencies
DEAL_II_WITH_SLEPC set up with external dependencies
DEAL_II_WITH_UMFPACK set up with external dependencies
```

If PETSc or SLEPc is missing there, the `*-dev` packages from step 1 were not found.

## 4. Build and install

```bash
cmake --build build -j$(nproc)
sudo cmake --install build
```

This takes a while (30 min to over an hour, depending on the machine and build type). With little RAM,
lower the number of parallel jobs (about 2 GB per job).

## 5. Use it

```bash
cd <this repo>/cpp
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

If you installed somewhere other than `/usr/local/deal.II`, add `-DDEAL_II_DIR=<install prefix>` or set
the `DEAL_II_DIR` environment variable.
