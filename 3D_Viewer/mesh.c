// mesh.c – полная реализация
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
    m.vertices = malloc(cap_vert * sizeof(vec3));
    m.normals  = malloc(cap_norm * sizeof(vec3));
    m.texcoords= malloc(cap_tex * sizeof(vec2));
    m.faces    = malloc(cap_face * sizeof(Face));

    while(fgets(line, sizeof(line), f)) {
        if(line[0] == 'v' && line[1] == ' ') {
            vec3 v; sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            if(m.num_vertices >= cap_vert) {
                cap_vert *= 2;
                m.vertices = realloc(m.vertices, cap_vert * sizeof(vec3));
            }
            m.vertices[m.num_vertices++] = v;
        }
        else if(line[0] == 'v' && line[1] == 't') {
            vec2 vt; sscanf(line, "vt %f %f", &vt.x, &vt.y);
            if(m.num_texcoords >= cap_tex) {
                cap_tex *= 2;
                m.texcoords = realloc(m.texcoords, cap_tex * sizeof(vec2));
            }
            m.texcoords[m.num_texcoords++] = vt;
        }
        else if(line[0] == 'v' && line[1] == 'n') {
            vec3 vn; sscanf(line, "vn %f %f %f", &vn.x, &vn.y, &vn.z);
            if(m.num_normals >= cap_norm) {
                cap_norm *= 2;
                m.normals = realloc(m.normals, cap_norm * sizeof(vec3));
            }
            m.normals[m.num_normals++] = vn;
        }
        else if (line[0] == 'f' && line[1] == ' ') {
    Face f_face = {0};
    char* p = line + 1;
    while (*p == ' ') p++;

    int v[4], vt[4], vn[4];
    int vertices_read = 0;
    memset(v, 0, sizeof(v));
    memset(vt, 0, sizeof(vt));
    memset(vn, 0, sizeof(vn));

    while (*p != '\0' && *p != '\n' && vertices_read < 4) {
        while (*p == ' ') p++;
        if (*p == '\0' || *p == '\n') break;

        // номер вершины
        if (sscanf(p, "%d", &v[vertices_read]) != 1) break;
        while (*p >= '0' && *p <= '9') p++;

        if (*p == '/') {
            p++;
            if (*p == '/') {  // формат v//vn
                p++;
                if (sscanf(p, "%d", &vn[vertices_read]) != 1) vn[vertices_read] = 0;
                while (*p >= '0' && *p <= '9') p++;
            } else {          // формат v/vt или v/vt/vn
                if (sscanf(p, "%d", &vt[vertices_read]) != 1) vt[vertices_read] = 0;
                while (*p >= '0' && *p <= '9') p++;
                if (*p == '/') {
                    p++;
                    if (sscanf(p, "%d", &vn[vertices_read]) != 1) vn[vertices_read] = 0;
                    while (*p >= '0' && *p <= '9') p++;
                }
            }
        }
        vertices_read++;
        while (*p != ' ' && *p != '\0' && *p != '\n') p++;
    }

    // Триангуляция
    if (vertices_read == 4) {
        // треугольник 1-2-3
        if (m.num_faces >= cap_face) {
            cap_face *= 2;
            m.faces = realloc(m.faces, cap_face * sizeof(Face));
        }
        Face f1;
        f1.v1 = v[0]; f1.vt1 = vt[0]; f1.vn1 = vn[0];
        f1.v2 = v[1]; f1.vt2 = vt[1]; f1.vn2 = vn[1];
        f1.v3 = v[2]; f1.vt3 = vt[2]; f1.vn3 = vn[2];
        m.faces[m.num_faces++] = f1;

        // треугольник 1-3-4
        if (m.num_faces >= cap_face) {
            cap_face *= 2;
            m.faces = realloc(m.faces, cap_face * sizeof(Face));
        }
        Face f2;
        f2.v1 = v[0]; f2.vt1 = vt[0]; f2.vn1 = vn[0];
        f2.v2 = v[2]; f2.vt2 = vt[2]; f2.vn2 = vn[2];
        f2.v3 = v[3]; f2.vt3 = vt[3]; f2.vn3 = vn[3];
        m.faces[m.num_faces++] = f2;
    } else if (vertices_read == 3) {
        if (m.num_faces >= cap_face) {
            cap_face *= 2;
            m.faces = realloc(m.faces, cap_face * sizeof(Face));
        }
        Face f1;
        f1.v1 = v[0]; f1.vt1 = vt[0]; f1.vn1 = vn[0];
        f1.v2 = v[1]; f1.vt2 = vt[1]; f1.vn2 = vn[1];
        f1.v3 = v[2]; f1.vt3 = vt[2]; f1.vn3 = vn[2];
        m.faces[m.num_faces++] = f1;
    } else {
        fprintf(stderr, "Face with %d vertices skipped\n", vertices_read);
    }
}
    }
    fclose(f);
    m.position = (vec3){0,0,0};
    m.rotation = (vec3){0,0,0};
    m.scale    = (vec3){1,1,1};
    m.color    = (vec3){1,1,1};
    m.texture_id = 0;
    return m;
}

void mesh_draw(Mesh* m, mat4 view, mat4 proj) {
    // Игнорируем переданные матрицы – используем текущие матрицы OpenGL,
    // которые уже установлены в display через gluLookAt.
    // Просто рисуем треугольники.

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
void mesh_free(Mesh* m) {
    free(m->vertices); free(m->normals); free(m->texcoords); free(m->faces);
    memset(m, 0, sizeof(Mesh));
}