// lights.c
#include "lights.h"
#include <GL/gl.h>

static vec3 light_color = {1,1,1};
static float ambient_intensity = 0.3f;
static float diffuse_intensity = 0.7f;

void init_lights(vec3 color, float ambient, float diffuse) {
    light_color = color;
    ambient_intensity = ambient;
    diffuse_intensity = diffuse;
    GLfloat light_pos[] = {2.0f, 5.0f, 3.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    set_light_intensity(ambient, diffuse);
    set_light_color(color);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void set_light_color(vec3 color) {
    GLfloat col[4] = {color.x, color.y, color.z, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);
}

void set_light_intensity(float ambient, float diffuse) {
    GLfloat amb[4] = {ambient, ambient, ambient, 1.0f};
    GLfloat dif[4] = {diffuse, diffuse, diffuse, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
}