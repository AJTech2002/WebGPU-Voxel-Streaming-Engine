#include "Renderer.h"
#include <webgpu/webgpu.h>

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
  colorAttachment.clearValue = {0.0f, 0.5f, 0.5f, 1.0f};
  colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

  renderPassDesc.colorAttachments = &colorAttachment;
  renderPassDesc.depthStencilAttachment = nullptr;
  renderPassDesc.timestampWrites = nullptr;

  WGPURenderPassEncoder renderPass =
      wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

  wgpuRenderPassEncoderEnd(renderPass);
  wgpuRenderPassEncoderRelease(
      renderPass); // release render pass after it's finished
}