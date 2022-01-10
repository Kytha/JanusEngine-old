#include "jnpch.h"

#include <glad/glad.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#include <filesystem>

#include "Core/Core.h"

#include "Graphics/Renderer.h"
#include "Graphics/Mesh.h"

namespace Janus
{
    glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4 &matrix)
    {
        glm::mat4 result;
        //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
        result[0][0] = matrix.a1;
        result[1][0] = matrix.a2;
        result[2][0] = matrix.a3;
        result[3][0] = matrix.a4;
        result[0][1] = matrix.b1;
        result[1][1] = matrix.b2;
        result[2][1] = matrix.b3;
        result[3][1] = matrix.b4;
        result[0][2] = matrix.c1;
        result[1][2] = matrix.c2;
        result[2][2] = matrix.c3;
        result[3][2] = matrix.c4;
        result[0][3] = matrix.d1;
        result[1][3] = matrix.d2;
        result[2][3] = matrix.d3;
        result[3][3] = matrix.d4;
        return result;
    }

    static const uint32_t s_MeshImportFlags =
        aiProcess_CalcTangentSpace |     // Create binormals/tangents just in case
        aiProcess_Triangulate |          // Make sure we're triangles
        aiProcess_SortByPType |          // Split meshes by primitive type
        aiProcess_GenNormals |           // Make sure we have legit normals
        aiProcess_GenUVCoords |          // Convert UVs if required
        aiProcess_ValidateDataStructure; // Validation

    Mesh::Mesh(const std::string &filename)
        : m_FilePath(filename)
    {
        m_Importer = std::make_unique<Assimp::Importer>();

        const aiScene *scene = m_Importer->ReadFile(filename, s_MeshImportFlags);
        if (!scene || !scene->HasMeshes())
        {
            JN_ASSERT(false, "ERROR::MESH:: Scene has no meshes");
        }

        m_MeshShader = Renderer::GetShaderLibrary()->Get("janus_pbr");
        m_BaseMaterial = Material::Create(m_MeshShader);

        m_InverseTransform = glm::inverse(Mat4FromAssimpMat4(scene->mRootNode->mTransformation));

        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;

        m_Submeshes.reserve(scene->mNumMeshes);
        for (size_t m = 0; m < scene->mNumMeshes; m++)
        {
            aiMesh *mesh = scene->mMeshes[m];

            Submesh &submesh = m_Submeshes.emplace_back();
            submesh.BaseVertex = vertexCount;
            submesh.BaseIndex = indexCount;
            submesh.MaterialIndex = mesh->mMaterialIndex;
            submesh.IndexCount = mesh->mNumFaces * 3;

            vertexCount += mesh->mNumVertices;
            indexCount += submesh.IndexCount;

            assert(mesh->HasPositions());
            assert(mesh->HasNormals());

            //auto& aabb = submesh.BoundingBox;
            //aabb.Min = { FLT_MAX, FLT_MAX, FLT_MAX };
            //aabb.Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

            for (size_t i = 0; i < mesh->mNumVertices; i++)
            {
                Vertex vertex;
                vertex.Position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
                vertex.Normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};

                //aabb.Min.x = glm::min(vertex.Position.x, aabb.Min.x);
                //aabb.Min.y = glm::min(vertex.Position.y, aabb.Min.y);
                //aabb.Min.z = glm::min(vertex.Position.z, aabb.Min.z);
                //aabb.Max.x = glm::max(vertex.Position.x, aabb.Max.x);
                //aabb.Max.y = glm::max(vertex.Position.y, aabb.Max.y);
                //aabb.Max.z = glm::max(vertex.Position.z, aabb.Max.z);

                if (mesh->HasTangentsAndBitangents())
                {
                    vertex.Tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
                    vertex.Binormal = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
                }

                if (mesh->HasTextureCoords(0))
                    vertex.Texcoord = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};

                m_Vertices.push_back(vertex);
            }

            // Indices
            for (size_t i = 0; i < mesh->mNumFaces; i++)
            {
                assert(mesh->mFaces[i].mNumIndices == 3);
                m_Indices.push_back({mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2]});
            }
            m_Submeshes.push_back(submesh);
        }

        TraverseNodes(scene->mRootNode);

        // Materials
        if (scene->HasMaterials())
        {
            m_Textures.resize(scene->mNumMaterials);
            m_Materials.resize(scene->mNumMaterials);
            for (uint32_t i = 0; i < scene->mNumMaterials; i++)
            {
                auto aiMaterial = scene->mMaterials[i];
                auto aiMaterialName = aiMaterial->GetName();

                auto mi = Ref<MaterialInstance>::Create(m_BaseMaterial, aiMaterialName.data);
                mi->SetFlag(MaterialFlag::TwoSided, false);
                m_Materials[i] = mi;

                aiString aiTexPath;
                uint32_t textureCount = aiMaterial->GetTextureCount(aiTextureType_DIFFUSE);

                aiColor3D aiColor;
                aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);

                float shininess, metalness;
                if (aiMaterial->Get(AI_MATKEY_SHININESS, shininess) != aiReturn_SUCCESS)
                    shininess = 80.0f; // Default value

                if (aiMaterial->Get(AI_MATKEY_REFLECTIVITY, metalness) != aiReturn_SUCCESS)
                    metalness = 0.0f;

                float roughness = 1.0f - glm::sqrt(shininess / 100.0f);
                bool hasAlbedoMap = aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexPath) == AI_SUCCESS;
                if (hasAlbedoMap)
                {

                    std::filesystem::path path = filename;
                    auto parentPath = path.parent_path();
                    parentPath /= std::string(aiTexPath.data);
                    std::string texturePath = parentPath.string();
                    auto texture = Ref<Texture>::Create(texturePath);
                    if (texture->Loaded())
                    {
                        m_Textures[i] = texture;
                        mi->Set("u_AlbedoTexture", m_Textures[i]);
                        mi->Set("u_AlbedoTexToggle", 1.0f);
                    }
                    else
                    {
                        mi->Set("u_AlbedoColor", glm::vec3{aiColor.r, aiColor.g, aiColor.b});
                    }
                }
                else
                {
                    mi->Set("u_AlbedoColor", glm::vec3{aiColor.r, aiColor.g, aiColor.b});
                }

                mi->Set("u_NormalTexToggle", 0.0f);
                if (aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath) == AI_SUCCESS)
                {
                    // TODO: Temp - this should be handled by Hazel's filesystem
                    std::filesystem::path path = filename;
                    auto parentPath = path.parent_path();
                    parentPath /= std::string(aiTexPath.data);
                    std::string texturePath = parentPath.string();
                    auto texture = Ref<Texture>::Create(texturePath);
                    if (texture->Loaded())
                    {
                        mi->Set("u_NormalTexture", texture);
                        mi->Set("u_NormalTexToggle", 1.0f);
                    }
                    else
                    {
                        JN_CORE_ERROR("Could not load texture: {0}", texturePath);
                    }
                }
                else
                {
                    JN_CORE_WARN("No normal map");
                }

                if (aiMaterial->GetTexture(aiTextureType_SHININESS, 0, &aiTexPath) == AI_SUCCESS)
                {
                    std::filesystem::path path = filename;
                    auto parentPath = path.parent_path();
                    parentPath /= std::string(aiTexPath.data);
                    std::string texturePath = parentPath.string();
                    auto texture = Ref<Texture>::Create(texturePath);
                    if (texture->Loaded())
                    {
                        mi->Set("u_RoughnessTexture", texture);
                        mi->Set("u_RoughnessTexToggle", 1.0f);
                    }
                    else
                    {
                        JN_CORE_ERROR("Could not load texture: {0}", texturePath);
                    }
                }
                bool metalnessTextureFound = false;
                for (uint32_t i = 0; i < aiMaterial->mNumProperties; i++)
                {
                    auto prop = aiMaterial->mProperties[i];
#if 0
					switch (prop->mSemantic)
					{
					case aiTextureType_NONE:
						JN_CORE_TRACE("  Semantic = aiTextureType_NONE");
						break;
					case aiTextureType_DIFFUSE:
						JN_CORE_TRACE("  Semantic = aiTextureType_DIFFUSE");
						break;
					case aiTextureType_SPECULAR:
						JN_CORE_TRACE("  Semantic = aiTextureType_SPECULAR");
						break;
					case aiTextureType_AMBIENT:
						JN_CORE_TRACE("  Semantic = aiTextureType_AMBIENT");
						break;
					case aiTextureType_EMISSIVE:
						JN_CORE_TRACE("  Semantic = aiTextureType_EMISSIVE");
						break;
					case aiTextureType_HEIGHT:
						JN_CORE_TRACE("  Semantic = aiTextureType_HEIGHT");
						break;
					case aiTextureType_NORMALS:
						JN_CORE_TRACE("  Semantic = aiTextureType_NORMALS");
						break;
					case aiTextureType_SHININESS:
						JN_CORE_TRACE("  Semantic = aiTextureType_SHININESS");
						break;
					case aiTextureType_OPACITY:
						JN_CORE_TRACE("  Semantic = aiTextureType_OPACITY");
						break;
					case aiTextureType_DISPLACEMENT:
						JN_CORE_TRACE("  Semantic = aiTextureType_DISPLACEMENT");
						break;
					case aiTextureType_LIGHTMAP:
						JN_CORE_TRACE("  Semantic = aiTextureType_LIGHTMAP");
						break;
					case aiTextureType_REFLECTION:
						JN_CORE_TRACE("  Semantic = aiTextureType_REFLECTION");
						break;
					case aiTextureType_UNKNOWN:
						JN_CORE_TRACE("  Semantic = aiTextureType_UNKNOWN");
						break;
					}
#endif
                    //std::cout << "Material Property: " << std::endl;
                    //std::cout << "Name = " << prop->mKey.data << std::endl << std::endl;
                    if (prop->mType == aiPTI_String)
                    {
                        uint32_t strLength = *(uint32_t *)prop->mData;
                        std::string str(prop->mData + 4, strLength);

                        std::string key = prop->mKey.data;
                        if (key == "$raw.ReflectionFactor|file")
                        {
                            metalnessTextureFound = true;
                            std::filesystem::path path = filename;
                            auto parentPath = path.parent_path();
                            parentPath /= str;
                            std::string texturePath = parentPath.string();
                            auto texture = Ref<Texture>::Create(texturePath);
                            if (texture->Loaded())
                            {
                                mi->Set("u_MetalnessTexture", texture);
                                mi->Set("u_MetalnessTexToggle", 1.0f);
                            }
                            else
                            {
                                JN_CORE_ERROR("Could not load texture: {0}", texturePath);
                                mi->Set("u_Metalness", metalness);
                                mi->Set("u_MetalnessTexToggle", 0.0f);
                            }
                            break;
                        }
                    }
                }

                if (!metalnessTextureFound)
                {
                    JN_CORE_WARN("No metalness map");
                    mi->Set("u_Metalness", metalness);
                    mi->Set("u_MetalnessTexToggle", 0.0f);
                }
            }
        }

        m_VertexBuffer = Ref<VertexBuffer>::Create(m_Vertices.data(), static_cast<uint32_t>(m_Vertices.size() * sizeof(Vertex)));

        BufferLayout vertexLayout = {
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float3, "a_Normal"},
            {ShaderDataType::Float3, "a_Tangent"},
            {ShaderDataType::Float3, "a_Binormal"},
            {ShaderDataType::Float2, "a_TexCoord"},
        };

        m_VertexBuffer->SetLayout(vertexLayout);
        m_IndexBuffer = Ref<IndexBuffer>::Create(m_Indices.data(), static_cast<uint32_t>(m_Indices.size() * sizeof(Index)));

        PipelineSpecification pipelineSpecification;
        pipelineSpecification.Layout = vertexLayout;
        m_Pipeline = Ref<Pipeline>::Create(pipelineSpecification);

        m_Scene = scene;
    }

    Mesh::~Mesh()
    {
    }

    void Mesh::TraverseNodes(aiNode *node, const glm::mat4 &parentTransform, uint32_t level)
    {
        glm::mat4 transform = parentTransform * Mat4FromAssimpMat4(node->mTransformation);
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            uint32_t mesh = node->mMeshes[i];
            m_Submeshes[mesh].Transform = transform;
        }

        // JN_CORE_TRACE("{0} {1}", LevelToSpaces(level), node->mName.C_Str());

        for (uint32_t i = 0; i < node->mNumChildren; i++)
            TraverseNodes(node->mChildren[i], transform, level + 1);
    }
}