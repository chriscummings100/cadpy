print("x")
import cadpy as cp

assert cp.add(1, 2) == 3


cp.BSP.cube(cp.float3(1, 2, 3)).to_tri_mesh()