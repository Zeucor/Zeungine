#include <zg/entities/SkyBox.hpp>
#include <zg/utilities.hpp>
using namespace zg::entities;
SkyBox::SkyBox(Window &window,
							 Scene &scene,
							 const std::vector<std::string_view> &texturePaths,
							 std::string_view name):
	Entity(window,
		{
			{
				"UV3", "Position", "TextureCube",
				"View", "Projection", "SkyBox"
			}
		},
		36,
		{
			0, 1, 2,  2, 3, 0,   // Front face
			6, 7, 4,  4, 5, 6,   // Back face
			8, 9, 10, 9, 11, 10,  // Left face
			14, 15, 12, 15, 13, 12, // Right face
			16, 17, 18, 17, 19, 18, // Top face
			20, 23, 22, 20, 21, 23  // Bottom face
		},
		24,
		{
			// Front face
			{ -1.0f, 1.0f, -1.0f },  // 0
			{ -1.0f, -1.0f, -1.0f },  // 1
			{  1.0f, -1.0f, -1.0f },  // 2
			{  1.0f, 1.0f, -1.0f },  // 3

			// Back face
			{ -1.0f, -1.0f, 1.0f },  // 4
			{ -1.0f, 1.0f, 1.0f },  // 5
			{  1.0f, 1.0f, 1.0f },  // 6
			{  1.0f, -1.0f, 1.0f },  // 7

			// Left face
			{ -1.0f, 1.0f, -1.0f },  // 8
			{ -1.0f, 1.0f, 1.0f },  // 9
			{ -1.0f, -1.0f, -1.0f },  // 10
			{ -1.0f, -1.0f, 1.0f },  // 11

			// Right face
			{  1.0f, 1.0f, -1.0f },  // 12
			{  1.0f, 1.0f, 1.0f },  // 13
			{  1.0f, -1.0f, -1.0f },  // 14
			{  1.0f, -1.0f, 1.0f },  // 15

			// Top face
			{ -1.0f, 1.0f, -1.0f },  // 16
			{  1.0f, 1.0f, -1.0f },  // 17
			{ -1.0f, 1.0f, 1.0f },  // 18
			{  1.0f, 1.0f, 1.0f },  // 19

			// Bottom face
			{ -1.0f, -1.0f, -1.0f },  // 20
			{ -1.0f, -1.0f, 1.0f },  // 21
			{  1.0f, -1.0f, -1.0f },  // 22
			{  1.0f, -1.0f, 1.0f },  // 23
		},
		{0, 0, 0},
		{0, 0, 0},
		{1, 1, 1},
		name.empty() ? "SkyBox " + std::to_string(++skyBoxesCount) : name
  ),
	uvs({
		// Front face
		{ -1.0f, -1.0f, -1.0f },  // 0
		{ -1.0f, 1.0f, -1.0f },  // 1
		{  1.0f, 1.0f, -1.0f },  // 2
		{  1.0f, -1.0f, -1.0f },  // 3

		// Back face
		{ -1.0f, 1.0f, 1.0f },  // 4
		{ -1.0f, -1.0f, 1.0f },  // 5
		{  1.0f, -1.0f, 1.0f },  // 6
		{  1.0f, 1.0f, 1.0f },  // 7

		// Left face
		{ -1.0f, -1.0f, -1.0f },  // 8
		{ -1.0f, -1.0f, 1.0f },  // 9
		{ -1.0f, 1.0f, -1.0f },  // 10
		{ -1.0f, 1.0f, 1.0f },  // 11

		// Right face
		{  1.0f, -1.0f, -1.0f },  // 12
		{  1.0f, -1.0f, 1.0f },  // 13
		{  1.0f, 1.0f, -1.0f },  // 14
		{  1.0f, 1.0f, 1.0f },  // 15

		// Top face
		{ -1.0f, 1.0f, 1.0f },  // 16
		{  1.0f, 1.0f, 1.0f },  // 17
		{ -1.0f, 1.0f, -1.0f },  // 18
		{  1.0f, 1.0f, -1.0f },  // 19

		// Bottom face
		{ -1.0f, -1.0f, 1.0f },  // 20
		{ -1.0f, -1.0f, -1.0f },  // 21
		{  1.0f, -1.0f, 1.0f },  // 22
		{  1.0f, -1.0f, -1.0f },  // 23
	}),
	texture(window, {0, 0, 0, 1}, texturePaths),
	scene(scene)
{
  affectedByShadows = false;
	updateIndices(indices);
	updateElements("UV3", uvs);
	updateElements("Position", positions);
};
void SkyBox::preRender()
{
#ifdef USE_GL
	auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(window.iRenderer);
	glRenderer.glContext->DepthFunc(GL_LEQUAL);
	GLcheck(glRenderer, "glDepthFunc");
	glRenderer.glContext->Disable(GL_DEPTH_TEST);
	GLcheck(glRenderer, "glDisable:GL_DEPTH_TEST");
	glRenderer.glContext->BlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);
	GLcheck(glRenderer, "glBlendFunc");
#endif
	shader.bind();
	scene.entityPreRender(*this);
	auto view = glm::mat4(glm::mat3(scene.view.matrix));
	shader.setBlock("View", view);
	shader.setBlock("Projection", scene.projection.matrix);
  shader.setTexture("TextureCube", texture, 0);
	shader.unbind();
};
void SkyBox::postRender()
{
#ifdef USE_GL
	auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(window.iRenderer);
	glRenderer.glContext->DepthFunc(GL_LESS);
	GLcheck(glRenderer, "glDepthFunc");
	glRenderer.glContext->Enable(GL_DEPTH_TEST);
	GLcheck(glRenderer, "glEnable:GL_DEPTH_TEST");
	glRenderer.glContext->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GLcheck(glRenderer, "glBlendFunc");
#endif
};