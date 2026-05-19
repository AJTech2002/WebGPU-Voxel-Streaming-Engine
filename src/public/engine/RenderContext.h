#ifndef RENDER_CONTEXT
#define RENDER_CONTEXT
#include <webgpu/webgpu.h>

struct RenderContext
{
    WGPUInstance instance;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;
    WGPUSurface surface;
    WGPUTextureFormat surfaceFormat;

    WGPUTextureView *activeTexture;
    WGPUCommandEncoder *activeEncoder;
};

#endif