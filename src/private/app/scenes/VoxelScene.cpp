#include "app/scenes/VoxelScene.h"
#include "engine/Buffer.h"
#include "engine/Renderer.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/glm.hpp"
#include "glm/trigonometric.hpp"
#include <iostream>
#include <webgpu/webgpu.h>

Uniform *uniforms = new Uniform();
Buffer<Uniform> uniformBuffer = Buffer<Uniform>(uniforms, sizeof(Uniform));
Compute compute;

void VoxelScene::start()
{
    const RenderResources &resources = this->engine->renderer.resources;
    compute.bindings().addStorageTextureBinding(
        0, resources.screenTextureView, WGPUStorageTextureAccess_WriteOnly,
        WGPUTextureFormat_RGBA8Unorm, WGPUShaderStage_Compute);

    glm::vec3 camPos = glm::vec3(0.0f, 20.0, 100.0f);
    glm::vec3 camTarget = camPos + glm::vec3(0.0, 0.0, 1.0);
    glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 viewProj = proj;
    glm::mat4 invViewProj = glm::inverse(viewProj);

    memcpy(uniforms->viewProj, &viewProj[0][0], sizeof(float) * 16);
    memcpy(uniforms->invViewProj, &invViewProj[0][0], sizeof(float) * 16);
    uniforms->camPos[0] = camPos.x;
    uniforms->camPos[1] = camPos.y;
    uniforms->camPos[2] = camPos.z;

    uniformBuffer.upload(); // ADD THIS

    WGPUBuffer uniformGPUBuffer = uniformBuffer.create();

    compute.bindings().addUniformBufferBinding(1, uniformGPUBuffer,
                                               WGPUShaderStage_Compute);

    Shader computeShader = Shader("compute.wgsl", Engine::get().ctx.device);
    compute.create(computeShader);

    std::cout << "Created shader & start" << std::endl;
};

float _time = 0.0;

void VoxelScene::onPreRender(float dt, WGPUCommandEncoder &activeEncoder)
{

    _time += dt;
    glm::vec3 newCamPos =
        glm::vec3(glm::sin<float>(_time) * 20.0, 20.0, 100.0f);

    uniforms->camPos[0] = newCamPos.x;
    uniformBuffer.upload(); // ADD THIS
    compute.dispatch(100, 100, 1, activeEncoder);
}

void VoxelScene::finish() {}