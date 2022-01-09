#include "window.h"
#include <GLFW/glfw3.h>
#include <assert.h>
#include <iostream>

Window::Window(GLFWwindow *window) : _window(window) {
    assert(window);
    // Make the window's context current
    glfwMakeContextCurrent(window);
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
        return false;
    }

    auto window = std::shared_ptr<Window>(new Window(glfwWindow));

    return window;
}

void Window::setOnDrop(void *p, GLFWdropfun callback) {
    glfwSetWindowUserPointer(_window, p);
    glfwSetDropCallback(_window, callback);
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
