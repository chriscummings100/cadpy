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

    m_polygons.push_back({
        .edge=first_edge,
        .debug_highlight=false
    });

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

        m_half_edges.push_back({
            .twin = EIdx::invalid(),
             .next = next_edge,
             .prev = prev_edge,
            .polygon = polygon,
            .vertex = vertex,
            .debug_highlight=false});

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

    res->create_polygon({v_0_1_0, v_1_1_0, v_1_0_0, v_0_0_0});
    res->create_polygon({v_0_0_1, v_1_0_1, v_1_1_1, v_0_1_1});
    res->create_polygon({v_1_0_0, v_1_0_1, v_0_0_1, v_0_0_0});
    res->create_polygon({v_0_1_0, v_0_1_1, v_1_1_1, v_1_1_0});
    res->create_polygon({v_0_0_1, v_0_1_1, v_0_1_0, v_0_0_0});
    res->create_polygon({v_1_0_0, v_1_1_0, v_1_1_1, v_1_0_1});

    return res;
}

std::shared_ptr<Mesh> BSP::to_tri_mesh() const
{
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

    for (auto& polygon : m_polygons)
    {
        int first_position = mesh->positions.size();

        auto e = get_edge(polygon.edge);
        while(true)
        {
            mesh->positions.push_back(get_vertex(e.vertex).position);
            if(e.next == polygon.edge)
                break;
            e = get_edge(e.next);
        }

        float3 n = float3::cross(mesh->positions[first_position + 1] - mesh->positions[first_position], mesh->positions[first_position + 2] - mesh->positions[first_position]);
        n = float3::normalize(n);

        for(int i = first_position; i < mesh->positions.size(); i++)
        {
            mesh->normals.push_back(n);
            if (polygon.debug_highlight)
                mesh->colors.push_back(float3(1, 0, 0));
            else
                mesh->colors.push_back(float3(1, 1, 1));
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

    /*for (auto& _vertex : m_vertices)
    {
        mesh->positions.push_back(_vertex.position);
        mesh->normals.push_back(float3());
        mesh->colors.push_back(float3(1, 1, 1));
    }

    for (auto& _edge : m_half_edges)
    {
        {
            auto _vert = _edge.vertex;
            mesh->indices.push_back(_vert.i);
        }

        {
            auto _next_edge = get_edge(_edge.next);
            auto _vert = _next_edge.vertex;
            mesh->indices.push_back(_vert.i);
        }
    }*/

    for (auto& _edge : m_half_edges) {
        float3 col = _edge.debug_highlight ? float3(1, 0, 0) : float3(0, 0, 0);

        {
            mesh->positions.push_back(get_vertex(_edge.vertex).position);
            mesh->normals.push_back(float3());
            mesh->colors.push_back(col);
            mesh->indices.push_back(mesh->positions.size() - 1);
        }

        {
            mesh->positions.push_back(get_vertex(get_edge(_edge.next).vertex).position);
            mesh->normals.push_back(float3());
            mesh->colors.push_back(col);
            mesh->indices.push_back(mesh->positions.size() - 1);
        }
    }

    return mesh;
}



void BSP::split(
    std::vector<PIdx> polygons,
    const Plane& plane,
    std::vector<PIdx>& coplanar,
    std::vector<PIdx>& front,
    std::vector<PIdx>& back
)
{
    std::vector<EIdx> edges_to_split;

    for(PIdx& polygon_idx : polygons)
    {
        Polygon& polygon = get_polygon(polygon_idx);
        polygon.split_id_0 = INT32_MIN;
        polygon.split_id_1 = INT32_MAX;
    }

    for(PIdx& polygon_idx : polygons)
    {
        Polygon& polygon = get_polygon(polygon_idx);
        EIdx first_edge_idx = polygon.edge;
        EIdx curr_edge_idx = first_edge_idx;
        do
        {
            HalfEdge& edge = get_edge(curr_edge_idx);
            EIdx next_edge_index = edge.next;
            if (!edge.twin || curr_edge_idx < edge.twin) {
                float3 v0 =   get_vertex(edge.vertex).position;
                float3 v1 = get_vertex(get_edge(next_edge_index).vertex).position;
                float d0 = plane.distance(v0);
                float d1 = plane.distance(v1);
                if(d0 * d1 < 0)
                {
                    edges_to_split.push_back(curr_edge_idx);

                    edge.debug_highlight = true;
                    polygon.debug_highlight = true;
                    polygon.split_id_0 = std::min(polygon.split_id_0, curr_edge_idx.i);
                    polygon.split_id_1 = std::max(polygon.split_id_1, curr_edge_idx.i);

                    if(edge.twin) {
                        EIdx twin_edge_idx = edge.twin;
                        HalfEdge& twin_edge = get_edge(twin_edge_idx);
                        Polygon& twin_polygon = get_polygon(twin_edge.polygon);
                        twin_edge.debug_highlight = true;
                        twin_polygon.debug_highlight = true;
                        twin_polygon.split_id_0 = std::min(twin_polygon.split_id_0, twin_edge_idx.i);
                        twin_polygon.split_id_1 = std::max(twin_polygon.split_id_1, twin_edge_idx.i);
                    }
                }
            }
            curr_edge_idx = next_edge_index;
        } while (curr_edge_idx != first_edge_idx);
    }

}
