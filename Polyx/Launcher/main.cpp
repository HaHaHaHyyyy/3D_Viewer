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

#include "screenshot.h" // обязательно, чтобы была save_screenshot

namespace fs = std::filesystem;

// ---------- Глобальные переменные ----------
std::vector<SceneInfo> g_scenes;
int g_selected_scene = -1;
bool g_showAboutWindow = false;
GLuint g_developerTexture = 0;

// Forward declarations
void display();
void reshape(int w, int h);
void timer(int value);

// ---------- Загрузка текстур ----------
GLuint LoadTexture(const char* filename)
{
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
    if (!data)
    {
        std::cout << "Failed to load image: " << filename << std::endl;
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    return texture;
}

// ---------- Функция рендера ----------
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

    ImGui::Begin("MainWindow", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoCollapse);

    // ===== Верхняя панель =====
    ImGui::Text("POLYX 2.0");
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 30);
    if (ImGui::Button("?")) g_showAboutWindow = true;

    ImGui::Separator();

    // ===== Левая панель =====
    ImGui::BeginChild("LeftPanel", ImVec2(220, 0), true);
    ImGui::Text("Scenes");
    ImGui::Separator();
    for (size_t i = 0; i < g_scenes.size(); i++)
    {
        bool selected = (g_selected_scene == (int)i);
        if (ImGui::Selectable(g_scenes[i].name.c_str(), selected))
            g_selected_scene = (int)i;
    }
    ImGui::EndChild();
    ImGui::SameLine();

    // ===== Правая панель с превью =====
    ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);

    constexpr float cardWidth = 220.0f;
    constexpr float cardHeight = 140.0f;
    int cardsPerRow = (int)(ImGui::GetContentRegionAvail().x / (cardWidth + 15));
    if (cardsPerRow < 1) cardsPerRow = 1;

    int cardIndex = 0;
    for (size_t i = 0; i < g_scenes.size(); i++)
    {
        ImGui::PushID((int)i);
        ImGui::BeginGroup();

        ImVec2 p = ImGui::GetCursorScreenPos();

        if (ImGui::InvisibleButton(g_scenes[i].name.c_str(), ImVec2(cardWidth, cardHeight)))
            g_selected_scene = (int)i;

        ImDrawList* draw = ImGui::GetWindowDrawList();

        if (g_scenes[i].previewTexture != 0)
        {
            draw->AddImage((ImTextureID)(uintptr_t)g_scenes[i].previewTexture, p, ImVec2(p.x + cardWidth, p.y + cardHeight));
        }
        else
        {
            draw->AddRectFilled(p, ImVec2(p.x + cardWidth, p.y + cardHeight), IM_COL32(70, 70, 90, 255));
        }

        draw->AddRect(p, ImVec2(p.x + cardWidth, p.y + cardHeight), IM_COL32(180, 180, 180, 255));

        draw->AddText(ImVec2(p.x + 10, p.y + 10), IM_COL32(255, 255, 255, 255), g_scenes[i].name.c_str());

        char buf[64];
        sprintf(buf, "%d objects", g_scenes[i].objectCount);
        draw->AddText(ImVec2(p.x + 10, p.y + cardHeight - 25), IM_COL32(220, 220, 220, 255), buf);

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Objects: %d\nCreated: %s", g_scenes[i].objectCount, g_scenes[i].creationDate.c_str());

        ImGui::EndGroup();
        ImGui::PopID();

        cardIndex++;
        if (cardIndex % cardsPerRow != 0)
            ImGui::SameLine();
    }

    // Карточка создания новой сцены
    ImGui::PushID("new_scene");
    ImGui::BeginGroup();
    if (ImGui::Button("+", ImVec2(cardWidth, cardHeight)))
        LaunchScene("");
    ImGui::Text("Add Scene...");
    ImGui::EndGroup();
    ImGui::PopID();

    // Информация о выбранной сцене
    if (g_selected_scene >= 0)
    {
        ImGui::Separator();
        const auto& scene = g_scenes[g_selected_scene];
        ImGui::Text("Selected: %s", scene.name.c_str());
        ImGui::Text("Objects: %d", scene.objectCount);
        ImGui::Text("Created: %s", scene.creationDate.c_str());
        if (ImGui::Button("Launch Scene"))
            LaunchScene(scene.sceneFile);
    }

    ImGui::EndChild();
    ImGui::End();

    // О разработчике
    if (g_showAboutWindow) ImGui::OpenPopup("About Developer");
    if (ImGui::BeginPopupModal("About Developer", &g_showAboutWindow, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("О РАЗРАБОТЧИКЕ");
        ImGui::Separator();

        ImGui::BeginGroup();
        if (g_developerTexture != 0)
            ImGui::Image((ImTextureID)(uintptr_t)g_developerTexture, ImVec2(220, 280));
        else
            ImGui::Text("Photo not found");
        ImGui::EndGroup();

        ImGui::SameLine();
        ImGui::BeginGroup();

        ImGui::Text("ФИО:"); ImGui::Text("Золотенков"); ImGui::Text("Василий"); ImGui::Text("Васильевич");
        ImGui::Spacing(); ImGui::Text("Группа:"); ImGui::Text("551002");
        ImGui::Spacing(); ImGui::Text("Университет:"); ImGui::Text("БГУИР");
        ImGui::Spacing(); ImGui::Text("Факультет:"); ImGui::Text("ФКСиС");
        ImGui::Spacing(); ImGui::Text("Специальность:"); ImGui::Text("ПИ");
        ImGui::Spacing(); ImGui::Text("Курс:"); ImGui::Text("1");

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
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glutSwapBuffers();
}

// ---------- reshape / timer ----------
void reshape(int w, int h)
{
    if (h == 0) h = 1;
    ImGui::GetIO().DisplaySize = ImVec2((float)w, (float)h);
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / h, 0.1, 100.0);
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

    // Создаём окно OpenGL
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Polyx Launcher");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Только теперь можно безопасно загружать текстуры
    for (auto& scene : g_scenes)
    {
        if (!scene.previewFile.empty() && fs::exists(scene.previewFile))
        {
            scene.previewTexture = LoadTexture(scene.previewFile.c_str());
            std::cout << "Loaded preview: " << scene.previewFile
                      << " texture=" << scene.previewTexture << std::endl;
        }
        else
        {
            std::cout << "Preview missing for scene: " << scene.name << std::endl;
            scene.previewTexture = 0;
        }
    }

    g_developerTexture = LoadTexture("../../Assets/developer.png");
    std::cout << "Developer texture ID: " << g_developerTexture << std::endl;

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/arial.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    io.DisplaySize = ImVec2(800.0f, 600.0f);

    ImGui::StyleColorsDark();
    ImGui_ImplGLUT_Init();
    ImGui_ImplGLUT_InstallFuncs();
    ImGui_ImplOpenGL3_Init();

    // GLUT callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);

    if (!g_scenes.empty()) g_selected_scene = 0;

    glutMainLoop();

    // Clean up
    if (g_developerTexture != 0)
        glDeleteTextures(1, &g_developerTexture);

    return 0;
}