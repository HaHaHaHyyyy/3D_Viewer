#include <GL/glut.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mesh.h"
#include "camera.h"
#include "lights.h"
#include "texture.h"
#include "screenshot.h"
#include "utils.h"
#include "Lists.h"

// ---------- Глобальные переменные ----------
Pt g_head = NULL;        // голова списка объектов
Pt g_tail = NULL;        // хвост списка
Pt g_selected = NULL;    // текущий выбранный объект

Camera camera;
int win_width = 1024, win_height = 768;
int last_time = 0;
int show_grid = 1, show_arrows = 0;
int mouse_down = 0, last_mx = 0, last_my = 0;

// ---------- Вспомогательные функции для работы со списком ----------
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
    // автоматически ставим на пол (нижняя грань на y=0)
    float minY = new_mesh.vertices[0].y;
    for (int i = 1; i < new_mesh.num_vertices; i++)
        if (new_mesh.vertices[i].y < minY) minY = new_mesh.vertices[i].y;
    new_mesh.position.y = -minY;
    AddElemToList(&g_head, &g_tail, &new_mesh);
    if (g_selected == NULL) g_selected = g_head;
    printf("Added: %s (vertices=%d, faces=%d)\n", filename, new_mesh.num_vertices, new_mesh.num_faces);
}

void delete_selected() {
    if (g_selected == NULL) return;
    DelElemFromList(&g_head, &g_tail, &g_selected->ObjData);
    // освобождаем память имени
    free(g_selected->ObjData.name);
    mesh_free(&g_selected->ObjData);
    // выбираем следующий объект
    g_selected = g_head;
    if (g_selected == NULL) printf("No objects left.\n");
}

void save_scene(const char* filename) {
    FILE* f = fopen(filename, "w");
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
    printf("Scene saved to %s\n", filename);
}

void load_scene(const char* filename) {
    // очищаем текущую сцену
    while (g_head) {
        Pt tmp = g_head;
        g_head = g_head->PNext;
        free(tmp->ObjData.name);
        mesh_free(&tmp->ObjData);
        free(tmp);
    }
    g_head = g_tail = g_selected = NULL;

    FILE* f = fopen(filename, "r");
    if (!f) { perror("load_scene"); return; }
    char line[512];
    Mesh m;
    memset(&m, 0, sizeof(Mesh));
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "obj ", 4) == 0) {
            // если предыдущий объект был загружен, добавляем его в список
            if (m.name != NULL) {
                AddElemToList(&g_head, &g_tail, &m);
                // очищаем для следующего
                memset(&m, 0, sizeof(Mesh));
            }
            char fname[256];
            sscanf(line, "obj %255s", fname);
            m = mesh_load_obj(fname);
            if (m.num_vertices == 0) {
                printf("Failed to load %s from scene file\n", fname);
                continue;
            }
            m.name = strdup(fname);
        } else if (strncmp(line, "pos ", 4) == 0) {
            sscanf(line, "pos %f %f %f", &m.position.x, &m.position.y, &m.position.z);
        } else if (strncmp(line, "rot ", 4) == 0) {
            sscanf(line, "rot %f %f %f", &m.rotation.x, &m.rotation.y, &m.rotation.z);
        } else if (strncmp(line, "scale ", 6) == 0) {
            sscanf(line, "scale %f %f %f", &m.scale.x, &m.scale.y, &m.scale.z);
        } else if (strncmp(line, "color ", 6) == 0) {
            sscanf(line, "color %f %f %f", &m.color.x, &m.color.y, &m.color.z);
        }
    }
    // добавляем последний объект
    if (m.name != NULL) {
        AddElemToList(&g_head, &g_tail, &m);
    }
    g_selected = g_head;
    fclose(f);
    printf("Scene loaded from %s\n", filename);
}

// ---------- Рисование сетки и стрелок (те же, что были) ----------
void draw_grid() {
    glDisable(GL_LIGHTING);
    glColor3f(0.6f, 0.6f, 0.6f);
    glBegin(GL_LINES);
    int limit = 100;
    for (int i = -limit; i <= limit; i++) {
        glVertex3f((float)i, 0.0f, (float)-limit);
        glVertex3f((float)i, 0.0f, (float) limit);
        glVertex3f((float)-limit, 0.0f, (float)i);
        glVertex3f((float) limit, 0.0f, (float)i);
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

// ---------- Display ----------
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

    if (show_grid) draw_grid();
    if (show_arrows) draw_arrows();

    // Отрисовка всех объектов
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

    // Рисуем рамку вокруг выбранного объекта
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

    glutSwapBuffers();
}

// ---------- Изменение размера окна ----------
void reshape(int w, int h) {
    win_width = w; win_height = h;
    glViewport(0, 0, w, h);
}

// ---------- Клавиатура ----------
void keyboard(unsigned char key, int x, int y) {
    int dt = glutGet(GLUT_ELAPSED_TIME) - last_time;
    if (dt > 50) dt = 50;
    last_time = glutGet(GLUT_ELAPSED_TIME);
    camera_process_key(&camera, key, dt);

    switch (key) {
        case 'm':
            show_grid = !show_grid;
            show_arrows = !show_arrows;
            break;
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
            camera.yaw = -90;
            camera.pitch = 0;
            camera_update(&camera);
            break;
        case 127: // Delete
            delete_selected();
            break;
        case 't':
            if (g_selected) {
                if (g_selected->ObjData.texture_id)
                    glDeleteTextures(1, &g_selected->ObjData.texture_id);
                g_selected->ObjData.texture_id = load_texture("Objects/texture.jpg");
            }
            break;
        case 'T':
            if (g_selected && g_selected->ObjData.texture_id) {
                glDeleteTextures(1, &g_selected->ObjData.texture_id);
                g_selected->ObjData.texture_id = 0;
            }
            break;
        case 'o':
            {
                char fname[256];
                printf("Enter OBJ filename (in Objects/): ");
                scanf("%255s", fname);
                char fullpath[512];
                snprintf(fullpath, sizeof(fullpath), "Objects/%s", fname);
                add_mesh_from_file(fullpath);
            }
            break;

        case 'x':   // вращение вокруг X на +5 градусов
            if (g_selected)
                g_selected->ObjData.rotation.x += 5.0f;
            break;
        case 'X':   // вращение вокруг X на -5 градусов (Shift+X)
            if (g_selected)
                g_selected->ObjData.rotation.x -= 5.0f;
            break;
        case 'z':   // вращение вокруг Z на +5 градусов
            if (g_selected)
                g_selected->ObjData.rotation.z += 5.0f;
            break;
        case 'Z':   // вращение вокруг Z на -5 градусов (Shift+Z)
            if (g_selected)
                g_selected->ObjData.rotation.z -= 5.0f;
            break;
        case 'R':   // сброс вращения выбранного объекта (Shift+r)
            if (g_selected) {
                g_selected->ObjData.rotation.x = 0;
                g_selected->ObjData.rotation.y = 0;
                g_selected->ObjData.rotation.z = 0;
            }
            break;

        case '\t': // Tab
            if (g_selected && g_selected->PNext)
                g_selected = g_selected->PNext;
            else if (g_selected)
                g_selected = g_head;
            if (g_selected)
                printf("Selected: %s\n", g_selected->ObjData.name);
            break;
        case 27:
            exit(0);
    }
    glutPostRedisplay();
}

void special_keys(int key, int x, int y) {
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
    }
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        mouse_down = (state == GLUT_DOWN);
        last_mx = x; last_my = y;
    }
}

void motion(int x, int y) {
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

    add_mesh_from_file("Objects/cube.obj");

    init_lights((vec3){1,1,1}, 0.3f, 0.7f);
    if (g_selected) g_selected->ObjData.color = (vec3){1.0f, 0.5f, 0.2f};

    camera_init(&camera, (vec3){0, 3, 8});
    camera_update(&camera);
    last_time = glutGet(GLUT_ELAPSED_TIME);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special_keys);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(0, timer, 0);

    printf("=== 3D Visualizer ===\n");
    printf("WASD+QE - camera, Mouse - rotate\n");
    printf("Arrows - move selected object\n");
    printf("PgUp/PgDn - rotate Y\n");
    printf("X - rotate X\n");
    printf("Z - rotate Z\n");
    printf("Home/End - move up/down\n");
    printf("+/- - scale\n");
    printf("M - grid/arrows\n");
    printf("T - load texture, Shift+T - remove\n");
    printf("Delete - delete object\n");
    printf("O - add OBJ (console)\n");
    printf("Tab - switch selection\n");
    printf("Ctrl+S - save scene, Ctrl+O - load scene (not implemented)\n");
    printf("Esc - exit\n");

    glutMainLoop();
    return 0;
}