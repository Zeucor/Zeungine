#include <anex/modules/gl/entities/SkyBox.hpp>
#include <anex/utilities.hpp>
using namespace anex::modules::gl::entities;
SkyBox::SkyBox(GLWindow &window, GLScene &scene, const std::vector<std::string_view> &texturePaths):
	GLEntity(window,
		{
			{
				"UV3", "Position", "TextureCube",
				"View", "Projection", "SkyBox"
			}
		},
		36, // adjust this for 36 vertices
		24,
		{0, 0, 0},
		{0, 0, 0},
		{1, 1, 1}
  ),
  indices{
//  	// Front face
//  	0, 1, 2,  2, 3, 0,
//		// Back face
//		4, 5, 6,  6, 7, 4,
//		// Left face
//		8, 9, 10, 10, 11, 8,
//		// Right face
//		12, 13, 14, 14, 15, 12,
//		// Top face
//		16, 17, 18, 18, 19, 16,
//		// Bottom face
//  	20, 21, 22, 22, 23, 20
		0, 1, 2,  2, 3, 0,   // Front face
		6, 7, 4,  4, 5, 6,   // Back face
		8, 9, 10, 9, 11, 10,  // Left face
		14, 15, 12, 15, 13, 12, // Right face
		16, 17, 18, 17, 19, 18, // Top face
		20, 23, 22, 20, 21, 23  // Bottom face
  },
	positions({
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
	}),
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
	texture({0, 0, 0, 1}, texturePaths),
	scene(scene)
{
  affectedByShadows = false;
	updateIndices(indices);
	updateElements("UV3", uvs.data());
	updateElements("Position", positions.data());
};
void SkyBox::preRender()
{
	glDepthFunc(GL_LEQUAL);
//	glDisable(GL_CULL_FACE);
	shader.bind();
	scene.entityPreRender(*this);
	auto view = glm::mat4(glm::mat3(scene.view.matrix));
	shader.setBlock("View", view);
	shader.setBlock("Projection", scene.projection);
  shader.setTexture("TextureCube", texture, 0);
	shader.unbind();
};
void SkyBox::postRender()
{
//	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
};