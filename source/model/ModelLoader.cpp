//
// Created by beloin on 13/07/24.
//

#include <iostream>
#include "ModelLoader.h"

ModelLoader::~ModelLoader() {
    Clear();
}

void ModelLoader::Clear() {
    for (int i = 0; i < m_Textures.size(); ++i) {
        SAFE_DELETE(m_Textures[i]);
    }

    if (m_Buffers[0] != 0) {
        glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
    }

    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}

bool ModelLoader::LoadMesh(const string &Filename) {
    // TODO: We will change this
    Clear();

    // Create VAO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // Create the buffer for the vertices attributes
    glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

    bool ret = false;
    Assimp::Importer importer;
    const aiScene *pScene = importer.ReadFile(Filename, ASSIMP_LOAD_FLAGS);

    if (pScene) {
        ret = InitFromScene(pScene, Filename);
    } else {
        std::cerr << "error parsing " << Filename << ":" << importer.GetErrorString() << std::endl;
    }

    glBindVertexArray(0);

    return ret;
}

bool ModelLoader::InitFromScene(const aiScene *pScene, const string &Filename) {
    m_Meshes.resize(pScene->mNumMeshes);
    m_Textures.resize(pScene->mNumMaterials);

    unsigned int NumVertices = 0;
    unsigned int NumIndices = 0;
    CountVerticeAndIndices(pScene, NumVertices, NumIndices);
    ReserveSpace(NumVertices, NumIndices);

    InitAllMeshes(pScene);

    if (!InitMaterials(pScene, Filename)) {
        return false;
    }

    PopulateBuffers();

    return GLCheckError();
}

bool ModelLoader::CountVerticeAndIndices(const aiScene *pScene, unsigned int &NumVertices, unsigned int &NumIndices) {
    for (unsigned int i = 0; i < m_Meshes.size(); ++i) {
        m_Meshes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
        m_Meshes[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3; // Triagulation
        m_Meshes[i].BaseVertex = NumVertices;
        m_Meshes[i].BaseIndex = NumIndices;

        NumVertices += pScene->mMeshes[i]->mNumVertices;
        NumIndices += m_Meshes[i].NumIndices;
    }
}

void ModelLoader::ReserveSpace(unsigned int NumVertices, unsigned int NumIndices) {
    m_Positions.reserve(NumVertices);
    m_Normals.reserve(NumVertices);
    m_TexCoords.reserve(NumVertices);
    m_Indices.reserve(NumIndices);
}

void ModelLoader::InitAllMeshes(const aiScene *pScene) {
    for (unsigned int i = 0; i < m_Meshes.size(); ++i) {
        const aiMesh *paiMesh = pScene->mMeshes[i];
        InitSingleMesh(paiMesh);
    }
}

void ModelLoader::InitSingleMesh(const aiMesh *paiMesh) {
    const aiVector3D zero3D{.0f, .0f, .0f};

    // Populate Vertex attribute vectors
    for (unsigned int i = 0; i < paiMesh->mNumVertices; ++i) {
        const aiVector3D &pPos = paiMesh->mVertices[i];
        const aiVector3D &pNormal = paiMesh->mNormals[i];
        const aiVector3D &pTexCoord = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : zero3D;

        m_Positions.emplace_back(pPos.x, pPos.y, pPos.z);
        m_Normals.emplace_back(pNormal.x, pNormal.y, pNormal.z);
        m_TexCoords.emplace_back(pTexCoord.x, pTexCoord.y);
    }

    // Populate index buffer
    for (unsigned int i = 0; i < paiMesh->mNumFaces; ++i) {
        const aiFace &face = paiMesh->mFaces[i];

        assert(face.mNumIndices == 3);
        m_Indices.push_back(face.mIndices[0]);
        m_Indices.push_back(face.mIndices[1]);
        m_Indices.push_back(face.mIndices[2]);
    }
}

bool ModelLoader::InitMaterials(const aiScene *pScene, const string &Filename) {
    auto slashIndex = Filename.find_last_of("/");
    string dir;

    if (slashIndex == string::npos) {
        dir = ".";
    } else if (slashIndex == 0) {
        dir = "/";
    } else {
        dir = Filename.substr(0, slashIndex);
    }

    bool status = true;

    for (unsigned int i = 0; i < pScene->mNumMaterials; ++i) {
        auto *pMaterial = pScene->mMaterials[i];

        m_Textures[i] = nullptr;

        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, nullptr, nullptr, nullptr, nullptr, nullptr) ==
                AI_SUCCESS) {
                string p{path.data};

                if (p.substr(0, 2) == ".\\") {
                    p = p.substr(2, p.size() - 2);
                }

//                string fullPath = dir + "/" + p; // This causes minimum 2 temporary strings
                string fullPath{dir};
                fullPath += "/";
                fullPath += p;
                Texture *&pTexture = m_Textures[i];
                pTexture = new Texture(GL_TEXTURE_2D, fullPath);

                if (!pTexture->Load()) {
                    std::cerr << "error loading texture " << fullPath << std::endl;
                    delete pTexture;
                    pTexture = nullptr;
                    status = false;
                } else {
                    std::cout << "loaded texture" << fullPath << std::endl;
                }
            }
        }
    }

    return status;
}

void ModelLoader::PopulateBuffers() {
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Positions[0]) * m_Positions.size(), &m_Positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_TexCoords[0]) * m_TexCoords.size(), &m_TexCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Normals[0]) * m_Normals.size(), &m_Normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Indices[0]) * m_Indices.size(), &m_Indices[0], GL_STATIC_DRAW);
}


