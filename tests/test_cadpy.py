import math
import cadpy as cp
import pytest
import numpy as np

assert cp.add(1, 2) == 3


def test_create_cube():

    # Make cube
    cube = cp.BSP.cube(cp.float3(1, 2, 3))

    # Check correct number of vertices, half edges and polygons
    verts = list(cube.vertices)
    half_edges = list(cube.half_edges)
    polygons = list(cube.polygons)
    assert len(verts) == 8
    assert len(half_edges) == 24
    assert len(polygons) == 6

    # Check centre
    sum_pos = sum([x.position for x in verts], start=cp.float3())
    centre = sum_pos / len(verts)
    assert cp.float3.similar(centre, cp.float3(0.5, 1, 1.5))

    # Check all half edges have a twin
    assert all([x.twin for x in half_edges])

    # Check all twins are unique
    unique_twins = set([x.twin for x in half_edges])
    assert len(unique_twins) == len(half_edges)

def test_cube_mesh():
    cube = cp.BSP.cube(cp.float3(1, 2, 3))
    mesh = cube.to_tri_mesh()
    assert mesh.positions.shape == (24, 3)
    assert mesh.normals.shape == (24, 3)
    assert mesh.indices.shape == (36,)

    # Check all normals are unit length
    assert all([ abs(np.dot(x,x)-1) < 0.0001 for x in mesh.normals])

    # Check all indices are in range
    assert all([0 <= x < len(mesh.positions) for x in mesh.indices])

if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
