#pragma once

#include "MathUtils.h"

namespace SoftRenderer
{
	class Camera
	{
	public:
		Camera(float fovy, float aspect, float near = 0.1f, float far = 100.0f);
		~Camera() = default;

		void lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.f));

		glm::mat4 getViewMatrix() const;

		glm::mat4 getProjMatrix() const;

		inline float getFov() const { return mFovy; }
		inline float getAspect() const { return mAspect; }
		inline float getNear() const { return mNear; }
		inline float getFar() const { return mFar; }

		inline const glm::vec3& getEye() const { return mEye; }
		inline const glm::vec3& getCenter() const { return mCenter; }
		inline const glm::vec3& getUp() const { return mUp; }
		inline const glm::vec3& getForward() const { return glm::normalize(mCenter - mEye); }

	private:
		glm::vec3 mEye = {};
		glm::vec3 mCenter = {};
		glm::vec3 mUp = {};

		float mAspect = 1.0f;
		float mFovy = 60.0f;
		float mNear = 0.01f;
		float mFar = 100.0f;
	};


}