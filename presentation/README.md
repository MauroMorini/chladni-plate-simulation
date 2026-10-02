# Presentation

`chladni.tex`: Beamer slides (metropolis theme) for high school students. They derive the
eigenvalue problem from Newton's law and explain the boundary conditions, in particular why the
shaken bolt is modelled as clamped (u = 0) in the eigenvalue problem.

Build (from this folder):

    latexmk -pdf -outdir=build chladni.tex

`figures/patterns.png` was made with the Python tools (from `python/`):

    uv run chladni-plot ../cpp/build/output/mode_00_*_top.csv ../cpp/build/output/mode_03_*_top.csv \
        ../cpp/build/output/mode_05_*_top.csv --save ../presentation/figures/patterns.png
