#include "Dockspace.h"
#include "commands/Shortcuts.h"
#include "widgets/ModalDialogs.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui.h>
#include <IconsFontAwesome5.h>
#include <GLFW/glfw3.h>
#include <FontAwesomeFree5.h>

void MenuItem::Draw() {
    if (name.empty()) {
        ImGui::Separator();

    } else {
        auto enable = action ? true : false;
        if (enable_callback) {
            enable = enable_callback();
        }
        if (ImGui::MenuItem(name.c_str(), key.c_str(), false, enable)) {
            if (action) {
                action();
            }
        }
    }
}

void Menu::Draw() {
    if (ImGui::BeginMenu(name.c_str())) {
        for (auto &item : items) {
            item.Draw();
        }
        ImGui::EndMenu();
    }
}

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

    const auto font_size = 24.0f;

    // Font
    ImFontConfig fontConfig;
    // auto fontDefault = io.Fonts->AddFontDefault(&fontConfig); // DroidSans
    auto fontDefault = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msgothic.ttc", font_size, &fontConfig,
                                                    io.Fonts->GetGlyphRangesJapanese()); // DroidSans

    // Icons (in font)
    static const ImWchar iconRanges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig iconsConfig;
    iconsConfig.MergeMode = true;
    iconsConfig.PixelSnapH = true;

    auto font = io.Fonts->AddFontFromMemoryCompressedTTF(fontawesomefree5_compressed_data, fontawesomefree5_compressed_size,
                                                         font_size, &iconsConfig, iconRanges);

    _menus = {{"File", {{}, {"Quit", "", [self = this]() { self->_shutdownRequested = true; }}}},
              {"Edit",
               {
                   {},
                   {"Cut", "CTRL+X"},
                   {"Copy", "CTRL+C"},
                   {"Paste", "CTRL+V"},
               }}};
}

Dockspace::~Dockspace() {
    // Shutdown imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Dockspace::DrawMainMenuBar() {

    if (ImGui::BeginMenuBar()) {
        for (auto &menu : _menus) {
            menu.Draw();
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

bool Dockspace::Render() {
    if (_shutdownRequested) {
        return false;
    }

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

        DrawCurrentModal();

        ///////////////////////
        // Top level shortcuts functions
        AddShortcut<UndoCommand, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_Z>();
        AddShortcut<RedoCommand, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_R>();

        EndBackgroundDock();
    }
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return true;
}
