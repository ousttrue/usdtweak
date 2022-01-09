#pragma once
#include <memory>

struct GLFWwindow;
class Window {
    GLFWwindow *_window = nullptr;
    Window(GLFWwindow *window);

  public:
    ~Window();
    static std::shared_ptr<Window> create(int width, int height);
    GLFWwindow *get() { return _window; }
    void setOnDrop(void *p, void (*callback)(GLFWwindow *window, int path_count, const char *paths[]));
    bool newFrame(int *pWidth, int *pHeight);
    void flush();
};
