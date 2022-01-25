#include "jnpch.h"
#include "Graphics/Environment.h"

#include "Graphics/SceneRenderer.h"

namespace Janus {
	Environment::Environment(const std::string& filepath, Ref<TextureCube> radianceMap, Ref<TextureCube> irradianceMap) 
		: FilePath(filepath), RadianceMap(radianceMap), IrradianceMap(irradianceMap) {}
	Environment::Environment(Ref<TextureCube> radianceMap, Ref<TextureCube> irradianceMap) 
		: RadianceMap(radianceMap), IrradianceMap(irradianceMap) {}
	Ref<Environment> Environment::Load(const std::string& filepath)
	{
		auto [radiance, irradiance] = SceneRenderer::CreateEnvironmentMap(filepath);
		return Ref<Environment>::Create(filepath, radiance, irradiance);
	}
}