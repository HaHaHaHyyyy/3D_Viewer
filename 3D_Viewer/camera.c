#include "camera.h"
#include <GL/glut.h>

void camera_init(Camera* cam, vec3 pos) {
    cam->pos = pos;
    cam->front = (vec3){0,0,-1};
    cam->up = (vec3){0,1,0};
    cam->right = (vec3){1,0,0};
    cam->yaw = -90.0f;
    cam->pitch = 0.0f;
    cam->speed = 3.0f;
    camera_update(cam);
}

void camera_update(Camera* cam) {
    vec3 front;
    front.x = cosf(cam->yaw * M_PI/180) * cosf(cam->pitch * M_PI/180);
    front.y = sinf(cam->pitch * M_PI/180);
    front.z = sinf(cam->yaw * M_PI/180) * cosf(cam->pitch * M_PI/180);
    cam->front = vec3_norm(front);
    cam->right = vec3_norm(vec3_cross(cam->front, (vec3){0,1,0}));
    cam->up = vec3_norm(vec3_cross(cam->right, cam->front));
}

void camera_process_key(Camera* cam, unsigned char key, int dt) {
    float vel = cam->speed * dt / 1000.0f;
    if(key == 'w') cam->pos = vec3_add(cam->pos, vec3_mul(vel, cam->front));
    if(key == 's') cam->pos = vec3_sub(cam->pos, vec3_mul(vel, cam->front));
    if(key == 'a') cam->pos = vec3_sub(cam->pos, vec3_mul(vel, cam->right));
    if(key == 'd') cam->pos = vec3_add(cam->pos, vec3_mul(vel, cam->right));
    if(key == 'q') cam->pos = vec3_add(cam->pos, vec3_mul(vel, (vec3){0,1,0}));
    if(key == 'e') cam->pos = vec3_sub(cam->pos, vec3_mul(vel, (vec3){0,1,0}));
}

void camera_process_mouse(Camera* cam, int dx, int dy) {
    cam->yaw += dx * 0.2f;
    cam->pitch -= dy * 0.2f;
    if(cam->pitch > 89.0f) cam->pitch = 89.0f;
    if(cam->pitch < -89.0f) cam->pitch = -89.0f;
    camera_update(cam);
}

mat4 camera_get_view(Camera* cam) {
    return mat4_lookat(cam->pos, vec3_add(cam->pos, cam->front), cam->up);
}