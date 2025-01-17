#pragma once

#include <vector>
#include <memory>
#include <span>

class float3
{
public:
    float x;
    float y;
    float z;
};

class Plane
{
public:
    float3 normal;
    float distance;
};

struct VIdx
{
    int i;

    VIdx() : i(-1) {}
    VIdx(int _i) : i(_i) {}

    bool operator==(const VIdx& other) const
    {
        return i == other.i;
    }

    operator bool() const 
    {
        return i != -1;
    }

    static VIdx invalid()
    {
        return { -1 };
    }
};

struct EIdx
{
    int i;

    EIdx() : i(-1) {}
    EIdx(int _i) : i(_i) {}

    bool operator==(const EIdx& other) const
    {
        return i == other.i;
    }

    operator bool() const 
    {
        return i != -1;
    }

    static EIdx invalid()
    {
        return { -1 };
    }
};

struct PIdx
{
    int i;

    PIdx() : i(-1) {}
    PIdx(int _i) : i(_i) {}

    bool operator==(const EIdx& other) const
    {
        return i == other.i;
    }

    operator bool() const 
    {
        return i != -1;
    }

    static PIdx invalid()
    {
        return { -1 };
    }
};

class Vertex
{
public:
    EIdx edge;
    float3 position;
};

class HalfEdge
{
public:
    EIdx twin;
    EIdx next;
    EIdx prev;
    PIdx polygon;
    VIdx vertex;

};

class Polygon
{
public:
    EIdx edge;
};

class TriangleMesh
{
public:
    std::vector<float3> positions;
    std::vector<float3> normals;
    std::vector<int> indices;

};


class Node
{
public:
    Plane plane;
    std::vector<PIdx> polygons;
    int front;
    int back;
};

class BSP
{
public:

    static std::shared_ptr<BSP> cube(float3 size);

    VIdx add_vertex(float3 position);

    PIdx add_polygon(std::span<const VIdx> indices);

    PIdx add_polygon(std::initializer_list<VIdx> indices)
    {
        std::span<const VIdx> indices_span(indices);
        return add_polygon(indices_span);
    }

    std::shared_ptr<TriangleMesh> to_tri_mesh() const;

    const HalfEdge& edge(EIdx idx) const
    {
        return m_half_edges[idx.i];
    }
    HalfEdge& edge(EIdx idx)
    {
        return m_half_edges[idx.i];
    }

    const Vertex& vertex(VIdx idx) const
    {
        return m_vertices[idx.i];
    }
    Vertex& vertex(VIdx idx)
    {
        return m_vertices[idx.i];
    }

    const Polygon& polygon(PIdx idx) const
    {
        return m_polygons[idx.i];
    }
    Polygon& polygon(PIdx idx)
    {
        return m_polygons[idx.i];
    }

private:
    std::vector<Vertex> m_vertices;
    std::vector<HalfEdge> m_half_edges;
    std::vector<Polygon> m_polygons;
    std::vector<Node> m_nodes;
};

class Context
{

};

