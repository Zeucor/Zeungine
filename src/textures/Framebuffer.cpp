#include <zg/Window.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/textures/Framebuffer.hpp>
#include <zg/textures/FramebufferFactory.hpp>
#include <zg/textures/Texture.hpp>
using namespace zg::textures;
Framebuffer::Framebuffer(IRenderer* iRenderer, const std::vector<TextureAttachmentPair>& textureAttachmentPairs, const BlendState& blendState) :
	iRenderer(iRenderer), textureAttachmentPairs(textureAttachmentPairs), blendState(blendState)
{
	FramebufferFactory::initFramebuffer(*this);
}
Framebuffer::~Framebuffer() { FramebufferFactory::destroyFramebuffer(*this); }
void Framebuffer::bind() const { iRenderer->bindFramebuffer(*this); }
void Framebuffer::unbind() { iRenderer->unbindFramebuffer(*this); }