#pragma once
#include <functional>
#include <list>
#include <string>
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

class Dockspace {
    std::list<Dock> _docks;

  public:
    Dockspace(GLFWwindow *window);
    ~Dockspace();
    void Render();
    std::list<Dock> &Docks() { return _docks; }

  private:
    void DrawMainMenuBar();
};
