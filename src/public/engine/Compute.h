#pragma once
#include "Bindings.h"
#include "Shader.h"
#include <webgpu/webgpu.h>

class Compute
{
  public:
    WGPUComputePipeline &create(Shader &shader);
    void dispatch(int x, int y, int z, WGPUCommandEncoder &encoder);
    Bindings &bindings() { return this->computeBindings; }

  private:
    Bindings computeBindings;

    WGPUComputePipeline pipeline;
};