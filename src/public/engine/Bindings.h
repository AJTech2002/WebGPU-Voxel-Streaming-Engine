#pragma once
#include <vector>
#include <webgpu/webgpu.h>
class Bindings
{
  public:
    std::vector<WGPUBindGroupLayoutEntry> layoutEntries;
    std::vector<WGPUBindGroupEntry> bindGroupEntries;
    bool created = false;
    WGPUBindGroupLayout layout;
    WGPUBindGroup bindGroup;
    WGPUBindGroup &create();

    void addTextureBinding(uint32_t binding, WGPUTextureView textureView);
    void addSamplerBinding(uint32_t binding, WGPUSampler sampler);
    void addStorageTextureBinding(uint32_t binding, WGPUTextureView textureView,
                                  WGPUStorageTextureAccess access,
                                  WGPUTextureFormat format,
                                  WGPUShaderStage visibility);
};