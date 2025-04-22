#include <zg/interfaces/IRenderer.hpp>
using namespace zg;
IRenderer::IRenderer()
{
    shaderContext = new ShaderContext;
}