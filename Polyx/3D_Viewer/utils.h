#ifndef HOSPITALAPP_UTILS_H
#define HOSPITALAPP_UTILS_H


#include <math.h>

typedef struct { float x, y, z; } vec2;
typedef struct { float x, y, z; } vec3;
typedef struct { float x, y, z, w; } vec4;
typedef struct { float m[4][4]; } mat4;

static inline vec3 vec3_add(vec3 a, vec3 b) { return (vec3){a.x+b.x, a.y+b.y, a.z+b.z}; }
static inline vec3 vec3_sub(vec3 a, vec3 b) { return (vec3){a.x-b.x, a.y-b.y, a.z-b.z}; }
static inline vec3 vec3_mul(float s, vec3 v) { return (vec3){s*v.x, s*v.y, s*v.z}; }
static inline float vec3_dot(vec3 a, vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline vec3 vec3_cross(vec3 a, vec3 b) { return (vec3){a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x}; }
static inline float vec3_len(vec3 v) { return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z); }
static inline vec3 vec3_norm(vec3 v) { float l = vec3_len(v); if(l<1e-6) return v; return (vec3){v.x/l, v.y/l, v.z/l}; }

mat4 mat4_identity(void);
mat4 mat4_translate(float x, float y, float z);
mat4 mat4_rotate_x(float angle);
mat4 mat4_rotate_y(float angle);
mat4 mat4_rotate_z(float angle);
mat4 mat4_scale(float x, float y, float z);
mat4 mat4_perspective(float fov, float aspect, float near, float far);
mat4 mat4_lookat(vec3 eye, vec3 center, vec3 up);
mat4 mat4_mul(mat4 a, mat4 b);
vec4 mat4_mul_vec4(mat4 m, vec4 v);
vec3 mat4_mul_vec3(mat4 m, vec3 v);

#endif