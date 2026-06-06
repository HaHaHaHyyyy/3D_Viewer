#ifndef HOSPITALAPP_MESH_H
#define HOSPITALAPP_MESH_H

#include "utils.h"

typedef struct {
    int v1, v2, v3;
    int vt1, vt2, vt3;
    int vn1, vn2, vn3;
} Face;

typedef struct {
    vec3* vertices;
    vec3* normals;
    vec2* texcoords;
    Face* faces;
    int num_vertices, num_normals, num_texcoords, num_faces;
    unsigned int texture_id;
    vec3 position, rotation, scale;
    vec3 color;
    char* name;   // имя файла модели (например, "cube.obj")
} Mesh;

Mesh mesh_load_obj(const char* filename);
void mesh_draw(Mesh* m, mat4 view, mat4 proj);
void mesh_free(Mesh* m);

#endif