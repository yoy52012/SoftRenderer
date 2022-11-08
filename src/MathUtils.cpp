#include "MathUtils.h"

namespace SoftRenderer
{
    const float Math::PI = 3.14159265359f;
    const float Math::TWO_PI = 6.28318530718f;
    const float Math::HALF_PI = 1.57079632679f;
    const float Math::EPSILON = 1e-5f;

    float Math::toRadians(float degrees)
    {
        return (PI / 180) * (degrees);
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
    glm::mat4 Math::lootAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up)
    {
        glm::vec3 zAxis = glm::normalize(eye -  target);
        glm::vec3 xAxis = glm::normalize(glm::cross(up, zAxis));
        glm::vec3 yAxis = glm::cross(zAxis, xAxis);

        glm::mat4 m = glm::mat4(1.0f);
        m[0][0] = xAxis.x; m[0][1] = yAxis.x; m[0][2] = zAxis.x; m[0][3] = 0.0f;
        m[1][0] = xAxis.y; m[1][1] = yAxis.y; m[1][2] = zAxis.y; m[1][3] = 0.0f;
        m[2][0] = xAxis.z; m[2][1] = yAxis.z; m[2][2] = zAxis.z; m[2][3] = 0.0f;
        m[3][0] = -glm::dot(xAxis, eye);
        m[3][1] = -glm::dot(yAxis, eye);
        m[3][2] = -glm::dot(zAxis, eye);
        m[3][3] = 1.0f;

        return m;
    }

    glm::mat4 Math::calViewportMatrix(int width, int height)
    {
        glm::mat4 viewportMat;
        float hWidth = width * 0.5f;
        float hHeight = height * 0.5f;
        viewportMat[0][0] = width * 0.5f;  viewportMat[0][1] = 0.0f;             viewportMat[0][2] = 0.0f; viewportMat[0][3] = 0.0f;
        viewportMat[1][0] = 0.0f;	       viewportMat[1][1] = -(height * 0.5f); viewportMat[1][2] = 0.0f; viewportMat[1][3] = 0.0f;
        viewportMat[2][0] = 0.0f;          viewportMat[2][1] = 0.0f;             viewportMat[2][2] = 1.0f; viewportMat[2][3] = 0.0f;
        viewportMat[3][0] = width * 0.5f;  viewportMat[3][1] = height * 0.5f;    viewportMat[3][2] = 0.0f; viewportMat[3][3] = 0.0f;
       
        return viewportMat;
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
	glm::mat4 Math::calPerspectiveProjMatrix(float fovy, float aspect, float near, float far)
	{
        glm::mat4 pMat;
        const float tanHalfFovy = std::tan(fovy * 0.5f);
        float f_n = far - near;
        pMat[0][0] = 1.0f / (aspect * tanHalfFovy); pMat[0][1] = 0.0f;				  pMat[0][2] = 0.0f;					 pMat[0][3] = 0.0f;
        pMat[1][0] = 0.0f;						    pMat[1][1] = 1.0f / tanHalfFovy;  pMat[1][2] = 0.0f;					 pMat[1][3] = 0.0f;
        pMat[2][0] = 0.0f;						    pMat[2][1] = 0.0f;			      pMat[2][2] = -(far + near) / f_n;		 pMat[2][3] = -1.0f;
        pMat[3][0] = 0.0f;						    pMat[3][1] = 0.0f;				  pMat[3][2] = -2.0f * near * far / f_n; pMat[3][3] = 0.0f;

        return pMat;
	}

    /*
     * left, right: the coordinates for the left and right clipping planes
     * bottom, top: the coordinates for the bottom and top clipping planes
     * near, far: the distances to the near and far depth clipping planes
     *
     * 2/(r-l)        0         0  -(r+l)/(r-l)
     *       0  2/(t-b)         0  -(t+b)/(t-b)
     *       0        0  -2/(f-n)  -(f+n)/(f-n)
     *       0        0         0             1
     *
     * see http://docs.gl/gl2/glOrtho
     */
	glm::mat4 Math::calOrthoProjMatrix(float left, float right, float bottom, float top, float near, float far)
	{
        glm::mat4 pMat;
        pMat[0][0] = 2.0f / (right - left);           pMat[0][1] = 0.0f;                           pMat[0][2] = 0.0f;                         pMat[0][3] = 0.0f;
        pMat[1][0] = 0.0f;				              pMat[1][1] = 2.0f / (top - bottom);          pMat[1][2] = 0.0f;                         pMat[1][3] = 0.0f;
        pMat[2][0] = 0.0f;                            pMat[2][1] = 0.0f;                           pMat[2][2] = -2.0f / (far - near);         pMat[2][3] = 0.0f;
        pMat[3][0] = -(left + right) / right - left;  pMat[3][1] = -(bottom + top) / top - bottom; pMat[3][2] = -(far + near) / (far - near); pMat[3][3] = 1.0f;
        
        return pMat;
	}

    float Math::uchar2float(unsigned char value)
    {
        return value / 255.0f;
    }

    unsigned char Math::float2uchar(float value)
    {
        return static_cast<unsigned char>(value * 255.0f);
    }
}


