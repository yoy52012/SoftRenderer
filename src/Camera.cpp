#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include "MathUtils.h"

namespace SoftRenderer
{
	static const glm::vec3 UP = { 0, 1, 0 };

	Camera::Camera(const glm::vec3& position, const glm::vec3& target, float aspect, float fovy, float near, float far)
		:mPosition(position)
		,mTarget(target)
		,mAspect(aspect)
		,mFovy(fovy)
		,mNear(near)
		,mFar(far)
	{
	}

	glm::vec3 Camera::getPosition() const
	{
		return mPosition;
	}

	glm::vec3 Camera::getForward() const
	{
		return glm::normalize(mTarget - mPosition);
	}

	glm::mat4 Camera::getViewMatrix() const
	{
		return glm::lookAt(mPosition, mTarget, UP);
	}

	glm::mat4 Camera::getProjMatrix() const
	{
		return glm::perspective(MathUtils::toRadians(mFovy), mAspect, mNear, mFar);
	}
}