#pragma once

#include "engine/Engine.h"
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <webgpu/webgpu.h>

template <typename T> class Buffer
{
    static_assert(alignof(T) >= 16,
                  "Buffer type must be 16-byte aligned (add alignas(16))");
    static_assert(sizeof(T) % 16 == 0,
                  "Buffer type size must be a multiple of 16 bytes");
    static_assert(std::is_trivially_copyable_v<T>,
                  "Buffer type must be trivially copyable for GPU upload");

  public:
    Buffer(T *data, std::size_t size) : data(data), size(size) {}
    Buffer() : data(nullptr), size(0) {}
    T *data;
    std::size_t size;
    WGPUBuffer &create();
    void upload();

    const WGPUBuffer &get() const { return gpuBuffer; }

  private:
    WGPUBuffer gpuBuffer;
    void resize(std::size_t newSize);
};

template <typename T> inline WGPUBuffer &Buffer<T>::create()
{
    assert(data != nullptr &&
           "Buffer data must not be null when creating GPU buffer!");

    if (gpuBuffer)
        return gpuBuffer;

    WGPUBufferDescriptor bufferDesc = WGPU_BUFFER_DESCRIPTOR_INIT;
    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform |
                       WGPUBufferUsage_Storage;

    gpuBuffer = wgpuDeviceCreateBuffer(Engine::get().ctx.device, &bufferDesc);
    wgpuQueueWriteBuffer(Engine::get().ctx.queue, gpuBuffer, 0, data, size);

    return gpuBuffer;
}

template <typename T> inline void Buffer<T>::upload()
{
    assert(data != nullptr &&
           "Buffer data must not be null when uploading to GPU!");

    if (!gpuBuffer)
    {
        create();
    }
    else
    {
        wgpuQueueWriteBuffer(Engine::get().ctx.queue, gpuBuffer, 0, data, size);
    }
}

template <typename T> inline void Buffer<T>::resize(std::size_t newSize)
{
    if (newSize == size)
        return;

    if (gpuBuffer)
    {
        wgpuBufferRelease(gpuBuffer);
        gpuBuffer = nullptr;
    }

    size = newSize;
    data = new T[size / sizeof(T)];

    create();
}