#pragma once
#include "image.h"

CLASS_PTR(Texture)
class Texture
{
public:
    static TextureUPtr CreateFromImage(const ImagePtr image);
    static TextureUPtr Create(int width, int height, uint32_t format,
                              uint32_t type = GL_UNSIGNED_BYTE);
    ~Texture();

private:
    Texture() {}
    void CreateTexture();
    void SetTextureFromImage(const ImagePtr image);
    void SetTextureFormat(int width, int height, uint32_t format, uint32_t type);

private:
    uint32_t m_texture{0};
    uint32_t m_type{GL_UNSIGNED_BYTE};
    int m_width{0};
    int m_height{0};
    uint32_t m_format{GL_RGBA};

public:
    void Bind() const;
    void SetFilter(uint32_t minFilter, uint32_t magFilter) const;
    void SetWrap(uint32_t sWrap, uint32_t tWrap) const;
    void SetBorderColor(const glm::vec4 &color) const;


    const uint32_t Get() const { return m_texture; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    uint32_t GetFormat() const { return m_format; }
    uint32_t GetType() const { return m_type; }

};

CLASS_PTR(CubeTexture)
class CubeTexture
{
public:
    static CubeTextureUPtr CreateFromImages(const std::vector<Image *> &images);
    ~CubeTexture();

    const uint32_t Get() const { return m_texture; }
    void Bind() const;

private:
    CubeTexture() {}
    bool InitFromImages(const std::vector<Image *> &images);
    uint32_t m_texture{0};
};
