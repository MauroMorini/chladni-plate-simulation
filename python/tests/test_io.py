import numpy as np
import pytest

from chladni_viz.io import load_top_surface, relative_field, title_from_filename

COORDINATES = [-1.0, 0.0, 1.0]


def write_csv(path, header, rows):
    path.write_text(header + "\n" + "".join(",".join(map(str, row)) + "\n" for row in rows))


def test_load_top_surface_reshapes_to_grid(tmp_path):
    # 3 x 3 grid, sorted by y then x, with u_z = x + 10 y and stress = 2 + x
    rows = [(x, y, x + 10 * y, 2 + x) for y in COORDINATES for x in COORDINATES]
    write_csv(tmp_path / "grid_top.csv", "x,y,uz,von_mises", rows)

    surface = load_top_surface(tmp_path / "grid_top.csv")

    assert surface.uz.shape == (3, 3)
    np.testing.assert_allclose(surface.x[0], COORDINATES)  # a row runs along x
    np.testing.assert_allclose(surface.y[:, 0], COORDINATES)  # a column runs along y
    np.testing.assert_allclose(surface.uz, surface.x + 10 * surface.y)
    np.testing.assert_allclose(surface.von_mises, 2 + surface.x)


def test_load_top_surface_without_stress_column(tmp_path):
    rows = [(x, y, 1.0) for y in COORDINATES for x in COORDINATES]
    write_csv(tmp_path / "old_top.csv", "x,y,uz", rows)

    surface = load_top_surface(tmp_path / "old_top.csv")

    assert surface.von_mises is None
    with pytest.raises(ValueError):
        relative_field(surface, "stress")


def test_load_top_surface_rejects_non_square(tmp_path):
    write_csv(tmp_path / "bad_top.csv", "x,y,uz", [(0, 0, 0), (1, 0, 0)])
    with pytest.raises(ValueError):
        load_top_surface(tmp_path / "bad_top.csv")


def test_relative_uz_is_scaled_to_one(tmp_path):
    rows = [(x, y, -4 * x, 1.0) for y in COORDINATES for x in COORDINATES]
    write_csv(tmp_path / "grid_top.csv", "x,y,uz,von_mises", rows)
    surface = load_top_surface(tmp_path / "grid_top.csv")

    np.testing.assert_allclose(relative_field(surface, "uz"), np.abs(surface.x))


def test_relative_stress_clips_a_single_peak(tmp_path):
    # 20 x 20 grid with stress 1 everywhere except one huge peak on 1 of 400 points (< 1%),
    # like the clamped bolt.
    coordinates = np.linspace(-1, 1, 20)
    rows = [(x, y, 1.0, 1000.0 if (i, j) == (10, 10) else 1.0)
            for j, y in enumerate(coordinates) for i, x in enumerate(coordinates)]
    write_csv(tmp_path / "peak_top.csv", "x,y,uz,von_mises", rows)
    surface = load_top_surface(tmp_path / "peak_top.csv")

    stress = relative_field(surface, "stress")
    assert stress.max() == 1.0
    np.testing.assert_allclose(np.median(stress), 1.0)  # the peak does not squash the rest


@pytest.mark.parametrize(
    "filename, title",
    [
        ("mode_03_506.1Hz_top.csv", "mode 3, 506.1 Hz"),
        ("forced_500.0Hz_top.csv", "forced, 500.0 Hz"),
        ("something.csv", "something.csv"),
    ],
)
def test_title_from_filename(filename, title):
    assert title_from_filename(filename) == title
