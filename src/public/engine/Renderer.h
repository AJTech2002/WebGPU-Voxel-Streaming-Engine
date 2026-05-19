#ifndef RENDERER
#define RENDERER
#include "Bindings.h"
#include "Compute.h"
#include <webgpu/webgpu.h>

struct RenderResources
{
    WGPUPipelineLayout pipelineLayout;
    WGPURenderPipeline pipeline;
    WGPUTexture screenTexture;
    WGPUSampler sampler;
    WGPUTextureView screenTextureView;
    Bindings screenBindings;
};

class Renderer
{
  public:
    RenderResources resources;
    void setup();
    void render(WGPUCommandEncoder &encoder, WGPUSurface &surface,
                WGPUTextureView &target);
    void cleanup();

  private:
    void setupResources();
    Compute compute;
};
#endif