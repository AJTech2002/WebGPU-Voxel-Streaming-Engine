#ifndef WGPU_UTILS_H
#define WGPU_UTILS_H
#include "webgpu/webgpu.h"
#include <cassert>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

WGPUAdapter requestAdapterSync(WGPUInstance instance,
                               WGPURequestAdapterOptions const *options);
WGPUDevice requestDeviceSync(WGPUAdapter adapter,
                             WGPUDeviceDescriptor const *descriptor);
void pollWgpuEvents([[maybe_unused]] WGPUDevice device,
                    [[maybe_unused]] bool yieldToBrowser,
                    [[maybe_unused]] bool *waitFor);
#define STRVIEW(str)                                                           \
    (WGPUStringView) { .data = (str), .length = WGPU_STRLEN }
#endif // WGPU_UTILS_H