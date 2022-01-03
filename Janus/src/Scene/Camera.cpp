#include "Camera.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Core/MouseButtonCodes.h"

#define M_PI 3.14159f
namespace Janus {
    Camera::Camera(const glm::mat4 &projectionMatrix) : m_ProjectionMatrix(projectionMatrix)
    {
        m_Position = glm::vec3{0, 0.3, 0.8};
        m_WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        m_Yaw = -90.0f;
        m_Pitch = 0.0f;
        m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
    }

    void Camera::Update(Timestep ts)
    {
        // Poll inputs
        if (Input::IsKeyPressed(JN_KEY_W))
            ProcessKeyboard(FORWARD, ts);
        if (Input::IsKeyPressed(JN_KEY_S))
            ProcessKeyboard(BACKWARD, ts);
        if (Input::IsKeyPressed(JN_KEY_A))
            ProcessKeyboard(LEFT, ts);
        if (Input::IsKeyPressed(JN_KEY_D))
            ProcessKeyboard(RIGHT, ts);
        if (Input::IsMouseButtonPressed(JN_MOUSE_BUTTON_LEFT))
        {
            std::pair<float, float> pos = Input::GetMousePosition();
            float x = std::get<0>(pos);
            float y = std::get<1>(pos);

            // Ensures that camera doesn't jump everytime the mouse is pressed
            if (m_FirstMouse)
            {
                m_LastX = x;
                m_LastY = y;
                m_FirstMouse = false;
            }

            float xoffset = x - m_LastX;
            float yoffset = m_LastY - y;
            m_LastX = x;
            m_LastY = y;

            ProcessMouseMovement(xoffset, yoffset, true);
        }
        else
        {
            m_FirstMouse = true;
        }

        glm::vec3 front;

        // Recalculate the front vector based on camera rotation
        front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front.y = sin(glm::radians(m_Pitch));
        front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_Front = glm::normalize(front);

        // Re-calculate the Right and Up vector
        m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp)); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        m_Up = glm::normalize(glm::cross(m_Right, m_Front));

        m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
        m_ProjectionMatrix = glm::perspective(glm::radians(m_Zoom), (float)m_ViewportWidth / (float)m_ViewportHeight, 0.1f, 100.0f);
    }

    void Camera::ProcessKeyboard(Camera_Movement direction, Timestep deltaTime)
    {
        // Update camera position
        
        float velocity = m_Speed * deltaTime;
        if (direction == FORWARD)
            m_Position += m_Front * velocity;
        if (direction == BACKWARD)
            m_Position -= m_Front * velocity;
        if (direction == LEFT)
            m_Position -= m_Right * velocity;
        if (direction == RIGHT)
            m_Position += m_Right * velocity;

    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
    {
        // Scale by sensitivity
        xoffset *= m_MouseSensitivity;
        yoffset *= m_MouseSensitivity;

        m_Yaw += xoffset;
        m_Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;
    }
}
