import cadpy as cp
import pytest

assert cp.add(1, 2) == 3


def test_create_cube():
    cube = cp.BSP.cube(cp.float3(1, 2, 3))
    verts = list(cube.vertices)
    pass

if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
