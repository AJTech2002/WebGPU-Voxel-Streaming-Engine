#include "webgpu/webgpu.h"
#include <iostream>
#include <cassert>
#include <string>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
WGPUAdapter requestAdapterSync(
    WGPUInstance instance,
    WGPURequestAdapterOptions const *options)
{
    struct UserData
    {
        WGPUAdapter adapter = nullptr;
        bool requestEnded = false;
    };

    UserData userData;

    auto onAdapterRequestEnded = [](
                                     WGPURequestAdapterStatus status,
                                     WGPUAdapter adapter,
                                     WGPUStringView message,
                                     void *userdata1,
                                     void *userdata2)
    {
        (void)userdata2;

        auto *userData = static_cast<UserData *>(userdata1);

        if (userData == nullptr)
        {
            std::cerr << "Adapter callback userdata was null\n";
            return;
        }

        if (status == WGPURequestAdapterStatus_Success)
        {
            userData->adapter = adapter;
        }
        else
        {
            std::string errorMessage;

            if (message.data != nullptr)
            {
                if (message.length == WGPU_STRLEN)
                {
                    errorMessage = message.data;
                }
                else
                {
                    errorMessage.assign(message.data, message.length);
                }
            }

            std::cerr << "Could not get WebGPU adapter: "
                      << errorMessage
                      << std::endl;
        }

        userData->requestEnded = true;
    };

    WGPURequestAdapterCallbackInfo callbackInfo =
        WGPU_REQUEST_ADAPTER_CALLBACK_INFO_INIT;

    callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
    callbackInfo.callback = onAdapterRequestEnded;
    callbackInfo.userdata1 = &userData;
    callbackInfo.userdata2 = nullptr;

    wgpuInstanceRequestAdapter(instance, options, callbackInfo);

#ifdef __EMSCRIPTEN__
    while (userData.requestEnded == false)
    {
        emscripten_sleep(100);
    }
#endif

    assert(userData.adapter != nullptr);
    return userData.adapter;
}

WGPUDevice requestDeviceSync(WGPUAdapter adapter, WGPUDeviceDescriptor const *descriptor)
{
    struct UserData
    {
        WGPUDevice device = nullptr;
        bool requestEnded = false;
    };

    UserData data;

    WGPURequestDeviceCallbackInfo callbackInfo = WGPU_REQUEST_DEVICE_CALLBACK_INFO_INIT;
    callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
    callbackInfo.userdata1 = &data;

    callbackInfo.callback = [](WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, WGPU_NULLABLE void *userdata1, WGPU_NULLABLE void *userdata2)
    {
        UserData &userData = *reinterpret_cast<UserData *>(userdata1);
        if (status == WGPURequestDeviceStatus_Success)
        {
            userData.device = device;
        }
        else
        {
            std::cout << " Could not get WebGPU Device : " << message.data << std::endl;
        }

        userData.requestEnded = true;
    };

    wgpuAdapterRequestDevice(adapter, descriptor, callbackInfo);

#ifdef __EMSCRIPTEN__
    while (data.requestEnded == false)
    {
        emscripten_sleep(100);
    }
#endif

    return data.device;
}