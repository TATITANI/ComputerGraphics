#pragma once
#include "image.h"

CLASS_PTR(Texture)
class Texture
{
public:
    static TextureUPtr CreateFromImage(const ImagePtr image);
    static TextureUPtr Create(int width, int height, uint32_t format);
    ~Texture();

    const uint32_t Get() const { return m_texture; }
    void Bind() const;
    void SetFilter(uint32_t minFilter, uint32_t magFilter) const;
    void SetWrap(uint32_t sWrap, uint32_t tWrap) const;

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    uint32_t GetFormat() const { return m_format; }

private:
    Texture() {}
    void CreateTexture();
    void SetTextureFromImage(const ImagePtr image);
    void SetTextureFormat(int width, int height, uint32_t format);

private:
    uint32_t m_texture{0};
    int m_width{0};
    int m_height{0};
    uint32_t m_format{GL_RGBA};
};
