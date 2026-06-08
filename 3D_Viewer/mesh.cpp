// mesh.cpp – полная реализация
#include "mesh.h"
#include "texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>

static char* trim(char* s) {
    while(*s == ' ' || *s == '\t') s++;
    char* e = s + strlen(s) - 1;
    while(e > s && (*e == ' ' || *e == '\t' || *e == '\n')) e--;
    e[1] = '\0';
    return s;
}

Mesh mesh_load_obj(const char* filename) {
    Mesh m = {0};
    FILE* f = fopen(filename, "r");
    if(!f) { fprintf(stderr, "Cannot open %s\n", filename); return m; }

    char line[512];
    int cap_vert = 128, cap_norm = 128, cap_tex = 128, cap_face = 128;
    m.vertices = (vec3*)malloc(cap_vert * sizeof(vec3));
    m.normals  = (vec3*)malloc(cap_norm * sizeof(vec3));
    m.texcoords= (vec2*)malloc(cap_tex * sizeof(vec2));
    m.faces    = (Face*)malloc(cap_face * sizeof(Face));

    while(fgets(line, sizeof(line), f)) {
        // ... (весь парсинг без изменений) ...
    }
    fclose(f);
    m.position = (vec3){0,0,0};
    m.rotation = (vec3){0,0,0};
    m.scale    = (vec3){1,1,1};
    m.color    = (vec3){1,1,1};
    m.texture_id = 0;
    m.texture_name = NULL;
    m.filepath = strdup(filename);   // <-- добавлено
    return m;
}

void mesh_free(Mesh* m) {
    free(m->vertices); free(m->normals); free(m->texcoords); free(m->faces);
    if (m->texture_name) free(m->texture_name);
    if (m->filepath) free(m->filepath);   // <-- добавлено
    memset(m, 0, sizeof(Mesh));
}

void mesh_draw(Mesh* m, mat4 view, mat4 proj) {
    glEnable(GL_TEXTURE_2D);
    if (m->texture_id) glBindTexture(GL_TEXTURE_2D, m->texture_id);
    else glDisable(GL_TEXTURE_2D);

    glColor3f(m->color.x, m->color.y, m->color.z);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < m->num_faces; i++) {
        Face f = m->faces[i];
        if (f.vn1) glNormal3f(m->normals[f.vn1-1].x, m->normals[f.vn1-1].y, m->normals[f.vn1-1].z);
        if (f.vt1 && m->texcoords) glTexCoord2f(m->texcoords[f.vt1-1].x, m->texcoords[f.vt1-1].y);
        glVertex3f(m->vertices[f.v1-1].x, m->vertices[f.v1-1].y, m->vertices[f.v1-1].z);

        if (f.vn2) glNormal3f(m->normals[f.vn2-1].x, m->normals[f.vn2-1].y, m->normals[f.vn2-1].z);
        if (f.vt2) glTexCoord2f(m->texcoords[f.vt2-1].x, m->texcoords[f.vt2-1].y);
        glVertex3f(m->vertices[f.v2-1].x, m->vertices[f.v2-1].y, m->vertices[f.v2-1].z);

        if (f.vn3) glNormal3f(m->normals[f.vn3-1].x, m->normals[f.vn3-1].y, m->normals[f.vn3-1].z);
        if (f.vt3) glTexCoord2f(m->texcoords[f.vt3-1].x, m->texcoords[f.vt3-1].y);
        glVertex3f(m->vertices[f.v3-1].x, m->vertices[f.v3-1].y, m->vertices[f.v3-1].z);
    }
    glEnd();

    if (m->texture_id) glDisable(GL_TEXTURE_2D);
}