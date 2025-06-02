#include <zg/textures/BlendState.hpp>
using namespace zg::textures;
BlendState BlendState::MainFramebuffer = {
    true,
    BlendFactor::One,
    BlendFactor::Zero,
    BlendFactor::One,
    BlendFactor::Zero
};
BlendState BlendState::Layout = {
    true,
    BlendFactor::One,
    BlendFactor::One,
    BlendFactor::One,
    BlendFactor::One
};
BlendState BlendState::Text = {
    true,
    BlendFactor::SrcAlpha,
    BlendFactor::OneMinusSrcAlpha,
    BlendFactor::One,
    BlendFactor::One
};
BlendState BlendState::SrcAlpha = {
    true,
    textures::BlendFactor::SrcAlpha,
    textures::BlendFactor::OneMinusSrcAlpha,
    textures::BlendFactor::SrcAlpha,
    textures::BlendFactor::OneMinusSrcAlpha
};