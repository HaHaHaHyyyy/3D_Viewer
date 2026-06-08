#include "ui.h"
#include "globals.h"
#include "scene.h"
#include "texture_manager.h"
#include "input.h"   // для safe_input
#include "imgui.h"
#include <string>

// Внешняя функция safe_input (объявлена в input.h)
std::string safe_input(const char* prompt);

void draw_ui() {
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
        std::string fname = safe_input("Enter OBJ filename (in Objects/): ");
        if (!fname.empty()) {
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "Objects/%s", fname.c_str());
            add_mesh_from_file(fullpath);
        } else {
            printf("No filename entered.\n");
        }
    }

    if (ImGui::Button("Save scene")) {
        std::string name = safe_input("Scene name: ");
        if (!name.empty()) {
            char fname[512];
            snprintf(fname, sizeof(fname), "%s.txt", name.c_str());
            save_scene(fname);
        } else {
            printf("No scene name entered.\n");
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Load scene")) {
        std::string name = safe_input("Scene name to load: ");
        if (!name.empty()) {
            char fname[512];
            snprintf(fname, sizeof(fname), "%s.txt", name.c_str());
            load_scene(fname);
        } else {
            printf("No scene name entered.\n");
        }
    }

    ImGui::Separator();
    ImGui::Text("Grouping");
    if (ImGui::Button("Create group")) {
        printf("=== Create group button pressed ===\n");
        fflush(stdout);
        std::string name = safe_input("Group name: ");
        printf("Name entered: '%s'\n", name.c_str());
        fflush(stdout);
        if (!name.empty()) {
            printf("Would create group with name '%s'\n", name.c_str());
            fflush(stdout);
            create_group(name.c_str());   // временно закомментировано
        } else {
            printf("Empty name.\n");
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Add to group")) {
        printf("=== Add to group button pressed ===\n");
        fflush(stdout);
        if (!g_selected) {
            printf("No object selected.\n");
        } else {
            printf("Selected object: %s, is_group=%d\n", g_selected->ObjData.name, g_selected->ObjData.is_group);
            if (g_selected->ObjData.is_group) {
                printf("Cannot add a group to another group.\n");
            } else {
                if (g_selected->ObjData.parent) {
                    printf("Object already in a group.\n");
                } else {
                    std::string gname = safe_input("Group name: ");
                    printf("Group name entered: '%s'\n", gname.c_str());
                    if (!gname.empty()) {
                        add_selected_to_group(gname.c_str());
                    } else {
                        printf("Empty group name, ignoring.\n");
                    }
                }
            }
        }
    }

    ImGui::Separator();

    if (g_selected && !g_selected->ObjData.is_group) {
        ImGui::Text("Selected: %s", g_selected->ObjData.name);
        ImGui::DragFloat3("Position", &g_selected->ObjData.position.x, 0.1f);
        ImGui::DragFloat3("Rotation", &g_selected->ObjData.rotation.x, 1.0f);
        ImGui::DragFloat3("Scale", &g_selected->ObjData.scale.x, 0.05f);
        ImGui::ColorEdit3("Color", &g_selected->ObjData.color.x);

        ImGui::Separator();
        ImGui::Text("Textures");
        if (ImGui::Button("Load texture...")) {
            std::string fname = safe_input("Enter texture filename (in Objects/): ");
            if (!fname.empty()) {
                char fullpath[512];
                snprintf(fullpath, sizeof(fullpath), "Objects/%s", fname.c_str());
                add_texture(fullpath);
            } else {
                printf("No texture filename entered.\n");
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove selected") && g_selected_texture_idx >= 0) {
            remove_texture(g_selected_texture_idx);
            g_selected_texture_idx = -1;
        }
        if (ImGui::BeginListBox("##texlist", ImVec2(200, 100))) {
            for (int i = 0; i < g_textures.Size; i++) {
                if (ImGui::Selectable(g_textures[i].name, g_selected_texture_idx == i)) {
                    g_selected_texture_idx = i;
                }
            }
            ImGui::EndListBox();
        }
        if (g_selected && g_selected_texture_idx >= 0) {
            if (ImGui::Button("Apply to selected")) {
                apply_texture_to_selected(g_selected_texture_idx);
            }
        }
    } else if (g_selected && g_selected->ObjData.is_group) {
        ImGui::Text("Selected GROUP: %s", g_selected->ObjData.name);
        ImGui::DragFloat3("Position", &g_selected->ObjData.position.x, 0.1f);
        ImGui::DragFloat3("Rotation", &g_selected->ObjData.rotation.x, 1.0f);
        ImGui::DragFloat3("Scale", &g_selected->ObjData.scale.x, 0.05f);
        ImGui::ColorEdit3("Color", &g_selected->ObjData.color.x);
    } else {
        ImGui::Text("No object selected.");
    }

    ImGui::Separator();
    ImGui::Checkbox("Show grid", &show_grid);
    ImGui::Checkbox("Show arrows", &show_arrows);
    ImGui::DragFloat("Grid Y", &grid_y, 0.05f);
    ImGui::Checkbox("Clipping plane", &clip_enabled);
    ImGui::Checkbox("Attach clip plane to selected object", &clip_attach_to_selected);
    ImGui::DragFloat("Clip offset", (float*)&clip_plane[3], 0.05f);
    ImGui::Text("Camera pos: %.2f %.2f %.2f", camera.pos.x, camera.pos.y, camera.pos.z);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();
}