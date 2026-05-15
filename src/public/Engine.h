#include "Renderer.h"
#include "webgpu/webgpu.h"
#include <utility>

class Engine
{

public:
  WGPUInstance instance;
  WGPUAdapter adapter;
  WGPUDevice device;
  WGPUQueue queue;
  WGPUSurface surface;

  Renderer raymarchedSurface;

  WGPUInstance initializeInstance();
  void onInit(WGPUSurface surface);
  void onFrame();
  void onFinish();

private:
  void requestAdapter();
  void configureSurface(WGPUSurface surface);
  void requestDevice();
  std::pair<WGPUSurfaceTexture, WGPUTextureView> getNextSurfaceTexture();
};
