#ifndef RENDERER
#define RENDERER
#include <webgpu/webgpu.h>

struct RenderResources
{
    WGPUBindGroup bindGroup;
    WGPUBindGroupLayout bindGroupLayout;
    WGPUPipelineLayout pipelineLayout;
    WGPURenderPipeline pipeline;
    WGPUTexture screenTexture;
    WGPUSampler sampler;
    WGPUTextureView screenTextureView;
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
};
#endif