#include "webgpu/webgpu.h"

class Engine
{

public:
    WGPUInstance instance;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;

    WGPUInstance initializeInstance();
    void onInit(WGPUSurface surface);
    void onFrame();
    void onFinish();

private:
    void requestAdapter(WGPUSurface surface);
    void requestDevice();
};