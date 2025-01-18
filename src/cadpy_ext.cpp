#include <nanobind/nanobind.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/bind_vector.h>
#include <nanobind/stl/string.h>

#include <fmt/format.h>

#include <vector>

#include "bsp.h"
#include <string>

namespace nb = nanobind;

using namespace nb::literals;

using VertexVector = std::vector<Vertex>;
using HalfEdgeVector = std::vector<HalfEdge>;
using PolygonVector = std::vector<Polygon>;

NB_MODULE(cadpy_ext, m) {
    m.doc() = "This is a \"hello world\" example with nanobind";

    nb::class_<float3>(m,"float3")
        .def(nb::init())
        .def("__init__", [](float3 *self, float x, float y, float z) {
            new (self) float3{x, y, z};
        }, "x"_a, "y"_a, "z"_a)
        .def_rw("x", &float3::x)
        .def_rw("y", &float3::y)
        .def_rw("z", &float3::z)
        .def("dot", &float3::dot)
        .def("cross", &float3::cross)
        .def("length", &float3::length)
        .def("normalize", &float3::normalize)
        .def("similar", &float3::similar)
        .def("__add__", [](const float3 &a, const float3 &b) { return a + b; })
        .def("__sub__", [](const float3 &a, const float3 &b) { return a - b; })
        .def("__mul__", [](const float3 &a, float b) { return a * b; })
        .def("__truediv__", [](const float3 &a, float b) { return a / b; })
        .def("__iadd__", [](float3 &a, const float3 &b) { a += b; return a; })
        .def("__isub__", [](float3 &a, const float3 &b) { a -= b; return a; })
        .def("__imul__", [](float3 &a, float b) { a *= b; return a; })
        .def("__itruediv__", [](float3 &a, float b) { a /= b; return a; });

    nb::class_<Plane>(m,"Plane")
        .def_rw("normal", &Plane::normal)
        .def_rw("distance", &Plane::distance);

    nb::class_<VIdx>(m,"VIdx");
    nb::class_<EIdx>(m,"EIdx");
    nb::class_<PIdx>(m,"PIdx");

    nb::bind_vector<VertexVector>(m, "VertexVector");
    nb::bind_vector<HalfEdgeVector>(m, "HalfEdgeVector");
    nb::bind_vector<PolygonVector>(m, "PolygonVector");

    nb::class_<Vertex>(m,"Vertex")
        .def_ro("edge", &Vertex::edge)
        .def_ro("position", &Vertex::position)
        .def("__repr__", [](const Vertex &v) {

            return fmt::format("Vertex(e={}, p=[{}, {}, {}])",
                v.edge.i,
                v.position.x,
                v.position.y,
                v.position.z);
        });

    nb::class_<HalfEdge>(m,"HalfEdge")
        .def_ro("twin", &HalfEdge::twin)
        .def_ro("next", &HalfEdge::next)
        .def_ro("prev", &HalfEdge::prev)
        .def_ro("polygon", &HalfEdge::polygon)
        .def_ro("vertex", &HalfEdge::vertex)
        .def("__repr__", [](const HalfEdge &e) {

            return fmt::format("HalfEdge(et={}, en={}, ep={}, p={}, v={})",
                e.twin.i,
                e.next.i,
                e.prev.i,
                e.polygon.i,
                e.vertex.i);
        });

    nb::class_<Polygon>(m,"Polygon")
        .def_ro("edge", &Polygon::edge)
        .def("__repr__", [](const Polygon &p) {

            return fmt::format("Polygon(e={})", p.edge.i);
        });

    nb::class_<Mesh>(m,"Mesh")
        .def_prop_ro("positions", [](Mesh* self){
            return nb::ndarray<float, nb::numpy>(self->positions.data(), {self->positions.size(), 3});
        }, nb::rv_policy::reference_internal)
        .def_prop_ro("normals", [](Mesh* self){
            return nb::ndarray<float, nb::numpy>(self->normals.data(), {self->normals.size(), 3});
        }, nb::rv_policy::reference_internal)
        .def_prop_ro("indices", [](Mesh* self){
            return nb::ndarray<int, nb::numpy>(self->indices.data(), {self->indices.size()});
        }, nb::rv_policy::reference_internal);

    nb::class_<Node>(m,"Node")
        .def_ro("plane", &Node::plane)
        .def_ro("polygons", &Node::polygons)
        .def_ro("front", &Node::front)
        .def_ro("back", &Node::back);

    nb::class_<BSP>(m,"BSP")
        .def_static("cube", &BSP::cube, "size"_a, "center"_a=false)
        .def_prop_ro("vertices", &BSP::vertices, nb::rv_policy::reference_internal)
        .def_prop_ro("half_edges", &BSP::half_edges, nb::rv_policy::reference_internal)
        .def_prop_ro("polygons", &BSP::polygons, nb::rv_policy::reference_internal)
        .def("to_tri_mesh", &BSP::to_tri_mesh)
        .def("to_edge_mesh", &BSP::to_edge_mesh);

    m.def("add", [](int a, int b) {
        return a + b;
    }, "a"_a, "b"_a);
    m.def("sub", [](int a, int b) {
        return a - b;
    }, "a"_a, "b"_a);
}
