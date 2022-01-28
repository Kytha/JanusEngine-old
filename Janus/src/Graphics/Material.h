#pragma once

#include <unordered_set>

#include "Core/Core.h"

#include "Graphics/Texture.h"
#include "Graphics/Shader.h"
#include "Graphics/Light.h"

namespace Janus
{

	enum class MaterialFlag
	{
		None = BIT(0),
		DepthTest = BIT(1),
		Blend = BIT(2),
		TwoSided = BIT(3)
	};

	class Material : public RefCounted
	{
		friend class MaterialInstance;

	public:
		Material(const Ref<Shader> &shader, const std::string& name);
		void Bind();

		uint32_t GetFlags() const { return m_MaterialFlags; }
		const std::string &GetName() const { return m_Name; }
		void SetFlag(MaterialFlag flag, bool value);
		bool GetFlag(MaterialFlag flag) const { return (uint32_t)flag & m_MaterialFlags; }
		Ref<Shader> GetShader() { return m_Shader; }

		void Set(const std::string& name, const Ref<TextureCube>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}

		void Set(const std::string& name, const Ref<Texture2D>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}

		template <typename T>
		void Set(const std::string &name, const T &value)
		{
			auto decl = FindUniformDeclaration(name);
			JN_ASSERT(decl, "MATERIAL_ERROR: Could not find uniform");
			auto &buffer = GetUniformBufferTarget(decl);
			buffer.Write((byte *)&value, decl->GetSize(), decl->GetOffset());

			for (auto mi : m_MaterialInstances)
				mi->OnMaterialValueUpdated(decl);
		}

		template <typename T>
		void Set(const std::string &name, const std::vector<T>& array)
		{
			if(array.size() <= 0)
				return;

			auto decl = FindUniformDeclaration(name);
			JN_ASSERT(decl, "MATERIAL_ERROR: Could not find uniform");
			auto &buffer = GetUniformBufferTarget(decl);
			uint32_t unitSize = sizeof(T);
			uint32_t declUnitSize = decl->GetSize() / decl->GetCount();
			JN_ASSERT(unitSize == declUnitSize, "MATERIAL_ERROR: Mismatch uniform datatype");
			uint32_t size = unitSize * array.size();
			buffer.Write((byte *)array.data(), size, decl->GetOffset());

			for (auto mi : m_MaterialInstances)
				mi->OnMaterialValueUpdated(decl);
		}

		void Set(const std::string &name, const Ref<Texture> &texture)
		{
			auto decl = FindResourceDeclaration(name);
			uint32_t slot = decl->GetRegister();
			if (m_Textures.size() <= slot)
				m_Textures.resize((size_t)slot + 1);
			m_Textures[slot] = texture;
		}

		template <typename T>
		T &Get(const std::string &name)
		{
			auto decl = FindUniformDeclaration(name);
			JN_ASSERT(decl, "MATERIAL_ERROR: Could not find uniform!");
			auto &buffer = GetUniformBufferTarget(decl);
			return buffer.Read<T>(decl->GetOffset());
		}

		template <typename T>
		Ref<T> GetResource(const std::string &name)
		{
			auto decl = FindResourceDeclaration(name);
			uint32_t slot = decl->GetRegister();
			JN_ASSERT(slot < m_Textures.size(), "MATERIAL_ERROR: Texture slot is invalid!");
			return m_Textures[slot];
		}

		template <typename T>
		Ref<T> TryGetResource (const std::string &name)
		{
			auto decl = FindResourceDeclaration(name);
			if (!decl)
				return nullptr;

			uint32_t slot = decl->GetRegister();
			if (slot >= m_Textures.size())
				return nullptr;

			return Ref<T>(m_Textures[slot]);
		}

		ShaderResourceDeclaration *FindResourceDeclaration(const std::string &name);

	public:
		static Ref<Material> Create(const Ref<Shader> &shader, const std::string &name);

	private:
		void AllocateStorage();
		void BindTextures();
		ShaderUniformDeclaration *FindUniformDeclaration(const std::string &name);
		Buffer &GetUniformBufferTarget(ShaderUniformDeclaration *uniformDeclaration);

		std::vector<Ref<Texture>> m_Textures;
		std::unordered_set<MaterialInstance *> m_MaterialInstances;
		Ref<Shader> m_Shader;
		Buffer m_VSUniformStorageBuffer;
		Buffer m_PSUniformStorageBuffer;
		std::string m_Name;
		uint32_t m_MaterialFlags;
	};

	class MaterialInstance : public RefCounted
	{
		friend class Material;

	public:
		MaterialInstance(const Ref<Material> &material, const std::string &name = "");
		virtual ~MaterialInstance();

		template <typename T>
		void Set(const std::string &name, const T &value)
		{
			auto decl = m_Material->FindUniformDeclaration(name);
			if (!decl)
			{
				JN_ASSERT(decl, "MATERIAL_ERROR: Could not find uniform!");
				return;
			}
			auto &buffer = GetUniformBufferTarget(decl);
			buffer.Write((byte *)&value, decl->GetSize(), decl->GetOffset());
			m_OverriddenValues.insert(name);
		}

		void Set(const std::string &name, const Ref<Texture> &texture)
		{
			auto decl = m_Material->FindResourceDeclaration(name);
			if (!decl)
			{
				JN_CORE_WARN("MATERIAL_ERROR: Cannot find material property: {0}", name);
				return;
			}
			uint32_t slot = decl->GetRegister();
			if (m_Textures.size() <= slot)
				m_Textures.resize((size_t)slot + 1);
			m_Textures[slot] = texture;
		}

		template <typename T>
		T &Get(const std::string &name)
		{
			auto decl = m_Material->FindUniformDeclaration(name);
			JN_ASSERT(decl, "MATERIAL_ERROR: Could not find uniform!");
			auto &buffer = GetUniformBufferTarget(decl);
			return buffer.Read<T>(decl->GetOffset());
		}

		template <typename T>
		Ref<T> GetResource(const std::string &name)
		{
			auto decl = m_Material->FindResourceDeclaration(name);
			JN_ASSERT(decl, "MATERIAL_ERROR: Could not find uniform!");
			uint32_t slot = decl->GetRegister();
			JN_ASSERT(slot < m_Textures.size(), "MATERIAL_ERROR: Texture slot is invalid!");
			return Ref<T>(m_Textures[slot]);
		}

		template <typename T>
		Ref<T> TryGetResource (const std::string &name)
		{
			auto decl = m_Material->FindResourceDeclaration(name);
			if (!decl)
				return nullptr;

			uint32_t slot = decl->GetRegister();
			if (slot >= m_Textures.size())
				return nullptr;

			return Ref<T>(m_Textures[slot]);
		}

		void Bind();

		uint32_t GetFlags() const { return m_Material->m_MaterialFlags; }
		bool GetFlag(MaterialFlag flag) const { return (uint32_t)flag & m_Material->m_MaterialFlags; }
		void SetFlag(MaterialFlag flag, bool value = true);

		Ref<Shader> GetShader() const { return m_Material->m_Shader; }

		const std::string &GetName() const { return m_Name; }

	public:
		static Ref<MaterialInstance> Create(const Ref<Material> &material);

	private:
		void AllocateStorage();
		void OnShaderReloaded();
		Buffer &GetUniformBufferTarget(ShaderUniformDeclaration *uniformDeclaration);
		void OnMaterialValueUpdated(ShaderUniformDeclaration *decl);

	private:
		Ref<Material> m_Material;
		std::string m_Name;

		Buffer m_VSUniformStorageBuffer;
		Buffer m_PSUniformStorageBuffer;
		std::vector<Ref<Texture>> m_Textures;

		// TODO: This is temporary; come up with a proper system to track overrides
		std::unordered_set<std::string> m_OverriddenValues;
	};
}