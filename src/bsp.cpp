#include "bsp.h"
#include <array>

VIdx BSP::create_vertex(float3 position)
{
    m_vertices.push_back({EIdx::invalid(), position});
    return {(int)(m_vertices.size() - 1)};
}

PIdx BSP::create_polygon(std::span<const VIdx> indices)
{
    PIdx polygon = (int)m_polygons.size();

    EIdx first_edge = {(int)m_half_edges.size()};
    int num_edges = indices.size();

    m_polygons.push_back({first_edge});

    for(int i = 0; i < num_edges; i++)
    {
        EIdx this_edge = first_edge.i + i;
        EIdx prev_edge = first_edge.i + (i + num_edges - 1) % num_edges;
        EIdx next_edge = first_edge.i + (i + 1) % num_edges;
        VIdx vertex = indices[i];
        VIdx next_vertex = indices[(i + 1) % num_edges];
        EdgeId edge_id = {vertex, next_vertex};
        EdgeId twin_edge_id = {next_vertex, vertex};

        bool equal = edge_id == twin_edge_id;

        m_edge_map.insert({edge_id, this_edge});

        m_half_edges.push_back({EIdx::invalid(), prev_edge, next_edge, polygon, vertex});

        if(m_edge_map.contains(twin_edge_id))
        {
            EIdx twin_edge = m_edge_map.at(twin_edge_id);
            m_half_edges[this_edge.i].twin = twin_edge;
            m_half_edges[twin_edge.i].twin = this_edge;
        }

        if(!m_vertices[vertex.i].edge)
            m_vertices[vertex.i].edge = this_edge;
    }

    return polygon;
}

std::shared_ptr<BSP> BSP::cube(float3 size, bool center)
{
    auto res = std::make_shared<BSP>();

    auto min = float3(0,0,0);
    auto max = size;
    if(center)
    {
        auto half_size = size / 2;
        min -= half_size;
        max -= half_size;
    }

    auto v_0_0_0 = res->create_vertex({min.x, min.y, min.z});
    auto v_1_0_0 = res->create_vertex({max.x, min.y, min.z});
    auto v_0_1_0 = res->create_vertex({min.x, max.y, min.z});
    auto v_1_1_0 = res->create_vertex({max.x, max.y, min.z});

    auto v_0_0_1 = res->create_vertex({min.x, min.y, max.z});
    auto v_1_0_1 = res->create_vertex({max.x, min.y, max.z});
    auto v_0_1_1 = res->create_vertex({min.x, max.y, max.z});
    auto v_1_1_1 = res->create_vertex({max.x, max.y, max.z});

    res->create_polygon({v_0_0_0, v_1_0_0, v_1_1_0, v_0_1_0});
    res->create_polygon({v_0_1_1, v_1_1_1, v_1_0_1, v_0_0_1});

    res->create_polygon({v_0_0_0, v_0_0_1, v_1_0_1, v_1_0_0});
    res->create_polygon({v_1_1_0, v_1_1_1, v_0_1_1, v_0_1_0});

    res->create_polygon({v_0_0_0, v_0_1_0, v_0_1_1, v_0_0_1});
    res->create_polygon({v_1_0_1, v_1_1_1, v_1_1_0, v_1_0_0});

    return res;
}

std::shared_ptr<Mesh> BSP::to_tri_mesh() const
{
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

    for (auto& polygon : m_polygons)
    {
        int first_position = mesh->positions.size();

        auto e = edge(polygon.edge);
        while(true)
        {
            mesh->positions.push_back(vertex(e.vertex).position);
            if(e.next == polygon.edge)
                break;
            e = edge(e.next);
        }

        float3 n = float3::cross(mesh->positions[first_position + 1] - mesh->positions[first_position], mesh->positions[first_position + 2] - mesh->positions[first_position]);
        n = float3::normalize(n);

        for(int i = first_position; i < mesh->positions.size(); i++)
        {
            mesh->normals.push_back(n);
        }

        int num_positions = mesh->positions.size() - first_position;

        int top = 0;
        int bottom = num_positions - 1;
        bool even = true;

        mesh->indices.push_back(first_position + top);
        mesh->indices.push_back(first_position + top + 1);
        mesh->indices.push_back(first_position + bottom);
        top++;

        while((bottom - top) >= 2)
        {
            if(even)
            {
                mesh->indices.push_back(first_position + top);
                mesh->indices.push_back(first_position + top + 1);
                mesh->indices.push_back(first_position + bottom);
                top++;
            }
            else
            {
                mesh->indices.push_back(first_position + bottom);
                mesh->indices.push_back(first_position + top);
                mesh->indices.push_back(first_position + bottom - 1);
                bottom--;
            }
            even = !even;
        }
    }

    return mesh;
}

std::shared_ptr<Mesh> BSP::to_edge_mesh() const
{
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

    for(auto& _vertex : m_vertices)
    {
        mesh->positions.push_back(_vertex.position);
        mesh->normals.push_back(float3());
    }

    for (auto& _edge : m_half_edges)
    {
        {
            auto _vert = _edge.vertex;
            mesh->indices.push_back(_vert.i);
        }

        {
            auto _next_edge = edge(_edge.next);
            auto _vert = _next_edge.vertex;
            mesh->indices.push_back(_vert.i);
        }
    }

    return mesh;
}

void BSP::split(std::span<const PIdx> polygons, const Plane& plane, std::vector<PIdx>& coplanar, std::vector<PIdx>& front, std::vector<PIdx>& back)
{
    
}
