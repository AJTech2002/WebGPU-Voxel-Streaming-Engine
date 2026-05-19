#include "Debug.h"
#include "Engine.h"
#include "glm/packing.hpp"
#include <glm/glm.hpp>
#include <vector>
using namespace glm;

/* Fill a texture with debug data (a gradient) and write it to the GPU. */
void debugTextureData(WGPUTextureDescriptor textureDesc, WGPUTexture texture)
{
    uint32_t width = textureDesc.size.width;
    uint32_t height = textureDesc.size.height;

    std::vector<uint32_t> pixels(2 * width * height);

    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            uint32_t *p = &pixels[2 * (y * width + x)];

            float r = 0.0f;
            float g = float(y) / float(height);
            float b = 0.5f;
            float a = 1.0f;

            p[0] = glm::packHalf2x16(glm::vec2(r, g)); // RG
            p[1] = glm::packHalf2x16(glm::vec2(b, a)); // BA
        }
    }

    WGPUTexelCopyTextureInfo destination = {};
    destination.texture = texture;
    destination.mipLevel = 0;
    destination.origin = {0, 0, 0};
    destination.aspect = WGPUTextureAspect_All;

    WGPUTexelCopyBufferLayout dataLayout = {};
    dataLayout.offset = 0;
    dataLayout.bytesPerRow = 8 * width;
    dataLayout.rowsPerImage = textureDesc.size.height;

    WGPUExtent3D writeSize = {};
    writeSize.width = textureDesc.size.width;
    writeSize.height = textureDesc.size.height;
    writeSize.depthOrArrayLayers = 1;

    wgpuQueueWriteTexture(Engine::get().ctx.queue, &destination, pixels.data(),
                          pixels.size() * sizeof(uint32_t), &dataLayout,
                          &writeSize);
}