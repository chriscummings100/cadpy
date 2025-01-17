#include "bsp.h"
#include <array>

VIdx BSP::add_vertex(float3 position)
{
    m_vertices.push_back({EIdx::invalid(), position});
    return {(int)(m_vertices.size() - 1)};
}

PIdx BSP::add_polygon(std::span<const VIdx> indices)
{
    PIdx polygon = (int)m_polygons.size();

    EIdx first_edge = {(int)m_half_edges.size()};
    int num_edges = indices.size();

    m_polygons.push_back({first_edge});

    for(int i = 0; i < num_edges; i++)
    {
        EIdx prev_edge = first_edge.i + (i + num_edges - 1) % num_edges;
        EIdx next_edge = first_edge.i + (i + 1) % num_edges;
        VIdx vertex = indices[i];

        m_half_edges.push_back({EIdx::invalid(), prev_edge, next_edge, polygon, vertex});

        if(!m_vertices[vertex.i].edge)
        {
            m_vertices[vertex.i].edge = first_edge.i + i;
        }
    }

    return polygon;
}

std::shared_ptr<BSP> BSP::cube(float3 size)
{
    auto res = std::make_shared<BSP>();

    auto v0 = res->add_vertex({-size.x, -size.y, -size.z});
    auto v1 = res->add_vertex({size.x, -size.y, -size.z});
    auto v2 = res->add_vertex({size.x, size.y, -size.z});
    auto v3 = res->add_vertex({-size.x, size.y, -size.z});

    auto v4 = res->add_vertex({-size.x, -size.y, size.z});
    auto v5 = res->add_vertex({size.x, -size.y, size.z});
    auto v6 = res->add_vertex({size.x, size.y, size.z});
    auto v7 = res->add_vertex({-size.x, size.y, size.z});

    res->add_polygon({v0, v1, v2, v3});

    return res;
}

std::shared_ptr<TriangleMesh> BSP::to_tri_mesh() const
{
    std::shared_ptr<TriangleMesh> mesh = std::make_shared<TriangleMesh>();

    for (auto& polygon : m_polygons)
    {
        std::vector<float3> positions;
        std::vector<float3> normals;
        std::vector<int> indices;

        int first_position = positions.size();

        auto e = edge(polygon.edge);
        while(true)
        {
            positions.push_back(vertex(e.vertex).position);
            normals.push_back(float3());
            if(e.next == polygon.edge)
                break;
            e = edge(e.next);
        }

        for(int i = 0; i < positions.size(); i++)
        {
            indices.push_back(first_position + i);
            indices.push_back(first_position + (i + 1) % positions.size());
            indices.push_back(first_position + (i + 2) % positions.size());
        }
    }

    return mesh;
}
