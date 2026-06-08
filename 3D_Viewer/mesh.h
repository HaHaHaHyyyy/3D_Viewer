#ifndef HOSPITALAPP_MESH_H
#define HOSPITALAPP_MESH_H

#include "utils.h"

// Предварительное объявление TNode (определён в Lists.h)
struct TNode;
typedef struct TNode* Pt;   // теперь Pt известен в mesh.h

typedef struct Mesh Mesh;

typedef struct {
    int v1, v2, v3;
    int vt1, vt2, vt3;
    int vn1, vn2, vn3;
} Face;

struct Mesh {
    vec3* vertices;
    vec3* normals;
    vec2* texcoords;
    Face* faces;
    char* name;           // отображаемое имя
    char* filepath;       // путь к OBJ-файлу (используется для загрузки геометрии)
    char* texture_name;
    int num_vertices, num_normals, num_texcoords, num_faces;
    unsigned int texture_id;
    vec3 position, rotation, scale;
    vec3 color;

    Pt parent;    // указатель на группу-родитель (NULL, если не в группе)
    Pt children;  // указатель на первый дочерний элемент
    Pt next;      // следующий элемент в списке детей (для обхода)
    int is_group; // 1, если объект является группой (не содержит геометрии)
};

Mesh mesh_load_obj(const char* filename);
void mesh_draw(Mesh* m, mat4 view, mat4 proj);
void mesh_free(Mesh* m);

#endif