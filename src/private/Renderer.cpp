#include "Renderer.h"
#include "Engine.h"
#include "RenderContext.h"
#include "Shader.h"
#include "WGPUUtils.h"
#include <webgpu/webgpu.h>

void Renderer::setup()
{
    WGPURenderPipelineDescriptor pipelineDescriptor =
        WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;

    pipelineDescriptor.depthStencil = nullptr;

    Shader shader = Shader("resources/main.wgsl");

    std::cout << "Shader source:\n" << shader.getSource() << std::endl;

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

    pipelineDescriptor.multisample.count = 1;
    pipelineDescriptor.multisample.mask = ~0u;
    pipelineDescriptor.multisample.alphaToCoverageEnabled = false;

    pipeline = wgpuDeviceCreateRenderPipeline(Engine::get().ctx.device,
                                              &pipelineDescriptor);
}

void Renderer::cleanup() { wgpuRenderPipelineRelease(pipeline); }

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

    wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);

    wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);
    wgpuRenderPassEncoderEnd(renderPass);

    wgpuRenderPassEncoderRelease(
        renderPass); // release render pass after it's finished
}