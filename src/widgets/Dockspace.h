#pragma once
#include <functional>
#include <list>
#include <string>
#include <algorithm>
#include <assert.h>

struct GLFWwindow;

using DockShowFunction = std::function<void(bool *p_open)>;

class Dock {
    std::string _name;
    DockShowFunction _callback;
    bool *_p_open = nullptr;

  public:
    Dock(const std::string &name, bool *p_open, const DockShowFunction &callback)
        : _name(name), _callback(callback), _p_open(p_open) {
        assert(_p_open);
    }

    void menu_item();

    void show() {
        if (*_p_open) {
            _callback(_p_open);
        }
    }
};

struct MenuItem {
    std::string name;
    std::string key;
    std::function<void()> action;
    std::function<bool()> enable_callback;
    void Draw();
};
struct Menu {
    std::string name;
    std::list<MenuItem> items;
    void Draw();
};

struct EditorSettings {
    /// Editor Windows state
    bool _showDebugWindow = false;
    bool _showPropertyEditor = true;
    bool _showOutliner = true;
    bool _showTimeline = false;
    bool _showLayerEditor = false;
    bool _showLayerHierarchyEditor = false;
    bool _showLayerStackEditor = false;
    bool _showContentBrowser = false;
    bool _showPrimSpecEditor = false;
    bool _showViewport = false;
    bool _textEditor = false;
};
class Dockspace {
    std::list<Dock> _docks;
    std::list<Menu> _menus;
    /// Setting _shutdownRequested to true will stop the main loop
    bool _shutdownRequested = false;

    ///
    /// Editor settings contains the windows states
    ///
    EditorSettings _settings;

  public:
    Dockspace(GLFWwindow *window);
    ~Dockspace();
    bool Render();
    std::list<Dock> &Docks() { return _docks; }

    Menu *GetMenu(const std::string &name) {
        auto found = std::find_if(_menus.begin(), _menus.end(), [name](const Menu &m) { return m.name == name; });
        if (found == _menus.end()) {
            return nullptr;
        }
        return &*found;
    }

    /// Make the layer editor visible
    void ShowLayerEditor() { _settings._showLayerEditor = true; }
    EditorSettings &Settings() { return _settings; }

    /// Interface with the settings
    void LoadSettings();
    void SaveSettings() const;

  private:
    void DrawMainMenuBar();
};
