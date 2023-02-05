#pragma once
#include "Core/Core.h"

namespace Janus
{
	enum class AssetFlag : uint16_t
	{
		None = 0,
		Missing = BIT(0),
		Invalid = BIT(1)
	}

	enum class AssetType : uint16_t
	{
		None = 0,
		Scene = 1,
		Mesh = 2,
		Material = 3,
		Texture = 4,
		EnvMap = 5,
	}

	namespace Utils
	{
		inline AssetType AssetTypeFromString(const std::string &assetType)
		{
			if (assetType == "None")
				return AssetType::None;
			if (assetType == "Scene")
				return AssetType::Scene;
			if (assetType == "Mesh")
				return AssetType::Mesh;
			if (assetType == "Material")
				return AssetType::Material;
			if (assetType == "Texture")
				return AssetType::Texture;
			if (assetType == "EnvMap")
				return AssetType::EnvMap;
			JN_ASSERT(false, "Unknown Asset Type");
			return AssetType::None;
		}

		inline const char *AssetTypeToString(AssetType assetType)
		{
			switch (assetType)
			{
			case AssetType::None:
				return "None";
			case AssetType::Scene:
				return "Scene";
			case AssetType::Mesh:
				return "Mesh";
			case AssetType::Material:
				return "Material";
			case AssetType::Texture:
				return "Texture";
			case AssetType::EnvMap:
				return "EnvMap";
			}

			JN_ASSERT(false, "Unknown Asset Type");
			return "None";
		}
	}
}