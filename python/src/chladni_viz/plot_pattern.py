"""Plot Chladni figures from the *_top.csv files written by the C++ simulation.

Each file contains the vertical displacement u_z and the von Mises stress on the top face of the
plate, on a regular grid.

--field uz (default): sand gathers where the plate does not move, i.e. where |u_z| is small.
                      These regions are drawn bright, moving regions dark.
--field stress:       heatmap of the von Mises stress, i.e. where the metal is bent the most.

Orange lines are the nodal lines u_z = 0.

Usage (from the python/ folder):
    uv run chladni-plot ../cpp/build/output/mode_*_top.csv
    uv run chladni-plot ../cpp/build/output/mode_*_top.csv --field stress
    uv run chladni-plot ../cpp/build/output/forced_500.0Hz_top.csv --save pattern.png
"""

import argparse
import math

import matplotlib.pyplot as plt
import numpy as np

from chladni_viz.io import FIELDS, load_top_surface, relative_field, title_from_filename


def plot_pattern(ax, filename, field="uz", show_nodal_lines=True):
    surface = load_top_surface(filename)
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

    ax.set_title(title_from_filename(filename))
    ax.set_aspect("equal")
    ax.set_xticks([])
    ax.set_yticks([])
    return image, label


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("files", nargs="+", help="one or more *_top.csv files")
    parser.add_argument("--save", help="save the figure to this file instead of showing it")
    parser.add_argument("--field", choices=FIELDS, default="uz", help="what to color (default: uz)")
    parser.add_argument("--no-nodal-lines", action="store_true", help="do not draw the u_z = 0 contour")
    args = parser.parse_args()

    files = sorted(args.files)
    n_columns = min(len(files), 3)
    n_rows = math.ceil(len(files) / n_columns)
    fig, axes = plt.subplots(
        n_rows, n_columns, figsize=(4 * n_columns + 1, 4 * n_rows), squeeze=False, layout="constrained"
    )

    for ax, filename in zip(axes.flat, files):
        image, label = plot_pattern(ax, filename, args.field, show_nodal_lines=not args.no_nodal_lines)
    for ax in axes.flat[len(files):]:
        ax.axis("off")

    fig.colorbar(image, ax=axes, label=label, shrink=0.8)
    if args.save:
        fig.savefig(args.save, dpi=150)
        print(f"saved {args.save}")
    else:
        plt.show()


if __name__ == "__main__":
    main()
