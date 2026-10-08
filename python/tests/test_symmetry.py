import numpy as np
import pytest

from chladni_viz.symmetry import align_pair, is_center_symmetric, is_rotated_pair, symmetry_overlaps

# Centered grid as in the *_top.csv files: rows = y, columns = x.
X, Y = np.meshgrid(np.linspace(-1, 1, 41), np.linspace(-1, 1, 41))


@pytest.mark.parametrize(
    "name, uz, excited",
    [
        ("fully symmetric (rings)", np.cos(3 * np.hypot(X, Y)), True),
        ("fully symmetric (x^2 + y^2 - 1)", X**2 + Y**2 - 1, True),
        ("antisymmetric in x (one of a pair)", X, False),
        ("diagonal pair member", X + Y, False),
        ("x^2 - y^2 (flips under rotation)", X**2 - Y**2, False),
        ("x y (flips under rotation)", X * Y, False),
        ("pinwheel x y (x^2 - y^2) (flips under mirroring)", X * Y * (X**2 - Y**2), False),
    ],
)
def test_is_center_symmetric(name, uz, excited):
    assert is_center_symmetric(uz) == excited, name


def test_overlaps_are_plus_minus_one():
    np.testing.assert_allclose(symmetry_overlaps(X**2 + Y**2), (1.0, 1.0))
    np.testing.assert_allclose(symmetry_overlaps(X**2 - Y**2), (-1.0, 1.0))
    np.testing.assert_allclose(symmetry_overlaps(X), (0.0, -1.0), atol=1e-12)


def test_sign_of_mode_does_not_matter():
    assert is_center_symmetric(-(X**2 + Y**2 - 1))


# A degenerate pair: three parallel lines in x (odd in x) and the same pattern rotated by 90 degrees.
LINES_X = np.sin(1.5 * np.pi * X) * (1 + 0.2 * Y**2)
LINES_Y = np.rot90(LINES_X)


def test_rotated_pair_is_recognized():
    mixed_a = 0.8 * LINES_X + 0.6 * LINES_Y  # what the eigensolver may return
    mixed_b = -0.6 * LINES_X + 0.8 * LINES_Y
    assert is_rotated_pair(mixed_a, mixed_b)


def test_symmetric_modes_are_no_pair():
    assert not is_rotated_pair(X**2 + Y**2, np.cos(3 * np.hypot(X, Y)))
    assert not is_rotated_pair(LINES_X, X**2 + Y**2)  # rotated LINES_X is not in their span


def test_align_pair_recovers_parallel_lines():
    mixed_a = 0.8 * LINES_X + 0.6 * LINES_Y
    mixed_b = -0.6 * LINES_X + 0.8 * LINES_Y
    aligned = align_pair(mixed_a, mixed_b)

    expected = [LINES_X / np.abs(LINES_X).max(), LINES_Y / np.abs(LINES_Y).max()]
    for field in aligned:
        assert np.abs(field).max() == pytest.approx(1.0)
        # each aligned field is +- one of the two pure line patterns (|cosine similarity| = 1)
        cosines = [abs(np.sum(field * e)) / (np.linalg.norm(field) * np.linalg.norm(e)) for e in expected]
        assert max(cosines) == pytest.approx(1.0)
