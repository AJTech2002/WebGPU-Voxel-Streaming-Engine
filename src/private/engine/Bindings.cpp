#include "engine/Bindings.h"
#include "engine/Engine.h"
#include <webgpu/webgpu.h>

WGPUBindGroup &Bindings::create()
{
    if (created)
        return this->bindGroup;

    WGPUBindGroupLayoutDescriptor layoutDescriptor =
        WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
    layoutDescriptor.entryCount = this->layoutEntries.size();
    layoutDescriptor.entries = this->layoutEntries.data();

    this->layout = wgpuDeviceCreateBindGroupLayout(Engine::get().ctx.device,
                                                   &layoutDescriptor);

    WGPUBindGroupDescriptor bindGroupDescriptor =
        WGPU_BIND_GROUP_DESCRIPTOR_INIT;
    bindGroupDescriptor.layout = this->layout;
    bindGroupDescriptor.entryCount = this->bindGroupEntries.size();
    bindGroupDescriptor.entries = this->bindGroupEntries.data();

    this->bindGroup = wgpuDeviceCreateBindGroup(Engine::get().ctx.device,
                                                &bindGroupDescriptor);
    created = true;
    return this->bindGroup;
};

void Bindings::addTextureBinding(uint32_t binding, WGPUTextureView textureView)
{
    WGPUBindGroupLayoutEntry layoutEntry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
    layoutEntry.binding = binding;
    layoutEntry.visibility = WGPUShaderStage_Fragment;
    layoutEntry.texture.sampleType = WGPUTextureSampleType_Float;
    layoutEntry.texture.viewDimension = WGPUTextureViewDimension_2D;

    this->layoutEntries.push_back(layoutEntry);

    WGPUBindGroupEntry bindGroupEntry = WGPU_BIND_GROUP_ENTRY_INIT;
    bindGroupEntry.binding = binding;
    bindGroupEntry.textureView = textureView;

    this->bindGroupEntries.push_back(bindGroupEntry);
}

void Bindings::addSamplerBinding(uint32_t binding, WGPUSampler sampler)
{
    WGPUBindGroupLayoutEntry layoutEntry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
    layoutEntry.binding = binding;
    layoutEntry.visibility = WGPUShaderStage_Fragment;
    layoutEntry.sampler.type = WGPUSamplerBindingType_Filtering;

    this->layoutEntries.push_back(layoutEntry);

    WGPUBindGroupEntry bindGroupEntry = WGPU_BIND_GROUP_ENTRY_INIT;
    bindGroupEntry.binding = binding;
    bindGroupEntry.sampler = sampler;

    this->bindGroupEntries.push_back(bindGroupEntry);
}

void Bindings::addStorageTextureBinding(uint32_t binding,
                                        WGPUTextureView textureView,
                                        WGPUStorageTextureAccess access,
                                        WGPUTextureFormat format,
                                        WGPUShaderStage visibility)
{
    WGPUBindGroupLayoutEntry layoutEntry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
    layoutEntry.binding = binding;
    layoutEntry.visibility = visibility;
    layoutEntry.storageTexture.access = access;
    layoutEntry.storageTexture.format = format;
    layoutEntry.storageTexture.viewDimension = WGPUTextureViewDimension_2D;

    this->layoutEntries.push_back(layoutEntry);

    WGPUBindGroupEntry bindGroupEntry = WGPU_BIND_GROUP_ENTRY_INIT;
    bindGroupEntry.binding = binding;
    bindGroupEntry.textureView = textureView;

    this->bindGroupEntries.push_back(bindGroupEntry);
}

void Bindings::addStorageBufferBinding(uint32_t binding, WGPUBuffer buffer,
                                       WGPUShaderStage visibility)
{
    WGPUBindGroupLayoutEntry layoutEntry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
    layoutEntry.binding = binding;
    layoutEntry.visibility = visibility;
    layoutEntry.buffer.type = WGPUBufferBindingType_Storage;

    this->layoutEntries.push_back(layoutEntry);

    WGPUBindGroupEntry bindGroupEntry = WGPU_BIND_GROUP_ENTRY_INIT;
    bindGroupEntry.binding = binding;
    bindGroupEntry.buffer = buffer;
    bindGroupEntry.offset = 0;
    bindGroupEntry.size = WGPU_WHOLE_SIZE;

    this->bindGroupEntries.push_back(bindGroupEntry);
}

void Bindings::addUniformBufferBinding(uint32_t binding, WGPUBuffer buffer,
                                       WGPUShaderStage visibility)
{
    WGPUBindGroupLayoutEntry layoutEntry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
    layoutEntry.binding = binding;
    layoutEntry.visibility = visibility;
    layoutEntry.buffer.type = WGPUBufferBindingType_Uniform;

    this->layoutEntries.push_back(layoutEntry);

    WGPUBindGroupEntry bindGroupEntry = WGPU_BIND_GROUP_ENTRY_INIT;
    bindGroupEntry.binding = binding;
    bindGroupEntry.buffer = buffer;
    bindGroupEntry.offset = 0;
    // TODO: Verify that this is correct. WGPU_WHOLE_SIZE is used for storage
    // buffers, but uniform buffers have a max binding size limit, so maybe this
    // should be set to the actual size of the buffer?
    bindGroupEntry.size = WGPU_WHOLE_SIZE;

    this->bindGroupEntries.push_back(bindGroupEntry);
}