#include <zg/entities/ui/Layout.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/textures/Framebuffer.hpp>
#include <zg/Registry.hpp>
#include <zg/Scene.hpp>
#include <zg/interfaces/IPlatformWindow.hpp>
#include <zg/textures/Framebuffer.hpp>
#include <zg/Window.hpp>
using namespace zg::entities::ui;
using namespace zg;
void layout_ensure_framebuffer(Window& window, Entity& entity, glm::vec3 size, bool isNDCSize)
{
    if (!size.x || !size.y)
        return;
    glm::ivec3 isize(size);
    auto& color_texture = entity.template getData<std::shared_ptr<textures::Texture>>("ColorTexture");
    if (color_texture && color_texture->size.x == isize.x && color_texture->size.y == isize.y)
        return;
    glm::vec2 texSize(
        std::ceil(isNDCSize ? (size.x * ((const float&)window.windowWidth / 2.f)) : size.x),
        std::ceil(isNDCSize ? (size.y * ((const float&)window.windowHeight / 2.f)) : size.y)
    );
    color_texture = std::make_shared<textures::Texture>(
        window.iRenderer,
        glm::ivec4(texSize.x, texSize.y, 1, 0),
        (const void*)0,
        DEFAULT_TEXTURE_FORMAT,
        DEFAULT_TEXTURE_TYPE,
        textures::Texture::Nearest,
        true,
        DEFAULT_TEXTURE_MULTISAMPLING,
        TEXTURE_CLAMP_EDGE
    );
    std::vector<textures::Framebuffer::TextureAttachmentPair> textureAPair({
        {color_texture, textures::Framebuffer::AttachmentType::Color}
    });
    auto& blendState = entity.template getData<textures::BlendState>("BlendState");
    auto framebuffer = std::make_shared<textures::Framebuffer>(window.iRenderer, textureAPair, blendState);
    auto entity_model_tuple = entity.decomposeModel();
    auto view = std::make_shared<vp::View>(std::get<0>(entity_model_tuple) + glm::vec3(0, 0, 5), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    auto projection = std::make_shared<vp::Projection>(window, texSize);
    entity.template make<std::shared_ptr<textures::Framebuffer>>("Framebuffer", framebuffer);
    entity.template make<std::shared_ptr<vp::View>>("View", view);
    entity.template make<std::shared_ptr<vp::Projection>>("Projection", projection);
}

glm::vec3 zg::entities::ui::LayoutGetSubPositionI(Entity& entity, glm::vec3 size, size_t index)
{
    auto entitySize = entity.template getData<bool>("Resizable") ?
        std::any_cast<glm::vec3>(entity.template setData<bool>("GetDimensionSize", true)) :
        entity.getBoundingSize();
    auto layoutDimension = entity.template getData<LayoutDimension>("LayoutDimension");
    glm::vec3 position(-(entitySize.x / 2.f), (entitySize.y / 2.f), -(entitySize.z / 2.f));
    size_t count = 0;
    for (auto& child : entity.children)
    {
        if (count >= index)
            break;
        ++count;
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
        // if LayoutAlignment == Center
        // position.y -= (size.y / 2.f);
        position.y -= (entitySize.y / 2.f);
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
};        
EntityCreateInfo zg::entities::ui::LayoutFactory(const std::string& name, LayoutDimension layoutDimension, glm::vec3 position, glm::vec3 size, IRenderer* irenderer, bool isNDCSize, const textures::BlendState& blendState)
{
    auto& window = *irenderer->platformWindowPointer->renderWindowPointer;
    auto color_texture = std::make_shared<textures::Texture>();
    auto layout_info = entities::PlaneFactory(color_texture, name + "Plane", position, rotate_identity, size);
    layout_info.typeName = "Layout";
    layout_info.dataMap = {
        {"ColorTexture", color_texture},
        {"oldOpaqueHash", (size_t)0},
        {"oldTransparentHash", (size_t)0},
        {"LayoutDimension", layoutDimension},
        {"BlendState", blendState}
    };
    layout_info.setDataFunctionMap = {
        {"GetSubSize", [](const auto& nAny, auto& entity) -> std::any {
            return entity.getBoundingSize() / std::any_cast<const glm::vec3&>(nAny);
        } },
        { "Relayout", [](auto& boolAny, auto& entity) -> std::any {
            auto& bool_val = std::any_cast<const bool&>(boolAny);
            if (!bool_val)
                return false;
            size_t index = 0;
            std::function<void(Entity&)> relayout_position_children_fn;
            relayout_position_children_fn = [&](auto& entity) {
                for (auto& child : entity.children)
                {
                    if (child.typeName == "Layout")
                    {
                        relayout_position_children_fn(child);

                    }
                    child.position = LayoutGetSubPositionI(entity, child.getBoundingSize(), index++);
                }
            };
            relayout_position_children_fn(entity);
            return true;
        } },
        { "EnsureFramebuffer", [isNDCSize](const auto& boolAny, auto& entity) -> std::any {
            auto& bool_val = std::any_cast<const bool&>(boolAny);
            if (!bool_val)
                return false;
            auto& window = Registry::GetSingleton().getWindow(entity.INDEX_STACK);
            auto size = entity.getBoundingSize();
            layout_ensure_framebuffer(window, entity, size, isNDCSize);
            return true;
        } },
        { "GetDimensionSize", [](const auto& boolAny, auto& entity) -> std::any {
            glm::vec3 total_size(0);
            auto& bool_val = std::any_cast<const bool&>(boolAny);
            if (!bool_val)
                return total_size;
            auto& dimension = entity.template getData<LayoutDimension>("LayoutDimension");
            for (auto& child : entity.children)
            {
                auto c_size = (child.typeName != "Layout") ?
                    child.getBoundingSize() :
                    (child.template getData<bool>("Resizable") ?
                        std::any_cast<glm::vec3>(child.template setData<bool>("GetDimensionSize", true)) :
                        child.getBoundingSize()
                    );
                switch (dimension)
                {
                case LayoutDimension::Horizontal:
                    total_size.x += c_size.x;
                    total_size.y = (std::max)(total_size.y, c_size.y);
                    total_size.z = (std::max)(total_size.z, c_size.z);
                    break;
                case LayoutDimension::Vertical:
                    total_size.x = (std::max)(total_size.x, c_size.x);
                    total_size.y += c_size.y;
                    total_size.z = (std::max)(total_size.z, c_size.z);
                    break;
                case LayoutDimension::Depth:
                    total_size.x = (std::max)(total_size.x, c_size.x);
                    total_size.y = (std::max)(total_size.y, c_size.y);
                    total_size.z += c_size.z;
                    break;
                }
            }
            return glm::ceil(total_size);
        } },
        { "FitLayoutToChildren", [](const auto& boolAny, auto& entity) -> std::any {
            auto& bool_val = std::any_cast<const bool&>(boolAny);
            if (!bool_val)
                return false;
            entity.scale = entity.template getData<bool>("Resizable") ?
                std::any_cast<glm::vec3>(entity.template setData<bool>("GetDimensionSize", true)) :
                *entity.scale;
            Entity* parent_entity = 0, *last_parent_entity = 0;
            size_t n_parent = 1;
            auto& rgy = Registry::GetSingleton();
            if (!rgy.getNthParentEntity(entity.INDEX_STACK, parent_entity, n_parent))
                return true;
            // last_parent_entity = parent_entity;
            entity.template setData<bool>("EnsureFramebuffer", true);
            size_t qi = 0;
            for (auto& child : entity.children)
            {
                child.position = LayoutGetSubPositionI(entity, child.getBoundingSize(), qi++);
                continue;
            }
            auto qEntityIndex = entity.template getData<size_t>("qChildIndex");
            auto entity_boundingSize = entity.getBoundingSize();
            auto& parent_entity_ref = *parent_entity;
            if (parent_entity_ref.typeName != "Layout")
                return true;
            parent_entity_ref.template setData<bool>("FitLayoutToChildren", true);
            return true;
        } }
    };
    layout_info.onAddedFunction = [size, isNDCSize](auto& entity) mutable {
        auto& window = Registry::GetSingleton().getWindow(entity.INDEX_STACK);
        layout_ensure_framebuffer(window, entity, size, isNDCSize);
    };
    layout_info.preUpdateFunction = [layoutDimension](auto& entity) {
        bool anyChildrenDirty = false;
        std::function<bool(Entity&)> setAnyChildrenDirty;
        setAnyChildrenDirty = [&](auto& entity) -> bool {
            for (auto& child : entity.children)
            {
                if (*child.isDirty)
                {
                    anyChildrenDirty = true;
                    return true;
                }
                if (setAnyChildrenDirty(child))
                    return true;
            }
            return false;
        };
        if (entity.children.size() && !setAnyChildrenDirty(entity))
            return;
		auto entity_model_tuple = entity.decomposeModel();
        auto& view = entity.template getData<std::shared_ptr<vp::View>>("View");
        if (!view)
            return;
        auto new_view_position = std::get<0>(entity_model_tuple) + glm::vec3(0, 0, 5);
        auto& view_ref = *view;
        if (view_ref.position != new_view_position)
        {
            view_ref.position = new_view_position;
            view_ref.update();
        }
        auto& projection = entity.template getData<std::shared_ptr<vp::Projection>>("Projection");
        if (!projection)
            return;
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
        if (!framebuffer)
            return;
        auto& framebufferRef = *framebuffer;
        framebufferRef.bind();
        // try
        // {
        //     if (!entity.template getData<std::shared_ptr<textures::Texture>>("ColorTexture"))
        //         throw "";
        // }
        // catch (...)
        // {
        //     entity.template make<std::shared_ptr<textures::Texture>>("ColorTexture", framebufferRef.getColorTexture());
        // }
        // if (framebufferRef.drawnOnce())
        entity.setTexture(0, 0, framebufferRef.getColorTexture());
        // else
        //     entity.setTexture(0, 0, entity.template getData<std::shared_ptr<textures::Texture>>("ColorTexture"));
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