#ifndef RENDERER
#define RENDERER
#include <webgpu/webgpu.h>

class Renderer
{
  public:
    void setup();
    void render(WGPUCommandEncoder &encoder, WGPUSurface &surface,
                WGPUTextureView &target);
    void cleanup();

  private:
    WGPURenderPipeline pipeline;
};
#endif