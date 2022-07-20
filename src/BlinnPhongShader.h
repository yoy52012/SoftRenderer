#pragma once
#include "Shader.h"

namespace SoftRenderer
{
    class BlinnPhongShaderAttributes : public BaseShaderAttributes
    {

    };

    class BlinnPhongShaderVaryings : public BaseShaderVaryings
    {

    };

    class BlinnPhongShaderUniforms : public BaseShaderUniforms
    {
        Sampler2D uEmissiveMap;
        Sampler2D uDiffuseMap;
        Sampler2D uNormalMap;
        Sampler2D uAoMap;
    };

    class BlinnPhongVertexShader : public BaseVertexShader
    {
        
    };

    class BlinnPhongFragmentShader : public BaseFragmentShader
    {

    };
}