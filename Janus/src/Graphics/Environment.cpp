#include "jnpch.h"
#include "Graphics/Environment.h"

#include "Graphics/SceneRenderer.h"

namespace Janus {

	Environment Environment::Load(const std::string& filepath)
	{
		auto [radiance, irradiance] = SceneRenderer::CreateEnvironmentMap(filepath);
		return { filepath, radiance, irradiance };
	}
}