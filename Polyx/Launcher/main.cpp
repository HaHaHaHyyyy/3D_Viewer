#include <iostream>
#include <filesystem>
#include "SceneScanner.h"
#include "LauncherUtils.h"

// ImGui
#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"

#include <GL/glut.h>
#include <GL/glu.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace fs = std::filesystem;

// ---------- Глобальные переменные ----------
std::vector<SceneInfo> g_scenes;
int g_selected_scene = -1;
bool g_showAboutWindow = false;
GLuint g_developerTexture = 0;
// ---------- Функции GLUT ----------
GLuint LoadTexture(const char* filename)
{
    int width;
    int height;
    int channels;

    unsigned char* data =
        stbi_load(
            filename,
            &width,
            &height,
            &channels,
            4);

    if (!data)
    {
        std::cout
            << "Failed to load image: "
            << filename
            << std::endl;

        return 0;
    }

    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data);

    stbi_image_free(data);

    return texture;
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGui::Begin(
    "MainWindow",
    nullptr,
    ImGuiWindowFlags_NoTitleBar |
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoCollapse
    );

    // ===== Верхняя панель =====

    ImGui::Text("POLYX 2.0");

    ImGui::SameLine(
        ImGui::GetWindowContentRegionMax().x - 30
    );

    if (ImGui::Button("?"))
    {
        g_showAboutWindow = true;
    }

    ImGui::Separator();

    // ===== Левая панель =====

    ImGui::BeginChild(
        "LeftPanel",
        ImVec2(220, 0),
        true
    );

    ImGui::Text("Scenes");

    ImGui::Separator();

    for (size_t i = 0; i < g_scenes.size(); i++)
    {
        bool selected =
            (g_selected_scene == (int)i);

        if (ImGui::Selectable(
                g_scenes[i].name.c_str(),
                selected))
        {
            g_selected_scene = (int)i;
        }
    }

    ImGui::EndChild();

    ImGui::SameLine();

    // ===== Правая панель =====

    ImGui::BeginChild(
        "RightPanel",
        ImVec2(0, 0),
        true
    );

    const float cardWidth = 220.0f;
    const float cardHeight = 140.0f;

    int cardsPerRow =
        (int)(ImGui::GetContentRegionAvail().x /
              (cardWidth + 15));

    if (cardsPerRow < 1)
        cardsPerRow = 1;

    int cardIndex = 0;

    for (size_t i = 0; i < g_scenes.size(); i++)
    {
        ImGui::PushID((int)i);

        ImGui::BeginGroup();

        if (ImGui::Button(
                g_scenes[i].name.c_str(),
                ImVec2(cardWidth, cardHeight)))
        {
            g_selected_scene = (int)i;
        }

        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(
                "Objects: %d\nCreated: %s",
                g_scenes[i].objectCount,
                g_scenes[i].creationDate.c_str()
            );
        }

        ImGui::Text(
            "%s",
            g_scenes[i].name.c_str()
        );

        ImGui::EndGroup();

        cardIndex++;

        if (cardIndex % cardsPerRow != 0)
            ImGui::SameLine();

        ImGui::PopID();
    }

    // ===== Карточка создания новой сцены =====

    ImGui::PushID("new_scene");

    ImGui::BeginGroup();

    if (ImGui::Button(
            "+",
            ImVec2(cardWidth, cardHeight)))
    {
        LaunchScene("");
    }

    ImGui::Text("Add Scene...");

    ImGui::EndGroup();

    ImGui::PopID();

    // ===== Информация о выбранной сцене =====

    if (g_selected_scene >= 0)
    {
        ImGui::Separator();

        const auto& scene =
            g_scenes[g_selected_scene];

        ImGui::Text(
            "Selected: %s",
            scene.name.c_str());

        ImGui::Text(
            "Objects: %d",
            scene.objectCount);

        ImGui::Text(
            "Created: %s",
            scene.creationDate.c_str());

        if (ImGui::Button("Launch Scene"))
        {
            LaunchScene(scene.sceneFile);
        }
    }

    ImGui::EndChild();

    ImGui::End();

    // ===== Окно "О разработчике" =====

    if (g_showAboutWindow)
    {
        ImGui::OpenPopup("About Developer");
    }

    if (ImGui::BeginPopupModal(
            "About Developer",
            &g_showAboutWindow,
            ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("О РАЗРАБОТЧИКЕ");
        ImGui::Separator();

        // Фото слева
        ImGui::BeginGroup();

        if (g_developerTexture)
        {
            ImGui::Image(
                (ImTextureID)(intptr_t)g_developerTexture,
                ImVec2(220, 280));
        }
        else
        {
            ImGui::Text("Photo not found");
        }

        ImGui::EndGroup();

        ImGui::SameLine();

        // Информация справа
        ImGui::BeginGroup();

        ImGui::Text("ФИО:");
        ImGui::Text("Золотенков");
        ImGui::Text("Василий");
        ImGui::Text("Васильевич");

        ImGui::Spacing();

        ImGui::Text("Группа:");
        ImGui::Text("551002");

        ImGui::Spacing();

        ImGui::Text("Университет:");
        ImGui::Text("БГУИР");

        ImGui::Spacing();

        ImGui::Text("Факультет:");
        ImGui::Text("ФКСиС");

        ImGui::Spacing();

        ImGui::Text("Специальность:");
        ImGui::Text("ПИ");

        ImGui::Spacing();

        ImGui::Text("Курс:");
        ImGui::Text("1");

        ImGui::EndGroup();

        ImGui::Separator();

        ImGui::Text("Начало работы: 18.02.2026");
        ImGui::Text("Конец работы: 08.06.2026");

        ImGui::Spacing();

        if (ImGui::Button("Закрыть"))
        {
            g_showAboutWindow = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(
        ImGui::GetDrawData());

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    if (h == 0)
        h = 1;

    ImGui::GetIO().DisplaySize =
        ImVec2((float)w, (float)h);

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(
        45.0,
        (double)w / (double)h,
        0.1,
        100.0);

    glMatrixMode(GL_MODELVIEW);
}

void timer(int value)
{
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ---------- main ----------
int main(int argc, char** argv)
{
    // Сканируем сцены
    g_scenes = ScanScenes();

    if (!g_scenes.empty())
    {
        g_selected_scene = 0;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Polyx Launcher");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    g_developerTexture =
    LoadTexture(
        "../../Assets/developer.png"
    );
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontFromFileTTF(
    "C:/Windows/Fonts/arial.ttf",
    16.0f,
    nullptr,
    io.Fonts->GetGlyphRangesCyrillic()
    );
    io.DisplaySize = ImVec2(800.0f, 600.0f);
    ImGui::StyleColorsDark();

    ImGui_ImplGLUT_Init();
    ImGui_ImplGLUT_InstallFuncs();
    ImGui_ImplOpenGL3_Init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}