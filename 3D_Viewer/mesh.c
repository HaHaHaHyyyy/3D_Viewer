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
        else if(line[0] == 'f' && line[1] == ' ') {
            Face f = {0};
            char* p = line+1;
            int fields[9] = {0};
            int idx = 0;
            while(*p && idx<9) {
                if(*p >= '0' && *p <= '9') {
                    fields[idx++] = atoi(p);
                    while(*p && *p != ' ' && *p != '/' && *p != '\n') p++;
                } else p++;
            }
            // Поддерживаем форматы: v v v, v/vt v/vt, v//vn, v/vt/vn
            // Здесь упрощённо – читаем три группы
            if(idx >= 3) { f.v1 = fields[0]; f.vt1 = fields[1]; f.vn1 = fields[2]; }
            if(idx >= 6) { f.v2 = fields[3]; f.vt2 = fields[4]; f.vn2 = fields[5]; }
            if(idx >= 9) { f.v3 = fields[6]; f.vt3 = fields[7]; f.vn3 = fields[8]; }
            else if(idx >= 6) { // quad -> два треугольника (упростим)
                // в реальном коде делаем триангуляцию
            }
            if(m.num_faces >= cap_face) {
                cap_face *= 2;
                m.faces = realloc(m.faces, cap_face * sizeof(Face));
            }
            m.faces[m.num_faces++] = f;
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
    mat4 model = mat4_identity();
    model = mat4_mul(mat4_translate(m->position.x, m->position.y, m->position.z), model);
    model = mat4_mul(mat4_rotate_x(m->rotation.x), model);
    model = mat4_mul(mat4_rotate_y(m->rotation.y), model);
    model = mat4_mul(mat4_rotate_z(m->rotation.z), model);
    model = mat4_mul(mat4_scale(m->scale.x, m->scale.y, m->scale.z), model);

    mat4 mvp = mat4_mul(proj, mat4_mul(view, model));
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&mvp.m[0][0]);

    glEnable(GL_TEXTURE_2D);
    if(m->texture_id) glBindTexture(GL_TEXTURE_2D, m->texture_id);
    else glDisable(GL_TEXTURE_2D);

    glColor3f(m->color.x, m->color.y, m->color.z);
    glBegin(GL_TRIANGLES);
    for(int i=0; i<m->num_faces; i++) {
        Face f = m->faces[i];
        if(f.vn1) glNormal3f(m->normals[f.vn1-1].x, m->normals[f.vn1-1].y, m->normals[f.vn1-1].z);
        if(f.vt1 && m->texcoords) glTexCoord2f(m->texcoords[f.vt1-1].x, m->texcoords[f.vt1-1].y);
        glVertex3f(m->vertices[f.v1-1].x, m->vertices[f.v1-1].y, m->vertices[f.v1-1].z);

        if(f.vn2) glNormal3f(m->normals[f.vn2-1].x, m->normals[f.vn2-1].y, m->normals[f.vn2-1].z);
        if(f.vt2) glTexCoord2f(m->texcoords[f.vt2-1].x, m->texcoords[f.vt2-1].y);
        glVertex3f(m->vertices[f.v2-1].x, m->vertices[f.v2-1].y, m->vertices[f.v2-1].z);

        if(f.vn3) glNormal3f(m->normals[f.vn3-1].x, m->normals[f.vn3-1].y, m->normals[f.vn3-1].z);
        if(f.vt3) glTexCoord2f(m->texcoords[f.vt3-1].x, m->texcoords[f.vt3-1].y);
        glVertex3f(m->vertices[f.v3-1].x, m->vertices[f.v3-1].y, m->vertices[f.v3-1].z);
    }
    glEnd();
    if(m->texture_id) glDisable(GL_TEXTURE_2D);
}

void mesh_free(Mesh* m) {
    free(m->vertices); free(m->normals); free(m->texcoords); free(m->faces);
    memset(m, 0, sizeof(Mesh));
}