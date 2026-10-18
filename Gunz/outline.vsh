// Outline shader with GPU skinning
// matIndex contains ABSOLUTE register indices (33, 36, 39...)
// mAllConst[0] = c33, mAllConst[1] = c34, etc.
// So: matIndex=33 -> offset=0, matIndex=36 -> offset=3
// Access: mAllConst[offset+0], mAllConst[offset+1], mAllConst[offset+2]

float4 main(
    float3 pos : POSITION,
    float2 weights : BLENDWEIGHT,
    float3 indices : BLENDINDICES,
    float3 normal : NORMAL,
    float2 uv : TEXCOORD0,
    uniform float4 mWorld[3] : register(c3),
    uniform float4 mViewProj[4] : register(c6),
    uniform float4 mAllConst[64] : register(c33),
    uniform float4 vSilhouette : register(c97)
) : POSITION
{
    // matIndex stores absolute register index (33, 36, 39...)
    // Subtract 33 (ANIMATION_MATRIX_BASE) to get offset into mAllConst
    int b0 = (int) round(indices.x) - 33;
    int b1 = (int) round(indices.y) - 33;
    int b2 = (int) round(indices.z) - 33;

    float w0 = weights.x;
    float w1 = weights.y;
    float w2 = 1.0f - w0 - w1;

    float4 p = float4(pos, 1);
    float4 n = float4(normal, 0);

    // Each bone = 3 consecutive rows in mAllConst
    // b0, b1, b2 are already the starting offsets (0, 3, 6...)
    float3 sp =
        float3(dot(p, mAllConst[b0 + 0]), dot(p, mAllConst[b0 + 1]), dot(p, mAllConst[b0 + 2])) * w0 +
        float3(dot(p, mAllConst[b1 + 0]), dot(p, mAllConst[b1 + 1]), dot(p, mAllConst[b1 + 2])) * w1 +
        float3(dot(p, mAllConst[b2 + 0]), dot(p, mAllConst[b2 + 1]), dot(p, mAllConst[b2 + 2])) * w2;

    float3 sn = normalize(
        float3(dot(n, mAllConst[b0 + 0]), dot(n, mAllConst[b0 + 1]), dot(n, mAllConst[b0 + 2])) * w0 +
        float3(dot(n, mAllConst[b1 + 0]), dot(n, mAllConst[b1 + 1]), dot(n, mAllConst[b1 + 2])) * w1 +
        float3(dot(n, mAllConst[b2 + 0]), dot(n, mAllConst[b2 + 1]), dot(n, mAllConst[b2 + 2])) * w2
    );

    float3 extruded = sp + sn * vSilhouette.x;

    // World (3x4 transposed)
    float4 e4 = float4(extruded, 1);
    float3 wp = float3(dot(e4, mWorld[0]), dot(e4, mWorld[1]), dot(e4, mWorld[2]));

    // ViewProj
    float4 wp4 = float4(wp, 1);
    return float4(
        dot(wp4, mViewProj[0]),
        dot(wp4, mViewProj[1]),
        dot(wp4, mViewProj[2]),
        dot(wp4, mViewProj[3])
    );
}