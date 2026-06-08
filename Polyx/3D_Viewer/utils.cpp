// utils.c
#include "utils.h"
#include <string.h>

mat4 mat4_identity() {
    mat4 m = {0};
    for(int i=0;i<4;i++) m.m[i][i] = 1.0f;
    return m;
}
mat4 mat4_translate(float x, float y, float z) {
    mat4 m = mat4_identity();
    m.m[0][3] = x; m.m[1][3] = y; m.m[2][3] = z;
    return m;
}
mat4 mat4_rotate_x(float angle) {
    float c = cosf(angle), s = sinf(angle);
    mat4 m = mat4_identity();
    m.m[1][1] = c; m.m[1][2] = -s;
    m.m[2][1] = s; m.m[2][2] = c;
    return m;
}
mat4 mat4_rotate_y(float angle) {
    float c = cosf(angle), s = sinf(angle);
    mat4 m = mat4_identity();
    m.m[0][0] = c; m.m[0][2] = s;
    m.m[2][0] = -s; m.m[2][2] = c;
    return m;
}
mat4 mat4_rotate_z(float angle) {
    float c = cosf(angle), s = sinf(angle);
    mat4 m = mat4_identity();
    m.m[0][0] = c; m.m[0][1] = -s;
    m.m[1][0] = s; m.m[1][1] = c;
    return m;
}
mat4 mat4_scale(float x, float y, float z) {
    mat4 m = mat4_identity();
    m.m[0][0] = x; m.m[1][1] = y; m.m[2][2] = z;
    return m;
}
mat4 mat4_perspective(float fov, float aspect, float near, float far) {
    float tan_half = tanf(fov/2);
    mat4 m = {0};
    m.m[0][0] = 1.0f/(aspect*tan_half);
    m.m[1][1] = 1.0f/tan_half;
    m.m[2][2] = -(far+near)/(far-near);
    m.m[2][3] = -(2*far*near)/(far-near);
    m.m[3][2] = -1.0f;
    return m;
}
mat4 mat4_lookat(vec3 eye, vec3 center, vec3 up) {
    vec3 f = vec3_norm(vec3_sub(center, eye));
    vec3 r = vec3_norm(vec3_cross(f, up));
    vec3 u = vec3_cross(r, f);
    mat4 m = mat4_identity();
    m.m[0][0] = r.x; m.m[0][1] = r.y; m.m[0][2] = r.z;
    m.m[1][0] = u.x; m.m[1][1] = u.y; m.m[1][2] = u.z;
    m.m[2][0] = -f.x; m.m[2][1] = -f.y; m.m[2][2] = -f.z;
    m.m[0][3] = -vec3_dot(r, eye);
    m.m[1][3] = -vec3_dot(u, eye);
    m.m[2][3] = vec3_dot(f, eye);
    return m;
}
mat4 mat4_mul(mat4 a, mat4 b) {
    mat4 res = {0};
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            for(int k=0;k<4;k++)
                res.m[i][j] += a.m[i][k] * b.m[k][j];
    return res;
}
vec4 mat4_mul_vec4(mat4 m, vec4 v) {
    vec4 r;
    r.x = m.m[0][0]*v.x + m.m[0][1]*v.y + m.m[0][2]*v.z + m.m[0][3]*v.w;
    r.y = m.m[1][0]*v.x + m.m[1][1]*v.y + m.m[1][2]*v.z + m.m[1][3]*v.w;
    r.z = m.m[2][0]*v.x + m.m[2][1]*v.y + m.m[2][2]*v.z + m.m[2][3]*v.w;
    r.w = m.m[3][0]*v.x + m.m[3][1]*v.y + m.m[3][2]*v.z + m.m[3][3]*v.w;
    return r;
}
vec3 mat4_mul_vec3(mat4 m, vec3 v) {
    vec4 v4 = {v.x, v.y, v.z, 1.0f};
    vec4 r = mat4_mul_vec4(m, v4);
    return (vec3){r.x/r.w, r.y/r.w, r.z/r.w};
}