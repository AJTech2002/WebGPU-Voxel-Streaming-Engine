#include "Application.h"
#include <iostream>

#ifndef __EMSCRIPTEN__
#include <glfw3webgpu.h>
#endif

void Application::onInit()
{
  if (!glfwInit())
  {
    std::cerr << "Could not initialize GLFW!" << std::endl;
    return;
  }

  glfwWindowHint(GLFW_CLIENT_API,
                 GLFW_NO_API); // <-- extra info for glfwCreateWindow
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window = glfwCreateWindow(800, 800, "Voxel Renderer", nullptr, nullptr);
  if (!window)
  {
    std::cerr << "Could not open window!" << std::endl;
    glfwTerminate();
    return;
  }

  WGPUInstance instance = this->engine.initializeInstance();

  WGPUSurface surface;

#ifdef __EMSCRIPTEN__
  WGPUEmscriptenSurfaceSourceCanvasHTMLSelector canvasDesc = {};
  canvasDesc.chain.sType = WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector;
  canvasDesc.chain.next = nullptr;
  canvasDesc.selector = {"canvas", WGPU_STRLEN};

  WGPUSurfaceDescriptor surfDesc = {};
  surfDesc.nextInChain = &canvasDesc.chain;
  surfDesc.label = {"Screen Surface", WGPU_STRLEN};

  surface = wgpuInstanceCreateSurface(instance, &surfDesc);
#else
  surface = glfwGetWGPUSurface(instance, window);
#endif

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