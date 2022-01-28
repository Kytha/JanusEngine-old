#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glad/glad.h>

#include "Core/Core.h"
#include "Core/Buffer.h"

#include "Graphics/ShaderUniform.h"


namespace Janus
{
	class Shader : public RefCounted
	{
	public:
		Shader(const std::string &filepath);
		~Shader();

		void Bind() const;
		void Unbind() const;
		uint32_t GetRendererID() const { return m_RendererID; }

		void SetVSMaterialUniformBuffer(Buffer buffer);
		void SetPSMaterialUniformBuffer(Buffer buffer);

		std::string ReadShaderFromFile(const std::string &filepath) const;
		std::unordered_map<GLenum, std::string> PreProcess(const std::string &source);
		void CompileAndUploadShader();

		void Parse();
		void ParseUniform(const std::string &statement, ShaderDomain domain);
		void ParseUniformStruct(const std::string &block, ShaderDomain domain);
		int32_t GetUniformLocation(const std::string &name) const;
		ShaderStruct *FindStruct(const std::string &name);

		void ResolveUniforms();
		void ValidateUniforms();

		void ResolveAndSetUniforms(const Ref<ShaderUniformBufferDeclaration> &decl, Buffer buffer);
		void ResolveAndSetUniform(ShaderUniformDeclaration *uniform, Buffer buffer);
		void ResolveAndSetUniformArray(ShaderUniformDeclaration *uniform, Buffer buffer);
		void ResolveAndSetUniformField(const ShaderUniformDeclaration &field, byte *data, int32_t offset);

		const std::string& GetName() const { return m_Name; }
		// Preprocess single source string into multiple shader sources

		// Mapping of strings to shader types. Currently only supports "vertex" and "fragment"
		GLenum ShaderTypeFromString(const std::string &type);

		void SetInt(const std::string &name, int value);
		void SetBool(const std::string &name, bool value);
		void SetFloat(const std::string &name, float value);
		void SetFloat2(const std::string &name, const glm::vec2 &value);
		void SetFloat3(const std::string &name, const glm::vec3 &value);
		void SetMat4(const std::string &name, const glm::mat4 &value);
		void SetMat4FromRenderThread(const std::string &name, const glm::mat4 &value, bool bind = true);

		void SetIntArray(const std::string &name, int *values, uint32_t size);

		const ShaderUniformBufferList &GetVSRendererUniforms() const { return m_VSRendererUniformBuffers; }
		const ShaderUniformBufferList &GetPSRendererUniforms() const { return m_PSRendererUniformBuffers; }
		bool HasVSMaterialUniformBuffer() const { return (bool)m_VSMaterialUniformBuffer; }
		bool HasPSMaterialUniformBuffer() const { return (bool)m_PSMaterialUniformBuffer; }
		const ShaderUniformBufferDeclaration &GetVSMaterialUniformBuffer() const { return *m_VSMaterialUniformBuffer; }
		const ShaderUniformBufferDeclaration &GetPSMaterialUniformBuffer() const { return *m_PSMaterialUniformBuffer; }
		const std::vector<ShaderResourceDeclaration *> &GetResources() const { return m_Resources; }

	private:
		void UploadUniformField(uint32_t location, const ShaderUniformDeclaration& field, byte *data, int32_t offset);
		void UploadUniformInt(uint32_t location, int32_t value);
		void UploadUniformIntArray(uint32_t location, int32_t *values, int32_t count);
		void UploadUniformFloat(uint32_t location, float value);
		void UploadUniformFloat2(uint32_t location, const glm::vec2 &value);
		void UploadUniformFloat3(uint32_t location, const glm::vec3 &value);
		void UploadUniformFloat4(uint32_t location, const glm::vec4 &value);
		void UploadUniformMat3(uint32_t location, const glm::mat3 &values);
		void UploadUniformMat4(uint32_t location, const glm::mat4 &values);
		void UploadUniformMat4Array(uint32_t location, const glm::mat4 &values, uint32_t count);

		void UploadUniformStruct(ShaderUniformDeclaration *uniform, byte *buffer, uint32_t offset);

		void UploadUniformInt(const std::string &name, int32_t value);
		void UploadUniformIntArray(const std::string &name, int32_t *values, uint32_t count);

		void UploadUniformFloat(const std::string &name, float value);
		void UploadUniformFloat2(const std::string &name, const glm::vec2 &value);
		void UploadUniformFloat3(const std::string &name, const glm::vec3 &value);
		void UploadUniformFloat4(const std::string &name, const glm::vec4 &value);

		void UploadUniformMat4(const std::string &name, const glm::mat4 &value);

		uint32_t m_RendererID = 0;
		std::string m_Name, m_AssetPath;
		std::unordered_map<GLenum, std::string> m_ShaderSource;

		bool m_Loaded = false;
		bool m_IsCompute = false;

		ShaderUniformBufferList m_VSRendererUniformBuffers;
		ShaderUniformBufferList m_PSRendererUniformBuffers;
		Ref<ShaderUniformBufferDeclaration> m_VSMaterialUniformBuffer;
		Ref<ShaderUniformBufferDeclaration> m_PSMaterialUniformBuffer;
		std::vector<ShaderResourceDeclaration *> m_Resources;
		ShaderStructList m_Structs;
	};
}