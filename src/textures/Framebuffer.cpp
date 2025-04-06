#include <zg/Window.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/textures/Framebuffer.hpp>
#include <zg/textures/FramebufferFactory.hpp>
#include <zg/textures/Texture.hpp>
using namespace zg::textures;
Framebuffer::Framebuffer(Window& window, const std::vector<TextureAttachmentPair>& textureAttachmentPairs) :
		window(window), textureAttachmentPairs(textureAttachmentPairs)
{
	FramebufferFactory::initFramebuffer(*this);
}
Framebuffer::~Framebuffer() { FramebufferFactory::destroyFramebuffer(*this); }
void Framebuffer::bind() const { window.iRenderer->bindFramebuffer(*this); }
void Framebuffer::unbind() { window.iRenderer->unbindFramebuffer(*this); }