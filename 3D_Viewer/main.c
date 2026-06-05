#include <GL/glut.h>
#include <GL/glu.h>
#include <stdio.h>
#include <math.h>
#include "camera.h"
#include "utils.h"

// Глобальные переменные
Camera camera;
int win_width = 1024, win_height = 768;
int last_time = 0;
int mouse_down = 0, last_mx = 0, last_my = 0;

// Функция отрисовки куба (immediate mode)
void draw_cube() {
    glBegin(GL_TRIANGLES);
    // Передняя грань
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f, -0.5f);
    // Задняя грань
    glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
    // Левая
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f, -0.5f);
    // Правая
    glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f( 0.5f,  0.5f, -0.5f);
    // Верхняя
    glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
    // Нижняя
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glEnd();
}

void draw_grid() {
    glColor3f(0.5f, 0.5f, 0.5f);   // серый цвет
    glBegin(GL_LINES);
    // Линии вдоль X
    for (int i = -10; i <= 10; i++) {
        glVertex3f((float)i, -0.5f, -10.0f);
        glVertex3f((float)i, -0.5f,  10.0f);
    }
    // Линии вдоль Z
    for (int i = -10; i <= 10; i++) {
        glVertex3f(-10.0f, -0.5f, (float)i);
        glVertex3f( 10.0f, -0.5f, (float)i);
    }
    glEnd();
}

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

    // Рисуем сетку и куб без освещения, чтобы цвета были чёткими
    glDisable(GL_LIGHTING);
    draw_grid();                      // <-- сетка добавлена
    glColor3f(1.0f, 0.0f, 0.0f);     // красный куб
    draw_cube();
    glEnable(GL_LIGHTING);

    glutSwapBuffers();
}

void reshape(int w, int h) {
    win_width = w; win_height = h;
    glViewport(0, 0, w, h);
}

void keyboard(unsigned char key, int x, int y) {
    int dt = glutGet(GLUT_ELAPSED_TIME) - last_time;
    if (dt > 50) dt = 50;              // ограничение резкого скачка
    last_time = glutGet(GLUT_ELAPSED_TIME);
    camera_process_key(&camera, key, dt);

    if(key == 27) exit(0);             // Esc для выхода
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if(button == GLUT_LEFT_BUTTON) {
        mouse_down = (state == GLUT_DOWN);
        last_mx = x; last_my = y;
    }
}

void motion(int x, int y) {
    if(mouse_down) {
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

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(win_width, win_height);
    glutCreateWindow("3D Visualizer - Polyx");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Инициализация камеры
    camera_init(&camera, (vec3){0, 2, 5});
    camera_update(&camera);
    last_time = glutGet(GLUT_ELAPSED_TIME);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(0, timer, 0);

    printf("=== 3D Visualizer ===\n");
    printf("WASD + Q/E - move camera\n");
    printf("Mouse - rotate camera\n");

    glutMainLoop();
    return 0;
}