#pragma once

#include "Shader.h"

namespace SoftRenderer
{
    class Uniform
    {
    public:
        explicit Uniform();

        virtual int32_t getLocation(const Program& program) = 0;
    }
}