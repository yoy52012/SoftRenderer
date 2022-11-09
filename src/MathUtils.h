#pragma once


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/type_aligned.hpp>


namespace SoftRenderer
{
	class Math
	{
	public:
        static float toRadians(float degrees);

        static glm::mat4 lootAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);

        static glm::mat4 calViewportMatrix(int width, int height);

        static glm::mat4 calPerspectiveProjMatrix(float fovy, float aspect, float near, float far);

        static glm::mat4 calOrthoProjMatrix(float left, float right, float bottom, float top, float near, float far);

        static float uchar2float(unsigned char value);

        static unsigned char float2uchar(float value);
		
	public:
		static const float PI;
		static const float TWO_PI;
		static const float HALF_PI;
		static const float EPSILON;
	};
}
