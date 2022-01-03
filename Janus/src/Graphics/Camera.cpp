#include "jnpch.h"
#include "Camera.h"

namespace Janus {

	Camera::Camera(const glm::mat4& projectionMatrix)
		: m_ProjectionMatrix(projectionMatrix)
	{
	}

}