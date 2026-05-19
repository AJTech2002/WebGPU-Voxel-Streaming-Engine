#include "Renderer.h"
#include "Debug.h"
#include "Engine.h"
#include "RenderContext.h"
#include "Shader.h"
#include "WGPUUtils.h"
#include <array>
#include <iostream>
#include <string>
#include <webgpu/webgpu.h>

void Renderer::setup()
{
    WGPURenderPipelineDescriptor pipelineDescriptor =
        WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;

    Shader shader = Shader("resources/main.wgsl");

    std::cout << "Shader source:\n" << shader.getSource() << std::endl;

    setupResources();

    /* #region Vertex Pipeline */
    pipelineDescriptor.vertex.bufferCount = 0;
    pipelineDescriptor.vertex.buffers = nullptr; // Quad in Shader

    pipelineDescriptor.vertex.module = shader.shaderModule;
    pipelineDescriptor.vertex.entryPoint = STRVIEW("vs_main");
    pipelineDescriptor.vertex.constantCount = 0;
    pipelineDescriptor.vertex.constants = nullptr;

    pipelineDescriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipelineDescriptor.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipelineDescriptor.primitive.frontFace = WGPUFrontFace_CCW;
    pipelineDescriptor.primitive.cullMode = WGPUCullMode_None;
    /* #endregion */

    /* #region Fragment Pipeline */

    WGPUFragmentState fragmentState = WGPU_FRAGMENT_STATE_INIT;
    fragmentState.module = shader.shaderModule;
    fragmentState.entryPoint = STRVIEW("fs_main");
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;

    // Handle Blending
    WGPUBlendState blendState = WGPU_BLEND_STATE_INIT;

    // rgb = src * srcFactor + dst * dstFactor
    blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
    blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    blendState.color.operation = WGPUBlendOperation_Add;

    // alpha = src * srcFactor + dst * dstFactor
    blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
    blendState.alpha.dstFactor = WGPUBlendFactor_One;
    blendState.alpha.operation = WGPUBlendOperation_Add;

    WGPUColorTargetState colorTarget = WGPU_COLOR_TARGET_STATE_INIT;
    colorTarget.format = Engine::get().ctx.surfaceFormat;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = WGPUColorWriteMask_All;
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    pipelineDescriptor.fragment = &fragmentState;
    /* #endregion */

    /* #region Pipeline Creation */

    pipelineDescriptor.depthStencil = nullptr;
    pipelineDescriptor.multisample.count = 1;
    pipelineDescriptor.multisample.mask = ~0u;
    pipelineDescriptor.multisample.alphaToCoverageEnabled = false;

    /* Bind Group Layout */
    WGPUBindGroupLayoutEntry texLayoutEntry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;

    texLayoutEntry.binding = 0;
    texLayoutEntry.visibility = WGPUShaderStage_Fragment;
    texLayoutEntry.texture.sampleType = WGPUTextureSampleType_Float;
    texLayoutEntry.texture.viewDimension = WGPUTextureViewDimension_2D;
    texLayoutEntry.texture.multisampled = false;

    WGPUBindGroupLayoutEntry samplerLayoutEntry =
        WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;

    samplerLayoutEntry.binding = 1;
    samplerLayoutEntry.visibility = WGPUShaderStage_Fragment;
    samplerLayoutEntry.sampler.type = WGPUSamplerBindingType_Filtering;

    std::array<WGPUBindGroupLayoutEntry, 2> layoutEntries = {
        texLayoutEntry, samplerLayoutEntry};

    WGPUBindGroupLayoutDescriptor layoutDescriptor =
        WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
    layoutDescriptor.entryCount = layoutEntries.size();
    layoutDescriptor.entries = layoutEntries.data();

    WGPUBindGroupLayout layout = wgpuDeviceCreateBindGroupLayout(
        Engine::get().ctx.device, &layoutDescriptor);

    WGPUPipelineLayoutDescriptor pipelineLayout =
        WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
    pipelineLayout.bindGroupLayoutCount = 1;
    pipelineLayout.bindGroupLayouts = &layout;

    WGPUPipelineLayout pipelineLayoutObj = wgpuDeviceCreatePipelineLayout(
        Engine::get().ctx.device, &pipelineLayout);

    pipelineDescriptor.layout = pipelineLayoutObj;

    this->resources.pipeline = wgpuDeviceCreateRenderPipeline(
        Engine::get().ctx.device, &pipelineDescriptor);
    /* #endregion */

    /* #region Bind Group Creation */
    WGPUBindGroupEntry texBindGroupEntry = WGPU_BIND_GROUP_ENTRY_INIT;
    texBindGroupEntry.binding = 0;
    texBindGroupEntry.textureView = this->resources.screenTextureView;

    WGPUBindGroupEntry samplerBindGroupEntry = WGPU_BIND_GROUP_ENTRY_INIT;
    samplerBindGroupEntry.binding = 1;
    samplerBindGroupEntry.sampler = this->resources.sampler;

    std::array<WGPUBindGroupEntry, 2> bindGroupEntries = {
        texBindGroupEntry, samplerBindGroupEntry};

    WGPUBindGroupDescriptor bindGroupDescriptor =
        WGPU_BIND_GROUP_DESCRIPTOR_INIT;
    bindGroupDescriptor.layout = layout;
    bindGroupDescriptor.entryCount = bindGroupEntries.size();
    bindGroupDescriptor.entries = bindGroupEntries.data();

    this->resources.bindGroup = wgpuDeviceCreateBindGroup(
        Engine::get().ctx.device, &bindGroupDescriptor);
    /* #endregion */
}

void Renderer::setupResources()
{
    WGPUTextureDescriptor textureDesc = WGPU_TEXTURE_DESCRIPTOR_INIT;
    textureDesc.nextInChain = nullptr;
    textureDesc.label = {"Compute Texture", strlen("Compute Texture")};
    textureDesc.size.width = 800;
    textureDesc.size.height = 800;
    textureDesc.usage = WGPUTextureUsage_StorageBinding |
                        WGPUTextureUsage_CopyDst |
                        WGPUTextureUsage_TextureBinding;
    textureDesc.format = WGPUTextureFormat_RGBA16Float;

    this->resources.screenTexture =
        wgpuDeviceCreateTexture(Engine::get().ctx.device, &textureDesc);

    debugTextureData(textureDesc, this->resources.screenTexture);

    WGPUTextureViewDescriptor viewDesc = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
    viewDesc.format = wgpuTextureGetFormat(this->resources.screenTexture);
    viewDesc.dimension = WGPUTextureViewDimension_2D;
    viewDesc.baseMipLevel = 0;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = 0;
    viewDesc.arrayLayerCount = 1;
    viewDesc.aspect = WGPUTextureAspect_All;

    this->resources.screenTextureView =
        wgpuTextureCreateView(this->resources.screenTexture, &viewDesc);

    WGPUSamplerDescriptor samplerDesc = WGPU_SAMPLER_DESCRIPTOR_INIT;
    samplerDesc.addressModeU = WGPUAddressMode_ClampToEdge;
    samplerDesc.addressModeV = WGPUAddressMode_ClampToEdge;
    samplerDesc.magFilter = WGPUFilterMode_Linear;
    samplerDesc.minFilter = WGPUFilterMode_Linear;
    samplerDesc.mipmapFilter = WGPUMipmapFilterMode_Linear;

    this->resources.sampler =
        wgpuDeviceCreateSampler(Engine::get().ctx.device, &samplerDesc);
}

void Renderer::cleanup()
{
    wgpuRenderPipelineRelease(this->resources.pipeline);
    wgpuTextureRelease(this->resources.screenTexture);
    wgpuTextureViewRelease(this->resources.screenTextureView);
    wgpuSamplerRelease(this->resources.sampler);
}

void Renderer::render(WGPUCommandEncoder &encoder, WGPUSurface &surface,
                      WGPUTextureView &targetView)
{
    WGPURenderPassDescriptor renderPassDesc = {};
    renderPassDesc.nextInChain = nullptr;
    renderPassDesc.colorAttachmentCount = 1;

    WGPURenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = targetView;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.loadOp = WGPULoadOp_Clear;
    colorAttachment.storeOp = WGPUStoreOp_Store;
    colorAttachment.clearValue = {1.0f, 0.5f, 0.5f, 1.0f};
    colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

    renderPassDesc.colorAttachments = &colorAttachment;
    renderPassDesc.depthStencilAttachment = nullptr;
    renderPassDesc.timestampWrites = nullptr;

    WGPURenderPassEncoder renderPass =
        wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

    wgpuRenderPassEncoderSetPipeline(renderPass, this->resources.pipeline);

    // Quad
    wgpuRenderPassEncoderSetBindGroup(renderPass, 0, this->resources.bindGroup,
                                      0, nullptr);
    wgpuRenderPassEncoderDraw(renderPass, 6, 1, 0, 0);
    wgpuRenderPassEncoderEnd(renderPass);

    wgpuRenderPassEncoderRelease(
        renderPass); // release render pass after it's finished
}