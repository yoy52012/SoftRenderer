#pragma once

#include "Camera.h"
#include "Window.h"

namespace SoftRenderer
{
    class OrbitControllers 
    {
    public:
        OrbitControllers(Camera& camera, Window* window);

        void update();
    
    private:
        float getAutoRotationAngle() const;

        float getZoomScale() const;

        void rotateLeft(float angle);

        void rotateUp(float angle);

        void panLeft(float distance, const glm::mat4& objectMatrix);

        void panUp(float distance, const glm::mat4& objectMatrix);

        void pan(float deltaX, float deltaY);

        void dollyOut(float dollyScale);

        void dollyIn(float dollyScale);
        void handleMouseDownRotate(float x, float y);
        void handleMouseDownDolly(float x, float y);
        void handleMouseDownPan(float x, float y);
        void handleMouseMoveRotate(float x, float y);
        void handleMouseMoveDolly(float x, float y);
        void handleMouseMovePan(float x, float y);
        void handleMouseUp(float x, float y);
        void handleMouseWheel(float deltaY);
        void handleKeyDown(int key);
        void onMouseDown(int button, float x, float y);
        void onMouseMove(float x, float y);
        void onMouseUp(int button, float x, float y);
        void onMouseWheel(float deltaY);
        void onKeyDown(int key);
    private:
        Camera& mCamera;


    private:
        // Set to false to disable this control
        bool enabled = true;

        // "target" sets the location of focus, where the object orbits around
        glm::vec3 target = {};

        // How far you can dolly in and out ( PerspectiveCamera only )
        float minDistance = 0;
        float maxDistance = std::numeric_limits<float>::infinity();

        // How far you can zoom in and out ( OrthographicCamera only )
        float minZoom = 0;
        float maxZoom = std::numeric_limits<float>::infinity();

        // How far you can orbit vertically, upper and lower limits.
        // Range is 0 to Math.PI radians.
        float minPolarAngle = 0; // radians
        float maxPolarAngle = Math::PI; // radians

        // How far you can orbit horizontally, upper and lower limits.
        // If set, the interval [ min, max ] must be a sub-interval of [ - 2 PI, 2 PI ], with ( max - min < 2 PI )
        float minAzimuthAngle = -std::numeric_limits<float>::infinity(); // radians
        float maxAzimuthAngle = std::numeric_limits<float>::infinity(); // radians

        // Set to true to enable damping (inertia)
        // If damping is enabled, you must call controls.update() in your animation loop
        bool enableDamping = false;
        float dampingFactor = 0.05;

        // This option actually enables dollying in and out; left as "zoom" for backwards compatibility.
        // Set to false to disable zooming
        bool enableZoom = true;
        float zoomSpeed = 1.0;

        // Set to false to disable rotating
        bool enableRotate = true;
        float rotateSpeed = 1.0;

        // Set to false to disable panning
        bool enablePan = true;
        float panSpeed = 1.0;
        bool screenSpacePanning = true; // if false, pan orthogonal to world-space direction camera.up
        float keyPanSpeed = 7.0;	// pixels moved per arrow key push

        // Set to true to automatically rotate around the target
        // If auto-rotate is enabled, you must call controls.update() in your animation loop
        bool autoRotate = false;
        float autoRotateSpeed = 2.0; // 30 seconds per orbit when fps is 60

        //// The four arrow keys
        //this.keys = { LEFT: 'ArrowLeft', UP : 'ArrowUp', RIGHT : 'ArrowRight', BOTTOM : 'ArrowDown' };

        //// Mouse buttons
        //this.mouseButtons = { LEFT: MOUSE.ROTATE, MIDDLE : MOUSE.DOLLY, RIGHT : MOUSE.PAN };


        //// for reset
        //this.target0 = this.target.clone();
        //this.position0 = this.object.position.clone();
        //this.zoom0 = this.object.zoom;

        //// the target DOM element for key events
        //this._domElementKeyEvents = null;
    };
}