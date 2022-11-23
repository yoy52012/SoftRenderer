#include "OrbitControls.h"

#include "InputManager.h"

namespace SoftRenderer
{

    enum STATE
    {
        NONE = -1,
        ROTATE = 0,
        DOLLY = 1,
        PAN = 2
    };

    STATE state = STATE::NONE;

    float EPS = 0.000001f;

    // current position in spherical coordinates
    Spherical spherical;
    Spherical sphericalDelta;

    float scale = 1.0f;
    glm::vec3 panOffset;
    bool zoomChanged = false;

    glm::vec2 rotateStart;
    glm::vec2 rotateEnd;
    glm::vec2 rotateDelta;

    glm::vec2 panStart;
    glm::vec2 panEnd;
    glm::vec2 panDelta;

    glm::vec2 dollyStart;
    glm::vec2 dollyEnd;
    glm::vec2 dollyDelta;

    float panSpeed = 1.0f;
    float rotateSpeed = 1.0f;
    float keyPanSpeed = 1.0f;

    OrbitControllers::OrbitControllers(Camera& camera, Window* window)
        :mCamera(camera)
    {
        
        window->mKeyPressedEvent.addListener(std::bind(&OrbitControllers::onKeyDown, this, std::placeholders::_1));

        window->mMouseButtonPressedEvent.addListener(std::bind(&OrbitControllers::onMouseDown, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        window->mMouseButtonReleasedEvent.addListener(std::bind(&OrbitControllers::onMouseUp, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        window->mCursorMoveEvent.addListener(std::bind(&OrbitControllers::onMouseMove, this, std::placeholders::_1, std::placeholders::_2));

        update();
    }

    float OrbitControllers::getAutoRotationAngle() const
    {
        return 2.0f * Math::PI / 60.0f / 60.0f * autoRotateSpeed;
    }

    float OrbitControllers::getZoomScale() const
    {
        return std::pow(0.95f, zoomSpeed);
    }

    void OrbitControllers::rotateLeft(float angle)
    {
        sphericalDelta.mTheta -= angle;
    }

    void OrbitControllers::rotateUp(float angle)
    {
        sphericalDelta.mPhi -= angle;
    }

    void OrbitControllers::update()
    {
        const auto cameraUp = mCamera.getUp();
        const auto cameraPosition = mCamera.getEye();
        const auto cameraTarget = mCamera.getCenter();

        auto offset = glm::vec3(0);

        // so camera.up is the orbit axis
        auto quat = glm::rotation(cameraUp, glm::vec3(0.0f, 1.0f, 0.0f));
        auto quatInverse = glm::inverse(quat);

        auto lastPosition = glm::vec3();
        auto lastQuaternion = glm::quat();

        auto position = cameraPosition;
        auto target = cameraTarget;

        offset = position - target;

        // rotate offset to "y-axis-is-up" space
        offset =glm::rotate(quat, offset);

        // angle from z-axis around y-axis
        spherical.setFromVector3(offset);

        if (autoRotate&& state == STATE::NONE) 
        {
            rotateLeft(getAutoRotationAngle());
        }

        if (enableDamping) 
        {
            spherical.mTheta += sphericalDelta.mTheta * dampingFactor;
            spherical.mPhi += sphericalDelta.mPhi * dampingFactor;
        }
        else 
        {
            spherical.mTheta += sphericalDelta.mTheta;
            spherical.mPhi += sphericalDelta.mPhi;
        }

        // restrict theta to be between desired limits

        auto minAzimuthAngle1 = minAzimuthAngle;
        auto maxAzimuthAngle1 = maxAzimuthAngle;

        if (std::isfinite(minAzimuthAngle1) && std::isfinite(maxAzimuthAngle1))
        {
            if (minAzimuthAngle1 < -Math::PI) minAzimuthAngle1 += Math::TWO_PI; else if (minAzimuthAngle1 > Math::PI) minAzimuthAngle1 -= Math::TWO_PI;

            if (maxAzimuthAngle1 < -Math::PI) maxAzimuthAngle1 += Math::TWO_PI; else if (maxAzimuthAngle1 > Math::PI) maxAzimuthAngle1 -= Math::TWO_PI;

            if (minAzimuthAngle1 <= maxAzimuthAngle1) 
            {
                spherical.mTheta = glm::max<float>(minAzimuthAngle1, glm::min<float>(maxAzimuthAngle1, spherical.mTheta));
            }
            else 
            {
                spherical.mTheta = (spherical.mTheta > (minAzimuthAngle1 + maxAzimuthAngle1) / 2) ?
                    glm::max<float>(minAzimuthAngle1, spherical.mTheta) :
                    glm::min<float>(maxAzimuthAngle1, spherical.mTheta);
            }

        }

        // restrict phi to be between desired limits
        spherical.mPhi = glm::max<float>(minPolarAngle, glm::min<float>(maxPolarAngle, spherical.mPhi));

        spherical.makeSafe();

        spherical.mRadius *= scale;

        // restrict radius to be between desired limits
        spherical.mRadius = glm::max<float>(minDistance, glm::min<float>(maxDistance, spherical.mRadius));

        // move target to panned location

        if (enableDamping == true) 
        {
            target += (panOffset * dampingFactor);
        }
        else 
        {
            target += panOffset;
        }

        auto setFromSpherical = [](const Spherical& s)->glm::vec3
        {
            const auto radius = s.mRadius;
            const auto phi = s.mPhi;
            const auto theta = s.mTheta;

            const auto sinPhiRadius = std::sin(phi) * radius;

            glm::vec3 result;

            result.x = sinPhiRadius * std::sin(theta);
            result.y = std::cos(phi) * radius;
            result.z = sinPhiRadius * std::cos(theta);

            return result;
        };

        offset = setFromSpherical(spherical);

        // rotate offset back to "camera-up-vector-is-up" space
        offset = glm::rotate(quatInverse,offset);

        position = target + offset;

        mCamera.lookAt(position, target);

        if (enableDamping == true) 
        {
            sphericalDelta.mTheta *= (1.0f - dampingFactor);
            sphericalDelta.mPhi *= (1.0f - dampingFactor);

            panOffset *= (1.0f - dampingFactor);
        }
        else 
        {
            sphericalDelta.set(0, 0, 0);

            panOffset = glm::vec3(0, 0, 0);
        }

        scale = 1;

        // update condition is:
        // min(camera displacement, camera rotation in radians)^2 > EPS
        // using small-angle approximation cos(x/2) = 1 - x^2 / 8
        auto quaternion = glm::toQuat(mCamera.getViewMatrix());
        
        if (zoomChanged ||
            glm::distance2(lastPosition, cameraPosition) > Math::EPSILON ||
            8.0f * (1.0f - glm::dot(lastQuaternion, quaternion)) > Math::EPSILON)
        {
            //dispatchEvent(changeEvent);

            lastPosition = cameraPosition;
            lastQuaternion = quaternion;
            zoomChanged = false;
        }
    }

    void OrbitControllers::panLeft(float distance, const glm::mat4& objectMatrix)
    {
        auto v = glm::vec3(0);

        v = objectMatrix[0]; // get X column of objectMatrix
        v *= -distance;

        panOffset += v;
    }

    void OrbitControllers::panUp(float distance, const glm::mat4& objectMatrix)
    {
        auto v = glm::vec3(0);

        if (screenSpacePanning == true)
        {
           v = objectMatrix[1];
        }
        else 
        {
           v = objectMatrix[0];

           v = glm::cross(mCamera.getUp(), v);
        }

        v *= distance;

        panOffset += v;
    }

    // deltaX and deltaY are in pixels; right and down are positive
    void OrbitControllers::pan(float deltaX, float deltaY)
    {
        auto offset = glm::vec3(0);

        // perspective
        auto position = mCamera.getEye();
        auto target = mCamera.getCenter();
        auto fov = mCamera.getFov();

        offset = position;
        offset -= target;

        auto targetDistance = glm::length(offset);

        // half of the fov is center to top of screen
        targetDistance *= std::tan((fov / 2.0f) * Math::PI / 180.0f);

        // we use only clientHeight here so aspect ratio does not distort speed
        // TODO
        panLeft(2.0f * deltaX * targetDistance / 500.0f, mCamera.getViewMatrix());
        panUp(2.0f * deltaY * targetDistance / 500.0f, mCamera.getViewMatrix());
    }

    void OrbitControllers::dollyOut(float dollyScale)
    {
        scale /= dollyScale;
    }

    void OrbitControllers::dollyIn(float dollyScale)
    {
        scale *= dollyScale;
    }

    void OrbitControllers::handleMouseDownRotate(float x, float y)
    {
        rotateStart.x = x;
        rotateStart.y = y;
    }

    void OrbitControllers::handleMouseDownDolly(float x, float y)
    {
        dollyStart.x = x;
        dollyStart.y = y;
    }

    void OrbitControllers::handleMouseDownPan(float x, float y)
    {
        panStart.x = x;
        panStart.y = y;
    }

    void OrbitControllers::handleMouseMoveRotate(float x, float y)
    {
        rotateEnd.x = x;
        rotateEnd.y = y;

        rotateDelta = rotateEnd - rotateStart;
        rotateDelta *=rotateSpeed;

        //TODO 
        rotateLeft(2 * Math::PI * rotateDelta.x / 500.0f); // yes, height

        rotateUp(2 * Math::PI * rotateDelta.y / 500.0f);

        rotateStart = rotateEnd;

        update();

    }

    void OrbitControllers::handleMouseMoveDolly(float x, float y)
    {
        dollyEnd.x = x;
        dollyEnd.y = y;

        dollyDelta = dollyEnd - dollyStart;

        if (dollyDelta.y > 0) 
        {
            dollyOut(getZoomScale());

        }
        else if (dollyDelta.y < 0) 
        {
            dollyIn(getZoomScale());
        }

        dollyStart = dollyEnd;

        update();
    }


    void OrbitControllers::handleMouseMovePan(float x ,float y)
    {
        panEnd.x = x;
        panEnd.y = y;

        panDelta = panEnd - panStart;
        panDelta *= panSpeed;

        pan(panDelta.x, panDelta.y);

        panStart = panEnd;

        update();
    }

    void OrbitControllers::handleMouseUp( float x, float y)
    {
        // no-op
    }

    void OrbitControllers::handleMouseWheel(float deltaY)
    {
        if (deltaY < 0) 
        {
            dollyIn(getZoomScale());
        }
        else if (deltaY > 0) 
        {
            dollyOut(getZoomScale());
        }

        update();
    }

    void OrbitControllers::handleKeyDown(int key)
    {
        bool needsUpdate = false;

        switch ((EKey)key) 
        {

        case EKey::KEY_UP:
            pan(0, keyPanSpeed);
            needsUpdate = true;
            break;

        case EKey::KEY_DOWN:
            pan(0, -keyPanSpeed);
            needsUpdate = true;
            break;

        case EKey::KEY_LEFT:
            pan(keyPanSpeed, 0);
            needsUpdate = true;
            break;

        case EKey::KEY_RIGHT:
            pan(-keyPanSpeed, 0);
            needsUpdate = true;
            break;

        }

        if (needsUpdate) 
        {
            // prevent the browser from scrolling on cursor keys
            //preventDefault();

            update();

        }
    }

    void OrbitControllers::onMouseDown(int button, float x, float y)
    {
        // Prevent the browser from scrolling.
        //preventDefault();

        // Manually set the focus since calling preventDefault above
        // prevents the browser from setting it automatically.

        switch (static_cast<EMouseButton>(button))
        {

        case EMouseButton::BUTTON_MIDDLE:

            //if (enableZoom == false) return;

            handleMouseDownDolly(x, y);

            state = STATE::DOLLY;

            break;

        case EMouseButton::BUTTON_LEFT:

            //if (event.ctrlKey || event.metaKey || event.shiftKey) {

            //    if (scope.enablePan == = false) return;

            //    handleMouseDownPan(event);

            //    state = STATE::PAN;
            //}
            //else 
            {
                //if (enableRotate == false) return;

                handleMouseDownRotate(x, y);

                state = STATE::ROTATE;
            }

            break;

        case EMouseButton::BUTTON_RIGHT:

            //if (event.ctrlKey || event.metaKey || event.shiftKey) {

            //    if (enableRotate == false) return;

            //    handleMouseDownRotate(event);

            //    state = STATE::ROTATE;

            //}
            //else 
            {
                if (enablePan == false) return;

                handleMouseDownPan(x, y);

                state = STATE::PAN;
            }

            break;

        default:
            state = STATE::NONE;
        }

    }

    void OrbitControllers::onMouseMove(float x, float y)
    {
        //if (enabled == false) return;

        //preventDefault();

        switch (state) 
        {
        case STATE::ROTATE:
            //if (enableRotate == false) return;
            handleMouseMoveRotate(x, y);
            break;
        case STATE::DOLLY:
            //if (enableZoom == false) return;
            handleMouseMoveDolly(x, y);
            break;
        case STATE::PAN:
            //if (enablePan == false) return;
            handleMouseMovePan(x, y);
            break;
        }
    }

    void OrbitControllers::onMouseUp(int button, float x, float y)
    {
        //if (enabled == false) return;

        handleMouseUp(x, y);

        //scope.dispatchEvent(endEvent);

        state = STATE::NONE;
    }

    void OrbitControllers::onMouseWheel(float deltaY)
    {
        //if (enabled == false || enableZoom == false || (state != = STATE::NONE && state != = STATE::ROTATE)) return;

        //preventDefault();
        //stopPropagation();

        //dispatchEvent(startEvent);

        handleMouseWheel(deltaY);

        //dispatchEvent(endEvent);
    }

    void OrbitControllers::onKeyDown(int key)
    {
        //if (enabled == false || enableKeys == false || enablePan == false) return;

        handleKeyDown(key);
    }

}