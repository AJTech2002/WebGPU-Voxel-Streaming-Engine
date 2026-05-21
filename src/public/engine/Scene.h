#pragma once
#include "Engine.h"
#include <webgpu/webgpu.h>

class Scene
{
  public:
    ~Scene() {};
    Scene(Engine *engine) : engine(engine) {};

    virtual void start() {}
    virtual void onPreRender(float dt, WGPUCommandEncoder &activeEncoder) {}
    virtual void finish() {}

  protected:
    Engine *engine;
};