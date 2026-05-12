#include "Application.h"
#include <iostream>
#include <glfw3webgpu.h>

void Application::onInit()
{
    if (!glfwInit())
    {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // <-- extra info for glfwCreateWindow
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(800, 800, "Voxel Renderer", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Could not open window!" << std::endl;
        glfwTerminate();
        return;
    }

    WGPUInstance instance = this->engine.initializeInstance();
    WGPUSurface surface = glfwGetWGPUSurface(instance, window);

    this->engine.onInit(surface);
}

bool Application::isRunning()
{
#ifdef __EMSCRIPTEN__
    return true;
#else
    return !glfwWindowShouldClose(window);
#endif
}

void Application::onFrame()
{
    glfwPollEvents();
    this->engine.onFrame();
}

void Application::onFinish()
{
    std::cout << "Voxel Engine Cleanup" << std::endl;
    this->engine.onFinish();

    if (window != nullptr)
        glfwDestroyWindow(this->window);

    glfwTerminate();
}