#pragma once
#include <zg/textures/Framebuffer.hpp>
#include <array>
#include <vector>
namespace zg::textures
{
    template <size_t N>
    struct NBuffered
    {
        std::array<std::shared_ptr<textures::Framebuffer>, N> buffers;
        size_t I = N - 1;
        std::array<bool, N> drawnIOnce;
        NBuffered(
            IRenderer* irenderer,
            const std::array<std::vector<textures::Framebuffer::TextureAttachmentPair>, N>& arrayTextureAttachmentPairs,
            const BlendState& blendState = {}
        )
        {
            for (size_t i = 0; i < N; i++)
            {
                buffers[i] = std::make_shared<textures::Framebuffer>(irenderer, arrayTextureAttachmentPairs[i], blendState);
            }
        }
        void bind()
        {
            I++;
            if (I == N)
                I = 0;
            buffers[I]->bind();
        }
        bool drawnOnce()
        {
            return drawnIOnce[0];
        }
        void unbind()
        {
            auto& drawnIOnceRef = drawnIOnce[I];
            if (!drawnIOnceRef)
                drawnIOnceRef = true;
            buffers[I]->unbind();
        }
        std::shared_ptr<textures::Texture> getColorTexture()
        {
            if (!drawnOnce())
                return buffers[I]->getColorTexture();
            auto J = (I == (N - 1)) ? 0 : I + 1;
            return buffers[J]->getColorTexture();
        }
        std::shared_ptr<textures::Texture> getColorResolveTexture()
        {
            if (!drawnOnce())
                return buffers[I]->getColorResolveTexture();
            auto J = (I == (N - 1)) ? 0 : I + 1;
            return buffers[J]->getColorResolveTexture();
        }
        std::shared_ptr<textures::Texture> getDepthTexture()
        {
            if (!drawnOnce())
                return buffers[I]->getDepthTexture();
            auto J = (I == (N - 1)) ? 0 : I + 1;
            return buffers[J]->getDepthTexture();
        }
        std::shared_ptr<textures::Texture> getDepthResolveTexture()
        {
            if (!drawnOnce())
                return buffers[I]->getDepthResolveTexture();
            auto J = (I == (N - 1)) ? 0 : I + 1;
            return buffers[J]->getDepthResolveTexture();
        }
    };
}