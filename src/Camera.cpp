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

    /*
     * eye: the position of the eye point
     * target: the position of the target point
     * up: the direction of the up vector
     *
     * x_axis.x  x_axis.y  x_axis.z  -dot(x_axis,eye)
     * y_axis.x  y_axis.y  y_axis.z  -dot(y_axis,eye)
     * z_axis.x  z_axis.y  z_axis.z  -dot(z_axis,eye)
     *        0         0         0                 1
     *
     * z_axis: normalize(eye-target), the backward vector
     * x_axis: normalize(cross(up,z_axis)), the right vector
     * y_axis: cross(z_axis,x_axis), the up vector
     *
     * see http://www.songho.ca/opengl/gl_camera.html
     */
    //glm::lookAt(mEye, mCenter, mUp)
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


    /*
     * fovy: the field of view angle in the y direction, in radians
     * aspect: the aspect ratio, defined as width divided by height
     * near, far: the distances to the near and far depth clipping planes
     *
     * 1/(aspect*tan(fovy/2))              0             0           0
     *                      0  1/tan(fovy/2)             0           0
     *                      0              0  -(f+n)/(f-n)  -2fn/(f-n)
     *                      0              0            -1           0
     *
     * this is the same as
     *     float half_h = near * (float)tan(fovy / 2);
     *     float half_w = half_h * aspect;
     *     mat4_frustum(-half_w, half_w, -half_h, half_h, near, far);
     *
     * see http://www.songho.ca/opengl/gl_projectionmatrix.html
     */
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