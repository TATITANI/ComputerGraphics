#pragma once

#include "texture.h"


/// @brief
// shadow map의 크기를 지정하면 해당 크기의 depth 텍스처를 만들어서
// framebuffer에 바인딩
// color / stencil은 사용하지 않음

CLASS_PTR(ShadowMap);
class ShadowMap
{
public:
    static ShadowMapUPtr Create(int width, int height);
    ~ShadowMap();

    const uint32_t Get() const { return m_framebuffer; }
    void Bind() const;
    const TexturePtr GetShadowMap() const { return m_shadowMap; }

private:
    ShadowMap() {}
    bool Init(int width, int height);

    uint32_t m_framebuffer{0}; // depth map에 렌더링을 하기 위한 프레임버퍼
    TexturePtr m_shadowMap;    // depth map 저장을 위한 텍스처
};