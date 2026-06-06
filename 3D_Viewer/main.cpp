#include <GL/glut.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

#include "mesh.h"
#include "camera.h"
#include "lights.h"
#include "texture.h"
#include "screenshot.h"
#include "utils.h"
#include "Lists.h"

// ImGui
#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"

// ---------- Глобальные переменные ----------
Pt g_head = NULL;
Pt g_tail = NULL;
Pt g_selected = NULL;

Camera camera;
int win_width = 1024, win_height = 768;
int last_time = 0;
bool show_grid = true, show_arrows = false;   // изменено на bool
bool move_grid_mode = false;                  // изменено на bool
bool clip_enabled = false;                    // изменено на bool
int mouse_down = 0, last_mx = 0, last_my = 0;

float grid_y = 0.0f;
double clip_plane[4] = {0, 1, 0, 0};

// ---------- Вспомогательные функции ----------
void add_mesh_from_file(const char* filename) {
    Mesh new_mesh = mesh_load_obj(filename);
    if (new_mesh.num_vertices == 0) {
        printf("Failed to load %s\n", filename);
        return;
    }
    new_mesh.name = strdup(filename);
    new_mesh.position = (vec3){0,0,0};
    new_mesh.rotation = (vec3){0,0,0};
    new_mesh.scale = (vec3){1,1,1};
    new_mesh.color = (vec3){1,1,1};
    float minY = new_mesh.vertices[0].y;
    for (int i = 1; i < new_mesh.num_vertices; i++)
        if (new_mesh.vertices[i].y < minY) minY = new_mesh.vertices[i].y;
    new_mesh.position.y = -minY;
    AddElemToList(&g_head, &g_tail, &new_mesh);
    if (g_selected == NULL) g_selected = g_head;
    printf("Added: %s (v=%d, f=%d)\n", filename, new_mesh.num_vertices, new_mesh.num_faces);
}

void delete_selected() {
    if (g_selected == NULL) {
        printf("No object to delete.\n");
        return;
    }

    Pt to_delete = g_selected;

    if (to_delete->PNext != NULL) {
        g_selected = to_delete->PNext;
    } else {
        g_selected = g_head;
        if (g_selected == to_delete) g_selected = NULL;
    }

    if (to_delete == g_head) {
        g_head = to_delete->PNext;
        if (g_head == NULL) g_tail = NULL;
    } else {
        Pt prev = g_head;
        while (prev && prev->PNext != to_delete) prev = prev->PNext;
        if (prev) {
            prev->PNext = to_delete->PNext;
            if (to_delete->PNext == NULL) g_tail = prev;
        }
    }

    free(to_delete->ObjData.name);
    mesh_free(&to_delete->ObjData);
    free(to_delete);

    if (g_selected) {
        printf("Deleted. Now selected: %s\n", g_selected->ObjData.name);
    } else {
        printf("Deleted. Scene is empty.\n");
    }
}

void duplicate_selected() {
    if (!g_selected) return;
    Mesh* orig = &g_selected->ObjData;
    Mesh copy = mesh_load_obj(orig->name);
    if (copy.num_vertices == 0) return;
    copy.name = strdup(orig->name);
    copy.position = orig->position;
    copy.rotation = orig->rotation;
    copy.scale = orig->scale;
    copy.color = orig->color;
    copy.texture_id = 0;
    copy.position.x += 1.5f;
    AddElemToList(&g_head, &g_tail, &copy);
    printf("Duplicated: %s\n", orig->name);
}

void save_scene(const char* filename) {
    char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "Scenes/%s", filename);
    FILE* f = fopen(fullpath, "w");
    if (!f) { perror("save_scene"); return; }
    Pt cur = g_head;
    while (cur) {
        Mesh* m = &cur->ObjData;
        fprintf(f, "obj %s\n", m->name);
        fprintf(f, "pos %f %f %f\n", m->position.x, m->position.y, m->position.z);
        fprintf(f, "rot %f %f %f\n", m->rotation.x, m->rotation.y, m->rotation.z);
        fprintf(f, "scale %f %f %f\n", m->scale.x, m->scale.y, m->scale.z);
        fprintf(f, "color %f %f %f\n", m->color.x, m->color.y, m->color.z);
        cur = cur->PNext;
    }
    fclose(f);
    printf("Scene saved to %s\n", fullpath);
}

void load_scene(const char* filename) {
    char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "Scenes/%s", filename);
    FILE* f = fopen(fullpath, "r");
    if (!f) { perror("load_scene"); return; }

    Pt cur = g_head;
    while (cur) {
        Pt next = cur->PNext;
        free(cur->ObjData.name);
        mesh_free(&cur->ObjData);
        free(cur);
        cur = next;
    }
    g_head = g_tail = g_selected = NULL;

    char line[512];
    Mesh m;
    int reading = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "obj ", 4) == 0) {
            if (reading) {
                AddElemToList(&g_head, &g_tail, &m);
            }
            char fname[256];
            sscanf(line, "obj %255s", fname);
            m = mesh_load_obj(fname);
            if (m.num_vertices == 0) {
                printf("Failed to load %s from scene\n", fname);
                reading = 0;
                continue;
            }
            m.name = strdup(fname);
            m.position = (vec3){0,0,0};
            m.rotation = (vec3){0,0,0};
            m.scale = (vec3){1,1,1};
            m.color = (vec3){1,1,1};
            reading = 1;
        } else if (reading) {
            if (strncmp(line, "pos ", 4) == 0)
                sscanf(line, "pos %f %f %f", &m.position.x, &m.position.y, &m.position.z);
            else if (strncmp(line, "rot ", 4) == 0)
                sscanf(line, "rot %f %f %f", &m.rotation.x, &m.rotation.y, &m.rotation.z);
            else if (strncmp(line, "scale ", 6) == 0)
                sscanf(line, "scale %f %f %f", &m.scale.x, &m.scale.y, &m.scale.z);
            else if (strncmp(line, "color ", 6) == 0)
                sscanf(line, "color %f %f %f", &m.color.x, &m.color.y, &m.color.z);
        }
    }
    if (reading) AddElemToList(&g_head, &g_tail, &m);
    g_selected = g_head;
    fclose(f);
    printf("Scene loaded from %s\n", fullpath);
}

// ---------- Рисование ----------
void draw_grid() {
    glDisable(GL_LIGHTING);
    glColor3f(0.6f, 0.6f, 0.6f);
    glBegin(GL_LINES);
    int limit = 100;
    for (int i = -limit; i <= limit; i++) {
        glVertex3f((float)i, grid_y, (float)-limit);
        glVertex3f((float)i, grid_y, (float) limit);
        glVertex3f((float)-limit, grid_y, (float)i);
        glVertex3f((float) limit, grid_y, (float)i);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void draw_arrows() {
    glDisable(GL_LIGHTING);
    glColor3f(1,0,0);
    glBegin(GL_LINES);
    glVertex3f(0,0,0); glVertex3f(1.5,0,0);
    glEnd();
    glColor3f(0,1,0);
    glBegin(GL_LINES);
    glVertex3f(0,0,0); glVertex3f(0,1.5,0);
    glEnd();
    glColor3f(0,0,1);
    glBegin(GL_LINES);
    glVertex3f(0,0,0); glVertex3f(0,0,1.5);
    glEnd();
    glEnable(GL_LIGHTING);
}

// Объявляем коллбэки как extern "C" для совместимости с GLUT
extern "C" {
    void display();
    void reshape(int w, int h);
    void keyboard(unsigned char key, int x, int y);
    void special_keys(int key, int x, int y);
    void mouse(int button, int state, int x, int y);
    void motion(int x, int y);
    void timer(int value);
}

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

    if (clip_enabled) {
        glEnable(GL_CLIP_PLANE0);
        glClipPlane(GL_CLIP_PLANE0, clip_plane);
    } else {
        glDisable(GL_CLIP_PLANE0);
    }

    if (show_grid) draw_grid();
    if (show_arrows) draw_arrows();

    Pt cur = g_head;
    while (cur) {
        Mesh* m = &cur->ObjData;
        glPushMatrix();
        glTranslatef(m->position.x, m->position.y, m->position.z);
        glRotatef(m->rotation.x, 1,0,0);
        glRotatef(m->rotation.y, 0,1,0);
        glRotatef(m->rotation.z, 0,0,1);
        glScalef(m->scale.x, m->scale.y, m->scale.z);
        mesh_draw(m, (mat4){0}, (mat4){0});
        glPopMatrix();
        cur = cur->PNext;
    }

    if (g_selected) {
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

    // ---------- ImGui ----------
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("3D Viewer Controls", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Objects in scene:");
    if (ImGui::BeginListBox("##list", ImVec2(200, 120))) {
        Pt cur2 = g_head;
        while (cur2) {
            bool is_selected = (cur2 == g_selected);
            if (ImGui::Selectable(cur2->ObjData.name, is_selected)) {
                g_selected = cur2;
            }
            cur2 = cur2->PNext;
        }
        ImGui::EndListBox();
    }

    if (ImGui::Button("Delete selected")) delete_selected();
    ImGui::SameLine();
    if (ImGui::Button("Duplicate selected")) duplicate_selected();
    if (ImGui::Button("Add OBJ...")) {
        char fname[256];
        printf("Enter OBJ filename (in Objects/): ");
        if (scanf("%255s", fname) == 1) {
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "Objects/%s", fname);
            add_mesh_from_file(fullpath);
        }
        while (getchar() != '\n');
    }
    if (ImGui::Button("Save scene")) {
        char name[256];
        printf("Scene name: ");
        if (scanf("%255s", name) == 1) {
            char fname[512];
            snprintf(fname, sizeof(fname), "%s.txt", name);
            save_scene(fname);
        }
        while (getchar() != '\n');
    }
    ImGui::SameLine();
    if (ImGui::Button("Load scene")) {
        char name[256];
        printf("Scene name to load: ");
        if (scanf("%255s", name) == 1) {
            char fname[512];
            snprintf(fname, sizeof(fname), "%s.txt", name);
            load_scene(fname);
        }
        while (getchar() != '\n');
    }
    ImGui::Separator();

    if (g_selected) {
        ImGui::Text("Selected: %s", g_selected->ObjData.name);
        ImGui::DragFloat3("Position", &g_selected->ObjData.position.x, 0.1f);
        ImGui::DragFloat3("Rotation", &g_selected->ObjData.rotation.x, 1.0f);
        ImGui::DragFloat3("Scale", &g_selected->ObjData.scale.x, 0.05f);
        ImGui::ColorEdit3("Color", &g_selected->ObjData.color.x);
        if (ImGui::Button("Load texture")) {
            if (g_selected->ObjData.texture_id)
                glDeleteTextures(1, &g_selected->ObjData.texture_id);
            g_selected->ObjData.texture_id = load_texture("Objects/texture.jpg");
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove texture")) {
            if (g_selected->ObjData.texture_id) {
                glDeleteTextures(1, &g_selected->ObjData.texture_id);
                g_selected->ObjData.texture_id = 0;
            }
        }
    } else {
        ImGui::Text("No object selected.");
    }
    ImGui::Separator();
    ImGui::Checkbox("Show grid", &show_grid);
    ImGui::Checkbox("Show arrows", &show_arrows);
    ImGui::DragFloat("Grid Y", &grid_y, 0.05f);
    ImGui::Checkbox("Clipping plane", &clip_enabled);
    ImGui::DragFloat("Clip offset", (float*)&clip_plane[3], 0.05f);
    ImGui::Text("Camera pos: %.2f %.2f %.2f", camera.pos.x, camera.pos.y, camera.pos.z);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glutSwapBuffers();
}

void reshape(int w, int h) {
    win_width = w; win_height = h;
    glViewport(0, 0, w, h);
    ImGui_ImplGLUT_ReshapeFunc(w, h);
}

void keyboard(unsigned char key, int x, int y) {
    ImGui_ImplGLUT_KeyboardFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    if (key >= 1 && key <= 26 && key != 9) {
        switch (key) {
            case 19: {
                char name[256];
                printf("Enter scene name (without .txt): ");
                if (scanf("%255s", name) == 1) {
                    char filename[512];
                    snprintf(filename, sizeof(filename), "%s.txt", name);
                    save_scene(filename);
                } else {
                    printf("Invalid name.\n");
                    while (getchar() != '\n');
                }
                glutPostRedisplay();
                return;
            }
            case 15: {
                char name[256];
                printf("Enter scene name to load (without .txt): ");
                if (scanf("%255s", name) == 1) {
                    char filename[512];
                    snprintf(filename, sizeof(filename), "%s.txt", name);
                    load_scene(filename);
                } else {
                    printf("Invalid name.\n");
                    while (getchar() != '\n');
                }
                glutPostRedisplay();
                return;
            }
            default:
                return;
        }
    }

    int dt = glutGet(GLUT_ELAPSED_TIME) - last_time;
    if (dt > 50) dt = 50;
    last_time = glutGet(GLUT_ELAPSED_TIME);
    camera_process_key(&camera, key, dt);

    switch (key) {
        case 'm': show_grid = !show_grid; show_arrows = !show_arrows; break;
        case '+': case '=':
            if (g_selected) {
                g_selected->ObjData.scale.x += 0.1f;
                g_selected->ObjData.scale.y += 0.1f;
                g_selected->ObjData.scale.z += 0.1f;
            }
            break;
        case '-':
            if (g_selected) {
                g_selected->ObjData.scale.x -= 0.1f;
                g_selected->ObjData.scale.y -= 0.1f;
                g_selected->ObjData.scale.z -= 0.1f;
            }
            break;
        case 'r':
            camera.yaw = -90; camera.pitch = 0; camera_update(&camera);
            break;
        case 127: delete_selected(); break;
        case 't':
            if (g_selected) {
                if (g_selected->ObjData.texture_id) glDeleteTextures(1, &g_selected->ObjData.texture_id);
                g_selected->ObjData.texture_id = load_texture("Objects/texture.jpg");
            }
            break;
        case 'T':
            if (g_selected && g_selected->ObjData.texture_id) {
                glDeleteTextures(1, &g_selected->ObjData.texture_id);
                g_selected->ObjData.texture_id = 0;
            }
            break;
        case 'o': {
            char fname[256];
            printf("Enter OBJ filename (in Objects/): ");
            scanf("%255s", fname);
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "Objects/%s", fname);
            add_mesh_from_file(fullpath);
            break;
        }
        case 'x': if (g_selected) g_selected->ObjData.rotation.x += 5.0f; break;
        case 'X': if (g_selected) g_selected->ObjData.rotation.x -= 5.0f; break;
        case 'z': if (g_selected) g_selected->ObjData.rotation.z += 5.0f; break;
        case 'Z': if (g_selected) g_selected->ObjData.rotation.z -= 5.0f; break;
        case 'R': if (g_selected) {
            g_selected->ObjData.rotation.x = 0;
            g_selected->ObjData.rotation.y = 0;
            g_selected->ObjData.rotation.z = 0;
        } break;
        case 'k': if (g_selected) g_selected->ObjData.scale.x -= 0.1f; break;
        case 'K': if (g_selected) g_selected->ObjData.scale.x += 0.1f; break;
        case 'l': if (g_selected) g_selected->ObjData.scale.y -= 0.1f; break;
        case 'L': if (g_selected) g_selected->ObjData.scale.y += 0.1f; break;
        case ';': if (g_selected) g_selected->ObjData.scale.z += 0.1f; break;
        case ':': if (g_selected) g_selected->ObjData.scale.z -= 0.1f; break;
        case 'u': duplicate_selected(); break;
        case '\t':
            if (!g_head) { printf("No objects.\n"); break; }
            if (!g_selected) g_selected = g_head;
            else if (g_selected->PNext) g_selected = g_selected->PNext;
            else g_selected = g_head;
            printf("Selected: %s\n", g_selected->ObjData.name);
            break;
        case 'g':
            move_grid_mode = !move_grid_mode;
            printf("Move grid mode: %s\n", move_grid_mode ? "ON" : "OFF");
            break;
        case 'c':
            clip_enabled = !clip_enabled;
            printf("Clipping %s\n", clip_enabled ? "enabled" : "disabled");
            break;
        case 27: exit(0);
        default: break;
    }
    glutPostRedisplay();
}

void special_keys(int key, int x, int y) {
    ImGui_ImplGLUT_SpecialFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    if (move_grid_mode) {
        switch (key) {
            case GLUT_KEY_UP: grid_y += 0.1f; break;
            case GLUT_KEY_DOWN: grid_y -= 0.1f; break;
            default: break;
        }
        printf("Grid Y = %.2f\n", grid_y);
        glutPostRedisplay();
        return;
    }

    int mod = glutGetModifiers();
    if (mod == GLUT_ACTIVE_SHIFT && clip_enabled) {
        switch (key) {
            case GLUT_KEY_UP: clip_plane[3] += 0.1f; break;
            case GLUT_KEY_DOWN: clip_plane[3] -= 0.1f; break;
            default: break;
        }
        printf("Clip offset: %.2f\n", clip_plane[3]);
        glutPostRedisplay();
        return;
    }

    if (!g_selected) return;
    switch (key) {
        case GLUT_KEY_UP:    g_selected->ObjData.position.z -= 0.1f; break;
        case GLUT_KEY_DOWN:  g_selected->ObjData.position.z += 0.1f; break;
        case GLUT_KEY_LEFT:  g_selected->ObjData.position.x -= 0.1f; break;
        case GLUT_KEY_RIGHT: g_selected->ObjData.position.x += 0.1f; break;
        case GLUT_KEY_PAGE_UP:   g_selected->ObjData.rotation.y += 5.0f; break;
        case GLUT_KEY_PAGE_DOWN: g_selected->ObjData.rotation.y -= 5.0f; break;
        case GLUT_KEY_HOME:      g_selected->ObjData.position.y += 0.1f; break;
        case GLUT_KEY_END:       g_selected->ObjData.position.y -= 0.1f; break;
        default: break;
    }
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    ImGui_ImplGLUT_MouseFunc(button, state, x, y);
    if (ImGui::GetIO().WantCaptureMouse) return;

    if (button == GLUT_LEFT_BUTTON) {
        mouse_down = (state == GLUT_DOWN);
        last_mx = x; last_my = y;
    }
}

void motion(int x, int y) {
    ImGui_ImplGLUT_MotionFunc(x, y);
    if (ImGui::GetIO().WantCaptureMouse) return;

    if (mouse_down) {
        int dx = x - last_mx;
        int dy = y - last_my;
        camera_process_mouse(&camera, dx, dy);
        last_mx = x; last_my = y;
        glutPostRedisplay();
    }
}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ---------- Главная ----------
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

    // Инициализация ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGLUT_Init();
    ImGui_ImplGLUT_InstallFuncs();
    ImGui_ImplOpenGL3_Init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special_keys);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(0, timer, 0);

    printf("=== 3D Visualizer ===\n");
    printf("Controls: WASD+QE - camera, Mouse - rotate\n");
    printf("GUI window is on top. Use mouse to interact with controls.\n");

    glutMainLoop();
    return 0;
}