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

class Scene;

class Renderer
{
  public:
    RenderResources resources;
    void setup();
    void render(WGPUCommandEncoder &encoder, WGPUSurface &surface,
                WGPUTextureView &target);
    void cleanup();
    void loadScene(Scene *scene);

  private:
    void setupResources();
};
#endif