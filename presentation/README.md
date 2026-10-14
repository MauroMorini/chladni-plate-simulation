# Presentation

Talk for high school students (German, ~30–40 min): waves and the wave equation, computer
simulation, and Chladni figures, followed by the hands-on experiment with the speakers.

```
slides.typ               the slides (Typst + Touying, downloaded automatically on first compile)
arbeitsblatt.typ         worksheet for the group work after the talk (typst compile arbeitsblatt.typ)
figures/make_figures.py  creates all plots in figures/
animations/              small HTML animations (open in a browser, no internet needed), linked from the slides
images/                  photos taken from old_presentations/TecDay2025.pdf
old_presentations/       earlier talks
```

## Build

```bash
typst compile slides.typ      # -> slides.pdf
typst watch slides.typ        # live preview while editing
```

## Regenerating the figures

The Chladni figures come from the deal.II simulation of the free plate in `../cpp`. Its parameter
file is `cpp/parameters/eigenmodes_free.prm` and its results go to `cpp/build/output/eigenmodes_free/`
(ignored by git):

```bash
(cd ../cpp/build && ./chladni ../parameters/eigenmodes_free.prm)   # 60 lowest eigenmodes, ~15 s
uv run --project ../python python figures/make_figures.py
```

The generated figures in `figures/` are committed, so the slides build without re-running the
simulation. Overview plots of the eigenmodes (e.g. `first24_free.pdf`, see the main README) are
also written to `cpp/build/output/` and can be referenced from there.

Speaker notes are in the `#speaker-note[...]` blocks of `slides.typ`.

## Animations

The red "▶ Animation" buttons on the slides link to the files in `animations/`. Whether a click opens
the browser depends on the PDF viewer; otherwise open the file directly in a browser.

| File | Slide | Controls |
|------|-------|----------|
| `ball.html` | Ableitungen und Bewegung | Space: pause, R: restart, T: tangent, ←/→: step |
| `raumzeit.html` | Partielle Ableitungen, Eigenschwingungen (`?modus=eigen`) | click into the map: choose x₀, Space: pause |
| `stehende_welle.html` | Stehende Wellen | ↑/↓: n, checkbox: non-integer n (string not fixed at x = L) |
