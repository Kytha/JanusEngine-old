#pragma once

#include <string>
#include <vector>

#include "Core/Core.h"
#include "Core/Log.h"

class ShaderResourceDeclaration;
class ShaderUniformBufferDeclaration;

namespace Janus {

    enum class ShaderDomain
    {
        None = 0, Vertex = 0, Pixel = 1
    };

    class ShaderUniformDeclaration : public RefCounted
    {
    private:
        friend class ShaderStruct;   
        friend class Shader;    
    public:
        enum class Type
        {
            NONE, FLOAT32, VEC2, VEC3, VEC4, MAT3, MAT4, INT32, BOOL, STRUCT
        };
    public:
		inline const std::string& GetName() const  { return m_Name; }
		inline uint32_t GetSize() const  { return m_Size; }
		inline uint32_t GetCount() const  { return m_Count; }
		inline uint32_t GetOffset() const  { return m_Offset; }
		inline ShaderDomain GetDomain() const { return m_Domain; }

		int32_t GetLocation() const { return m_Location; }
		inline Type GetType() const { return m_Type; }
		inline bool IsArray() const { return m_Count > 1; }
        inline const ShaderStruct& GetShaderUniformStruct() const { return *m_Struct; }
        ShaderUniformDeclaration(ShaderDomain domain, Type type, const std::string& name, uint32_t count = 1);
        ShaderUniformDeclaration(ShaderDomain domain, ShaderStruct* uniformStruct, const std::string& name, uint32_t count = 1);

    public:
		static uint32_t SizeOfUniformType(Type type);
		static Type StringToType(const std::string& type);
		static std::string TypeToString(Type type);
        void SetOffset(uint32_t offset);
    private:
        std::string m_Name;
        uint32_t m_Size;
        uint32_t m_Count;
        uint32_t m_Offset;
        ShaderDomain m_Domain;

        Type m_Type;
        ShaderStruct* m_Struct;
        mutable int32_t m_Location;
    };

    typedef std::vector<ShaderUniformDeclaration*> ShaderUniformList;

    class ShaderUniformBufferDeclaration : public RefCounted
    {
	private:
		friend class Shader;
	private:
		std::string m_Name;
		ShaderUniformList m_Uniforms;
		uint32_t m_Register;
		uint32_t m_Size;
		ShaderDomain m_Domain;
	public:
		ShaderUniformBufferDeclaration(const std::string& name, ShaderDomain domain);

		void PushUniform(ShaderUniformDeclaration* uniform);

		inline const std::string& GetName() const  { return m_Name; }
		inline uint32_t GetRegister() const  { return m_Register; }
		inline uint32_t GetSize() const  { return m_Size; }
		virtual ShaderDomain GetDomain() const { return m_Domain; }
		inline const ShaderUniformList& GetUniformDeclarations() const  { return m_Uniforms; }

		ShaderUniformDeclaration* FindUniform(const std::string& name);
    };

    typedef std::vector<ShaderUniformBufferDeclaration*> ShaderUniformBufferList;

    class ShaderStruct
    {
    private:
        friend class Shader;
        friend class ShaderUniformDeclaration;
    private:
        std::string m_Name;
        std::vector<ShaderUniformDeclaration*> m_Fields;
        uint32_t m_Size;
        uint32_t m_Offset;
    public:
        ShaderStruct(const std::string& name);
        void AddField(ShaderUniformDeclaration* field)
        {
            m_Size += field->GetSize();
            uint32_t offset = 0;
            if(m_Fields.size()) {
                ShaderUniformDeclaration* previous = m_Fields.back();
                offset = previous->GetOffset() + previous->GetSize();
            }
            field->SetOffset(offset);
            m_Fields.push_back(field);
        }

        inline void SetOffset(uint32_t offset) {m_Offset = offset;}

        inline const std::string& GetName() const {return m_Name; }
        inline uint32_t GetSize() const {return m_Size;}
        inline uint32_t GetOffset() const {return m_Offset;}
        inline const std::vector<ShaderUniformDeclaration*>& GetFields() const {return m_Fields;}
    };

    typedef std::vector<ShaderStruct*> ShaderStructList;

	typedef std::vector<ShaderResourceDeclaration*> ShaderResourceList;

    class ShaderResourceDeclaration : public RefCounted
    {
    public:
        enum class Type
        {
            NONE, TEXTURE2D, TEXTURECUBE
        };
    private:
        friend class Shader;
    private:
        std::string m_Name;
        uint32_t m_Register;
        uint32_t m_Count;
        Type m_Type;
    public:
        ShaderResourceDeclaration(Type type, const std::string& name, uint32_t count);

        inline const std::string& GetName() const {return m_Name;}
        inline uint32_t GetRegister() const {return m_Register;}
        inline uint32_t GetCount() const {return m_Count;}
    	inline Type GetType() const { return m_Type; }
	public:
		static Type StringToType(const std::string& type);
		static std::string TypeToString(Type type); 
    };
}