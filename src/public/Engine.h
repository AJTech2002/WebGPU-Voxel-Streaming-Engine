#ifndef ENGINE
#define ENGINE
#include "RenderContext.h"
#include "Renderer.h"
#include "webgpu/webgpu.h"

#include <utility>
class Engine
{

  public:
    static Engine &get()
    {
        static Engine instance;
        return instance;
    }

    Renderer renderer;
    RenderContext ctx;

    WGPUInstance initializeInstance();
    void onInit(WGPUSurface surface);
    void onFrame();
    void onFinish();

  private:
    void requestAdapter();
    void configureSurface(WGPUSurface surface);
    void requestDevice();
    std::pair<WGPUSurfaceTexture, WGPUTextureView> getNextSurfaceTexture();

    // Singleton
  private:
    Engine() = default;

    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;
};
#endif