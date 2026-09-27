"""Rotatable 3D view of a vibrating plate from a *_top.csv file written by the C++ simulation.

The top face of the plate is drawn deformed by the vertical displacement u_z. The deformation is
exaggerated (real amplitudes are micrometers) so that the shape is visible. Drag with the mouse
to rotate, use the right mouse button to zoom.

--field uz (default): color by the signed displacement, red = up, blue = down, white = at rest.
--field stress:       color by the von Mises stress.
--animate:            let the plate oscillate, u(x, t) = u_hat(x) cos(omega t).

Usage (from the python/ folder):
    uv run chladni-3d ../cpp/build/output/mode_03_506.1Hz_top.csv
    uv run chladni-3d ../cpp/build/output/mode_03_506.1Hz_top.csv --field stress --animate
"""

import argparse

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.animation import FuncAnimation

from chladni_viz.io import FIELDS, load_top_surface, relative_field, title_from_filename


def surface_colors(surface, field):
    """RGBA color for every grid point."""
    if field == "uz":
        signed = surface.uz / np.abs(surface.uz).max()  # in [-1, 1]
        return plt.get_cmap("coolwarm")((signed + 1) / 2)
    return plt.get_cmap(FIELDS[field][1])(relative_field(surface, field))


def draw_plate(ax, surface, colors, height_scale, phase=0.0):
    """Draw the top face at the oscillation phase omega t = @p phase."""
    ax.clear()
    ax.plot_surface(
        surface.x,
        surface.y,
        height_scale * surface.uz * np.cos(phase),
        facecolors=colors,
        rstride=1,
        cstride=1,
        linewidth=0,
        antialiased=False,
        shade=False,
    )
    largest_deflection = height_scale * np.abs(surface.uz).max()
    ax.set_zlim(-1.2 * largest_deflection, 1.2 * largest_deflection)
    ax.set_box_aspect((1, 1, 0.35), zoom=1.3)
    ax.set_axis_off()


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("file", help="a *_top.csv file")
    parser.add_argument("--field", choices=FIELDS, default="uz", help="what to color (default: uz)")
    parser.add_argument(
        "--exaggeration",
        type=float,
        default=0.15,
        help="largest deflection shown, as a fraction of the side length (default: 0.15)",
    )
    parser.add_argument("--animate", action="store_true", help="let the plate oscillate")
    parser.add_argument("--save", help="save a still image (or .gif with --animate) instead of showing it")
    args = parser.parse_args()

    surface = load_top_surface(args.file)
    colors = surface_colors(surface, args.field)

    side = surface.x.max() - surface.x.min()
    height_scale = args.exaggeration * side / np.abs(surface.uz).max()

    fig = plt.figure(figsize=(8, 7))
    ax = fig.add_subplot(projection="3d")
    fig.suptitle(f"{title_from_filename(args.file)}  ({args.field}, deformation exaggerated)")

    if args.animate:
        # The camera angle is kept between frames, so the plate can be rotated while it moves.
        def update(frame):
            elevation, azimuth = ax.elev, ax.azim
            draw_plate(ax, surface, colors, height_scale, phase=2 * np.pi * frame / 30)
            ax.view_init(elevation, azimuth)

        ax.view_init(elev=35, azim=-60)
        animation = FuncAnimation(fig, update, frames=30, interval=50)
    else:
        draw_plate(ax, surface, colors, height_scale)
        ax.view_init(elev=35, azim=-60)

    if args.save:
        if args.animate:
            animation.save(args.save, writer="pillow", fps=20)
        else:
            fig.savefig(args.save, dpi=150)
        print(f"saved {args.save}")
    else:
        plt.show()


if __name__ == "__main__":
    main()
