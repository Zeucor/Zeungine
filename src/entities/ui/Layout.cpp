#include <zg/entities/ui/Layout.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/textures/Framebuffer.hpp>
#include <zg/Registry.hpp>
#include <zg/Scene.hpp>
#include <zg/interfaces/IPlatformWindow.hpp>
#include <zg/Window.hpp>
using namespace zg::entities::ui;
using namespace zg;
EntityCreateInfo zg::entities::ui::LayoutFactory(const std::string& name, LayoutDimension layoutDimension, glm::vec3 position, glm::vec3 size, IRenderer* irenderer, bool isNDCSize)
{
    auto& window = *irenderer->platformWindowPointer->renderWindowPointer;
    glm::vec2 texSize(
        isNDCSize ? (size.x * (window.windowWidth / 2.f)) : size.x,
        isNDCSize ? (size.y * (window.windowHeight / 2.f)) : size.y
    );
    auto color_texture = std::make_shared<textures::Texture>(irenderer, glm::ivec4(texSize.x, texSize.y, 1, 0), (const void*)0, textures::Texture::Format::RGBA8, textures::Texture::Type::UnsignedByte, textures::Texture::FilterType::Linear, true);
    auto depth_texture = std::make_shared<textures::Texture>(irenderer, glm::ivec4(texSize.x, texSize.y, 1, 0), (const void*)0, textures::Texture::Format::Depth, textures::Texture::Type::Float, textures::Texture::FilterType::Linear, true);
    auto layout_info = entities::PlaneFactory(color_texture, name + "Plane", position, rotate_identity, size);
    layout_info.typeName = "Layout";
    layout_info.dataMap = {
        {"ColorTexture", color_texture},
        {"DepthTexture", depth_texture},
        {"oldOpaqueHash", (size_t)0},
        {"oldTransparentHash", (size_t)0}
    };
    layout_info.setDataFunctionMap = {
        {"GetSubSize", [](const auto& nAny, auto& entity) -> std::any {
            return entity.getBoundingSize() / std::any_cast<const glm::vec3&>(nAny);
        } },
        {"GetSubPosition", [layoutDimension](const auto& sizeAny, auto& entity) -> std::any {
            const auto& size = std::any_cast<const glm::vec3&>(sizeAny);
            auto entitySize = entity.getBoundingSize();
            glm::vec3 position(-(entitySize.x / 2.f), (entitySize.y / 2.f), -(entitySize.z / 2.f));
            for (auto& child : entity.children)
            {
                auto childSize = child.getBoundingSize();
                switch (layoutDimension)
                {
                case LayoutDimension::Horizontal:
                    position.x += childSize.x;
                    break;
                case LayoutDimension::Vertical:
                    position.y -= childSize.y;
                    break;
                case LayoutDimension::Depth:
                    position.z += childSize.z;
                    break;
                }
            }
            switch (layoutDimension)
            {
            case LayoutDimension::Horizontal:
                position.x += (size.x / 2.f);
                break;
            case LayoutDimension::Vertical:
                position.y -= (size.y / 2.f);
                break;
            case LayoutDimension::Depth:
                position.z += (size.z / 2.f);
                break;
            }
            switch (layoutDimension)
            {
            case LayoutDimension::Horizontal:
                position.y -= (size.y / 2.f);
                position.z = entity.position.z + 0.1;
                break;
            case LayoutDimension::Vertical:
                position.x += (size.x / 2.f);
                position.z = entity.position.z + 0.1;
                break;
            case LayoutDimension::Depth:
                position.y -= (size.y / 2.f);
                position.x += (size.x / 2.f);
                break;
            }
            return position;
        } }
    };
    layout_info.onAddedFunction = [size](auto& entity) mutable {
        auto& color_texture = entity.template getData<std::shared_ptr<textures::Texture>>("ColorTexture");
        auto& depth_texture = entity.template getData<std::shared_ptr<textures::Texture>>("DepthTexture");
        std::vector<textures::Framebuffer::TextureAttachmentPair> textureAttachmentPairs({
            {color_texture, textures::Framebuffer::AttachmentType::Color},
            {depth_texture, textures::Framebuffer::AttachmentType::Depth}
        });
        auto& window = Registry::GetSingleton().getWindow(entity.INDEX_STACK);
        auto framebuffer = std::make_shared<textures::Framebuffer>(window.iRenderer, textureAttachmentPairs);
        auto view = std::make_shared<vp::View>(entity.position + glm::vec3(0, 0, 5), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
        auto projection = std::make_shared<vp::Projection>(window, glm::vec2(size));
        entity.template make<std::shared_ptr<textures::Framebuffer>>("Framebuffer", framebuffer);
        entity.template make<std::shared_ptr<vp::View>>("View", view);
        entity.template make<std::shared_ptr<vp::Projection>>("Projection", projection);
    };
    layout_info.preUpdateFunction = [layoutDimension](auto& entity) {
        for (auto& child : entity.children)
            child.skipRender = false;
        auto childDrawList = entity.getChildDrawList();
        auto& scene = Registry::GetSingleton().getScene(entity.INDEX_STACK);
        auto& oldOpaqueHash = entity.template getData<size_t>("oldOpaqueHash");
        auto& oldTransparentHash = entity.template getData<size_t>("oldTransparentHash");
        auto& framebuffer = entity.template getData<std::shared_ptr<textures::Framebuffer>>("Framebuffer");
        auto& view = entity.template getData<std::shared_ptr<vp::View>>("View");
        auto& projection = entity.template getData<std::shared_ptr<vp::Projection>>("Projection");
        auto& framebufferRef = *framebuffer;
        framebufferRef.bind();
        for (auto& drawPair : childDrawList)
        {
            drawPair.first->viewPointer = view;
            drawPair.first->projectionPointer = projection;
            drawPair.second->render(*drawPair.first);
        }
        framebufferRef.unbind();
        for (auto& child : entity.children)
            child.skipRender = true;
    };
    return layout_info;
}