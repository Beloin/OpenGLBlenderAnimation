//
// Created by beloin on 13/07/24.
//

#ifndef OPENGLTEMPLATE_TEXTURE_H
#define OPENGLTEMPLATE_TEXTURE_H

#include <string>
#include <GL/glew.h>

class Texture
{
public:
    Texture(GLenum TextureTarget, const std::string& FileName);

    Texture(GLenum TextureTarget);

    // Should be called once to load the texture
    bool Load();

    void Load(unsigned int BufferSize, void* pImageData);

    void Load(const std::string& Filename);

    void LoadRaw(int Width, int Height, int BPP, const unsigned char* pImageData);

    void LoadF32(int Width, int Height, const float* pImageData);

    // Must be called at least once for the specific texture unit
    void Bind(GLenum TextureUnit);

    void GetImageSize(int& ImageWidth, int& ImageHeight)
    {
        ImageWidth = m_imageWidth;
        ImageHeight = m_imageHeight;
    }

    GLuint GetTexture() const { return m_textureObj; }

private:
    void LoadInternal(const void* pImageData);
    void LoadInternalNonDSA(const void* pImageData);
    void LoadInternalDSA(const void* pImageData);

    void BindInternalNonDSA(GLenum TextureUnit);
    void BindInternalDSA(GLenum TextureUnit);

    std::string m_fileName;
    GLenum m_textureTarget;
    GLuint m_textureObj;
    int m_imageWidth = 0;
    int m_imageHeight = 0;
    int m_imageBPP = 0;
};


#endif //OPENGLTEMPLATE_TEXTURE_H
