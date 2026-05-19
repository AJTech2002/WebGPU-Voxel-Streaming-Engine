#include "engine/Engine.h"
#include "engine/RenderContext.h"
#include "utils/WGPUUtils.h"
#include <iostream>
#include <webgpu/webgpu.h>

void Engine::onInit(WGPUSurface surface)
{
    ctx.surface = surface;

    requestAdapter();
    requestDevice();

    configureSurface(surface);
    renderer.setup();
}

void Engine::configureSurface(WGPUSurface surface)
{

    WGPUSurfaceCapabilities capabilities;
    wgpuSurfaceGetCapabilities(surface, ctx.adapter, &capabilities);

    ctx.surfaceFormat = WGPUTextureFormat_Undefined;
    if (capabilities.formatCount > 0)
    {
        ctx.surfaceFormat = capabilities.formats[0];
    }
    else
    {
        std::cerr << "No supported surface formats found!" << std::endl;
        return;
    }

    WGPUSurfaceConfiguration config = WGPU_SURFACE_CONFIGURATION_INIT;
    config.nextInChain = nullptr;
    config.format = ctx.surfaceFormat;
    config.width = 800;
    config.height = 800;
    config.viewFormatCount = 0;
    config.viewFormats = nullptr;
    config.usage = WGPUTextureUsage_RenderAttachment;
    config.device = ctx.device;
    config.presentMode = WGPUPresentMode_Fifo;
    config.alphaMode = WGPUCompositeAlphaMode_Auto;

    wgpuSurfaceConfigure(surface, &config);
}

std::pair<WGPUSurfaceTexture, WGPUTextureView> Engine::getNextSurfaceTexture()
{
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(ctx.surface, &surfaceTexture);

    if (surfaceTexture.status !=
        WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal)
    {
        std::cerr << "Failed to acquire next surface texture!" << std::endl;
        return {surfaceTexture, nullptr};
    }

    WGPUTextureViewDescriptor viewDescriptor;
    viewDescriptor.nextInChain = nullptr;
    viewDescriptor.label = {"Surface Texture View",
                            strlen("Surface Texture View")};
    viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
    viewDescriptor.dimension = WGPUTextureViewDimension_2D;
    viewDescriptor.baseMipLevel = 0;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect = WGPUTextureAspect_All;
    viewDescriptor.usage = WGPUTextureUsage_RenderAttachment;

    WGPUTextureView targetView =
        wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);
    return {surfaceTexture, targetView};
}

void Engine::onFrame()
{
    auto [surfaceTexture, targetView] = getNextSurfaceTexture();
    if (!targetView)
        return;

    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.nextInChain = nullptr;

    WGPUCommandEncoder encoder =
        wgpuDeviceCreateCommandEncoder(ctx.device, &encoderDesc);

    this->renderer.render(encoder, ctx.surface, targetView);

    WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.nextInChain = nullptr;

    WGPUCommandBuffer command =
        wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);

    wgpuCommandEncoderRelease(encoder); // release encoder after it's finished

    // Finally submit the command queue
    wgpuQueueSubmit(ctx.queue, 1, &command);

    for (int i = 0; i < 5; ++i)
    {
#if defined(WEBGPU_BACKEND_DAWN)
        wgpuDeviceTick(device);
#elif defined(WEBGPU_BACKEND_WGPU)
        wgpuDevicePoll(device, false, nullptr);
#endif
#if defined(WEBGPU_BACKEND_EMSCRIPTEN)
        emscripten_sleep(100);
#else
        wgpuInstanceProcessEvents(ctx.instance);
#endif
    }

    wgpuCommandBufferRelease(command);

    wgpuTextureViewRelease(targetView);
#ifndef __EMSCRIPTEN__
    wgpuSurfacePresent(ctx.surface);
#endif

    wgpuTextureRelease(surfaceTexture.texture);
}

void Engine::onFinish()
{
    renderer.cleanup();
    wgpuSurfaceUnconfigure(ctx.surface);
    wgpuDeviceRelease(ctx.device);
    wgpuAdapterRelease(ctx.adapter);
    wgpuInstanceRelease(ctx.instance);
    wgpuQueueRelease(ctx.queue);
}

WGPUInstance Engine::initializeInstance()
{
    WGPUInstanceDescriptor desc = {};
    desc.nextInChain = nullptr;

#ifdef WEBGPU_BACKEND_EMSCRIPTEN
    ctx.instance = wgpuCreateInstance(nullptr);
#else  //  WEBGPU_BACKEND_EMSCRIPTEN
    ctx.instance = wgpuCreateInstance(&desc);
#endif //  WEBGPU_BACKEND_EMSCRIPTEN

    // We can check whether there is actually an instance created
    if (!ctx.instance)
    {
        std::cerr << "Could not initialize WebGPU!" << std::endl;
        return nullptr;
    }

    // Display the object (WGPUInstance is a simple pointer, it may be
    // copied around without worrying about its size).
    std::cout << "WGPU instance: " << ctx.instance << std::endl;

    return ctx.instance;
}

void Engine::requestAdapter()
{
    WGPURequestAdapterOptions options = WGPU_REQUEST_ADAPTER_OPTIONS_INIT;
    options.powerPreference = WGPUPowerPreference_HighPerformance;
    options.nextInChain = nullptr;
    options.compatibleSurface = ctx.surface;

    std::cout << "Requesting adapter..." << std::endl;

    ctx.adapter = requestAdapterSync(ctx.instance, &options);

    std::cout << "Got adapter: " << ctx.adapter << std::endl;
}

void Engine::requestDevice()
{
    WGPUDeviceDescriptor deviceDesc = {};

    deviceDesc.nextInChain = nullptr;
    deviceDesc.label = {"My Device", strlen("My Device")};
    deviceDesc.defaultQueue.nextInChain = nullptr;
    deviceDesc.requiredFeatureCount = 0;

    WGPULimits limits = WGPU_LIMITS_INIT;
    limits.maxSampledTexturesPerShaderStage = 16;
    deviceDesc.requiredLimits = &limits;

    deviceDesc.deviceLostCallbackInfo = {};
    deviceDesc.deviceLostCallbackInfo.callback = nullptr;

    WGPUUncapturedErrorCallbackInfo errorCallbackInfo = {};

    errorCallbackInfo.callback = [](WGPUDevice const *device,
                                    WGPUErrorType type, WGPUStringView message,
                                    void *userdata1, void *userdata2)
    { std::cerr << "Uncaptured error: " << message.data << std::endl; };

    errorCallbackInfo.nextInChain = nullptr;

    deviceDesc.uncapturedErrorCallbackInfo = errorCallbackInfo;

    ctx.device = requestDeviceSync(ctx.adapter, &deviceDesc);

    std::cout << "Got Device : " << ctx.device << std::endl;

    ctx.queue = wgpuDeviceGetQueue(ctx.device);
}
