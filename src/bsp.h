#pragma once

#include <vector>
#include <memory>
#include <span>
#include <map>

class float3
{
public:
    float x;
    float y;
    float z;

    // Dot product
    static float dot(const float3& a, const float3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    // Cross product
    static float3 cross(const float3& a, const float3& b)
    {
        return float3{
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    static float length(const float3& a)
    {
        return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
    }

    static float3 normalize(const float3& a)
    {
        float len = sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
        return {a.x / len, a.y / len, a.z / len};
    }

    // Addition
    float3 operator+(const float3& other) const
    {
        return float3{x + other.x, y + other.y, z + other.z};
    }

    // Subtraction
    float3 operator-(const float3& other) const
    {
        return float3{x - other.x, y - other.y, z - other.z};
    }

    // Multiplication by scalar
    float3 operator*(float scalar) const
    {
        return float3{x * scalar, y * scalar, z * scalar};
    }

    // Division by scalar
    float3 operator/(float scalar) const
    {
        return float3{x / scalar, y / scalar, z / scalar};
    }

    // Compound assignment operators
    float3& operator+=(const float3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    float3& operator-=(const float3& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    float3& operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    float3& operator/=(float scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }
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

    bool operator<(const VIdx& other) const {
        return i < other.i;
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

    bool operator<(const EIdx& other) const {
        return i < other.i;
    }
};

struct PIdx
{
    int i;

    PIdx() : i(-1) {}
    PIdx(int _i) : i(_i) {}

    bool operator==(const PIdx& other) const
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

    bool operator<(const PIdx& other) const {
        return i < other.i;
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

struct EdgeId
{
    VIdx v0;
    VIdx v1;

    bool operator<(const EdgeId& other) const {
        if(v0 < other.v0) return true;
        if(other.v0 < v0) return false;
        return v1 < other.v1;
    }

    bool operator==(const EdgeId& other) const {
        return v0 == other.v0 && v1 == other.v1;
    }
};

class Polygon
{
public:
    EIdx edge;
};

class Mesh
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

    std::shared_ptr<Mesh> to_tri_mesh() const;
    std::shared_ptr<Mesh> to_edge_mesh() const;

    const std::vector<Vertex>& vertices() const
    {
        return m_vertices;
    }

    const std::vector<HalfEdge>& half_edges() const
    {
        return m_half_edges;
    }

    const std::vector<Polygon>& polygons() const
    {
        return m_polygons;
    }

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
    std::map<EdgeId, EIdx> m_edge_map;
};

class Context
{

};

