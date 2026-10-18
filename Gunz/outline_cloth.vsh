// outline_cloth_only.vsh
// Cloth-only outline vertex shader (VS 1.1 compatible)
// c1.x = outline thickness
// c2 = outline color (rgba)
// c3 = WORLD_MATRIX (m4x3) – 옷의 월드 변환
// c6 = VIEW_PROJECTION_MATRIX (m4x4)
// c10.x = 1.0 (사용 편의를 위해)

vs.1.1

dcl_position    v0
dcl_normal      v3
dcl_texcoord0   v4

// world-space position
m4x3 r4.xyz, v0, c3
mov r4.w, c10.x

// world-space normal
m3x3 r3.xyz, v3, c3

// offset along normal
mul r1.xyz, r3.xyz, c1.x

// add offset to world position
add r4.xyz, r4.xyz, r1.xyz

// transform to clip space
m4x4 oPos, r4, c6

// output solid outline color
mov oD0.xyz, c2.xyz
mov oD0.w, c2.w

// pass-through texcoord
mov oT0.xy, v4

// pass-through fog
mov oFog, c10.x
