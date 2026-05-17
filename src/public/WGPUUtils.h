#ifndef WGPU_UTILS_H
#define WGPU_UTILS_H
#include "webgpu/webgpu.h"
#include <cassert>
#include <iostream>
#include <string>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

WGPUAdapter requestAdapterSync(WGPUInstance instance,
                               WGPURequestAdapterOptions const *options);
WGPUDevice requestDeviceSync(WGPUAdapter adapter,
                             WGPUDeviceDescriptor const *descriptor);
#define STRVIEW(str)                                                           \
    (WGPUStringView) { .data = (str), .length = WGPU_STRLEN }
#endif // WGPU_UTILS_H