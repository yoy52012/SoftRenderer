#include "Camera.h"

namespace SoftRenderer
{
    Camera::Camera(float fovy, float aspect, float n, float f)
        : mFovy(fovy)
        , mAspect(aspect)
        , mNear(n)
        , mFar(f)
    {
    }

    void Camera::lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
    {
        mEye = eye;
        mCenter = center;
        mUp = up;
    }

    // glm::lookAt(mEye, mCenter, mUp)
	glm::mat4 Camera::getViewMatrix() const
	{
        glm::vec3 forward(glm::normalize(mCenter - mEye));
        glm::vec3 right(glm::normalize(glm::cross(forward, mUp)));
        glm::vec3 up(glm::cross(right, forward));

        glm::mat4 view(1.f);

        view[0][0] = right.x;
        view[1][0] = right.y;
        view[2][0] = right.z;
        view[3][0] = -glm::dot(right, mEye);

        view[0][1] = up.x;
        view[1][1] = up.y;
        view[2][1] = up.z;
        view[3][1] = -glm::dot(up, mEye);

        view[0][2] = -forward.x;
        view[1][2] = -forward.y;
        view[2][2] = -forward.z;
        view[3][2] = glm::dot(forward, mEye);

        return view;
	}

    // glm::perspective(glm::radians(mFovy), mAspect, mNear, mFar)
	glm::mat4 Camera::getProjMatrix() const
	{
        float tanHalfFovInverse = 1.0f / std::tan((glm::radians(mFovy) * 0.5f));

        glm::mat4 projection(0.0f);
        projection[0][0] = tanHalfFovInverse / mAspect;
        projection[1][1] = tanHalfFovInverse;
        projection[2][2] = -mFar / (mFar - mNear);
        projection[3][2] = -(mFar * mNear) / (mFar - mNear);
        projection[2][3] = -1.f;

        return projection;
	}
}