// SPDX-License-Identifier: Apache-2.0

float4x4 g_world_from_local;
float4x4 g_proj_from_local;

struct V2F {
    float4 pos : SV_Position;
    float3 normal : TEXCOORD0;
    float3 world_pos : TEXCOORD1;
    float3 local_normal : TEXCOORD2;
};

[shader("vertex")]
V2F vertex_main(float3 pos: POSITION, float3 normal: NORMAL)
{
    V2F o;
    o.pos = mul(g_proj_from_local, float4(pos, 1.0));
    o.normal = normalize(mul(g_world_from_local, float4(normal, 0.0)).xyz);
    o.world_pos = mul(g_world_from_local, float4(pos, 1.0)).xyz;
    o.local_normal = normal;
    return o;
}

[shader("fragment")]
float4 fragment_main(V2F v)
    : SV_Target
{
    // return float4(abs(v.normal), 1);

    float dp = -dot(normalize(v.normal), normalize(float3(0,-1,1)));
    dp = dp * 0.5 + 0.5;
    dp = dp * dp;

    float3 col = dp;

    return float4(col, 1);
}
