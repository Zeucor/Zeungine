#pragma once
#include <zg/Entity.hpp>
#include <zg/observable_ptr.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/textures/BlendState.hpp>
namespace zg::entities::ui
{
    enum class LayoutDimension
    {
        Horizontal = 1,
        Vertical = 2,
        Depth = 4
    };
    EntityCreateInfo LayoutFactory(const std::string& name, LayoutDimension layoutDimension, glm::vec3 position, glm::vec3 size, IRenderer* irenderer, bool isNDCSize = true, const textures::BlendState& blendState = textures::BlendState::Layout);
    template <typename HostT>
    struct LayoutBuilder
    {
    private:
        std::vector<size_t> CURRENT_ID_STACK;
        HostT& host;
        bool isNDCSizing;
        Entity& get_entity()
        {
            auto CURRENT_ID_STACK_size = CURRENT_ID_STACK.size();
            if (CURRENT_ID_STACK_size == 0)
            {
                throw std::runtime_error("No entities are on the stack!");
            }
            Entity* ptr_entity = &Registry::GetSingleton().getEntity(CURRENT_ID_STACK[0]);
            for (size_t j = 1; j < CURRENT_ID_STACK_size; ++j)
            {
                auto find_iter = ptr_entity->children.find_id(CURRENT_ID_STACK[j]);
                if (find_iter == ptr_entity->children.end())
                {
                    break;
                }
                ptr_entity = &find_iter.value();
            }
            return *ptr_entity;
        }
    public:
        LayoutBuilder(HostT& host, bool isNDCSizing = true):
            host(host),
            isNDCSizing(isNDCSizing)
        {};
        LayoutBuilder& add_layout(const std::string& name, LayoutDimension dimension, glm::vec3 size, const textures::BlendState& blendState = textures::BlendState::Layout)
        {
            auto& window = Registry::GetSingleton().getWindow(host.INDEX_STACK);
            if (CURRENT_ID_STACK.size() == 0)
            {
                auto layout_tuple = host.addEntity(
                    LayoutFactory(name, dimension, {0, 0, 0}, size, window.iRenderer, isNDCSizing, blendState)
                );
                CURRENT_ID_STACK.push_back(
                    std::get<KEY_ID_VECTOR_ID_INDEX>(
                        layout_tuple
                    )
                );
                auto& layout = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(layout_tuple);
                layout.template make<size_t>("TextCount", 0);
            }
            else
            {
                auto& entity = get_entity();
                glm::vec3 new_position(0);
                if (entity.typeName == "Layout")
                {
                    new_position = std::any_cast<glm::vec3>(entity.template setData<glm::vec3>("GetSubPosition", size));
                }
                auto layout_tuple = entity.addEntity(LayoutFactory(name, dimension, new_position, size, window.iRenderer, isNDCSizing, blendState));
                CURRENT_ID_STACK.push_back(
                    std::get<KEY_ID_VECTOR_ID_INDEX>(
                        layout_tuple
                    )
                );
                auto& layout = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(layout_tuple);
                layout.template make<size_t>("TextCount", 0);
            }
            return *this;
        }
        LayoutBuilder& set_position(glm::vec3 position)
        {
            auto& entity = get_entity();
            entity.position = position;
            return *this;
        }
        LayoutBuilder& add_panel(const std::string& name, glm::vec3 size, glm::vec4 color)
        {
            auto& entity = get_entity();
            glm::vec3 new_position(0);
            if (entity.typeName == "Layout")
            {
                new_position = std::any_cast<glm::vec3>(entity.template setData<glm::vec3>("GetSubPosition", size));
            }
            auto to_be_panel = entities::PlaneFactory(color, name, new_position, rotate_identity, size);
            auto panel_tuple = entity.addEntity(to_be_panel);
            auto panel_ID = std::get<KEY_ID_VECTOR_ID_INDEX>(panel_tuple);
            auto& panel = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(panel_tuple);
            panel.template make<bool>("Showing", true);
            panel.template make<float>("AnimationT", 0.f);
            panel.template make<std::shared_ptr<bool>>("AnimationRunning", std::shared_ptr<bool>{});
            panel.template make<size_t>("TextCount", 0);
            CURRENT_ID_STACK.push_back(panel_ID);
            return *this;
        };
        LayoutBuilder& add_text(const std::string& text, float fontSize = 42.f)
        {
            auto& entity = get_entity();
            auto& window = Registry::GetSingleton().getWindow(entity.INDEX_STACK);
            auto& textCount = entity.template getData<size_t>("TextCount");
            auto textKey = "Text[" + std::to_string(++textCount) + "]";
            auto& textGlyphIDs = entity.template make<std::vector<size_t>>(textKey + "GlyphIDs");
            auto& textCursorIndex = entity.template make<int64_t>(textKey + "CursorIndex");
            auto& textCursorID = entity.template make<size_t>(textKey + "CursorID");
            auto& font = *host.template getData<std::shared_ptr<fonts::freetype::FreetypeFont>>("Font");
            auto& textFontSize = entity.template make<float>(textKey + "FontSize", fontSize);
            auto& textLineHeight = entity.template make<float>(textKey + "LineHeight", 0.0f);
            auto textSize = font.stringSize(text, textFontSize * 2.f, textLineHeight, {0, 0});
            glm::vec3 textScale(0.5f, 0.5f, 1.f);
            glm::vec2 scaledSize(textSize);
            if (isNDCSizing)
            {
                textScale = {(2.f / window.windowWidth / 2.f) * 0.5f, (2.f / window.windowHeight / 2.f) * 0.5f, 1.f};
                scaledSize = textSize * glm::vec2(textScale);
            }
            auto entity_size = entity.getBoundingSize();
            glm::vec3 new_position(-entity_size.x / 2.f, entity_size.y / 2.f, 0.2);
            if (entity.typeName == "Layout")
            {
                new_position = std::any_cast<glm::vec3>(entity.template setData<glm::vec3>("GetSubPosition", glm::vec3(scaledSize, 1.f)));
            }
            auto backing_entity_info = PlaneFactory(glm::vec4(0, 0, 0, 0), textKey + "Backing Plane", new_position, rotate_identity, glm::vec3(scaledSize, 1));
            auto& backing_entity = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(entity.addEntity(backing_entity_info));
            new_position = {0, 0, 0};
            new_position.x -= (scaledSize.x / 2.f);
            new_position.y -= scaledSize.y / 2.f;
            font.stringToHost(text, new_position, {1,1,1,1}, rotate_identity,
										textScale, textFontSize * 2.f, textLineHeight, scaledSize + glm::vec2(0.001, 0.001),
												zg::enums::EBreakStyle::None, backing_entity,
												textGlyphIDs, textCursorIndex,
												textCursorID);
            return *this;
        }
        LayoutBuilder& add_bound_text(const zg::observable_ptr<std::string>& text_observable)
        {
            return *this;
        }
        LayoutBuilder& move_up_stack()
        {
            if (CURRENT_ID_STACK.size() == 0)
            {
                throw std::runtime_error("No entities are on the stack!");
            }
            CURRENT_ID_STACK.erase(CURRENT_ID_STACK.end() - 1);
            return *this;
        }
        LayoutBuilder& move_down_stack(size_t child_index)
        {
            CURRENT_ID_STACK.push_back(child_index);
            return *this;
        }
        LayoutBuilder& add_key_press_handler(Key key, const std::function<void(Window&, Entity&, bool, glm::vec3)>& handler)
        {
            auto& entity = get_entity();
            auto& window = Registry::GetSingleton().getWindow(entity.INDEX_STACK);
            auto original_position = entity.position;
            window.addKeyPressHandler(key, [entity_ID = entity.ID, handler, original_position](auto pressed) {
                auto& rgy = Registry::GetSingleton();
                auto& entity = rgy.getEntity(entity_ID);
                auto& window = rgy.getWindow(entity.INDEX_STACK);
                handler(window, entity, pressed, original_position);
            });
            return *this;
        }
    };
}