#pragma once

#include "Graphics/Texture.h"

namespace Janus {

	class Environment : public RefCounted
	{
	public:
		Environment(const std::string& filepath, Ref<TextureCube> radianceMap, Ref<TextureCube> irradianceMap);
		Environment(Ref<TextureCube> radianceMap, Ref<TextureCube> irradianceMap);
		std::string FilePath;
		Ref<TextureCube> RadianceMap;
		Ref<TextureCube> IrradianceMap;

		static Ref<Environment> Load(const std::string& filepath);
	};


}