#include <webgpu/webgpu.h>
class Renderer
{
public:
  void render(WGPUCommandEncoder &encoder, WGPUSurface &surface,
              WGPUTextureView &target);
};
