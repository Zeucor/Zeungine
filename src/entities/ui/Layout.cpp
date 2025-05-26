#include <zg/entities/ui/Layout.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/textures/Framebuffer.hpp>
#include <zg/Registry.hpp>
#include <zg/Scene.hpp>
#include <zg/interfaces/IPlatformWindow.hpp>
#include <zg/Window.hpp>
using namespace zg::entities::ui;
using namespace zg;
EntityCreateInfo zg::entities::ui::LayoutFactory(const std::string& name, LayoutDimension layoutDimension, glm::vec3 position, glm::vec3 size, IRenderer* irenderer, bool isNDCSize, const textures::BlendState& blendState)
{
    auto& window = *irenderer->platformWindowPointer->renderWindowPointer;
    glm::vec2 texSize(
        isNDCSize ? (size.x * (window.windowWidth / 2.f)) : size.x,
        isNDCSize ? (size.y * (window.windowHeight / 2.f)) : size.y
    );
    auto color_texture = std::make_shared<textures::Texture>(irenderer, glm::ivec4(texSize.x, texSize.y, 1, 0), (const void*)0, textures::Texture::Format::RGBA8, textures::Texture::Type::UnsignedByte, textures::Texture::FilterType::Linear, true);
    color_texture->isTransparent = true;
    // auto depth_texture = std::make_shared<textures::Texture>(irenderer, glm::ivec4(texSize.x, texSize.y, 1, 0), (const void*)0, textures::Texture::Format::Depth, textures::Texture::Type::Float, textures::Texture::FilterType::Linear, true);
    auto layout_info = entities::PlaneFactory(color_texture, name + "Plane", position, rotate_identity, size);
    layout_info.typeName = "Layout";
    layout_info.dataMap = {
        {"ColorTexture", color_texture},
        // {"DepthTexture", depth_texture},
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
    layout_info.onAddedFunction = [size, blendState](auto& entity) mutable {
        auto& color_texture = entity.template getData<std::shared_ptr<textures::Texture>>("ColorTexture");
        // auto& depth_texture = entity.template getData<std::shared_ptr<textures::Texture>>("DepthTexture");
        std::vector<textures::Framebuffer::TextureAttachmentPair> textureAttachmentPairs({
            {color_texture, textures::Framebuffer::AttachmentType::Color}//,
            // {depth_texture, textures::Framebuffer::AttachmentType::Depth}
        });
        auto& window = Registry::GetSingleton().getWindow(entity.INDEX_STACK);
        auto framebuffer = std::make_shared<textures::Framebuffer>(window.iRenderer, textureAttachmentPairs, blendState);
		auto entity_model_tuple = entity.decomposeModel();
        auto view = std::make_shared<vp::View>(std::get<0>(entity_model_tuple) + glm::vec3(0, 0, 5), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
        auto projection = std::make_shared<vp::Projection>(window, glm::vec2(size));
        entity.template make<std::shared_ptr<textures::Framebuffer>>("Framebuffer", framebuffer);
        entity.template make<std::shared_ptr<vp::View>>("View", view);
        entity.template make<std::shared_ptr<vp::Projection>>("Projection", projection);
    };
    layout_info.preUpdateFunction = [layoutDimension](auto& entity) {
		auto entity_model_tuple = entity.decomposeModel();
        auto& view = entity.template getData<std::shared_ptr<vp::View>>("View");
        auto new_view_position = std::get<0>(entity_model_tuple) + glm::vec3(0, 0, 5);
        auto& view_ref = *view;
        if (view_ref.position != new_view_position)
        {
            view_ref.position = new_view_position;
            view_ref.update();
        }
        auto& projection = entity.template getData<std::shared_ptr<vp::Projection>>("Projection");
        std::function<void(Entity&, bool)> setSkip;
        setSkip = [&](auto& entity, bool skip) {
            for (auto& child : entity.children)
            {
                if (!skip)
                {
                    if (child.viewPointer.get() != view.get())
                        child.viewPointer = view;
                    if (child.projectionPointer.get() != projection.get())
                        child.projectionPointer = projection;
                }
                child.skipRender = skip;
                if (child.typeName != "Layout")
                    setSkip(child, skip);
            }
        };
        auto& scene = Registry::GetSingleton().getScene(entity.INDEX_STACK);
        auto& framebuffer = entity.template getData<std::shared_ptr<textures::Framebuffer>>("Framebuffer");
        auto& framebufferRef = *framebuffer;
        framebufferRef.bind();
        setSkip(entity, false);
        auto opaqueChildDrawList = entity.getOpaqueChildDrawList();
        auto transparentChildDrawList = entity.getTransparentChildDrawList();
        for (auto& drawPair : opaqueChildDrawList)
        {
            drawPair.second->render(*drawPair.first);
        }
        for (auto& drawPair : transparentChildDrawList)
        {
            drawPair.second->render(*drawPair.first);
        }
        framebufferRef.unbind();
        setSkip(entity, true);
    };
    return layout_info;
}