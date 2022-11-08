#include "Camera.h"
namespace SoftRenderer
{
	static const glm::vec3 UP = { 0, 1, 0 };

	Camera::Camera(const glm::vec3& eye, const glm::vec3& target, float aspect, float fovy, float near, float far)
		:mEye(eye)
		,mTarget(target)
		,mAspect(aspect)
		,mFovy(fovy)
		,mNear(near)
		,mFar(far)
	{
	}

	glm::mat4 Camera::getViewMatrix() const
	{
		return glm::lookAt(mEye, mTarget, UP);
	}

	glm::mat4 Camera::getProjMatrix() const
	{
		return glm::perspective(Math::toRadians(mFovy), mAspect, mNear, mFar);
	}
}