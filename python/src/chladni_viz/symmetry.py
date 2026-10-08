"""Which modes can a shaker at the center excite?

The shaker pushes the center of the plate straight up and down. That push looks the same after
rotating the square by 90 degrees or mirroring it, so it can only excite modes that have the same
full symmetry. For every other mode the push gives as much energy to one half of the plate as it
takes away from the mirrored half, and the mode never builds up.

To test a mode we compare u_z with its rotated and mirrored copies via the normalized overlap

    c = sum(u * u_transformed) / sum(u * u),

which is +1 if the copy is identical, -1 if it is the same with flipped sign and 0 if the two are
unrelated (as for the members of a degenerate pair).
"""

import numpy as np


def overlap(uz, transformed):
    """Normalized overlap of a field with a transformed copy, in [-1, 1]."""
    return float(np.sum(uz * transformed) / np.sum(uz * uz))


def symmetry_overlaps(uz):
    """Overlaps of u_z (rows = y, columns = x, centered grid) with its copy rotated by 90 degrees
    and with its copy mirrored at the y axis (x -> -x)."""
    return overlap(uz, np.rot90(uz)), overlap(uz, uz[:, ::-1])


def is_center_symmetric(uz, threshold=0.9):
    """True if u_z is unchanged by a 90 degree rotation and by mirroring, i.e. if the mode can be
    excited by a shaker at the center. The other mirror lines (y -> -y, diagonals) follow from
    these two."""
    rotation, mirror = symmetry_overlaps(uz)
    return rotation > threshold and mirror > threshold


# Degenerate pairs
# ----------------
# Because the plate is square, many modes come in pairs with exactly the same frequency: a mode and
# (essentially) its copy rotated by 90 degrees. Every combination a u_1 + b u_2 is then an eigenmode
# too, and the eigensolver returns an arbitrary one, which often looks skewed. On the real plate
# small imperfections pick the orientation; typically one sees parallel (wavy) lines. We pick the
# two combinations that are unchanged or sign-flipped by mirroring x -> -x, which gives exactly
# these parallel-line patterns.


def is_rotated_pair(uz_a, uz_b, threshold=0.9):
    """True if two modes form a pair that is turned into itself by a 90 degree rotation.

    Single modes that a rotation maps to +-themselves (e.g. the fully symmetric ones) are not
    part of such a pair, so we first require that u_a is clearly changed by the rotation, and
    then that the rotated u_a lies (almost) in the span of u_a and u_b.
    """
    if abs(overlap(uz_a, np.rot90(uz_a))) > 0.5:
        return False
    basis, _ = np.linalg.qr(np.stack([uz_a.ravel(), uz_b.ravel()], axis=1))
    rotated = np.rot90(uz_a).ravel()
    return float(np.linalg.norm(basis.T @ rotated) / np.linalg.norm(rotated)) > threshold


def align_pair(uz_a, uz_b):
    """The two combinations of a pair that are symmetric / antisymmetric under x -> -x, each scaled
    to max |u_z| = 1."""
    u = [uz_a / np.linalg.norm(uz_a), uz_b / np.linalg.norm(uz_b)]
    # Matrix of the mirror operation restricted to the pair; its eigenvectors are the combinations
    # that the mirror maps to +-themselves.
    mirror = np.array([[np.sum(u_i * u_j[:, ::-1]) for u_j in u] for u_i in u])
    _, coefficients = np.linalg.eigh((mirror + mirror.T) / 2)
    aligned = [coefficients[0, k] * u[0] + coefficients[1, k] * u[1] for k in range(2)]
    return [field / np.abs(field).max() for field in aligned]
