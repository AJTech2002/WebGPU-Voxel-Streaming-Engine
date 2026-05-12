#include "Engine.h"
#include <iostream>
#include <webgpu/webgpu.h>
#include "WGPUUtils.h"

void Engine::onInit(WGPUSurface surface)
{
    std::cout << "Voxel Engine Init" << std::endl;
    requestAdapter(surface);
    requestDevice();
}

void Engine::onFrame()
{

    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.nextInChain = nullptr;
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);
    WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.nextInChain = nullptr;
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(encoder); // release encoder after it's finished

    // Finally submit the command queue
    wgpuQueueSubmit(queue, 1, &command);

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
        wgpuInstanceProcessEvents(instance);
#endif
    }

    wgpuCommandBufferRelease(command);
}

void Engine::onFinish()
{
    wgpuDeviceRelease(this->device);
    wgpuAdapterRelease(this->adapter);
    wgpuInstanceRelease(this->instance);
    wgpuQueueRelease(this->queue);
}

WGPUInstance Engine::initializeInstance()
{
    WGPUInstanceDescriptor desc = {};
    desc.nextInChain = nullptr;

#ifdef WEBGPU_BACKEND_EMSCRIPTEN
    this->instance = wgpuCreateInstance(nullptr);
#else  //  WEBGPU_BACKEND_EMSCRIPTEN
    this->instance = wgpuCreateInstance(&desc);
#endif //  WEBGPU_BACKEND_EMSCRIPTEN

    // We can check whether there is actually an instance created
    if (!this->instance)
    {
        std::cerr << "Could not initialize WebGPU!" << std::endl;
        return nullptr;
    }

    // Display the object (WGPUInstance is a simple pointer, it may be
    // copied around without worrying about its size).
    std::cout << "WGPU instance: " << this->instance << std::endl;

    return instance;
}

void Engine::requestAdapter(WGPUSurface surface)
{
    WGPURequestAdapterOptions options = WGPU_REQUEST_ADAPTER_OPTIONS_INIT;
    options.powerPreference = WGPUPowerPreference_HighPerformance;
    options.nextInChain = nullptr;
    options.compatibleSurface = surface;

    std::cout << "Requesting adapter..." << std::endl;

    this->adapter = requestAdapterSync(this->instance, &options);

    std::cout << "Got adapter: " << this->adapter << std::endl;
}

void Engine::requestDevice()
{
    WGPUDeviceDescriptor deviceDesc = {};

    deviceDesc.nextInChain = nullptr;
    deviceDesc.label = {
        "My Device", strlen("My Device")};
    deviceDesc.defaultQueue.nextInChain = nullptr;
    deviceDesc.requiredFeatureCount = 0;
    deviceDesc.requiredLimits = nullptr;

    deviceDesc.deviceLostCallbackInfo = {};
    deviceDesc.deviceLostCallbackInfo.callback = nullptr;

    std::cout << "Looking for device" << std::endl;

    this->device = requestDeviceSync(this->adapter, &deviceDesc);

    std::cout << "Got Device : " << this->device << std::endl;

    this->queue = wgpuDeviceGetQueue(device);
}