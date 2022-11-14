#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/type_aligned.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


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

        static glm::quat createQuatFromVector(const glm::vec3& from, const glm::vec3& to);
		
	public:
		static const float PI;
		static const float TWO_PI;
		static const float HALF_PI;
		static const float EPSILON;
	};

    class Spherical 
    {
    public:
        Spherical(float radius, float phi, float theta) 
            :mRadius(radius)
            ,mPhi(phi)
            ,mTheta(theta)
        {
        }

        Spherical() = default;
        ~Spherical() = default;

        Spherical(const Spherical& other)
        {
            mRadius = other.mRadius;
            mPhi = other.mPhi;
            mTheta = other.mTheta;
        }

        Spherical& operator=(const Spherical& other)
        {
            mRadius = other.mRadius;
            mPhi = other.mPhi;
            mTheta = other.mTheta;
        }

        void set(float radius, float phi, float theta) 
        {
            mRadius = radius;
            mPhi = phi;
            mTheta = theta;
        }


        // restrict phi to be between EPS and PI-EPS
        void makeSafe() 
        {
            const float EPS = 0.000001f;
            mPhi = glm::max(EPS, glm::min(Math::PI - EPS, mPhi));
        }

        void setFromVector3(const glm::vec3& v) 
        {
            return setFromCartesianCoords(v.x, v.y, v.z);
        }

        void setFromCartesianCoords(float x, float y, float z) 
        {
            mRadius = std::sqrt(x * x + y * y + z * z);

            if (mRadius == 0) 
            {
                mTheta = 0;
                mPhi = 0;

            }
            else 
            {
                mTheta = std::atan2(x, z);
                mPhi = std::acos(glm::clamp<float>(y / mRadius, -1.0f, 1.0f));
            }
        }

    public:
        float mRadius = 1.0f;

        // polar angle
        float mPhi = 0.0f;

        // azimuthal angle
        float mTheta = 0.0f;
    };
}
