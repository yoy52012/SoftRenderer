#pragma once

#include <glm/glm.hpp>

namespace SoftRenderer
{
	class Camera
	{
	public:
		Camera(const glm::vec3& position, const glm::vec3& target, float aspect, float fovy = 45.0f, float near = 0.01f, float far = 10000.0f);
		~Camera() = default;

		glm::vec3 getPosition() const;

		glm::vec3 getForward() const;

		glm::mat4 getViewMatrix() const;

		glm::mat4 getProjMatrix() const;

	private:
		glm::vec3 mPosition;
		glm::vec3 mTarget;
		float mAspect;
		float mFovy;
		float mNear;
		float mFar;
	};


}