#include "app/scenes/VoxelScene.h"
#include "engine/Buffer.h"
#include "engine/Renderer.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/glm.hpp"
#include "glm/trigonometric.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <webgpu/webgpu.h>

#define MAX_VOXELS 10000

Uniform *uniforms = new Uniform();
Buffer<Uniform> uniformBuffer = Buffer<Uniform>(uniforms, sizeof(Uniform));

Voxel voxels[MAX_VOXELS];
int currentVoxelCount = 0;

Buffer<Voxel> voxelBuffer = Buffer<Voxel>(voxels, sizeof(Voxel) * MAX_VOXELS);

Compute compute;

static float randomRange(float minValue, float maxValue)
{
    const float t = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    return minValue + (maxValue - minValue) * t;
}

void VoxelScene::setupUniforms()
{

    glm::vec3 camPos = glm::vec3(0.0f, 20.0, 200.0f);
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

    uniformBuffer.upload();
}

void VoxelScene::setupVoxels()
{
    int resolution = 12; // increase for more voxels, smoother sphere
    float radius = 10.0f;
    glm::vec3 center(0.0f, 15.0f, 60.0f);

    // resolution^3 max, but only voxels on the sphere surface are kept
    int voxelIndex = 0;

    for (int x = -resolution; x <= resolution; x++)
        for (int y = -resolution; y <= resolution; y++)
            for (int z = -resolution; z <= resolution; z++)
            {
                glm::vec3 offset(x, y, z);
                float dist = glm::length(offset);

                // Only keep voxels within a shell on the sphere surface
                float shellThickness = 1.2f;
                if (dist < resolution - shellThickness || dist > resolution)
                    continue;
                if (voxelIndex >= MAX_VOXELS)
                    break;

                // Normal points outward from center — gives smooth shading
                glm::vec3 normal = glm::normalize(offset);

                // Scale position to match radius
                glm::vec3 pos = center + normal * radius;

                float voxelSize = (radius / resolution) *
                                  2.2f; // slight overlap to avoid gaps

                Voxel newVoxel{
                    {pos.x, pos.y, pos.z},
                    voxelSize,
                    {normal.x * 0.5f + 0.5f, // visualize normal as color
                     normal.y * 0.5f + 0.5f, normal.z * 0.5f + 0.5f},
                    0,
                    {normal.x, normal.y, normal.z},
                };

                voxelBuffer.data[voxelIndex++] = newVoxel;
            }

    uniforms->voxelCount = voxelIndex;
    voxelBuffer.upload();
    uniformBuffer.upload();
}

void VoxelScene::start()
{
    srand(static_cast<unsigned int>(time(nullptr)));

    const RenderResources &resources = this->engine->renderer.resources;
    compute.bindings().addStorageTextureBinding(
        0, resources.screenTextureView, WGPUStorageTextureAccess_WriteOnly,
        WGPUTextureFormat_RGBA8Unorm, WGPUShaderStage_Compute);

    // Populate Voxels
    setupUniforms();
    setupVoxels();

    WGPUBuffer uniformGPUBuffer = uniformBuffer.create();
    WGPUBuffer voxelGPUBuffer = voxelBuffer.create();

    compute.bindings().addUniformBufferBinding(1, uniformGPUBuffer,
                                               WGPUShaderStage_Compute);

    compute.bindings().addStorageBufferBinding(2, voxelGPUBuffer,
                                               WGPUShaderStage_Compute, true);

    Shader computeShader =
        Shader("compute-iterative.wgsl", Engine::get().ctx.device);
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