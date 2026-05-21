#include "engine/Scene.h"
#include <webgpu/webgpu.h>

struct alignas(16) Uniform
{
    float viewProj[16];    // 0
    float invViewProj[16]; // 64
    float camPos[3];       // 128
    uint32_t voxelCount;   // 140
    float _pad[3];         // 144 — force struct to next 16-byte boundary
};

class VoxelScene : public Scene
{
  public:
    VoxelScene(Engine *engine) : Scene(engine) {}
    ~VoxelScene() {};

    void onPreRender(float dt, WGPUCommandEncoder &activeEncoder) override;
    void start() override;
    void finish() override;
};