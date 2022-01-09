#include "Dockspace.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui.h>
#include "Shortcuts.h"
#include <IconsFontAwesome5.h>
#include <GLFW/glfw3.h>


static void BeginBackgoundDock() {
    // Setup dockspace using experimental imgui branch
    static bool alwaysOpened = true;
    static ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_None;
    static ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    windowFlags |=
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", &alwaysOpened, windowFlags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspaceid = ImGui::GetID("dockspace");
    ImGui::DockSpace(dockspaceid, ImVec2(0.0f, 0.0f), dockFlags);
}

static void EndBackgroundDock() { ImGui::End(); }

void Dock::menu_item() { ImGui::MenuItem(_name.c_str(), nullptr, _p_open); }

Dockspace::Dockspace(GLFWwindow *window) {
    // Create a context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // ImGuiStyle& style = ImGui::GetStyle();
    // style.Colors[ImGuiCol_Tab] = style.Colors[ImGuiCol_FrameBg];
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
}

Dockspace::~Dockspace() {
    // Shutdown imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Dockspace::DrawMainMenuBar() {

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem(ICON_FA_FILE " New")) {
                // DrawModalDialog<CreateUsdFileModalDialog>(*this);
            }
            if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open")) {
                // DrawModalDialog<OpenUsdFileModalDialog>(*this);
            }
            ImGui::Separator();
            // const bool hasLayer = GetCurrentLayer() != SdfLayerRefPtr();
            // if (ImGui::MenuItem(ICON_FA_SAVE " Save layer", "CTRL+S", false, hasLayer)) {
            //     GetCurrentLayer()->Save(true);
            // }
            // if (ImGui::MenuItem(ICON_FA_SAVE " Save layer as", "CTRL+F", false, hasLayer)) {
            //     DrawModalDialog<SaveLayerAs>(*this);
            // }

            ImGui::Separator();
            if (ImGui::MenuItem("Quit")) {
                // _shutdownRequested = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {
                // ExecuteAfterDraw<UndoCommand>();
            }
            if (ImGui::MenuItem("Redo", "CTRL+R")) {
                // ExecuteAfterDraw<RedoCommand>();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X", false, false)) {
            }
            if (ImGui::MenuItem("Copy", "CTRL+C", false, false)) {
            }
            if (ImGui::MenuItem("Paste", "CTRL+V", false, false)) {
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Windows")) {
            for (auto &dock : _docks) {
                dock.menu_item();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void Dockspace::Render() {

    // imgui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        // Dock
        BeginBackgoundDock();

        // Main Menu bar
        DrawMainMenuBar();

        for (auto &dock : _docks) {
            dock.show();
        }

        // DrawCurrentModal();

        ///////////////////////
        // Top level shortcuts functions
        AddShortcut<UndoCommand, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_Z>();
        AddShortcut<RedoCommand, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_R>();

        EndBackgroundDock();
    }
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
