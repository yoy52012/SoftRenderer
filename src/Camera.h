#pragma once

#include <glm/glm.hpp>

namespace SoftRenderer
{
	class Camera
	{
	public:
		Camera(const glm::vec3& eye, const glm::vec3& target, float aspect, float fovy = 45.0f, float near = 0.01f, float far = 10000.0f);
		~Camera() = default;

		glm::mat4 getViewMatrix() const;

		glm::mat4 getProjMatrix() const;

		inline float getFov() const { return mFovy; }
		inline float getAspect() const { return mAspect; }
		inline float getNear() const { return mNear; }
		inline float getFar() const { return mFar; }

		inline const glm::vec3& getEye() const { return mEye; }
		inline const glm::vec3& getTarget() const { return mTarget; }
		inline const glm::vec3& getUp() const { return mUp; }
		inline const glm::vec3& getForward() const { return glm::normalize(mTarget - mEye); }

	private:
		glm::vec3 mEye;
		glm::vec3 mTarget;
		glm::vec3 mUp;

		float mAspect;
		float mFovy;
		float mNear;
		float mFar;
	};


}