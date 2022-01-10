#include "Dockspace.h"
#include "commands/Shortcuts.h"
#include "widgets/ModalDialogs.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <IconsFontAwesome5.h>
#include <GLFW/glfw3.h>
#include <FontAwesomeFree5.h>
#include <iostream>
#include "DefaultImGuiIni.h"

#define GUI_CONFIG_FILE "usdtweak_gui.ini"

#ifdef _WIN64
#include <sstream>
#include <locale>
#include <codecvt>
#include <winerror.h>
#include <shlobj_core.h>
#include <shlwapi.h>

std::string GetConfigFilePath() {
    PWSTR localAppDataDir = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, &localAppDataDir))) {
        std::wstringstream configFilePath;
        configFilePath << localAppDataDir << L"\\" GUI_CONFIG_FILE;
        CoTaskMemFree(localAppDataDir);
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>
            converter; // TODO: this is deprecated in C++17, find another solution
        return converter.to_bytes(configFilePath.str());
    }
    return GUI_CONFIG_FILE;
}
#else // Not Win64

std::string GetConfigFilePath() { return GUI_CONFIG_FILE; }

#endif

static void *UsdTweakDataReadOpen(ImGuiContext *, ImGuiSettingsHandler *iniHandler, const char *name) {
    auto loader = static_cast<Dockspace *>(iniHandler->UserData);
    return loader;
}

static void UsdTweakDataReadLine(ImGuiContext *, ImGuiSettingsHandler *iniHandler, void *loaderPtr, const char *line) {
    // Loader could be retrieved with
    // ResourcesLoader *loader = static_cast<ResourcesLoader *>(loaderPtr);
    int value = 0;

    // Editor settings
    auto dockspace = static_cast<Dockspace *>(iniHandler->UserData);
    auto &settings = dockspace->Settings();
    if (sscanf(line, "ShowLayerEditor=%i", &value) == 1) {
        settings._showLayerEditor = static_cast<bool>(value);
    } else if (sscanf(line, "ShowLayerHierarchyEditor=%i", &value) == 1) {
        settings._showLayerHierarchyEditor = static_cast<bool>(value);
    } else if (sscanf(line, "ShowLayerStackEditor=%i", &value) == 1) {
        settings._showLayerStackEditor = static_cast<bool>(value);
    } else if (sscanf(line, "ShowPropertyEditor=%i", &value) == 1) {
        settings._showPropertyEditor = static_cast<bool>(value);
    } else if (sscanf(line, "ShowOutliner=%i", &value) == 1) {
        settings._showOutliner = static_cast<bool>(value);
    } else if (sscanf(line, "ShowTimeline=%i", &value) == 1) {
        settings._showTimeline = static_cast<bool>(value);
    } else if (sscanf(line, "ShowContentBrowser=%i", &value) == 1) {
        settings._showContentBrowser = static_cast<bool>(value);
    } else if (sscanf(line, "ShowPrimSpecEditor=%i", &value) == 1) {
        settings._showPrimSpecEditor = static_cast<bool>(value);
    } else if (sscanf(line, "ShowViewport=%i", &value) == 1) {
        settings._showViewport = static_cast<bool>(value);
    } else if (sscanf(line, "ShowDebugWindow=%i", &value) == 1) {
        settings._showDebugWindow = static_cast<bool>(value);
    }
}

static void UsdTweakDataWriteAll(ImGuiContext *ctx, ImGuiSettingsHandler *iniHandler, ImGuiTextBuffer *buf) {
    // ResourcesLoader *loader = static_cast<ResourcesLoader *>(iniHandler->UserData);
    buf->reserve(2048); // ballpark reserve

    // Saving the editor settings
    auto dockspace = (Dockspace *)iniHandler->UserData;
    auto &settings = dockspace->Settings();
    buf->appendf("[%s][%s]\n", iniHandler->TypeName, "Editor");
    buf->appendf("ShowLayerEditor=%d\n", settings._showLayerEditor);
    buf->appendf("ShowLayerHierarchyEditor=%d\n", settings._showLayerHierarchyEditor);
    buf->appendf("ShowLayerStackEditor=%d\n", settings._showLayerStackEditor);
    buf->appendf("ShowPropertyEditor=%d\n", settings._showPropertyEditor);
    buf->appendf("ShowOutliner=%d\n", settings._showOutliner);
    buf->appendf("ShowTimeline=%d\n", settings._showTimeline);
    buf->appendf("ShowContentBrowser=%d\n", settings._showContentBrowser);
    buf->appendf("ShowPrimSpecEditor=%d\n", settings._showPrimSpecEditor);
    buf->appendf("ShowViewport=%d\n", settings._showViewport);
    buf->appendf("ShowDebugWindow=%d\n", settings._showDebugWindow);
}

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

    // Install handlers to read and write the settings
    ImGuiContext *imGuiContext = ImGui::GetCurrentContext();
    ImGuiSettingsHandler iniHandler;
    iniHandler.TypeName = "UsdTweakData";
    iniHandler.TypeHash = ImHashStr("UsdTweakData");
    iniHandler.ReadOpenFn = UsdTweakDataReadOpen;
    iniHandler.ReadLineFn = UsdTweakDataReadLine;
    iniHandler.WriteAllFn = UsdTweakDataWriteAll;
    iniHandler.UserData = this;
    imGuiContext->SettingsHandlers.push_back(iniHandler);

    // Ini file
    // The first time the application is open, there is no default ini and the UI is all over the place.
    // This bit of code adds a default configuration
    ImFileHandle f;
    const std::string configFilePath = GetConfigFilePath();
    std::cout << "Config file : " << configFilePath << std::endl;
    if ((f = ImFileOpen(configFilePath.c_str(), "rb")) == nullptr) {
        ImGui::LoadIniSettingsFromMemory(imgui, 0);
    } else {
        ImFileClose(f);
        ImGui::LoadIniSettingsFromDisk(configFilePath.c_str());
    }

    LoadSettings();

    // menu
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
    SaveSettings();

    // Save the configuration file when the application closes the resources
    const std::string configFilePath = GetConfigFilePath();
    ImGui::SaveIniSettingsToDisk(configFilePath.c_str());

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

void Dockspace::LoadSettings() {}

void Dockspace::SaveSettings() const {}
