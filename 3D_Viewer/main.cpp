#include <GL/glut.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

#include "globals.h"
#include "scene.h"
#include "texture_manager.h"
#include "undo.h"
#include "input.h"
#include "ui.h"
#include "camera.h"
#include "lights.h"
#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"

// Определение глобальных переменных
Pt g_head = NULL;
Pt g_tail = NULL;
Pt g_selected = NULL;

Camera camera;
int win_width = 1024, win_height = 768;
int last_time = 0;
bool show_grid = true, show_arrows = false;
bool move_grid_mode = false;
bool clip_enabled = false;
int mouse_down = 0, last_mx = 0, last_my = 0;

float grid_y = 0.0f;
double clip_plane[4] = {0, 1, 0, 0};
bool clip_attach_to_selected = false;

ImVector<TextureEntry> g_textures;
int g_selected_texture_idx = -1;

ImVector<SceneState> undo_states;
int undo_index = -1;

// НЕ НУЖНО объявлять коллбэки здесь – они уже в input.h

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)win_width/win_height, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    vec3 center = vec3_add(camera.pos, camera.front);
    gluLookAt(camera.pos.x, camera.pos.y, camera.pos.z,
              center.x, center.y, center.z,
              camera.up.x, camera.up.y, camera.up.z);

    if (clip_attach_to_selected && g_selected && !g_selected->ObjData.is_group) {
        clip_plane[3] = -g_selected->ObjData.position.y;
    }

    if (clip_enabled) {
        glEnable(GL_CLIP_PLANE0);
        glClipPlane(GL_CLIP_PLANE0, clip_plane);
    } else {
        glDisable(GL_CLIP_PLANE0);
    }

    if (show_grid) draw_grid();
    if (show_arrows) draw_arrows();

    draw_all_objects();

    if (g_selected && !g_selected->ObjData.parent && !g_selected->ObjData.is_group) {
        Mesh* m = &g_selected->ObjData;
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(1,1,1);
        glPushMatrix();
        glTranslatef(m->position.x, m->position.y, m->position.z);
        glRotatef(m->rotation.x, 1,0,0);
        glRotatef(m->rotation.y, 0,1,0);
        glRotatef(m->rotation.z, 0,0,1);
        glScalef(m->scale.x, m->scale.y, m->scale.z);
        mesh_draw(m, (mat4){0}, (mat4){0});
        glPopMatrix();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();
    ImGui::NewFrame();
    draw_ui();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glutSwapBuffers();
}

void reshape(int w, int h) {
    win_width = w; win_height = h;
    glViewport(0, 0, w, h);
    ImGui_ImplGLUT_ReshapeFunc(w, h);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(win_width, win_height);
    glutCreateWindow("3D Visualizer - Polyx");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

#ifdef _WIN32
    _mkdir("Objects");
    _mkdir("Scenes");
    _mkdir("Screenshots");
#else
    mkdir("Objects", 0755);
    mkdir("Scenes", 0755);
    mkdir("Screenshots", 0755);
#endif

    add_mesh_from_file("Objects/cube.obj");
    init_lights((vec3){1,1,1}, 0.3f, 0.7f);
    if (g_selected) g_selected->ObjData.color = (vec3){1.0f, 0.5f, 0.2f};

    camera_init(&camera, (vec3){0, 3, 8});
    camera_update(&camera);
    last_time = glutGet(GLUT_ELAPSED_TIME);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGLUT_Init();
    ImGui_ImplOpenGL3_Init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special_keys);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(0, timer, 0);   // три аргумента!

    printf("=== 3D Visualizer ===\n");
    printf("Controls: WASD+QE - camera, Mouse - rotate\n");
    printf("GUI window is on top. Use mouse to interact with controls.\n");
    printf("Textures: use GUI to load, select and apply textures to objects.\n");
    printf("Grouping: create group, add objects, remove from group.\n");
    printf("Undo: Ctrl+Z\n");
    printf("Clip plane attach: checkbox in GUI\n");

    glutMainLoop();
    return 0;
}