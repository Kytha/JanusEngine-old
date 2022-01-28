#include "jnpch.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>

#include "Graphics/Renderer.h"
#include "Graphics/Shader.h"


GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

namespace Janus
{

	static bool IsTypeStringResource(const std::string &type)
	{
		if (type == "sampler1D")
			return true;
		if (type == "sampler2D")
			return true;
		if (type == "sampler2DMS")
			return true;
		if (type == "samplerCube")
			return true;
		if (type == "sampler2DShadow")
			return true;
		return false;
	}

	Shader::Shader(const std::string &filepath)
		: m_AssetPath(filepath)
	{
		JN_PROFILE_FUNCTION();

		size_t found = filepath.find_last_of("/\\");
		m_Name = found != std::string::npos ? filepath.substr(found + 1) : filepath;
		found = m_Name.find_last_of(".");
		m_Name = found != std::string::npos ? m_Name.substr(0, found) : m_Name;
		
		std::string source = ReadShaderFromFile(filepath);
		m_ShaderSource = PreProcess(source);

		if (!m_IsCompute)
			Parse();

		Renderer::Submit([=]()
						 {
							 if (m_RendererID)
								 glDeleteProgram(m_RendererID);
							 CompileAndUploadShader();
							 if (!m_IsCompute)
							 {
								 ResolveUniforms();
								 ValidateUniforms();
							 }
							 m_Loaded = true;
						 });
	}

	bool StartsWith(const std::string &string, const std::string &start)
	{
		return string.find(start) == 0;
	}

	std::vector<std::string> SplitString(const std::string &string, const std::string &delimiters)
	{
		size_t start = 0;
		size_t end = string.find_first_of(delimiters);

		std::vector<std::string> result;

		while (end <= std::string::npos)
		{
			std::string token = string.substr(start, end - start);
			if (!token.empty())
				result.push_back(token);

			if (end == std::string::npos)
				break;

			start = end + 1;
			end = string.find_first_of(delimiters, start);
		}

		return result;
	}

	const char *FindToken(const char *str, const std::string &token)
	{
		const char *t = str;
		while (t = strstr(t, token.c_str()))
		{
			bool left = str == t || isspace(t[-1]);
			bool right = !t[token.size()] || isspace(t[token.size()]);
			if (left && right)
				return t;

			t += token.size();
		}
		return nullptr;
	}

	const char *FindToken(const std::string &string, const std::string &token)
	{
		return FindToken(string.c_str(), token);
	}

	std::vector<std::string> Tokenize(const std::string &string)
	{
		return SplitString(string, " \t\n\r");
	}

	std::vector<std::string> GetLines(const std::string &string)
	{
		return SplitString(string, "\n");
	}

	std::string GetBlock(const char *str, const char **outPosition)
	{
		const char *end = strstr(str, "}");
		if (!end)
			return str;

		if (outPosition)
			*outPosition = end;
		uint32_t length = end - str + 1;
		return std::string(str, length);
	}

	std::string GetStatement(const char *str, const char **outPosition)
	{
		const char *end = strstr(str, ";");
		if (!end)
			return str;

		if (outPosition)
			*outPosition = end;
		uint32_t length = end - str + 1;
		return std::string(str, length);
	}

	std::string Shader::ReadShaderFromFile(const std::string &filepath) const
	{
		JN_PROFILE_FUNCTION();
		std::string code;
		std::stringstream ss;
		std::ifstream in;

		in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			in.open(filepath);
			ss << in.rdbuf();
			in.close();
			code = ss.str();
			return code;
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
			return code;
		}
	}

	void Shader::Parse()
	{
		JN_PROFILE_FUNCTION();
		const char *token;
		const char *vstr;
		const char *fstr;
		m_Resources.clear();
		m_Structs.clear();
		m_VSMaterialUniformBuffer.Reset();
		m_PSMaterialUniformBuffer.Reset();

		auto &vertexSource = m_ShaderSource[GL_VERTEX_SHADER];
		auto &fragmentSource = m_ShaderSource[GL_FRAGMENT_SHADER];

		vstr = vertexSource.c_str();
		while (token = FindToken(vstr, "struct"))
			ParseUniformStruct(GetBlock(token, &vstr), ShaderDomain::Vertex);
		vstr = vertexSource.c_str();
		while (token = FindToken(vstr, "uniform"))
			ParseUniform(GetStatement(token, &vstr), ShaderDomain::Vertex);

		fstr = fragmentSource.c_str();
		while (token = FindToken(fstr, "struct"))
			ParseUniformStruct(GetBlock(token, &fstr), ShaderDomain::Pixel);

		fstr = fragmentSource.c_str();
		while (token = FindToken(fstr, "uniform"))
			ParseUniform(GetStatement(token, &fstr), ShaderDomain::Pixel);
	}

	ShaderStruct *Shader::FindStruct(const std::string &name)
	{
		for (ShaderStruct *s : m_Structs)
		{
			if (s->GetName() == name)
				return s;
		}
		return nullptr;
	}

	void Shader::ParseUniform(const std::string &statement, ShaderDomain domain)
	{
		JN_PROFILE_FUNCTION();
		std::vector<std::string> tokens = Tokenize(statement);
		uint32_t index = 0;

		index++;
		std::string typeString = tokens[index++];
		std::string name = tokens[index++];

		if (const char *s = strstr(name.c_str(), ";"))
			name = std::string(name.c_str(), s - name.c_str());
		std::string n(name);
		int32_t count = 1;
		const char *namestr = n.c_str();
		if (const char *s = strstr(namestr, "["))
		{
			name = std::string(namestr, s - namestr);
			const char *end = strstr(namestr, "]");
			std::string c(s + 1, end - s);
			count = atoi(c.c_str());
		}

		if (IsTypeStringResource(typeString))
		{
			ShaderResourceDeclaration *declaration = new ShaderResourceDeclaration(ShaderResourceDeclaration::StringToType(typeString), name, count);
			m_Resources.push_back(declaration);
		}
		else
		{
			ShaderUniformDeclaration::Type t = ShaderUniformDeclaration::StringToType(typeString);
			ShaderUniformDeclaration *declaration = nullptr;

			if (t == ShaderUniformDeclaration::Type::NONE)
			{
				ShaderStruct *s = FindStruct(typeString);
				JN_ASSERT(s, "SHADER_ERROR: Unknown uniform datatype!");
				declaration = new ShaderUniformDeclaration(domain, s, name, count);
			}
			else
			{
				declaration = new ShaderUniformDeclaration(domain, t, name, count);
			}
			if (StartsWith(name, "r_"))
			{
				if (domain == ShaderDomain::Vertex)
					((ShaderUniformBufferDeclaration *)m_VSRendererUniformBuffers.front())->PushUniform(declaration);
				else if (domain == ShaderDomain::Pixel)
					((ShaderUniformBufferDeclaration *)m_PSRendererUniformBuffers.front())->PushUniform(declaration);
			}
			else
			{
				if (domain == ShaderDomain::Vertex)
				{
					if (!m_VSMaterialUniformBuffer)
						m_VSMaterialUniformBuffer.Reset(new ShaderUniformBufferDeclaration("", domain));

					m_VSMaterialUniformBuffer->PushUniform(declaration);
				}
				else if (domain == ShaderDomain::Pixel)
				{
					if (!m_PSMaterialUniformBuffer)
						m_PSMaterialUniformBuffer.Reset(new ShaderUniformBufferDeclaration("", domain));

					m_PSMaterialUniformBuffer->PushUniform(declaration);
				}
			}
		}
	}

	void Shader::ParseUniformStruct(const std::string &block, ShaderDomain domain)
	{
		JN_PROFILE_FUNCTION();
		std::vector<std::string> tokens = Tokenize(block);

		uint32_t index = 0;
		index++; // struct
		std::string name = tokens[index++];
		ShaderStruct *uniformStruct = new ShaderStruct(name);
		index++; // {
		while (index < tokens.size())
		{
			if (tokens[index] == "}")
				break;

			std::string type = tokens[index++];
			std::string name = tokens[index++];

			// Strip ; from name if present
			if (const char *s = strstr(name.c_str(), ";"))
				name = std::string(name.c_str(), s - name.c_str());

			uint32_t count = 1;
			const char *namestr = name.c_str();
			if (const char *s = strstr(namestr, "["))
			{
				name = std::string(namestr, s - namestr);

				const char *end = strstr(namestr, "]");
				std::string c(s + 1, end - s);
				count = atoi(c.c_str());
			}
			ShaderUniformDeclaration *field = new ShaderUniformDeclaration(domain, ShaderUniformDeclaration::StringToType(type), name, count);
			uniformStruct->AddField(field);
		}
		m_Structs.push_back(uniformStruct);
	}

	void Shader::ResolveUniforms()
	{
		JN_PROFILE_FUNCTION();
		glUseProgram(m_RendererID);
		/*
		for (size_t i = 0; i < m_VSRendererUniformBuffers.size(); i++)
		{
			ShaderUniformBufferDeclaration *decl = m_VSRendererUniformBuffers[i];
			const ShaderUniformList &uniforms = decl->GetUniformDeclarations();
			for (size_t j = 0; j < uniforms.size(); j++)
			{
				ShaderUniformDeclaration *uniform = uniforms[j];
				if (uniform->GetType() == ShaderUniformDeclaration::Type::STRUCT)
				{
					const ShaderStruct &s = uniform->GetShaderUniformStruct();
					const auto &fields = s.GetFields();
					for (size_t k = 0; k < fields.size(); k++)
					{
						ShaderUniformDeclaration *field = fields[k];
						field->m_Location = GetUniformLocation(uniform->GetName() + "." + field->GetName());
					}
				}
				else
				{
					uniform->m_Location = GetUniformLocation(uniform->GetName());
				}
			}
		}
		for (size_t i = 0; i < m_PSRendererUniformBuffers.size(); i++)
		{
			ShaderUniformBufferDeclaration *decl = m_PSRendererUniformBuffers[i];
			const ShaderUniformList &uniforms = decl->GetUniformDeclarations();
			for (size_t j = 0; j < uniforms.size(); j++)
			{
				ShaderUniformDeclaration *uniform = uniforms[j];
				if (uniform->GetType() == ShaderUniformDeclaration::Type::STRUCT)
				{
					const ShaderStruct &s = uniform->GetShaderUniformStruct();
					const auto &fields = s.GetFields();
					for (size_t k = 0; k < fields.size(); k++)
					{
						ShaderUniformDeclaration *field = fields[k];
						field->m_Location = GetUniformLocation(uniform->GetName() + "." + field->GetName());
					}
				}
				else
				{
					uniform->m_Location = GetUniformLocation(uniform->GetName());
				}
			}
		}
		
		*/
		{
			const auto &decl = m_VSMaterialUniformBuffer;
			if (decl)
			{
				const ShaderUniformList &uniforms = decl->GetUniformDeclarations();
				for (size_t j = 0; j < uniforms.size(); j++)
				{
					ShaderUniformDeclaration *uniform = uniforms[j];
					if (uniform->GetType() == ShaderUniformDeclaration::Type::STRUCT)
					{
						const ShaderStruct &s = uniform->GetShaderUniformStruct();
						const auto &fields = s.GetFields();
						for (size_t k = 0; k < fields.size(); k++)
						{
							ShaderUniformDeclaration *field = fields[k];
							if(!uniform->IsArray())
								field->m_Location = GetUniformLocation(uniform->m_Name + "." + field->m_Name);
						}
					}
					else
					{
						uniform->m_Location = GetUniformLocation(uniform->m_Name);
					}
				}
			}
		}

		{
			const auto &decl = m_PSMaterialUniformBuffer;
			if (decl)
			{
				const ShaderUniformList &uniforms = decl->GetUniformDeclarations();
				for (size_t j = 0; j < uniforms.size(); j++)
				{
					ShaderUniformDeclaration *uniform = uniforms[j];
					if (uniform->GetType() == ShaderUniformDeclaration::Type::STRUCT)
					{
						const ShaderStruct &s = uniform->GetShaderUniformStruct();
						const auto &fields = s.GetFields();
						for (size_t k = 0; k < fields.size(); k++)
						{
							ShaderUniformDeclaration *field = fields[k];
							if(!uniform->IsArray())
								field->m_Location = GetUniformLocation(uniform->m_Name + "." + field->m_Name);
						}
					}
					else
					{
						uniform->m_Location = GetUniformLocation(uniform->m_Name);
					}
				}
			}
		}
		
		uint32_t sampler = 0;
		for (size_t i = 0; i < m_Resources.size(); i++)
		{
			ShaderResourceDeclaration *resource = m_Resources[i];
			int32_t location = GetUniformLocation(resource->m_Name);

			if (resource->GetCount() == 1)
			{
				resource->m_Register = sampler;
				if (location != -1)
					UploadUniformInt(location, sampler);

				sampler++;
			}
			else if (resource->GetCount() > 1)
			{
				resource->m_Register = sampler;
				uint32_t count = resource->GetCount();
				int *samplers = new int[count];
				for (uint32_t s = 0; s < count; s++)
					samplers[s] = sampler++;
				UploadUniformIntArray(resource->GetName(), samplers, count);
				delete[] samplers;
			}
		}
	}

	void Shader::CompileAndUploadShader()
	{
		JN_PROFILE_FUNCTION();
		// CODE COPIED FROM OPENGL WIKI
		std::vector<GLuint> shaderRendererIDs;

		GLuint program = glCreateProgram();
		for (auto &kv : m_ShaderSource)
		{
			GLenum type = kv.first;
			std::string &source = kv.second;
			GLuint shaderRendererID = glCreateShader(type);
			const GLchar *sourceCstr = (const GLchar *)source.c_str();
			glShaderSource(shaderRendererID, 1, &sourceCstr, 0);

			glCompileShader(shaderRendererID);

			GLint isCompiled = 0;
			glGetShaderiv(shaderRendererID, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shaderRendererID, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shaderRendererID, maxLength, &maxLength, &infoLog[0]);

				std::cout << "ERROR::SHADER::SHADER COMPILATION FAILED" << &infoLog[0] << std::endl;

				// We don't need the shader anymore.
				glDeleteShader(shaderRendererID);
			}

			shaderRendererIDs.push_back(shaderRendererID);
			glAttachShader(program, shaderRendererID);
		}
		// Link our program
		glLinkProgram(program);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
			std::cout << "ERROR::SHADER::SHADER LINKING FAIL" << m_Name << std::endl;
			glCheckError();

			GLint pSize = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &pSize);

			GLchar *plog = new GLchar[pSize];
			for(int i = 0; i < pSize; i++) {
				plog[i] = 0;
			}
			glGetProgramInfoLog(program, pSize, NULL, plog);
			std::cout << plog;

			// We don't need the program anymore.
			glDeleteProgram(program);
			// Don't leak shaders either.
			for (auto id : shaderRendererIDs)
				glDeleteShader(id);
		}

		// Always detach shaders after a successful link.
		for (auto id : shaderRendererIDs)
			glDetachShader(program, id);

		m_RendererID = program;
	}

	void Shader::ValidateUniforms()
	{
	}

	void Shader::SetVSMaterialUniformBuffer(Buffer buffer)
	{
		Renderer::Submit([this, buffer]()
						 {
							 glUseProgram(m_RendererID);
							 ResolveAndSetUniforms(m_VSMaterialUniformBuffer, buffer);
						 });
	}

	void Shader::SetPSMaterialUniformBuffer(Buffer buffer)
	{
		Renderer::Submit([this, buffer]()
						 {
							 glUseProgram(m_RendererID);
							 ResolveAndSetUniforms(m_PSMaterialUniformBuffer, buffer);
						 });
	}

	GLenum Shader::ShaderTypeFromString(const std::string &type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;
		if (type == "compute")
			return GL_COMPUTE_SHADER;
		return GL_NONE;
	}

	std::unordered_map<GLenum, std::string> Shader::PreProcess(const std::string &source)
	{
		std::unordered_map<GLenum, std::string> shaderSources;
		// token to indicate shader type
		const char *typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);

		while (pos != std::string::npos)
		{
			// Parse current line
			// This will always be a shader type declartion
			size_t eol = source.find_first_of("\r\n", pos);
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);

			// Jump to next type declation
			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);

			// Add everything between inital position and jump to map as a new shader program
			shaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
			if (ShaderTypeFromString(type) == GL_COMPUTE_SHADER)
			{
				m_IsCompute = true;
				break;
			}
		}
		return shaderSources;
	}

	int32_t Shader::GetUniformLocation(const std::string &name) const
	{
		int32_t result = glGetUniformLocation(m_RendererID, name.c_str());
		if (result == -1)
			JN_CORE_WARN("Could not find uniform {0} in shader", name);
		return result;
	}

	void Shader::ResolveAndSetUniforms(const Ref<ShaderUniformBufferDeclaration> &decl, Buffer buffer)
	{
		const ShaderUniformList &uniforms = decl->GetUniformDeclarations();
		for (size_t i = 0; i < uniforms.size(); i++)
		{
			ShaderUniformDeclaration *uniform = (ShaderUniformDeclaration *)uniforms[i];
			if (uniform->IsArray())
				ResolveAndSetUniformArray(uniform, buffer);
			else
				ResolveAndSetUniform(uniform, buffer);
		}
	}

	void Shader::ResolveAndSetUniform(ShaderUniformDeclaration *uniform, Buffer buffer)
	{
		if (uniform->GetLocation() == -1)
			return;

		JN_ASSERT(uniform->GetLocation() != -1, "SHADER_ERROR: Uniform has invalid location!");

		uint32_t offset = uniform->GetOffset();
		switch (uniform->GetType())
		{
		case ShaderUniformDeclaration::Type::BOOL:
			UploadUniformFloat(uniform->GetLocation(), *(bool *)&buffer.Data[offset]);
			break;
		case ShaderUniformDeclaration::Type::FLOAT32:
			UploadUniformFloat(uniform->GetLocation(), *(float *)&buffer.Data[offset]);
			break;
		case ShaderUniformDeclaration::Type::INT32:
			UploadUniformInt(uniform->GetLocation(), *(int32_t *)&buffer.Data[offset]);
			break;
		case ShaderUniformDeclaration::Type::VEC2:
			UploadUniformFloat2(uniform->GetLocation(), *(glm::vec2 *)&buffer.Data[offset]);
			break;
		case ShaderUniformDeclaration::Type::VEC3:
			UploadUniformFloat3(uniform->GetLocation(), *(glm::vec3 *)&buffer.Data[offset]);
			break;
		case ShaderUniformDeclaration::Type::VEC4:
			UploadUniformFloat4(uniform->GetLocation(), *(glm::vec4 *)&buffer.Data[offset]);
			break;
		case ShaderUniformDeclaration::Type::MAT3:
			UploadUniformMat3(uniform->GetLocation(), *(glm::mat3 *)&buffer.Data[offset]);
			break;
		case ShaderUniformDeclaration::Type::MAT4:
			UploadUniformMat4(uniform->GetLocation(), *(glm::mat4 *)&buffer.Data[offset]);
			break;
		case ShaderUniformDeclaration::Type::STRUCT:
			UploadUniformStruct(uniform, buffer.Data, offset);
			break;
		default:
			JN_ASSERT(false, "SHADER_ERROR: Unknown uniform type!");
		}
	}

	void Shader::ResolveAndSetUniformArray(ShaderUniformDeclaration *uniform, Buffer buffer)
	{
		//JN_CORE_ASSERT(uniform->GetLocation() != -1, "Uniform has invalid location!");

		uint32_t offset = uniform->GetOffset();
		switch (uniform->GetType())
		{
		case ShaderUniformDeclaration::Type::BOOL:
			UploadUniformFloat(uniform->GetLocation(), *(bool *)&buffer.Data[offset]);
			break;
		case ShaderUniformDeclaration::Type::FLOAT32:
			UploadUniformFloat(uniform->GetLocation(), *(float *)&buffer.Data[offset]);
			break;
		case ShaderUniformDeclaration::Type::INT32:
			UploadUniformInt(uniform->GetLocation(), *(int32_t *)&buffer.Data[offset]);
			break;
		case ShaderUniformDeclaration::Type::VEC2:
			UploadUniformFloat2(uniform->GetLocation(), *(glm::vec2 *)&buffer.Data[offset]);
			break;
		case ShaderUniformDeclaration::Type::VEC3:
			UploadUniformFloat3(uniform->GetLocation(), *(glm::vec3 *)&buffer.Data[offset]);
			break;
		case ShaderUniformDeclaration::Type::VEC4:
			UploadUniformFloat4(uniform->GetLocation(), *(glm::vec4 *)&buffer.Data[offset]);
			break;
		case ShaderUniformDeclaration::Type::MAT3:
			UploadUniformMat3(uniform->GetLocation(), *(glm::mat3 *)&buffer.Data[offset]);
			break;
		case ShaderUniformDeclaration::Type::MAT4:
			UploadUniformMat4Array(uniform->GetLocation(), *(glm::mat4 *)&buffer.Data[offset], uniform->GetCount());
			break;
		case ShaderUniformDeclaration::Type::STRUCT:
			UploadUniformStruct(uniform, buffer.Data, offset);
			break;
		default:
			JN_ASSERT(false, "SHADER_ERROR: Unknown uniform type!");
		}
	}

	void Shader::UploadUniformField(uint32_t location, const ShaderUniformDeclaration& field, byte *data, int32_t offset)
	{
		switch (field.GetType())
		{
		case ShaderUniformDeclaration::Type::BOOL:
			UploadUniformFloat(location, *(bool *)&data[offset]);
			break;
		case ShaderUniformDeclaration::Type::FLOAT32:
			UploadUniformFloat(location, *(float *)&data[offset]);
			break;
		case ShaderUniformDeclaration::Type::INT32:
			UploadUniformInt(location, *(int32_t *)&data[offset]);
			break;
		case ShaderUniformDeclaration::Type::VEC2:
			UploadUniformFloat2(location, *(glm::vec2 *)&data[offset]);
			break;
		case ShaderUniformDeclaration::Type::VEC3:
			glm::vec3 vec = *(glm::vec3 *)&data[offset];

			UploadUniformFloat3(location, *(glm::vec3 *)&data[offset]);
			break;
		case ShaderUniformDeclaration::Type::VEC4:
			UploadUniformFloat4(location, *(glm::vec4 *)&data[offset]);
			break;
		case ShaderUniformDeclaration::Type::MAT3:
			UploadUniformMat3(location, *(glm::mat3 *)&data[offset]);
			break;
		case ShaderUniformDeclaration::Type::MAT4:
			UploadUniformMat4(location, *(glm::mat4 *)&data[offset]);
			break;
		default:
			JN_ASSERT(false, "SHADER_ERROR: Unknown uniform type!");
		}
	}
	
	void Shader::ResolveAndSetUniformField(const ShaderUniformDeclaration &field, byte *data, int32_t offset)
	{
		switch (field.GetType())
		{
		case ShaderUniformDeclaration::Type::BOOL:
			UploadUniformFloat(field.GetLocation(), *(bool *)&data[offset]);
			break;
		case ShaderUniformDeclaration::Type::FLOAT32:
			UploadUniformFloat(field.GetLocation(), *(float *)&data[offset]);
			break;
		case ShaderUniformDeclaration::Type::INT32:
			UploadUniformInt(field.GetLocation(), *(int32_t *)&data[offset]);
			break;
		case ShaderUniformDeclaration::Type::VEC2:
			UploadUniformFloat2(field.GetLocation(), *(glm::vec2 *)&data[offset]);
			break;
		case ShaderUniformDeclaration::Type::VEC3:
			UploadUniformFloat3(field.GetLocation(), *(glm::vec3 *)&data[offset]);
			break;
		case ShaderUniformDeclaration::Type::VEC4:
			UploadUniformFloat4(field.GetLocation(), *(glm::vec4 *)&data[offset]);
			break;
		case ShaderUniformDeclaration::Type::MAT3:
			UploadUniformMat3(field.GetLocation(), *(glm::mat3 *)&data[offset]);
			break;
		case ShaderUniformDeclaration::Type::MAT4:
			UploadUniformMat4(field.GetLocation(), *(glm::mat4 *)&data[offset]);
			break;
		default:
			JN_ASSERT(false, "SHADER_ERROR: Unknown uniform type!");
		}
	}

	Shader::~Shader()
	{
		glDeleteProgram(m_RendererID);
	}

	void Shader::Bind() const
	{
		Renderer::Submit([=]()
						 { JN_PROFILE_FUNCTION(); glUseProgram(m_RendererID); });
	}

	void Shader::Unbind() const
	{
		Renderer::Submit([=]()
						 { glUseProgram(0); });
	}

	void Shader::SetFloat(const std::string &name, float value)
	{
		Renderer::Submit([=]()
						 { UploadUniformFloat(name, value); });
	}

	void Shader::SetInt(const std::string &name, int value)
	{
		Renderer::Submit([=]()
						 { UploadUniformInt(name, value); });
	}

	void Shader::SetBool(const std::string &name, bool value)
	{
		Renderer::Submit([=]()
						 { UploadUniformInt(name, value); });
	}

	void Shader::SetFloat2(const std::string &name, const glm::vec2 &value)
	{
		Renderer::Submit([=]()
						 { UploadUniformFloat2(name, value); });
	}

	void Shader::SetFloat3(const std::string &name, const glm::vec3 &value)
	{
		Renderer::Submit([=]()
						 { UploadUniformFloat3(name, value); });
	}

	void Shader::SetMat4(const std::string &name, const glm::mat4 &value)
	{
		Renderer::Submit([=]()
						 { UploadUniformMat4(name, value); });
	}

	void Shader::SetMat4FromRenderThread(const std::string &name, const glm::mat4 &value, bool bind)
	{
		if (bind)
		{
			UploadUniformMat4(name, value);
		}
		else
		{
			int location = glGetUniformLocation(m_RendererID, name.c_str());
			if (location != -1)
				UploadUniformMat4(location, value);
		}
	}

	void Shader::SetIntArray(const std::string &name, int *values, uint32_t size)
	{
		Renderer::Submit([=]()
						 { UploadUniformIntArray(name, values, size); });
	}

	void Shader::UploadUniformInt(uint32_t location, int32_t value)
	{
		glUniform1i(location, value);
	}

	void Shader::UploadUniformIntArray(uint32_t location, int32_t *values, int32_t count)
	{
		glUniform1iv(location, count, values);
	}

	void Shader::UploadUniformFloat(uint32_t location, float value)
	{
		glUniform1f(location, value);
	}

	void Shader::UploadUniformFloat2(uint32_t location, const glm::vec2 &value)
	{
		glUniform2f(location, value.x, value.y);
	}

	void Shader::UploadUniformFloat3(uint32_t location, const glm::vec3 &value)
	{
		glUniform3f(location, value.x, value.y, value.z);
	}

	void Shader::UploadUniformFloat4(uint32_t location, const glm::vec4 &value)
	{
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void Shader::UploadUniformMat3(uint32_t location, const glm::mat3 &value)
	{
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void Shader::UploadUniformMat4(uint32_t location, const glm::mat4 &value)
	{
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void Shader::UploadUniformMat4Array(uint32_t location, const glm::mat4 &values, uint32_t count)
	{
		glUniformMatrix4fv(location, count, GL_FALSE, glm::value_ptr(values));
	}

	void Shader::UploadUniformStruct(ShaderUniformDeclaration *uniform, byte *buffer, uint32_t offset)
	{
		const ShaderStruct &s = uniform->GetShaderUniformStruct();
		const auto &fields = s.GetFields();
		for (size_t i = 0; i < uniform->GetCount(); i++) {
			for (size_t k = 0; k < fields.size(); k++)
			{
				ShaderUniformDeclaration *field = (ShaderUniformDeclaration *)fields[k];
				uint32_t location = field->GetLocation();
				if(uniform->IsArray()) {
					const std::string name = uniform->m_Name + "[" + std::to_string(i) + "]." + field->m_Name;
					location = GetUniformLocation(name);
				}
				UploadUniformField(location, *field, buffer, offset);
				offset += field->m_Size;
			}
		}
	}

	void Shader::UploadUniformInt(const std::string &name, int32_t value)
	{
		int32_t location = GetUniformLocation(name);
		glUniform1i(location, value);
	}

	void Shader::UploadUniformIntArray(const std::string &name, int32_t *values, uint32_t count)
	{
		int32_t location = GetUniformLocation(name);
		glUniform1iv(location, count, values);
	}

	void Shader::UploadUniformFloat(const std::string &name, float value)
	{
		glUseProgram(m_RendererID);
		auto location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location != -1)
			glUniform1f(location, value);
		else
			JN_CORE_WARN("Uniform '{0}' not found!", name);
	}

	void Shader::UploadUniformFloat2(const std::string &name, const glm::vec2 &values)
	{
		glUseProgram(m_RendererID);
		auto location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location != -1)
			glUniform2f(location, values.x, values.y);
		else
			JN_CORE_WARN("Uniform '{0}' not found!", name);
	}

	void Shader::UploadUniformFloat3(const std::string &name, const glm::vec3 &values)
	{
		glUseProgram(m_RendererID);
		auto location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location != -1)
			glUniform3f(location, values.x, values.y, values.z);
		else
			JN_CORE_WARN("Uniform '{0}' not found!", name);
	}

	void Shader::UploadUniformFloat4(const std::string &name, const glm::vec4 &values)
	{
		glUseProgram(m_RendererID);
		auto location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location != -1)
			glUniform4f(location, values.x, values.y, values.z, values.w);
		else
			JN_CORE_WARN("Uniform '{0}' not found!", name);
	}

	void Shader::UploadUniformMat4(const std::string &name, const glm::mat4 &values)
	{
		glUseProgram(m_RendererID);
		auto location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location != -1)
			glUniformMatrix4fv(location, 1, GL_FALSE, (const float *)&values);
		else
			JN_CORE_WARN("Uniform '{0}' not found!", name);
	}
}