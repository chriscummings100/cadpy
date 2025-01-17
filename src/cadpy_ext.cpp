#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/shared_ptr.h>
#include "bsp.h"

namespace nb = nanobind;

using namespace nb::literals;

NB_MODULE(cadpy_ext, m) {
    m.doc() = "This is a \"hello world\" example with nanobind";

    nb::class_<float3>(m,"float3")
        .def(nb::init())
        .def("__init__", [](float3 *self, float x, float y, float z) {
            new (self) float3{x, y, z};
        }, "x"_a, "y"_a, "z"_a)
        .def_rw("x", &float3::x)
        .def_rw("y", &float3::y)
        .def_rw("z", &float3::z);

    nb::class_<Plane>(m,"Plane")
        .def_rw("normal", &Plane::normal)
        .def_rw("distance", &Plane::distance);

    nb::class_<VIdx>(m,"VIdx");
    nb::class_<EIdx>(m,"EIdx");
    nb::class_<PIdx>(m,"PIdx");

    nb::class_<Vertex>(m,"Vertex")
        .def_ro("edge", &Vertex::edge)
        .def_ro("position", &Vertex::position);
    
    nb::class_<HalfEdge>(m,"HalfEdge")
        .def_ro("twin", &HalfEdge::twin)
        .def_ro("next", &HalfEdge::next)
        .def_ro("prev", &HalfEdge::prev)
        .def_ro("polygon", &HalfEdge::polygon)
        .def_ro("vertex", &HalfEdge::vertex);
    
    nb::class_<Polygon>(m,"Polygon");

    nb::class_<TriangleMesh>(m,"TriangleMesh")
        .def_ro("positions", &TriangleMesh::positions)
        .def_ro("normals", &TriangleMesh::normals)
        .def_ro("indices", &TriangleMesh::indices);

    nb::class_<Node>(m,"Node")
        .def_ro("plane", &Node::plane)
        .def_ro("polygons", &Node::polygons)
        .def_ro("front", &Node::front)
        .def_ro("back", &Node::back);

    nb::class_<BSP>(m,"BSP")
        .def_static("cube", &BSP::cube)
        .def("to_tri_mesh", &BSP::to_tri_mesh);

    m.def("add", [](int a, int b) { 
        return a + b; 
    }, "a"_a, "b"_a);
    m.def("sub", [](int a, int b) { 
        return a - b; 
    }, "a"_a, "b"_a);
}
