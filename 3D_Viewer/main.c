#include <GL/glut.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mesh.h"
#include "camera.h"
#include "lights.h"
#include "texture.h"
#include "screenshot.h"
#include "utils.h"

// ---------- Глобальные переменные ----------
Mesh current_mesh;
Camera camera;
int win_width = 1024, win_height = 768;
int last_time = 0;
int show_grid = 1, show_arrows = 0;        // сетка включена по умолчанию
int mouse_down = 0, last_mx = 0, last_my = 0;

// ---------- Рисование сетки (бесконечный пол на y=0) ----------
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

// ---------- Отрисовка стрелок (упрощённые оси) ----------
void draw_arrows() {
    glDisable(GL_LIGHTING);
    // X – красный
    glColor3f(1,0,0);
    glBegin(GL_LINES);
    glVertex3f(0,0,0); glVertex3f(1.5,0,0);
    glEnd();
    // Y – зелёный
    glColor3f(0,1,0);
    glBegin(GL_LINES);
    glVertex3f(0,0,0); glVertex3f(0,1.5,0);
    glEnd();
    // Z – синий
    glColor3f(0,0,1);
    glBegin(GL_LINES);
    glVertex3f(0,0,0); glVertex3f(0,0,1.5);
    glEnd();
    glEnable(GL_LIGHTING);
}

void draw_cube_immediate() {
    glBegin(GL_TRIANGLES);
    // Передняя грань
    glVertex3f(-1, -1, -1); glVertex3f( 1, -1, -1); glVertex3f( 1,  1, -1);
    glVertex3f(-1, -1, -1); glVertex3f( 1,  1, -1); glVertex3f(-1,  1, -1);
    // Задняя
    glVertex3f(-1, -1,  1); glVertex3f( 1, -1,  1); glVertex3f( 1,  1,  1);
    glVertex3f(-1, -1,  1); glVertex3f( 1,  1,  1); glVertex3f(-1,  1,  1);
    // Левая
    glVertex3f(-1, -1, -1); glVertex3f(-1, -1,  1); glVertex3f(-1,  1,  1);
    glVertex3f(-1, -1, -1); glVertex3f(-1,  1,  1); glVertex3f(-1,  1, -1);
    // Правая
    glVertex3f( 1, -1, -1); glVertex3f( 1, -1,  1); glVertex3f( 1,  1,  1);
    glVertex3f( 1, -1, -1); glVertex3f( 1,  1,  1); glVertex3f( 1,  1, -1);
    // Верхняя
    glVertex3f(-1,  1, -1); glVertex3f( 1,  1, -1); glVertex3f( 1,  1,  1);
    glVertex3f(-1,  1, -1); glVertex3f( 1,  1,  1); glVertex3f(-1,  1,  1);
    // Нижняя
    glVertex3f(-1, -1, -1); glVertex3f( 1, -1, -1); glVertex3f( 1, -1,  1);
    glVertex3f(-1, -1, -1); glVertex3f( 1, -1,  1); glVertex3f(-1, -1,  1);
    glEnd();
}

// ---------- Display ----------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Проекция
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)win_width/win_height, 0.1, 100.0);

    // Камера
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    vec3 center = vec3_add(camera.pos, camera.front);
    gluLookAt(camera.pos.x, camera.pos.y, camera.pos.z,
              center.x, center.y, center.z,
              camera.up.x, camera.up.y, camera.up.z);

    // Рисуем неподвижную сетку и стрелки (пол)
    if (show_grid) draw_grid();
    if (show_arrows) draw_arrows();

    // Применяем трансформации объекта (перемещение, поворот, масштаб)
    glPushMatrix();  // сохраняем матрицу камеры
    glTranslatef(current_mesh.position.x, current_mesh.position.y, current_mesh.position.z);
    glRotatef(current_mesh.rotation.x, 1.0f, 0.0f, 0.0f);
    glRotatef(current_mesh.rotation.y, 0.0f, 1.0f, 0.0f);
    glRotatef(current_mesh.rotation.z, 0.0f, 0.0f, 1.0f);
    glScalef(current_mesh.scale.x, current_mesh.scale.y, current_mesh.scale.z);

    // Отрисовка модели
    mesh_draw(&current_mesh, (mat4){0}, (mat4){0});

    glPopMatrix();  // восстанавливаем матрицу камеры

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
        case 'm':                 // переключение режимов
            show_grid = !show_grid;
            show_arrows = !show_arrows;
            break;
        case '+': case '=':       // масштаб объекта
            current_mesh.scale.x += 0.1f;
            current_mesh.scale.y += 0.1f;
            current_mesh.scale.z += 0.1f;
            break;
        case '-':
            current_mesh.scale.x -= 0.1f;
            current_mesh.scale.y -= 0.1f;
            current_mesh.scale.z -= 0.1f;
            break;
        case 'r':                 // сброс поворота камеры
            camera.yaw = -90;
            camera.pitch = 0;
            camera_update(&camera);
            break;
        case 127:                 // Delete – удалить объект
            mesh_free(&current_mesh);
            break;
        case 't':                 // загрузить текстуру
            if (current_mesh.texture_id)
                glDeleteTextures(1, &current_mesh.texture_id);
            current_mesh.texture_id = load_texture("Objects/texture.jpg");
            break;
        case 'T':                 // удалить текстуру
            if (current_mesh.texture_id)
                glDeleteTextures(1, &current_mesh.texture_id);
            current_mesh.texture_id = 0;
            break;
        case 27:                  // Esc – выход
            exit(0);
    }
    glutPostRedisplay();
}

// ---------- Специальные клавиши (стрелки, PgUp/PgDn) ----------
void special_keys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:    current_mesh.position.z -= 0.1f; break;
        case GLUT_KEY_DOWN:  current_mesh.position.z += 0.1f; break;
        case GLUT_KEY_LEFT:  current_mesh.position.x -= 0.1f; break;
        case GLUT_KEY_RIGHT: current_mesh.position.x += 0.1f; break;
        case GLUT_KEY_PAGE_UP:   current_mesh.rotation.y += 5.0f; break;
        case GLUT_KEY_PAGE_DOWN: current_mesh.rotation.y -= 5.0f; break;
    }
    glutPostRedisplay();
}

// ---------- Мышь ----------
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

// ---------- Таймер для непрерывного обновления ----------
void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ---------- Главная функция ----------
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(win_width, win_height);
    glutCreateWindow("3D Visualizer - Polyx");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Загрузка модели (измеряем время)
    clock_t start = clock();
    current_mesh = mesh_load_obj("Objects/cube.obj");
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;
    printf("OBJ load time: %.2f ms\n", elapsed);

    // Сохраняем время в CSV (Excel)
    FILE* log = fopen("times.csv", "a");
    fprintf(log, "%s,%.2f\n", "cube.obj", elapsed);
    fclose(log);

    // Освещение (цвет, интенсивность)
    init_lights((vec3){1,1,1}, 0.3f, 0.7f);

    // Цвет объекта (можно менять через colbObjColor, но здесь константа)
    current_mesh.color = (vec3){1.0f, 0.5f, 0.2f};  // оранжевый
    float minY = current_mesh.vertices[0].y;
    for (int i = 1; i < current_mesh.num_vertices; i++)
        if (current_mesh.vertices[i].y < minY) minY = current_mesh.vertices[i].y;
    current_mesh.position.y = -minY;

    // Камера
    camera_init(&camera, (vec3){0, 3, 8});
    camera_update(&camera);
    last_time = glutGet(GLUT_ELAPSED_TIME);

    // Регистрация callback'ов
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special_keys);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(0, timer, 0);

    printf("=== 3D Visualizer ===\n");
    printf("WASD + Q/E - move camera\n");
    printf("Mouse - rotate camera\n");
    printf("Arrows - move object\n");
    printf("PgUp/PgDn - rotate object\n");
    printf("+/- - scale object\n");
    printf("M - toggle grid/arrows (Edit Mode)\n");
    printf("T - load texture, Shift+T - remove texture\n");
    printf("Delete - remove object\n");
    printf("F12 - save screenshot\n");
    printf("Vertices loaded: %d, Faces loaded: %d\n", current_mesh.num_vertices, current_mesh.num_faces);

    glutMainLoop();
    return 0;
}