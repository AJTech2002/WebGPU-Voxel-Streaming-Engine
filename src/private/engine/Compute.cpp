#include "engine/Compute.h"
#include "engine/Engine.h"
#include "engine/Shader.h"
#include "utils/WGPUUtils.h"
#include <webgpu/webgpu.h>

WGPUComputePipeline &Compute::create(Shader &shader)
{
    this->computeBindings.create();
    WGPUComputePipelineDescriptor pipelineDescriptor =
        WGPU_COMPUTE_PIPELINE_DESCRIPTOR_INIT;

    pipelineDescriptor.compute.entryPoint = STRVIEW("cs_main");
    pipelineDescriptor.compute.module = shader.shaderModule;

    WGPUPipelineLayoutDescriptor pipelineLayout =
        WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
    pipelineLayout.bindGroupLayoutCount = 1;
    pipelineLayout.bindGroupLayouts = &computeBindings.layout;

    WGPUPipelineLayout pipelineLayoutObj = wgpuDeviceCreatePipelineLayout(
        Engine::get().ctx.device, &pipelineLayout);

    pipelineDescriptor.layout = pipelineLayoutObj;
    this->pipeline = wgpuDeviceCreateComputePipeline(Engine::get().ctx.device,
                                                     &pipelineDescriptor);

    return this->pipeline;
}

void Compute::dispatch(int x, int y, int z, WGPUCommandEncoder &encoder)
{
    WGPUComputePassEncoder encoderObj =
        wgpuCommandEncoderBeginComputePass(encoder, nullptr);

    wgpuComputePassEncoderSetPipeline(encoderObj, this->pipeline);
    wgpuComputePassEncoderSetBindGroup(encoderObj, 0, computeBindings.bindGroup,
                                       0, nullptr);
    wgpuComputePassEncoderDispatchWorkgroups(encoderObj, x, y, z);
    wgpuComputePassEncoderEnd(encoderObj);
}