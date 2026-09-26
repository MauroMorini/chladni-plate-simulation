"""Reading the files written by the C++ simulation."""

import math
import re
from dataclasses import dataclass
from pathlib import Path

import numpy as np


@dataclass
class TopSurface:
    """Fields on the top face of the plate, as 2D arrays of shape (n, n).

    Row i belongs to the i-th y value, column j to the j-th x value.
    """

    x: np.ndarray
    y: np.ndarray
    uz: np.ndarray  # vertical displacement [m] (arbitrary scale for eigenmodes)
    von_mises: np.ndarray | None  # von Mises stress [Pa], None for files without that column


def load_top_surface(filename):
    """Read a *_top.csv file (columns x, y, uz[, von_mises], sorted by y, then x)."""
    data = np.loadtxt(filename, delimiter=",", skiprows=1, ndmin=2)
    n = math.isqrt(len(data))
    if n * n != len(data):
        raise ValueError(f"{filename}: {len(data)} points do not form a square grid")
    columns = [column.reshape(n, n) for column in data.T]
    von_mises = columns[3] if len(columns) > 3 else None
    return TopSurface(x=columns[0], y=columns[1], uz=columns[2], von_mises=von_mises)


# Scalar fields that can be shown: name -> (label, matplotlib colormap)
FIELDS = {
    "uz": ("|u_z| (relative)", "gray_r"),
    "stress": ("von Mises stress (relative, top 1% clipped)", "inferno"),
}


def relative_field(surface, name):
    """The field `name` ("uz" or "stress") scaled to values in [0, 1].

    Eigenmodes have no physical amplitude, so only relative values are meaningful.

    - uz: |u_z| divided by its maximum.
    - stress: the von Mises stress divided by its 99th percentile and clipped at 1. The clamped
      bolt causes a stress peak (a singularity at the edge of the clamp) on about 1% of the plate,
      which would otherwise take up the whole color range.
    """
    if name == "uz":
        values = np.abs(surface.uz)
        return values / values.max()
    if name == "stress":
        if surface.von_mises is None:
            raise ValueError("this file has no von Mises stress column (re-run the C++ simulation)")
        return np.minimum(surface.von_mises / np.percentile(surface.von_mises, 99), 1.0)
    raise ValueError(f"unknown field '{name}', choose one of {list(FIELDS)}")


def title_from_filename(filename):
    """'mode_03_506.1Hz_top.csv' -> 'mode 3, 506.1 Hz', 'forced_500.0Hz_top.csv' -> 'forced, 500.0 Hz'"""
    name = Path(filename).name
    frequency = re.search(r"(\d+(?:\.\d+)?)Hz", name)
    mode = re.match(r"mode_(\d+)", name)
    if mode and frequency:
        return f"mode {int(mode.group(1))}, {frequency.group(1)} Hz"
    if frequency:
        return f"forced, {frequency.group(1)} Hz"
    return name
