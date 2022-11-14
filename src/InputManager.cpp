#include "InputManager.h"

namespace SoftRenderer
{
    InputManager::InputManager(Window& window) 
        : mWindow(window)
    {
        mKeyPressedListener = mWindow.mKeyPressedEvent.addListener(std::bind(&InputManager::onKeyPressed, this, std::placeholders::_1));
        mKeyReleasedListener = mWindow.mKeyReleasedEvent.addListener(std::bind(&InputManager::onKeyReleased, this, std::placeholders::_1));
        mMouseButtonPressedListener = mWindow.mMouseButtonPressedEvent.addListener(std::bind(&InputManager::onMouseButtonPressed, this, std::placeholders::_1));
        mMouseButtonReleasedListener = mWindow.mMouseButtonReleasedEvent.addListener(std::bind(&InputManager::onMouseButtonReleased, this, std::placeholders::_1));
    }

    InputManager::~InputManager()
    {
        mWindow.mKeyPressedEvent.removeListener(mKeyPressedListener);
        mWindow.mKeyReleasedEvent.removeListener(mKeyReleasedListener);
        mWindow.mMouseButtonPressedEvent.removeListener(mMouseButtonPressedListener);
        mWindow.mMouseButtonReleasedEvent.removeListener(mMouseButtonReleasedListener);
    }

    EKeyState InputManager::getKeyState(EKey key) const
    {
        switch (mWindow.getKeyAction(static_cast<int>(key)))
        {
        case InputAction::PRESS:	return EKeyState::KEY_DOWN;
        case InputAction::RELEASE:	return EKeyState::KEY_UP;
        }

        return EKeyState::KEY_UP;
    }

    EMouseButtonState InputManager::getMouseButtonState(EMouseButton button) const
    {
        switch (mWindow.getMouseButtonAction(static_cast<int>(button)))
        {
        case InputAction::PRESS:	return EMouseButtonState::MOUSE_DOWN;
        case InputAction::RELEASE:	return EMouseButtonState::MOUSE_UP;
        }

        return EMouseButtonState::MOUSE_UP;
    }

    bool InputManager::isKeyPressed(EKey key) const
    {
        return mKeyEvents.find(key) != mKeyEvents.end() && mKeyEvents.at(key) == EKeyState::KEY_DOWN;
    }

    bool InputManager::isKeyReleased(EKey key) const
    {
        return mKeyEvents.find(key) != mKeyEvents.end() && mKeyEvents.at(key) == EKeyState::KEY_UP;
    }

    bool InputManager::isMouseButtonPressed(EMouseButton button) const
    {
        return mMouseButtonEvents.find(button) != mMouseButtonEvents.end() && mMouseButtonEvents.at(button) == EMouseButtonState::MOUSE_DOWN;
    }

    bool InputManager::isMouseButtonReleased(EMouseButton p_button) const
    {
        return mMouseButtonEvents.find(p_button) != mMouseButtonEvents.end() && mMouseButtonEvents.at(p_button) == EMouseButtonState::MOUSE_UP;
    }

    std::pair<double, double> InputManager::getMousePosition() const
    {
        std::pair<double, double> result;
        mWindow.getCursorPos(&result.first, &result.second);
        return result;
    }

    void InputManager::clearEvents()
    {
        mKeyEvents.clear();
        mMouseButtonEvents.clear();
    }

    void InputManager::onKeyPressed(int key)
    {
        mKeyEvents[static_cast<EKey>(key)] = EKeyState::KEY_DOWN;
    }

    void InputManager::onKeyReleased(int key)
    {
        mKeyEvents[static_cast<EKey>(key)] = EKeyState::KEY_UP;
    }

    void InputManager::onMouseButtonPressed(int button)
    {
        mMouseButtonEvents[static_cast<EMouseButton>(button)] = EMouseButtonState::MOUSE_DOWN;
    }

    void InputManager::onMouseButtonReleased(int button)
    {
        mMouseButtonEvents[static_cast<EMouseButton>(button)] = EMouseButtonState::MOUSE_UP;
    }
}