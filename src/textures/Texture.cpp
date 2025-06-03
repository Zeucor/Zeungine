#include <zg/Serial.hpp>
#include <zg/Window.hpp>
#include <zg/interfaces/IRenderer.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/textures/Texture.hpp>
#include <zg/textures/TextureFactory.hpp>
using namespace zg::textures;
Texture::Texture(IRenderer* iRenderer, const glm::ivec4& size, const void* data, const Format& format, const Type& type,
								 const FilterType& filterType, bool isFramebufferAttachment, Multisampling multisampling,
								 AddressMode addressMode, bool flip) :
		iRenderer(iRenderer), size(size), format(format), type(type), filterType(filterType),
		isFramebufferAttachment(isFramebufferAttachment),
    multisampling(multisampling),
	addressMode(addressMode),
	flip(flip)
{
	TextureFactory::initTexture(*this, data);
	isTransparent = testIsTransparent(data); // TODO: run this check in other constructors
}
Texture::Texture(IRenderer* iRenderer, const glm::ivec4& size, const std::vector<void*> datas, const Format& format,
								 const Type& type, const FilterType& filterType, bool isFramebufferAttachment, Multisampling multisampling,
								 AddressMode addressMode, bool flip) :
		iRenderer(iRenderer), size(size), format(format), type(type), filterType(filterType),
		isFramebufferAttachment(isFramebufferAttachment),
    multisampling(multisampling),
	addressMode(addressMode),
	flip(flip)
{
	TextureFactory::initTexture(*this, datas);
}
Texture::Texture(IRenderer* iRenderer, const glm::ivec4& size, const std::string_view path, const Format& format,
								 const Type& type, const FilterType& filterType, bool isFramebufferAttachment, Multisampling multisampling,
								 AddressMode addressMode, bool flip) :
		iRenderer(iRenderer), size(size), format(format), type(type), filterType(filterType),
		isFramebufferAttachment(isFramebufferAttachment),
    multisampling(multisampling),
	addressMode(addressMode),
	flip(flip)
{
	TextureFactory::initTexture(*this, path);
}
Texture::Texture(IRenderer* iRenderer, const glm::ivec4& size, const std::vector<std::string_view>& paths,
								 const Format& format, const Type& type, const FilterType& filterType, bool isFramebufferAttachment, Multisampling multisampling,
								 AddressMode addressMode, bool flip) :
		iRenderer(iRenderer), size(size), format(format), type(type), filterType(filterType),
		isFramebufferAttachment(isFramebufferAttachment),
    multisampling(multisampling),
	addressMode(addressMode),
	flip(flip)
{
	TextureFactory::initTexture(*this, paths);
}
Texture::~Texture() { TextureFactory::destroyTexture(*this); }
void Texture::bind() const { iRenderer->bindTexture(*this); }
void Texture::unbind() const { iRenderer->unbindTexture(*this); }
void Texture::update(const void* data) { TextureFactory::updateTexture(*this, data); }
void Texture::update(const std::string_view path) { TextureFactory::updateTexture(*this, path); }
void Texture::update(const std::vector<std::string_view>& paths) { TextureFactory::updateTexture(*this, paths); }
template <>
Serial& serialize(Serial& serial, const std::pair<std::string, std::shared_ptr<Texture>>& pair)
{
	return serial << pair.first << pair.second;
};
template <>
Serial& deserialize(Serial& serial, std::pair<std::string, std::shared_ptr<Texture>>& pair)
{
	return serial >> pair.first >> pair.second;
};
template <>
Serial& serialize(Serial& serial, const std::shared_ptr<Texture>& texturePointer)
{
	auto& texture = *texturePointer;
	serial << true;
	auto textureDatasSize = texture.datas.size();
	serial << texture.size << textureDatasSize;
	for (auto& dataPair : texture.datas)
	{
		serial << dataPair.first;
		serial.writeBytes(dataPair.second.get(), dataPair.first);
	}
	serial << texture.format << texture.type << texture.filterType;
	return serial;
}
template <>
Serial& deserialize(Serial& serial, std::shared_ptr<Texture>& texturePointer)
{
	bool wroteBit = false;
	serial >> wroteBit;
	if (!wroteBit)
		return serial;
	glm::ivec4 size;
	serial >> size;
	std::vector<void*> datas;
	auto textureDatasSize = datas.size();
	serial >> textureDatasSize;
	datas.resize(textureDatasSize);
	for (auto& pointer : datas)
	{
		size_t datasize = 0;
		serial >> datasize;
		char* data = (char*)malloc(datasize);
		serial.readBytes(data, datasize);
		datas.push_back(data);
	}
	Texture::Format format;
	Texture::Type type;
	Texture::FilterType filterType;
	serial >> format >> type >> filterType;
	auto windowPointer = (zg::Window*)serial.getContextPointer("Window");
	texturePointer =
		std::make_shared<Texture>(windowPointer->iRenderer, size, datas, format, type, filterType);
	return serial;
}
bool Texture::testIsTransparent(const void* data)
{
	if (!data)
		return false;
	auto colors = (uvec*)data;
	size_t n = size.x * size.y;
	for (size_t i = 0; i < n; ++i)
		// std::cout << "colors[" << i << "]: " << glm::to_string(colors[i]) << std::endl; 
		if (colors[i].a < 255)
			return true;
	return false;
}