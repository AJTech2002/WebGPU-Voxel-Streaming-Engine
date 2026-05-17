#pragma once
#include "Engine.h"
#include "RenderContext.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <webgpu/webgpu.h>

class Shader
{
  public:
    std::string source;
    WGPUShaderModule shaderModule;

    Shader(const std::string &filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open shader file: " + filePath);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        source = buffer.str();

        file.close();

        if (source.empty())
        {
            throw std::runtime_error("Shader file is empty: " + filePath);
        }

        WGPUShaderModuleDescriptor shaderDesc =
            WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
#ifdef WEBGPU_BACKEND_WGPU
        shaderDesc.hintCount = 0;
        shaderDesc.hints = nullptr;
#endif

        WGPUShaderSourceWGSL shaderSource = WGPU_SHADER_SOURCE_WGSL_INIT;
        shaderSource.code = {.data = (source.c_str()), .length = WGPU_STRLEN};
        shaderSource.chain.next = nullptr;
        shaderSource.chain.sType = WGPUSType_ShaderSourceWGSL;

        shaderDesc.nextInChain = &shaderSource.chain;
        shaderModule =
            wgpuDeviceCreateShaderModule(Engine::get().ctx.device, &shaderDesc);
    }

    const std::string &getSource() const { return source; }

    ~Shader()
    {
        if (shaderModule)
        {
            wgpuShaderModuleRelease(shaderModule);
        }
    }
};