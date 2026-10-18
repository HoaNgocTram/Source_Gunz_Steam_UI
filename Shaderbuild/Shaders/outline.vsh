vs.1.1

dcl_position v0

dcl_blendweight v1

dcl_blendindices v2

dcl_normal v3

mov a0.x, v2.
x
m4x3r1.xyz, v0, c[a0.x]
mul r1.xyz, r1.xyz, v1.
x
m3x3r4.xyz, v3, c[a0.x]
mul r4.xyz, r4.xyz, v1.
x

mova0.x, v2.
y
m4x3r0.xyz, v0, c[a0.x]
mad r1.xyz, r0.xyz, v1.y, r1.
xyz
m3x3r0.xyz, v3, c[a0.x]
mad r4.xyz, r0.xyz, v1.y, r4.
xyz

addr2.x, v1.x, v1.
y
addr2.w, c10.x, -r2.
x
mova0.x, v2.
z
m4x3r0.xyz, v0, c[a0.x]
mad r1.xyz, r0.xyz, r2.w, r1.
xyz
m3x3r0.xyz, v3, c[a0.x]
mad r4.xyz, r0.xyz, r2.w, r4.
xyz

movr1.w, c10.
x
m4x3r5.xyz, r1,
c3
m3x3r3.xyz, r4,
c3

dp3r0.x, r3,
r3
rsqr0.x, r0.
x
mulr3.xyz, r3.xyz, r0.
x
madr5.xyz, r3.xyz, c15.x, r5.
xyz

movr5.w, c10.
x
m4x4oPos, r5,
c6
movoD0,
c13
movoFog, c10.x
