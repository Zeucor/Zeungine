#pragma once
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/textures/BlendState.hpp>
#include <zg/Window.hpp>
namespace zg::entities::ui
{
    enum class LayoutDimension
    {
        Horizontal = 1,
        Vertical = 2,
        Depth = 4
    };
    EntityCreateInfo LayoutFactory(const std::string& name, LayoutDimension layoutDimension, glm::vec3 position, glm::vec3 size, IRenderer* irenderer, bool isNDCSize = true, const textures::BlendState& blendState = textures::BlendState::Layout);
    glm::vec3 LayoutGetSubPositionI(Entity& entity, glm::vec3 size, size_t index);
    template <typename HostT>
    struct LayoutBuilder
    {
    private:
        std::vector<size_t> CURRENT_ID_STACK;
        std::unordered_map<std::string, zg::observable_ptr<glm::vec3>> keySizes;
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
        LayoutBuilder& add_layout
        (
            const std::string& name,
            LayoutDimension dimension, glm::vec3 size,
            const textures::BlendState& blendState = textures::BlendState::Layout,
            bool resizable = false
        )
        {
            auto& window = Registry::GetSingleton().getWindow(host.INDEX_STACK);
            size_t qChildIndex = 0;
            if (CURRENT_ID_STACK.size() == 0)
            {
                auto layout_tuple = host.addEntity(
                    LayoutFactory(name, dimension, {size.x / 2.f, size.y / 2.f, 0}, size, window.iRenderer, isNDCSizing, blendState)
                );
                CURRENT_ID_STACK.push_back(
                    std::get<KEY_ID_VECTOR_ID_INDEX>(
                        layout_tuple
                    )
                );
                auto& layout = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(layout_tuple);
                layout.template make<size_t>("TextCount", 0);
                layout.template make<size_t>("qChildIndex", qChildIndex);
                layout.template make<bool>("Resizable", resizable);
                layout.template make<glm::vec3>("OriginalSize", size);
                keySizes.emplace(name, layout.scale);
            }
            else
            {
                bool isLayout = false;
                auto& entity = get_entity();
                glm::vec3 new_position(0);
                qChildIndex = entity.children.size();
                if (entity.typeName == "Layout")
                {
                    isLayout = true;
                    new_position = LayoutGetSubPositionI(entity, size, entity.children.size());
                }
                auto layout_tuple = entity.addEntity(LayoutFactory(name, dimension, new_position, size, window.iRenderer, isNDCSizing, blendState));
                CURRENT_ID_STACK.push_back(
                    std::get<KEY_ID_VECTOR_ID_INDEX>(
                        layout_tuple
                    )
                );
                auto& index = *std::get<KEY_ID_VECTOR_INDEX_INDEX>(layout_tuple);
                auto& layout = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(layout_tuple);
                layout.template make<size_t>("TextCount", 0);
                layout.template make<size_t>("qChildIndex", qChildIndex);
                layout.template make<bool>("Resizable", resizable);
                layout.template make<glm::vec3>("OriginalSize", size);
                keySizes.emplace(name, layout.scale);
                if (isLayout)
                {
                    // layout.scale.observe([entity_ID = entity.ID, index](auto& old_scale, auto& new_scale) {
                    //     auto& rgy = Registry::GetSingleton(); 
                    //     auto& entity = rgy.getEntity(entity_ID);
                    //     // entity.template setData<bool>("Relayout", true);
                    // });
                }
            }
            return *this;
        }
        template <typename F>
        LayoutBuilder& add_layout_with(
            const std::string& name,
            LayoutDimension dimension, glm::vec3 size,
            F f,
            const textures::BlendState& blendState = textures::BlendState::Layout,
            bool resizable = false)
        {
            add_layout(name, dimension, size, blendState, resizable);
            f(*this);
            if (resizable)
                fit_layout_to_children();
            return move_up_stack();
        }
        LayoutBuilder& set_position(glm::vec3 position)
        {
            auto& entity = get_entity();
            entity.position = position;
            return *this;
        }
        const glm::vec3& get_size(const std::string& key) const
        {
            auto size_iter = keySizes.find(key);
            if (size_iter == keySizes.end())
                throw std::runtime_error("Key not found");
            return *size_iter->second;
        }
        LayoutBuilder& add_size(const std::string& key, glm::vec3 size)
        {
            keySizes.emplace(key, new glm::vec3(size));
            return *this;
        }
        LayoutBuilder& add_panel(const std::string& name, glm::vec3 size, glm::vec4 color)
        {
            auto& entity = get_entity();
            glm::vec3 new_position(0);
            if (entity.typeName == "Layout")
            {
                new_position = LayoutGetSubPositionI(entity, size, entity.children.size());
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
            auto textMultiplier = 1.f;
            auto textSize = font.stringSize(text, textFontSize * textMultiplier, textLineHeight, {0, 0}, enums::EBreakStyle::None, false);
            auto textScaler = 1.f / textMultiplier;
            glm::vec3 textScale(textScaler, textScaler, 1.f);
            glm::vec2 scaledSize(textSize);
            if (isNDCSizing)
            {
                textScale = {(2.f / (const float&)window.windowWidth / 2.f) * textScaler, (2.f / (const float&)window.windowHeight / 2.f) * textScaler, 1.f};
                scaledSize = textSize * glm::vec2(textScale);
            }
            auto entity_size = entity.getBoundingSize();
            glm::vec3 new_position(-entity_size.x / 2.f, entity_size.y / 2.f, 0.2);
            if (entity.typeName == "Layout")
            {
                new_position = LayoutGetSubPositionI(entity, glm::vec3(scaledSize, 1.f), entity.children.size());
            }
            auto backing_entity_info = PlaneFactory(glm::vec4(0, 0, 0, 0), textKey + "Backing Plane", new_position, rotate_identity, glm::vec3(scaledSize, 1));
            auto& backing_entity = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(entity.addEntity(backing_entity_info));
            new_position = {0, 0, 0};
            new_position.x -= (scaledSize.x / 2.f);
            new_position.y -= (scaledSize.y / 2.f);
            new_position.y -= (font.fontHandlePointer->face->size->metrics.descender / 64.0f);
            font.stringToHost(text, new_position, rotate_identity,
										textScale, textFontSize * textMultiplier, textLineHeight, scaledSize + glm::vec2(0.001, 0.001),
												zg::enums::EBreakStyle::None, backing_entity,
												textGlyphIDs, textCursorIndex,
												textCursorID, false);
            return *this;
        }
        template <typename T, typename D>
        LayoutBuilder& add_observed_T(zg::observable_ptr<T, D>& observable, float fontSize = 42.f)
        {
            return add_observed_T_formatted([](auto& val) {
                return std::to_string(val);
            }, observable, fontSize);
        }
        template <typename T, typename D, typename F>
        LayoutBuilder& add_observed_T_formatted(F formatter, zg::observable_ptr<T, D>& observable, float fontSize = 42.f)
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
            auto& backing_entity_ID = entity.template make<size_t>(textKey + "BackingEntityID", 0);
            static constexpr auto textMultiplier = 1.f;
            auto backing_index = entity.children.size();
            auto observer_ID = observable.observe(
            [
                formatter,
                IDNDCSizing = this->isNDCSizing,
                host_ID = host.ID,
                entity_ID = entity.ID,
                backing_entity_ID, textKey,
                backing_index
            ]
            (const auto& old_value, const auto& new_value)
            {
                std::string text = formatter(new_value);
                auto& rgy = Registry::GetSingleton();
                auto& entity = rgy.getEntity(entity_ID);
                auto& window = rgy.getWindow(entity.INDEX_STACK);
                auto& textGlyphIDs = entity.template getData<std::vector<size_t>>(textKey + "GlyphIDs");
                auto& textCursorIndex = entity.template getData<int64_t>(textKey + "CursorIndex");
                auto& textCursorID = entity.template getData<size_t>(textKey + "CursorID");
                auto& font = *rgy.getT<HostT>(host_ID).template getData<std::shared_ptr<fonts::freetype::FreetypeFont>>("Font");
                auto& textFontSize = entity.template getData<float>(textKey + "FontSize");
                auto& textLineHeight = entity.template getData<float>(textKey + "LineHeight");
                auto& backing_entity_ID = entity.template getData<size_t>(textKey + "BackingEntityID");
                auto textSize = font.stringSize(text, textFontSize * textMultiplier, textLineHeight, {0, 0}, enums::EBreakStyle::None, false);
                auto textScaler = 1.f / textMultiplier;
                glm::vec3 textScale(textScaler, textScaler, 1.f);
                glm::vec2 scaledSize(textSize);
                if (IDNDCSizing)
                {
                    textScale = {(2.f / (const float&)window.windowWidth / 2.f) * textScaler, (2.f / (const float&)window.windowHeight / 2.f) * textScaler, 1.f};
                    scaledSize = textSize * glm::vec2(textScale);
                }
                auto entity_size = entity.getBoundingSize();
                glm::vec3 new_position(-entity_size.x / 2.f, entity_size.y / 2.f, 0.2);
                bool isLayout = false;
                bool just_created_backing_entity = false;
                if (!backing_entity_ID)
                {
                    auto backing_entity_info = PlaneFactory(glm::vec4(0, 0, 0, 0), textKey + "Backing Plane", new_position, rotate_identity, glm::vec3(scaledSize, 1));
                    auto backing_entity_tuple = entity.addEntity(backing_entity_info);
                    backing_entity_ID = std::get<KEY_ID_VECTOR_ID_INDEX>(backing_entity_tuple);
                    just_created_backing_entity = true;
                }
                auto& backing_entity = rgy.getEntity(backing_entity_ID);
                if (!just_created_backing_entity)
                {
                    backing_entity.scale = glm::vec3(scaledSize, 1);
                }
                entity.template setData<bool>("FitLayoutToChildren", true);
                new_position = {0, 0, 0};
                new_position.x -= (scaledSize.x / 2.f);
                new_position.y -= (scaledSize.y / 2.f);
                new_position.y -= (font.fontHandlePointer->face->size->metrics.descender / 64.0f);
                font.stringToHost(text, new_position, rotate_identity,
                                            textScale, textFontSize * textMultiplier, textLineHeight, scaledSize + glm::vec2(0.001, 0.001),
                                                    zg::enums::EBreakStyle::None, backing_entity,
                                                    textGlyphIDs, textCursorIndex,
                                                    textCursorID, false);
            }, true);
            entity.onRemoveFunctionMap[textKey + "Observer"] = [observer_ID, observable](auto& entity) mutable {
                observable.remove_observer(observer_ID);
            };
            return *this;
        }
        LayoutBuilder& fit_layout_to_children()
        {
            auto& entity = get_entity();
            if (entity.typeName != "Layout")
                throw std::runtime_error("Current entity is not a layout");
            entity.template setData<bool>("FitLayoutToChildren", true);
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
            window.registerHandler(EVENT_KEY_PRESS, [entity_ID = entity.ID, handler, original_position](auto& event) {
                auto& pressed = event.template castData<bool>();
                auto& rgy = Registry::GetSingleton();
                auto& entity = rgy.getEntity(entity_ID);
                auto& window = rgy.getWindow(entity.INDEX_STACK);
                handler(window, entity, pressed, original_position);
            });
            return *this;
        }
    };
}