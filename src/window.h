#pragma once
#include "window.h"
#include <memory>
#include <functional>

struct GLFWwindow;

using OnDropCallback = std::function<void(int count, const char *paths[])>;

class Window {
    GLFWwindow *_window = nullptr;
    Window(GLFWwindow *window);
    OnDropCallback _onDropFiles;

  public:
    ~Window();
    static std::shared_ptr<Window> create(int width, int height);
    GLFWwindow *get() { return _window; }
    void setOnDrop(const OnDropCallback &callback) { _onDropFiles = callback; }
    bool newFrame(int *pWidth, int *pHeight);
    void flush();

  private:
    static void dropCallback(GLFWwindow *window, int count, const char **paths);
};
