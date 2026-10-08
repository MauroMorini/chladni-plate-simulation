"""Plot Chladni figures from the *_top.csv files written by the C++ simulation.

Each file contains the vertical displacement u_z and the von Mises stress on the top face of the
plate, on a regular grid. All files are shown in one figure, sorted by frequency.

--field uz (default): sand gathers where the plate does not move, i.e. where |u_z| is small.
                      These regions are drawn bright, moving regions dark.
--field stress:       heatmap of the von Mises stress, i.e. where the metal is bent the most.

Orange lines are the nodal lines u_z = 0.

--align-pairs:        modes that come in pairs with the same frequency (a pattern and its 90 degree
                      rotation) are shown as parallel (wavy) lines, as they usually appear on the
                      plate, instead of the arbitrary skewed mix returned by the eigensolver.
--symmetric-only:     only show modes that are unchanged by a 90 degree rotation and by mirroring.
                      A perfectly centered shaker excites only these, so they are the strongest
                      patterns. Imperfections (e.g. a slightly rocking bolt) also excite the others.
--first N:            only show the N lowest frequencies (after the filters), numbered 1..N.
--save FILE:          write the figure to FILE (.pdf, .png, .svg, ...) instead of opening a window.

Usage (from the python/ folder):
    uv run chladni-plot ../cpp/build/output/mode_*_top.csv
    uv run chladni-plot ../cpp/build/output/mode_*_top.csv --align-pairs --first 24 --save first24.pdf
    uv run chladni-plot ../cpp/build/output/mode_*_top.csv --symmetric-only --first 9
    uv run chladni-plot ../cpp/build/output/mode_*_top.csv --field stress
    uv run chladni-plot ../cpp/build/output/forced_500.0Hz_top.csv --save pattern.png
"""

import argparse
import dataclasses
import math

import matplotlib.pyplot as plt

from chladni_viz.io import (
    FIELDS,
    frequency_from_filename,
    load_top_surface,
    relative_field,
    sorted_by_frequency,
    title_from_filename,
)
from chladni_viz.symmetry import align_pair, is_center_symmetric, is_rotated_pair


def plot_pattern(ax, surface, title, field="uz", show_nodal_lines=True):
    x, y = surface.x, surface.y
    extent = [x.min(), x.max(), y.min(), y.max()]

    label, colormap = FIELDS[field]
    image = ax.imshow(
        relative_field(surface, field),
        origin="lower",
        extent=extent,
        cmap=colormap,
        vmin=0,
        vmax=1,
        interpolation="bilinear",
    )

    if show_nodal_lines:
        ax.contour(x, y, surface.uz, levels=[0.0], colors="tab:orange", linewidths=1.0)

    ax.set_title(title)
    ax.set_aspect("equal")
    ax.set_xticks([])
    ax.set_yticks([])
    return image, label


def align_pairs(surfaces, frequencies):
    """Replace every pair of neighbouring modes with (nearly) the same frequency that are rotated
    copies of each other by their parallel-line combinations. Returns the number of pairs."""
    n_pairs = 0
    k = 0
    while k + 1 < len(surfaces):
        f_a, f_b = frequencies[k], frequencies[k + 1]
        same_frequency = f_a is not None and f_b is not None and abs(f_a - f_b) <= 0.1 + 1e-4 * f_a
        if same_frequency and is_rotated_pair(surfaces[k].uz, surfaces[k + 1].uz):
            uz_a, uz_b = align_pair(surfaces[k].uz, surfaces[k + 1].uz)
            # The stress is not linear in u, so it cannot be recombined: drop it.
            surfaces[k] = dataclasses.replace(surfaces[k], uz=uz_a, von_mises=None)
            surfaces[k + 1] = dataclasses.replace(surfaces[k + 1], uz=uz_b, von_mises=None)
            n_pairs += 1
            k += 2
        else:
            k += 1
    return n_pairs


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("files", nargs="+", help="one or more *_top.csv files")
    parser.add_argument("--save", help="save the figure to this file (.pdf, .png, ...) instead of showing it")
    parser.add_argument("--field", choices=FIELDS, default="uz", help="what to color (default: uz)")
    parser.add_argument("--no-nodal-lines", action="store_true", help="do not draw the u_z = 0 contour")
    parser.add_argument(
        "--align-pairs",
        action="store_true",
        help="show pairs of modes with equal frequency as parallel lines (only with --field uz)",
    )
    parser.add_argument(
        "--symmetric-only",
        action="store_true",
        help="only show modes a centered shaker excites (unchanged by 90 degree rotation and mirroring)",
    )
    parser.add_argument("--first", type=int, metavar="N", help="only show the N lowest frequencies, numbered 1..N")
    parser.add_argument("--columns", type=int, help="number of columns (default: about square)")
    args = parser.parse_args()
    if args.align_pairs and args.field != "uz":
        parser.error("--align-pairs only works with --field uz (the stress of a combination is unknown)")

    files = sorted_by_frequency(args.files)
    surfaces = [load_top_surface(f) for f in files]
    frequencies = [frequency_from_filename(f) for f in files]
    titles = [title_from_filename(f) for f in files]

    if args.align_pairs:
        print(f"aligned {align_pairs(surfaces, frequencies)} pairs of modes with equal frequency")

    if args.symmetric_only:
        keep = [k for k, surface in enumerate(surfaces) if is_center_symmetric(surface.uz)]
        print(f"{len(keep)} of {len(files)} modes are excited by a centered shaker")
        surfaces, frequencies, titles = ([items[k] for k in keep] for items in (surfaces, frequencies, titles))
        if not surfaces:
            return

    if args.first:
        if len(surfaces) < args.first:
            print(f"warning: only {len(surfaces)} modes, compute more eigenmodes to get {args.first}")
        surfaces, frequencies = surfaces[: args.first], frequencies[: args.first]
        titles = [f"{k}: {f} Hz" for k, f in enumerate(frequencies, start=1)]

    n = len(surfaces)
    n_columns = args.columns or min(n, max(3, math.ceil(math.sqrt(n))))
    n_rows = math.ceil(n / n_columns)
    fig, axes = plt.subplots(
        n_rows, n_columns, figsize=(3 * n_columns + 1, 3 * n_rows), squeeze=False, layout="constrained"
    )

    for ax, surface, title in zip(axes.flat, surfaces, titles):
        image, label = plot_pattern(ax, surface, title, args.field, show_nodal_lines=not args.no_nodal_lines)
    for ax in axes.flat[n:]:
        ax.axis("off")

    fig.colorbar(image, ax=axes, label=label, shrink=0.6)
    if args.save:
        fig.savefig(args.save, dpi=150)
        print(f"saved {args.save}")
    else:
        plt.show()


if __name__ == "__main__":
    main()
