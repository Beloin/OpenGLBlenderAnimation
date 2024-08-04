//
// Created by beloin on 13/07/24.
//

#ifndef OPENGLTEMPLATE_MODELLOADER_H
#define OPENGLTEMPLATE_MODELLOADER_H

#include "assimp/Importer.hpp"
#include "engine/world_transform.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "engine/Texture.h"

class ModelLoader {

public:
    ModelLoader() {};

    ~ModelLoader();

    bool LoadMesh(const std::string &Filename);

    // TODO: Add this methods of render and World transform inside other
    // ModelClass
    void Render();

    void Render(unsigned int NumInstances, const Matrix4f *WVPMats,
                const Matrix4f WorldMats);

    WorldTrans &GetWorldTransform() { return m_WorldTransform; }

private:
    enum BufferType {
        INDEX_BUFFER = 0,
        POS_VB,
        TEXCOORD_VB,
        NORMAL_VB,
        WVP_MAT_VB,
        WORLD_MAT_VB,
        NUM_BUFFERS // TODO: Weird code here
    };

    WorldTrans m_WorldTransform;
    GLuint m_VAO;
    GLuint m_Buffers[NUM_BUFFERS] = {0};

#define INVALID_MATERIAL 0xFFFFFFFF

    struct BasicMeshEntry {
        BasicMeshEntry() {
            NumIndices = 0;
            BaseVertex = 0;
            BaseIndex = 0;
            MaterialIndex = INVALID_MATERIAL;
        }

        unsigned int NumIndices;
        unsigned int BaseVertex;
        unsigned int BaseIndex;
        unsigned int MaterialIndex;
    };

    std::vector<BasicMeshEntry> m_Meshes;
    std::vector<Texture*> m_Textures;

    // Temporary space for vertex before loads into GPU
    // All base on that vectors are a continuous space of memory
    std::vector<Vector3f> m_Positions;
    std::vector<Vector3f> m_Normals;
    std::vector<Vector2f> m_TexCoords;
    std::vector<unsigned int> m_Indices;

    void Clear();

    bool InitFromScene(const aiScene *pScene, const std::string &Filename);

    bool CountVerticeAndIndices(const aiScene *pScene, unsigned int &NumVertices, unsigned int &NumIndices);

    void ReserveSpace(unsigned int NumVertices, unsigned int NumIndices);

    void InitAllMeshes(const aiScene *pScene);

    void InitSingleMesh(const aiMesh *paiMesh);

    bool InitMaterials(const aiScene *pScene, const std::string &Filename);

    void PopulateBuffers();
};

#endif // OPENGLTEMPLATE_MODELLOADER_H
