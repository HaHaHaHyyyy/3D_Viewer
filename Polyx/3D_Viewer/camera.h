//
// Created by progr on 30.05.2026.
//

#ifndef HOSPITALAPP_CAMERA_H
#define HOSPITALAPP_CAMERA_H

#include "utils.h"

typedef struct {
    vec3 pos, front, up, right;
    float yaw, pitch;
    float speed;
} Camera;

void camera_init(Camera* cam, vec3 pos);
void camera_update(Camera* cam);
void camera_process_key(Camera* cam, unsigned char key, int dt);
void camera_process_mouse(Camera* cam, int dx, int dy);
mat4 camera_get_view(Camera* cam);

#endif