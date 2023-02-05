#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Core/UUID.h"
#include "Graphics/Texture.h"
#include "Graphics/Mesh.h"
#include "Graphics/Camera.h"

namespace Janus
{

	struct IDComponent
	{
		UUID ID = 0;
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent &other) = default;
		TagComponent(const std::string &tag)
			: Tag(tag) {}

		operator std::string &() { return Tag; }
		operator const std::string &() const { return Tag; }
	};

	struct RelationshipComponent
	{
		UUID ParentHandle = 0;
		std::vector<UUID> Children;

		RelationshipComponent() = default;
		RelationshipComponent(const RelationshipComponent &other) = default;
		RelationshipComponent(UUID parent)
			: ParentHandle(parent) {}
	};

	struct TransformComponent
	{
		glm::vec3 Translation = {0.0f, 0.0f, 0.0f};
		glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
		glm::vec3 Scale = {1.0f, 1.0f, 1.0f};

		glm::vec3 Up = {0.0F, 1.0F, 0.0F};
		glm::vec3 Right = {1.0F, 0.0F, 0.0F};
		glm::vec3 Forward = {0.0F, 0.0F, -1.0F};

		TransformComponent() = default;
		TransformComponent(const TransformComponent &other) = default;
		TransformComponent(const glm::vec3 &translation)
			: Translation(translation) {}

		glm::mat4 GetTransform() const
		{
			return glm::translate(glm::mat4(1.0f), Translation) * glm::toMat4(glm::quat(Rotation)) * glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct MeshComponent
	{
		Ref<Janus::Mesh> Mesh;

		MeshComponent() = default;
		MeshComponent(const MeshComponent &other) = default;
		MeshComponent(const Ref<Janus::Mesh> &mesh)
			: Mesh(mesh) {}

		operator Ref<Janus::Mesh>() { return Mesh; }
	};

	struct DirectionalLightComponent
	{
		glm::vec3 Radiance = {1.0f, 1.0f, 1.0f};
		float Intensity = 1.0f;
		bool CastShadows = true;
		bool SoftShadows = true;
	};

	struct PointLightComponent
	{
		glm::vec3 Radiance = {1.0f, 1.0f, 1.0f};
		float Intensity = 1.0f;
		float Radius = 10.f;
		bool CastsShadows = true;
		bool SoftShadows = true;
		float Falloff = 1.f;
	};
	struct SkyLightComponent
	{
		Ref<Environment> SceneEnvironment;
		float Intensity = 1.0f;
		float Angle = 0.0f;
		float LOD = 1.0f;
		SkyLightComponent() = default;
		SkyLightComponent(const Ref<Environment> &environment)
			: SceneEnvironment(environment) {}
	};
}