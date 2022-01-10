#include "window.h"
#include <GLFW/glfw3.h>
#include <assert.h>
#include <cstddef>
#include <iostream>

Window::Window(GLFWwindow *window) : _window(window) {
    assert(window);
    // Make the window's context current
    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(_window, this);
    glfwSetDropCallback(_window, &Window::dropCallback);
}

Window::~Window() {
    glfwDestroyWindow(_window);
    glfwTerminate();
}

std::shared_ptr<Window> Window::create(int width, int height) {

    // Initialize glfw
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "glfwInit" << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif

#ifdef DISABLE_DOUBLE_BUFFER
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
#endif
    auto glfwWindow = glfwCreateWindow(width, height, "USD Tweak", NULL, NULL);
    if (!glfwWindow) {
        glfwTerminate();
        std::cerr << "unable to create a window, exiting" << std::endl;
        return nullptr;
    }

    auto window = std::shared_ptr<Window>(new Window(glfwWindow));

    return window;
}

/// Handle drag and drop from external applications
/// Call back for dropping a file in the ui
/// TODO Drop callback should popup a modal dialog with the different options available
void Window::dropCallback(GLFWwindow *window, int count, const char **paths) {
    void *userPointer = glfwGetWindowUserPointer(window);
    if (userPointer) {
        auto window = static_cast<Window *>(userPointer);

        // TODO: Create a task, add a callback
        if (window && window->_onDropFiles) {
            window->_onDropFiles(count, paths);
        }
    }
}

bool Window::newFrame(int *pWidth, int *pHeight) {
    if (glfwWindowShouldClose(_window)) {
        return false;
    }

    // Poll and process events
    glfwMakeContextCurrent(_window);
    glfwPollEvents();

    if (pWidth && pHeight) {
        glfwGetFramebufferSize(_window, pWidth, pHeight);
    }

    return true;
}

void Window::flush() {
    // Swap front and back buffers
    glfwSwapBuffers(_window);
}
