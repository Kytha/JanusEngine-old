#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Core/Core.h"
#include "Core/Timestep.h"

#include "Graphics/IndexBuffer.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/Shader.h"
#include "Graphics/Material.h"
#include "Graphics/Pipeline.h"
#include "Graphics/ShaderLibrary.h"
//#include "AABB.h"

struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;

namespace Assimp
{
    class Importer;
}

namespace Janus
{
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec3 Binormal;
        glm::vec2 Texcoord;
    };

    static const int NumAttributes = 5;

    struct Index
    {
        uint32_t V1, V2, V3;
    };

    static_assert(sizeof(Index) == 3 * sizeof(uint32_t));

    class Submesh
    {
    public:
        uint32_t BaseVertex;
        uint32_t BaseIndex;
        uint32_t MaterialIndex;
        uint32_t IndexCount;
        glm::mat4 Transform;
        //AABB BoundingBox;
    };

    class Mesh : public RefCounted
    {
    public:
        Mesh(const std::string &filename);
        ~Mesh();

        void OnUpdate(Timestep ts);
        void DumpVertexBuffer();

        Ref<Shader> GetMeshShader() { return m_MeshShader; }
        const std::vector<Ref<Material>> GetMaterials() { return m_Materials; }
        const std::vector<Ref<Texture>> &GetTextures() const { return m_Textures; }
        const std::string &GetFilePath() const { return m_FilePath; }
        std::vector<Submesh> m_Submeshes;

    private:
        void TraverseNodes(aiNode *node, const glm::mat4 &parentTransform = glm::mat4(1.0f), uint32_t level = 0);

    private:
        std::unique_ptr<Assimp::Importer> m_Importer;

        glm::mat4 m_InverseTransform;

        Ref<Pipeline> m_Pipeline;
        Ref<VertexBuffer> m_VertexBuffer;
        Ref<IndexBuffer> m_IndexBuffer;
        Ref<Shader> m_MeshShader;
        std::vector<Vertex> m_Vertices;
        std::vector<Index> m_Indices;
        const aiScene *m_Scene;

        // Materials
        std::vector<Ref<Texture>> m_Textures;
        std::vector<Ref<Texture>> m_NormalMaps;
        std::vector<Ref<Material>> m_Materials;

        std::string m_FilePath;

        friend class Renderer;
    };
}
