# Presentation

Talk for high school students (German, ~30–40 min): waves and the wave equation, computer
simulation, and Chladni figures, followed by the hands-on experiment with the speakers.

```
slides.typ               the slides (Typst + Touying, downloaded automatically on first compile)
figures/make_figures.py  creates all plots in figures/
simulation/*.prm         deal.II run used for the Chladni figures
images/                  photos taken from old_presentations/TecDay2025.pdf
old_presentations/       earlier talks
```

## Build

```bash
typst compile slides.typ      # -> slides.pdf
typst watch slides.typ        # live preview while editing
```

## Regenerating the figures

```bash
cd simulation
../../cpp/build/chladni eigenmodes.prm   # 40 lowest eigenmodes, ~10 s
cd ..
uv run --project ../python python figures/make_figures.py
```

The simulation output goes to `simulation/output/` (ignored by git). The generated figures are
committed, so the slides build without re-running the simulation.

Speaker notes are in the `#speaker-note[...]` blocks of `slides.typ`.
