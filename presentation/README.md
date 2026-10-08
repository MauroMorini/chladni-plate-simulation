# Presentation

Talk for high school students (German, ~30–40 min): waves and the wave equation, computer
simulation, and Chladni figures, followed by the hands-on experiment with the speakers.

```
slides.typ               the slides (Typst + Touying, downloaded automatically on first compile)
figures/make_figures.py  creates all plots in figures/
images/                  photos taken from old_presentations/TecDay2025.pdf
old_presentations/       earlier talks
```

## Build

```bash
typst compile slides.typ      # -> slides.pdf
typst watch slides.typ        # live preview while editing
```

## Regenerating the figures

The Chladni figures come from the deal.II simulation in `../cpp`. Its parameter file is
`cpp/parameters/eigenmodes.prm` and its results go to `cpp/build/output/eigenmodes/` (ignored by git):

```bash
(cd ../cpp/build && ./chladni ../parameters/eigenmodes.prm)   # 40 lowest eigenmodes, ~10 s
uv run --project ../python python figures/make_figures.py
```

The generated figures in `figures/` are committed, so the slides build without re-running the
simulation. Overview plots of the eigenmodes (e.g. `first24_free.pdf`, see the main README) are
also written to `cpp/build/output/` and can be referenced from there.

Speaker notes are in the `#speaker-note[...]` blocks of `slides.typ`.
