"""Create the figures for the slides (slides.typ).

Most figures are small 1D toy computations done right here with numpy. The Chladni figures come
from the deal.II simulation of the free plate, run with cpp/parameters/eigenmodes_free.prm. Its
results are read from cpp/build/output/eigenmodes_free/.

Run from the presentation/ folder:
    (cd ../cpp/build && ./chladni ../parameters/eigenmodes_free.prm)
    uv run --project ../python python figures/make_figures.py
"""

import dataclasses
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

from chladni_viz.io import load_top_surface, sorted_by_frequency, frequency_from_filename
from chladni_viz.plot_pattern import align_pairs
from chladni_viz.view3d import surface_colors

HERE = Path(__file__).parent
SIMULATION = HERE.parent.parent / "cpp" / "build" / "output"

# Colors, matching slides.typ
DARK = "#2D373C"
MINT = "#A5D7D2"
RED = "#D20537"
BLUE = "#1F6F8B"
GRAY = "#9AA5AA"

plt.rcParams.update(
    {
        "font.family": "Inter",
        "font.size": 13,
        "mathtext.fontset": "cm",
        "axes.edgecolor": DARK,
        "axes.labelcolor": DARK,
        "xtick.color": DARK,
        "ytick.color": DARK,
        "text.color": DARK,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "lines.linewidth": 2.5,
        "savefig.transparent": True,
    }
)


def save(fig, name):
    fig.savefig(HERE / name, bbox_inches="tight")
    plt.close(fig)
    print(f"saved figures/{name}")


def bare_axes(ax):
    """Axes without ticks, only the two arrows-like spines."""
    ax.set_xticks([])
    ax.set_yticks([])


# ---------------------------------------------------------------------------------------------
# Part 1: derivatives and the wave equation
# ---------------------------------------------------------------------------------------------


def derivative_slope():
    """First derivative = slope of the tangent."""
    x = np.linspace(0, 6, 400)
    u = lambda x: 1.5 + np.sin(x) + 0.15 * x
    du = lambda x: np.cos(x) + 0.15
    x0 = 1.0

    fig, ax = plt.subplots(figsize=(6.5, 3.6))
    ax.plot(x, u(x), color=DARK)
    tangent = u(x0) + du(x0) * (x - x0)
    mask = np.abs(x - x0) < 1.6
    ax.plot(x[mask], tangent[mask], color=RED, lw=2)
    # slope triangle
    dx = 0.8
    ax.plot([x0, x0 + dx, x0 + dx], [u(x0), u(x0), u(x0) + du(x0) * dx], color=RED, lw=1.2, ls="--")
    ax.text(x0 + dx / 2, u(x0) - 0.25, "1", ha="center", color=RED)
    ax.text(x0 + dx + 0.12, u(x0) + du(x0) * dx / 2 - 0.15, r"$u'(x_0)$", va="center", color=RED,
            bbox=dict(facecolor="white", edgecolor="none", pad=1))
    ax.plot([x0], [u(x0)], "o", color=RED, ms=8)
    ax.text(x0 + 1.3, u(x0) + du(x0) * 1.3 + 0.15, "Tangente", color=RED)
    ax.set_xlabel("$x$")
    ax.set_ylabel("$u(x)$")
    ax.set_xticks([x0], ["$x_0$"])
    ax.set_yticks([])
    ax.set_ylim(0, 3.6)
    save(fig, "derivative_slope.svg")


def derivative_motion():
    """Position, velocity, acceleration of a ball thrown upwards."""
    t = np.linspace(0, 2, 200)
    v0, g = 9.81, 9.81
    position = v0 * t - g * t**2 / 2
    velocity = v0 - g * t
    acceleration = -g * np.ones_like(t)

    fig, axes = plt.subplots(1, 3, figsize=(11, 3.0), layout="constrained")
    data = [
        (position, "Höhe  $h(t)$", "m"),
        (velocity, "Geschwindigkeit  $h'(t)$", "m/s"),
        (acceleration, "Beschleunigung  $h''(t)$", "m/s²"),
    ]
    for ax, (values, title, unit), color in zip(axes, data, [DARK, BLUE, RED]):
        ax.plot(t, values, color=color)
        ax.axhline(0, color=GRAY, lw=0.8)
        ax.set_title(title, fontsize=13)
        ax.set_xlabel("Zeit $t$ [s]")
        ax.set_ylabel(unit)
        ax.set_ylim(-11, 11)
    save(fig, "derivative_motion.svg")


def curvature():
    """Second derivative = curvature: sign tells in which direction the curve bends."""
    x = np.linspace(0, 2 * np.pi, 600)
    u = np.sin(x)
    second = -np.sin(x)

    fig, ax = plt.subplots(figsize=(7.5, 3.4))
    for mask, color in [(second < 0, RED), (second >= 0, BLUE)]:
        ax.plot(np.where(mask, x, np.nan), np.where(mask, u, np.nan), color=color, lw=3.5)
    ax.axhline(0, color=GRAY, lw=0.8)
    ax.annotate("", xy=(np.pi / 2, 0.55), xytext=(np.pi / 2, 0.95), arrowprops=dict(arrowstyle="-|>", color=RED, lw=2))
    ax.annotate("", xy=(3 * np.pi / 2, -0.55), xytext=(3 * np.pi / 2, -0.95), arrowprops=dict(arrowstyle="-|>", color=BLUE, lw=2))
    ax.text(np.pi / 2, 1.12, r"$u'' < 0$: Kurve biegt nach unten", ha="center", color=RED)
    ax.text(3 * np.pi / 2, -1.3, r"$u'' > 0$: Kurve biegt nach oben", ha="center", color=BLUE)
    ax.set_ylim(-1.5, 1.4)
    ax.set_xlabel("$x$")
    ax.set_ylabel("$u(x)$")
    bare_axes(ax)
    save(fig, "curvature.svg")


def string_shape(x, t):
    """A small superposition of two standing waves on [0, 1], used for the u(x, t) figures."""
    return 0.8 * np.sin(np.pi * x) * np.cos(np.pi * t) + 0.3 * np.sin(3 * np.pi * x) * np.cos(3 * np.pi * t + 0.6)


def partial_derivatives():
    """u(x, t): a photo at fixed t versus one point followed over time."""
    x = np.linspace(0, 1, 300)
    t = np.linspace(0, 4, 600)
    t0, x0 = 0.25, 0.3

    fig, (left, right) = plt.subplots(1, 2, figsize=(11, 3.4), layout="constrained")
    left.plot(x, string_shape(x, t0), color=DARK)
    left.plot([x0], [string_shape(x0, t0)], "o", color=RED, ms=8)
    left.axhline(0, color=GRAY, lw=0.8)
    left.set_title(r"Foto: feste Zeit $t_0$, Ort $x$ variiert", fontsize=13)
    left.set_xlabel("Ort $x$")
    left.set_ylabel("$u(x, t_0)$")

    right.plot(t, string_shape(x0, t), color=RED)
    right.axhline(0, color=GRAY, lw=0.8)
    right.set_title(r"Ein Punkt $x_0$ im Lauf der Zeit", fontsize=13)
    right.set_xlabel("Zeit $t$")
    right.set_ylabel("$u(x_0, t)$")
    for ax in (left, right):
        ax.set_ylim(-1.2, 1.2)
        ax.set_yticks([])
        ax.set_xticks([])
    save(fig, "partial_derivatives.svg")


def space_time():
    """u(x, t) as a map over the (x, t) plane: a row is a photo, a column is one point over time."""
    x = np.linspace(0, 1, 300)
    t = np.linspace(0, 4, 400)
    t0, x0 = 0.25, 0.3
    values = string_shape(x[None, :], t[:, None])

    colormap = matplotlib.colors.LinearSegmentedColormap.from_list("uni", [BLUE, "white", RED])
    fig, ax = plt.subplots(figsize=(6.5, 4.2))
    image = ax.imshow(values, extent=[0, 1, 0, 4], origin="lower", aspect="auto", cmap=colormap, vmin=-1.1, vmax=1.1)
    ax.axhline(t0, color=DARK, lw=2.5)
    ax.axvline(x0, color=DARK, lw=2.5, ls="--")
    ax.plot([x0], [t0], "o", color=DARK, ms=8)
    ax.text(0.98, t0 + 0.12, r"Foto ($t = t_0$ fest): $\partial u / \partial x$", ha="right", fontsize=13,
            bbox=dict(facecolor="white", edgecolor="none", alpha=0.85, pad=2))
    ax.text(x0 + 0.02, 3.75, r"Punkt ($x = x_0$ fest): $\partial u / \partial t$", fontsize=13,
            bbox=dict(facecolor="white", edgecolor="none", alpha=0.85, pad=2))
    ax.set_xlabel("Ort $x$")
    ax.set_ylabel("Zeit $t$")
    ax.set_xticks([])
    ax.set_yticks([])
    colorbar = fig.colorbar(image, ax=ax, ticks=[-1, 0, 1], shrink=0.8)
    colorbar.ax.set_yticklabels(["unten", "0", "oben"])
    colorbar.set_label("Auslenkung $u$")
    save(fig, "space_time.svg")


def wave_equation_forces():
    """Acceleration of each piece of string is proportional to the curvature u_xx."""
    x = np.linspace(0, 1, 400)
    u = np.exp(-((x - 0.5) / 0.12) ** 2)
    uxx = np.gradient(np.gradient(u, x), x)

    fig, ax = plt.subplots(figsize=(8, 3.6))
    ax.plot(x, u, color=DARK, lw=3)
    ax.axhline(0, color=GRAY, lw=0.8, ls="--")
    scale = 0.45 / np.abs(uxx).max()
    for xi in np.linspace(0.26, 0.74, 17):
        i = np.argmin(np.abs(x - xi))
        color = RED if uxx[i] < 0 else BLUE
        ax.annotate(
            "",
            xy=(x[i], u[i] + scale * uxx[i]),
            xytext=(x[i], u[i]),
            arrowprops=dict(arrowstyle="-|>", color=color, lw=1.8),
        )
    ax.text(0.5, 1.12, "stark nach unten gebogen → beschleunigt nach unten", ha="center", color=RED)
    ax.text(0.02, -0.42, "nach oben gebogen → beschleunigt nach oben", color=BLUE, fontsize=12)
    ax.set_ylim(-0.6, 1.3)
    ax.set_xlabel("Ort $x$")
    ax.set_ylabel("Auslenkung $u$")
    bare_axes(ax)
    save(fig, "wave_equation_forces.svg")


# ---------------------------------------------------------------------------------------------
# Part 2: computer simulation
# ---------------------------------------------------------------------------------------------


def discretization():
    """Continuous curve versus values on grid points."""
    x = np.linspace(0, 1, 400)
    u = lambda x: np.sin(2 * np.pi * x) * np.exp(-x) + 0.4 * x
    grid = np.linspace(0, 1, 11)

    fig, (left, right) = plt.subplots(1, 2, figsize=(11, 3.2), layout="constrained", sharey=True)
    left.plot(x, u(x), color=DARK)
    left.set_title("Kontinuum: unendlich viele Werte", fontsize=13)
    right.plot(grid, u(grid), color=GRAY, lw=1, ls="--")
    right.plot(grid, u(grid), "o", color=RED, ms=8)
    right.set_title("Computer: endlich viele Werte $u_0, u_1, \\dots, u_N$", fontsize=13)
    right.annotate("", xy=(grid[4], -0.75), xytext=(grid[5], -0.75), arrowprops=dict(arrowstyle="<->", color=DARK, lw=1.5))
    right.text((grid[4] + grid[5]) / 2, -0.68, "$h$", ha="center")
    for ax in (left, right):
        ax.set_xlabel("$x$")
        ax.set_ylim(-0.85, 1.0)
        bare_axes(ax)
    save(fig, "discretization.svg")


def secants():
    """Difference quotients approach the derivative as h gets smaller."""
    x = np.linspace(-0.2, 3.3, 400)
    u = lambda x: 0.3 + 0.4 * x**2
    du = lambda x: 0.8 * x
    x0 = 0.5

    fig, ax = plt.subplots(figsize=(7, 4.2))
    ax.plot(x, u(x), color=DARK)
    for h, alpha in [(2.5, 0.35), (1.5, 0.55), (0.75, 0.75), (0.25, 0.95)]:
        slope = (u(x0 + h) - u(x0)) / h
        line_x = np.array([x0 - 0.5, x0 + h + 0.3])
        ax.plot(line_x, u(x0) + slope * (line_x - x0), color=BLUE, lw=1.5, alpha=alpha)
        ax.plot([x0 + h], [u(x0 + h)], "o", color=BLUE, alpha=alpha, ms=6)
        if h > 1:
            ax.text(x0 + h + 0.1, u(x0 + h) - 0.15, f"$h = {h:g}$", color=BLUE, fontsize=12, alpha=max(alpha, 0.7))
    line_x = np.array([x0 - 0.5, x0 + 2.8])
    ax.plot(line_x, u(x0) + du(x0) * (line_x - x0), color=RED, lw=2)
    ax.plot([x0], [u(x0)], "o", color=RED, ms=8)
    ax.text(x0 + 2.85, u(x0) + du(x0) * 2.8, "Tangente", color=RED, va="center")
    # the slopes approach u'(x0) = 0.4
    table = "\n".join(f"$h = {h:<4g}$:  Steigung {(u(x0 + h) - u(x0)) / h:.2f}" for h in [2.5, 1.5, 0.75, 0.25, 0.01])
    ax.text(0.02, 0.97, table, transform=ax.transAxes, va="top", color=BLUE, fontsize=12)
    ax.text(0.02, 0.53, f"Tangente:  Steigung {du(x0):.2f}", transform=ax.transAxes, va="top", color=RED, fontsize=12)
    ax.set_xticks([x0], ["$x$"])
    ax.set_yticks([])
    ax.set_xlim(-0.3, 4.1)
    ax.set_ylim(0, 5)
    save(fig, "secants.svg")



def leapfrog(n_cells, length, c, dt, n_steps, initial, left_boundary=None):
    """Simulate u_tt = c^2 u_xx on [0, length] with the explicit finite difference scheme

        u_i^{n+1} = 2 u_i^n - u_i^{n-1} + (c dt / h)^2 (u_{i-1}^n - 2 u_i^n + u_{i+1}^n).

    Both ends are fixed (u = 0), unless left_boundary(t) is given. Returns x and all time levels.
    """
    x = np.linspace(0, length, n_cells + 1)
    h = x[1] - x[0]
    courant2 = (c * dt / h) ** 2
    previous = initial(x)
    current = previous.copy()  # starts at rest
    history = [current.copy()]
    for n in range(1, n_steps + 1):
        new = np.zeros_like(current)
        new[1:-1] = 2 * current[1:-1] - previous[1:-1] + courant2 * (current[:-2] - 2 * current[1:-1] + current[2:])
        if left_boundary is not None:
            new[0] = left_boundary(n * dt)
        previous, current = current, new
        history.append(current.copy())
    return x, np.array(history)



def refinement():
    """Accuracy of the driven string for different mesh sizes H."""
    length, c, end_time = 10.0, 1.0, 6.0
    drive = lambda t: np.sin(2 * np.pi * t)
    x_fine = np.linspace(0, length, 2000)
    exact = np.where(x_fine < c * end_time, np.sin(2 * np.pi * (end_time - x_fine / c)), 0.0)

    fig, axes = plt.subplots(2, 2, figsize=(11, 5.2), layout="constrained", sharex=True, sharey=True)
    for ax, h in zip(axes.flat, [1.0, 0.25, 0.1, 0.025]):
        dt = min(0.5 * h, 0.01)
        n_steps = round(end_time / dt)
        x, history = leapfrog(round(length / h), length, c, dt, n_steps, np.zeros_like, left_boundary=drive)
        ax.plot(x_fine, exact, color=MINT, lw=4, label="exakte Lösung")
        ax.plot(x, history[-1], "-o", color=RED, lw=1.2, ms=4 if h > 0.05 else 0, label="Simulation")
        ax.set_title(f"$H = {h:g}$   ({len(x)} Punkte)", fontsize=13)
        ax.set_ylim(-1.6, 1.6)
        ax.set_yticks([])
    for ax in axes[1]:
        ax.set_xlabel("Ort $x$")
    axes[0, 0].legend(loc="upper right", fontsize=11, frameon=False)
    save(fig, "refinement.svg")


def fem_guitar():
    """A triangle mesh of a guitar body, to show what a finite element mesh looks like."""
    import matplotlib.tri as mtri

    spacing = 0.09

    def body(px, py):
        """> 0 inside the body: a smooth union of a big (lower) and a small (upper) circle."""
        lower = 1 - (px**2 + py**2)
        upper = 1 - (px**2 + (py - 1.35) ** 2) / 0.78**2
        return np.log(np.exp(8 * lower) + np.exp(8 * upper)) / 8

    hole_center, hole_radius = np.array([0.0, 0.95]), 0.25

    def resample(curve, step):
        """Points with (roughly) equal distance `step` along a closed curve."""
        arc = np.concatenate([[0], np.cumsum(np.linalg.norm(np.diff(curve, axis=0), axis=1))])
        s = np.linspace(0, arc[-1], int(arc[-1] / step), endpoint=False)
        return np.column_stack([np.interp(s, arc, curve[:, 0]), np.interp(s, arc, curve[:, 1])])

    gx, gy = np.meshgrid(np.linspace(-1.2, 1.2, 400), np.linspace(-1.2, 2.3, 600))
    contour = plt.contour(gx, gy, body(gx, gy), levels=[0])
    outline = max(contour.allsegs[0], key=len)
    plt.close()
    phi = np.linspace(0, 2 * np.pi, 200)
    hole = hole_center + hole_radius * np.column_stack([np.cos(phi), np.sin(phi)])
    boundary = np.vstack([resample(outline, spacing), resample(hole, spacing * 0.8)])

    rng = np.random.default_rng(1)
    gx, gy = np.meshgrid(np.arange(-1.2, 1.2, spacing), np.arange(-1.2, 2.3, spacing * 0.87))
    gx = gx + 0.5 * spacing * (np.arange(gx.shape[0])[:, None] % 2)  # staggered rows give nicer triangles
    interior = np.column_stack([gx.ravel(), gy.ravel()]) + rng.normal(0, 0.1 * spacing, (gx.size, 2))
    distance_to_hole = np.linalg.norm(interior - hole_center, axis=1)
    interior = interior[(body(interior[:, 0], interior[:, 1]) > 0.6 * spacing) & (distance_to_hole > hole_radius + 0.6 * spacing)]
    points = np.vstack([boundary, interior])

    triangulation = mtri.Triangulation(points[:, 0], points[:, 1])
    centers = points[triangulation.triangles].mean(axis=1)
    outside = (body(centers[:, 0], centers[:, 1]) < 0) | (np.linalg.norm(centers - hole_center, axis=1) < hole_radius)
    triangulation.set_mask(outside)

    fig, ax = plt.subplots(figsize=(4, 5.5))
    ax.triplot(triangulation, color=DARK, lw=0.6)
    ax.set_aspect("equal")
    ax.axis("off")
    save(fig, "fem_guitar.svg")


# ---------------------------------------------------------------------------------------------
# Part 3: standing waves, eigenfunctions, Chladni
# ---------------------------------------------------------------------------------------------


def standing_waves():
    """The first four standing waves (eigenfunctions) of a string, with their nodes."""
    x = np.linspace(0, 1, 400)
    fig, axes = plt.subplots(4, 1, figsize=(7, 5.2), sharex=True, layout="constrained")
    for n, ax in enumerate(axes, start=1):
        for phase in np.linspace(0, np.pi, 7):
            ax.plot(x, np.sin(n * np.pi * x) * np.cos(phase), color=BLUE, lw=1.2, alpha=0.35)
        ax.plot(x, np.sin(n * np.pi * x), color=DARK, lw=2.5)
        nodes = np.arange(n + 1) / n
        ax.plot(nodes, np.zeros_like(nodes), "o", color=RED, ms=7, zorder=5)
        ax.set_ylim(-1.25, 1.25)
        ax.set_yticks([])
        ax.spines["left"].set_visible(False)
        ax.spines["bottom"].set_visible(False)
        ax.text(1.04, 0, f"$f_{n} = {n}\\cdot f_1$", va="center", fontsize=14)
    axes[-1].set_xticks([])
    save(fig, "standing_waves.svg")


def eigen_product():
    """u(x, t) = phi(x) cos(2 pi f t): a fixed shape times a clock."""
    x = np.linspace(0, 1, 300)
    t = np.linspace(0, 2, 300)
    phi = np.sin(2 * np.pi * x)

    fig, axes = plt.subplots(1, 3, figsize=(12, 3.0), layout="constrained")
    axes[0].plot(x, phi, color=DARK)
    axes[0].set_title(r"Form  $\varphi(x)$", fontsize=14)
    axes[0].set_xlabel("Ort $x$")
    axes[1].plot(t, np.cos(2 * np.pi * t), color=RED)
    axes[1].set_title(r"Takt  $\cos(2\pi f\, t)$", fontsize=14)
    axes[1].set_xlabel("Zeit $t$")
    for phase in np.linspace(0, np.pi, 9):
        axes[2].plot(x, phi * np.cos(phase), color=BLUE, lw=1.5, alpha=0.6)
    axes[2].plot([0, 0.5, 1], [0, 0, 0], "o", color=RED, ms=7, zorder=5)
    axes[2].set_title(r"Schwingung  $u(x,t) = \varphi(x)\cos(2\pi f\, t)$", fontsize=14)
    axes[2].set_xlabel("Ort $x$")
    for ax in axes:
        ax.axhline(0, color=GRAY, lw=0.6)
        ax.set_ylim(-1.2, 1.2)
        ax.set_xticks([])
        ax.set_yticks([])
    save(fig, "eigen_product.svg")


def mode_files():
    """All eigenmode *_top.csv files of the free plate with their frequency, sorted by frequency."""
    files = sorted_by_frequency((SIMULATION / "eigenmodes_free").glob("mode_*_top.csv"))
    return [(frequency_from_filename(f), f) for f in files]


def aligned_surfaces():
    """(frequency, surface) of all modes; pairs with equal frequency are turned into the parallel
    line patterns seen on the real plate (as chladni-plot --align-pairs does)."""
    modes = mode_files()
    frequencies = [frequency for frequency, _ in modes]
    surfaces = [load_top_surface(filename) for _, filename in modes]
    align_pairs(surfaces, frequencies)
    return list(zip(frequencies, surfaces))


def closest_mode(modes, target):
    return min(modes, key=lambda mode: abs(mode[0] - target))


def quadratic_interpolation(n_nodes, per_cell):
    """Matrix that evaluates the piecewise quadratic finite element function (FE_Q(2): three nodes
    per cell) given by its n_nodes values on per_cell points per cell.

    The *_top.csv files contain exactly these nodal values (65 x 65 for 32 cells). Showing them as
    an image directly gives the pixelated look; this evaluates the actual solution in between."""
    rows = []
    for cell in range((n_nodes - 1) // 2):
        for s in np.arange(per_cell) / per_cell:
            row = np.zeros(n_nodes)
            row[2 * cell : 2 * cell + 3] = [2 * (s - 0.5) * (s - 1), -4 * s * (s - 1), 2 * s * (s - 0.5)]
            rows.append(row)
    row = np.zeros(n_nodes)
    row[-1] = 1
    return np.array(rows + [row])


def refine(surface, per_cell=16):
    """The surface on a grid with per_cell points per cell instead of 2."""
    rows = quadratic_interpolation(surface.uz.shape[0], per_cell)
    columns = quadratic_interpolation(surface.uz.shape[1], per_cell)
    fine = lambda values: rows @ values @ columns.T
    return dataclasses.replace(surface, x=fine(surface.x), y=fine(surface.y), uz=fine(surface.uz), von_mises=None)


def sand(surface, width=0.06):
    """Where the sand ends up: bright where |u_z| is small (relative to its maximum)."""
    relative = np.abs(surface.uz) / np.abs(surface.uz).max()
    return np.exp(-((relative / width) ** 2))


def draw_sand(ax, surface):
    ax.imshow(sand(refine(surface)), cmap="gray", origin="lower", vmin=0, vmax=1, interpolation="antialiased")
    ax.set_xticks([])
    ax.set_yticks([])
    for spine in ax.spines.values():
        spine.set_visible(False)


def save_png(fig, name):
    fig.savefig(HERE / name, dpi=250, bbox_inches="tight", transparent=False, facecolor="white")
    plt.close(fig)
    print(f"saved figures/{name}")


def chladni_gallery(frequencies, name, columns=4, title=True):
    """Sand patterns of the eigenmodes closest to the given frequencies."""
    modes = aligned_surfaces()
    rows = int(np.ceil(len(frequencies) / columns))
    fig, axes = plt.subplots(rows, columns, figsize=(2.6 * columns, 2.9 * rows), layout="constrained", squeeze=False)
    for ax, target in zip(axes.flat, frequencies):
        frequency, surface = closest_mode(modes, target)
        draw_sand(ax, surface)
        if title:
            ax.set_title(f"{frequency:.0f} Hz", fontsize=15)
    for ax in axes.flat[len(frequencies):]:
        ax.axis("off")
    save_png(fig, name)


def plate_3d_and_sand(target=529.0):
    """One eigenmode: vibrating plate in 3D next to the sand pattern."""
    frequency, surface = closest_mode(aligned_surfaces(), target)
    fine = refine(surface, per_cell=4)

    fig = plt.figure(figsize=(11, 4.6))
    ax3d = fig.add_subplot(1, 2, 1, projection="3d")
    side = fine.x.max() - fine.x.min()
    height = 0.15 * side / np.abs(fine.uz).max() * fine.uz
    ax3d.plot_surface(fine.x, fine.y, height, facecolors=surface_colors(fine, "uz"), rstride=1, cstride=1,
                      linewidth=0, antialiased=True, shade=False)
    ax3d.set_zlim(1.2 * height.min(), 1.2 * height.max())
    ax3d.set_box_aspect((1, 1, 0.35), zoom=1.3)
    ax3d.set_axis_off()
    ax3d.view_init(elev=30, azim=-60)
    ax3d.set_title("rot = nach oben, blau = nach unten", fontsize=13)

    ax2d = fig.add_subplot(1, 2, 2)
    draw_sand(ax2d, surface)
    ax2d.set_title("Sand sammelt sich auf den Knotenlinien", fontsize=13)
    save_png(fig, "plate_3d_and_sand.png")


if __name__ == "__main__":
    derivative_slope()
    derivative_motion()
    curvature()
    partial_derivatives()
    space_time()
    wave_equation_forces()
    discretization()
    secants()
    refinement()
    fem_guitar()
    standing_waves()
    eigen_product()
    chladni_gallery([82.9, 105.1, 293.8, 529.0, 722.8, 924.8, 1281.6, 2010.1], "chladni_gallery.png")
    # the mode that matches the photo images/sand_pattern_photo.png (four loops and a cross)
    chladni_gallery([924.8], "chladni_single.png", columns=1)
    plate_3d_and_sand()
