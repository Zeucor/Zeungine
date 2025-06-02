#include <iostream>
#include <stdexcept>
#include <zg/Logger.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/strings/Utf8Iterator.hpp>
#include <zg/fonts/FontContext.hpp>
#include <zg/fonts/ParseFontEscapes.hpp>
#include <zg/Escapes.hpp>
using namespace zg::fonts::freetype;
FT_Library FreetypeFont::freetypeLibrary;
bool FreetypeFont::freetypeLoaded = ([]()
									 {
	if (FT_Init_FreeType(&freetypeLibrary))
  {
    throw std::runtime_error("Failed to initialize freetype library");
  }
	return true; })();
struct ft_error
{
	int err;
	const char* str;
};
#undef __FTERRORS_H__
#define FT_ERRORDEF(e, v, s) {(e), (s)},
#define FT_ERROR_START_LIST
#define FT_ERROR_END_LIST {0, NULL}
static const struct ft_error ft_errors[] = {
#include FT_ERRORS_H
};
const char* ft_errorstring(int err)
{
	const struct ft_error* e;

	for (e = ft_errors; e->str != NULL; e++)
		if (e->err == err)
			return e->str;

	return "Unknown error";
};
void FreetypeFont::FT_PRINT_AND_THROW_ERROR(const FT_Error& error, const std::string& fontPath)
{
	if (error)
	{
		auto errorString = "Error loading font[" + fontPath + "]" + std::string(ft_errorstring(error));
		Logger::print(Logger::Error, errorString);
		throw std::runtime_error(errorString);
	}
};
FreetypeCharacter::FreetypeCharacter(IRenderer* iRenderer, const FreetypeFont& freeTypeFont, FT_UInt glyph_index,
																		 float fontSize, bool msdf):
	glyphIndex(glyph_index)
{
	auto& face = freeTypeFont.fontHandlePointer->face;
	FT_Set_Pixel_Sizes(face, 0, fontSize);
	auto loadCharCode = FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER | FT_RENDER_MODE_NORMAL | FT_LOAD_COLOR);
	if (loadCharCode)
	{
		throw std::runtime_error("Failed to load glyph: " + std::to_string(FT_Get_Char_Index(face, glyphIndex)));
	}
	size = {face->glyph->bitmap.width, face->glyph->bitmap.rows};
	bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top};
	advance = face->glyph->advance.x;
	auto& renderer = iRenderer->renderer;
	if (size.x == 0 || size.y == 0)
	{
		return;
	}
	if (msdf)
	{
		msdfgen::Shape shape;
		double advance = 0.0;
		if (msdfgen::loadGlyph(shape, (msdfgen::FontHandle*)(freeTypeFont.fontHandlePointer), msdfgen::GlyphIndex(glyphIndex),
													 msdfgen::FONT_SCALING_EM_NORMALIZED, &advance))
		{
			shape.normalize();
			msdfgen::edgeColoringSimple(shape, 3.);
			static constexpr auto msdf_size = 64;
			static constexpr auto msdf_width = msdf_size;
			static constexpr auto msdf_height = msdf_size;
			static constexpr auto autoFrame = true;
			static constexpr auto scaleSpecified = false;
			static constexpr auto scanlinePass = true;
			static constexpr auto explicitErrorCorrectionMode = false;
			msdfgen::FillRule fillRule = msdfgen::FILL_NONZERO;
			msdfgen::Range range(1);
			msdfgen::Range px_range(2);
			msdfgen::Vector2 translate;
			msdfgen::Vector2 scale = 1;
			enum
			{
				RANGE_UNIT,
				RANGE_PX
			} rangeMode = RANGE_PX;
			enum
			{
				NO_PREPROCESS,
				WINDING_PREPROCESS,
				FULL_PREPROCESS
			} geometryPreproc =
				(
#ifdef MSDFGEN_USE_SKIA
					FULL_PREPROCESS
#else
					NO_PREPROCESS
#endif
				);
			double avgScale = .5 * (scale.x + scale.y);
			msdfgen::Shape::Bounds bounds = {};
			if (autoFrame)
				bounds = shape.getBounds();
			if (autoFrame)
			{
				double l = bounds.l, b = bounds.b, r = bounds.r, t = bounds.t;
				msdfgen::Vector2 frame(msdf_width, msdf_height);
				if (!scaleSpecified)
				{
					if (rangeMode == RANGE_UNIT)
						l += range.lower, b += range.lower, r -= range.lower, t -= range.lower;
					else
						frame += 2 * px_range.lower;
				}
				if (l >= r || b >= t)
					l = 0, b = 0, r = 1, t = 1;
				if (frame.x <= 0 || frame.y <= 0)
					throw std::runtime_error("Cannot fit the specified pixel range.");
				msdfgen::Vector2 dims(r - l, t - b);
				if (scaleSpecified)
					translate = .5 * (frame / scale - dims) - msdfgen::Vector2(l, b);
				else
				{
					if (dims.x * frame.y < dims.y * frame.x)
					{
						translate.set(.5 * (frame.x / frame.y * dims.y - dims.x) - l, -b);
						scale = avgScale = frame.y / dims.y;
					}
					else
					{
						translate.set(-l, .5 * (frame.y / frame.x * dims.x - dims.y) - b);
						scale = avgScale = frame.x / dims.x;
					}
				}
				if (rangeMode == RANGE_PX && !scaleSpecified)
					translate -= px_range.lower / scale;
			}
			if (rangeMode == RANGE_PX)
				range = px_range / (std::min)(scale.x, scale.y);
			msdfgen::Bitmap<float, 4> mtsdf(msdf_width, msdf_height);
			msdfgen::SDFTransformation transformation(msdfgen::Projection(scale, translate), range);
			msdfgen::MSDFGeneratorConfig generatorConfig;
			generatorConfig.overlapSupport = geometryPreproc == NO_PREPROCESS;
			msdfgen::MSDFGeneratorConfig postErrorCorrectionConfig(generatorConfig);
			if (scanlinePass)
			{
				if (explicitErrorCorrectionMode &&
						generatorConfig.errorCorrection.distanceCheckMode != msdfgen::ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE)
				{
					const char* fallbackModeName = "unknown";
					switch (generatorConfig.errorCorrection.mode)
					{
					case msdfgen::ErrorCorrectionConfig::DISABLED:
						fallbackModeName = "disabled";
						break;
					case msdfgen::ErrorCorrectionConfig::INDISCRIMINATE:
						fallbackModeName = "distance-fast";
						break;
					case msdfgen::ErrorCorrectionConfig::EDGE_PRIORITY:
						fallbackModeName = "auto-fast";
						break;
					case msdfgen::ErrorCorrectionConfig::EDGE_ONLY:
						fallbackModeName = "edge-fast";
						break;
					}
					fprintf(stderr, "Selected error correction mode not compatible with scanline pass, falling back to %s.\n",
									fallbackModeName);
				}
				generatorConfig.errorCorrection.mode = msdfgen::ErrorCorrectionConfig::DISABLED;
				postErrorCorrectionConfig.errorCorrection.distanceCheckMode =
					msdfgen::ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE;
			}
			msdfgen::generateMTSDF(mtsdf, shape, transformation, generatorConfig);
			msdfgen::distanceSignCorrection(mtsdf, shape, transformation, fillRule);
			msdfgen::msdfErrorCorrection(mtsdf, shape, transformation, postErrorCorrectionConfig);
			auto bitmap_bytes = mtsdf.operator const float*();
			texturePointer.reset(new textures::Texture(
				iRenderer, {msdf_width, msdf_height, 1, 0}, (const void*)bitmap_bytes, textures::Texture::Format::RGBA32F,
				textures::Texture::Type::Float, textures::Texture::FilterType::Nearest, false,
				textures::Texture::Multisampling::x1, textures::Texture::AddressMode::ClampToEdge, false));
			double left   = (bounds.l + translate.x) * scale.x;
			double bottom = (bounds.b + translate.y) * scale.y;
			double right  = (bounds.r + translate.x) * scale.x;
			double top    = (bounds.t + translate.y) * scale.y;
			double uv_min_x = left / msdf_width;
			double uv_max_x = right / msdf_width;
			double uv_min_y = top / msdf_height;
			double uv_max_y = bottom / msdf_height;
			uv2s = {
				glm::vec2(uv_min_x, uv_min_y), glm::vec2(uv_max_x, uv_max_y), glm::vec2(uv_min_x, uv_max_y),
				glm::vec2(uv_max_x, uv_min_y), glm::vec2(uv_max_x, uv_max_y), glm::vec2(uv_min_x, uv_min_y),
			};
#ifdef MSDF_RENDER_TEST_GLYPH
			int testWidth = 1024;
			int testHeight = 1024;
			msdfgen::Bitmap<float, 4> render(testWidth, testHeight);
			msdfgen::renderSDF(render, mtsdf, avgScale * range);
			msdfgen::savePng(render, ("glyph-" + std::to_string(codepoint) + ".png").c_str());
#endif
		}
	}
	else
	{
		auto flipY = (renderer == RENDERER_VULKAN);
		texturePointer.reset(new textures::Texture(iRenderer, {size.x, size.y, 1, 0}, face->glyph->bitmap.buffer,
							textures::Texture::Format::R8, textures::Texture::Type::UnsignedByte,
							textures::Texture::FilterType::Nearest, false, DEFAULT_TEXTURE_MULTISAMPLING, TEXTURE_CLAMP_EDGE, flipY));
	}
};
FreetypeFont::FreetypeFont(IRenderer* iRenderer, interfaces::IFile& fontFile) :
		fontHandlePointer(new msdfgen::FontHandle), fontFileBytes(fontFile.toBytes()), iRenderer(iRenderer),
		fontPath(fontFile.filePath)
{
	auto fontFileSize = fontFile.size();
	auto& face = fontHandlePointer->face;
	FT_PRINT_AND_THROW_ERROR(FT_New_Memory_Face(freetypeLibrary, (uint8_t*)fontFileBytes.get(), fontFileSize, 0, &face),
													 fontPath.string());
	FT_PRINT_AND_THROW_ERROR(FT_Select_Charmap(face, FT_ENCODING_UNICODE), fontPath.string());
	hasKerning = FT_HAS_KERNING(face);
	hbFont = hb_ft_font_create(face, 0);
	if (!hbFont)
	{
		throw std::runtime_error("Error: Could not create HarfBuzz font from FreeType face.");
	}
};
FreetypeFont::~FreetypeFont()
{
	if (hbFont)
	{
		hb_font_destroy(hbFont);
	}
	FT_Done_Face(fontHandlePointer->face);
	delete fontHandlePointer;
}
std::pair<float, uint32_t> FreetypeFont::calculateSegmentWidth(const std::string_view segment, float fontSize) const
{
	auto& face = fontHandlePointer->face;
	if (segment.empty())
	{
		return {0.0f, 0};
	}
	if (FT_Set_Pixel_Sizes(face, 0, fontSize))
	{
		throw std::runtime_error("Warning: Could not set font pixel size for segment width calculation.");
	}

	hb_buffer_t* hb_buffer = hb_buffer_create();
	if (!hb_buffer)
	{
		throw std::runtime_error("Error: Could not create HarfBuzz buffer for segment.");
	}

	hb_buffer_add_utf8(hb_buffer, segment.data(), static_cast<int>(segment.length()), 0,
										 static_cast<int>(segment.length()));

	hb_buffer_set_direction(hb_buffer, HB_DIRECTION_LTR);
	hb_buffer_set_script(hb_buffer, HB_SCRIPT_LATIN);
	hb_buffer_set_language(hb_buffer, hb_language_from_string("en", -1));

	hb_shape(hbFont, hb_buffer, nullptr, 0);

	int total_advance_x = 0;
	unsigned int glyph_count;
	hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(hb_buffer, &glyph_count);

	for (unsigned int i = 0; i < glyph_count; ++i)
	{
		total_advance_x += glyph_pos[i].x_advance;
	}

	hb_buffer_destroy(hb_buffer);

	return {static_cast<float>(total_advance_x) / 64.0f, glyph_count};
}
float ftTextureScale = 1.f;
bool shorten_segment_to_escapes(auto& ctx_fn_map, auto& line_start_char_idx, auto& line_end_char_idx)
{
	for (size_t index = line_start_char_idx + 1; index < line_end_char_idx; ++index)
	{
		auto ctx_fn_iter = ctx_fn_map.find(index);
		if (ctx_fn_iter != ctx_fn_map.end())
		{
			line_end_char_idx = index;
			return true;
		}
	}
	return false;
}
const glm::vec2 FreetypeFont::stringSize(const std::string_view raw_string, float fontSize, float& lineHeight,
																				 glm::vec2 bounds, enums::EBreakStyle breakStyle, bool msdf)
{
    auto parsed_pair = fonts::parseFontEscapes(raw_string);
    auto& string = parsed_pair.first;
    auto& ctx_fn_map = parsed_pair.second;
    auto& face = fontHandlePointer->face;
    if (FT_Set_Pixel_Sizes(face, 0, fontSize))
    {
        throw std::runtime_error("Error: Could not set font pixel size to " + std::to_string(fontSize) + ".");
    }
    hb_ft_font_changed(hbFont);

    lineHeight = static_cast<float>(face->size->metrics.height) / 64.0f;
    if (lineHeight <= 0.0f)
    {
        lineHeight = fontSize * 1.2f;
    }

    glm::vec4 foreground_color(1);
    glm::vec4 background_color(0);
    glm::vec3 position(0);
    float start_line_x = position.x;
    FontContext ctx{
        fontSize,
        fontSize,
        lineHeight,
        position.x,
        position.y,
        start_line_x,
        foreground_color,
        background_color,
        foreground_color,
        background_color,
        [&](auto& ctx){
            if (FT_Set_Pixel_Sizes(face, 0, ctx.fontSize))
            {
                throw std::runtime_error("Error: Could not set font pixel size to " + std::to_string(fontSize) + ".");
            }
            hb_ft_font_changed(hbFont);
        }
    };

    if (auto esc_iter = ctx_fn_map.find(0); esc_iter != ctx_fn_map.end())
    {
        for (auto& esc : esc_iter->second)
        {
            esc.second(ctx);
        }
    }

    if (string.empty())
    {
        return glm::vec2(0.0f, 0.0f);
    }

    size_t codepoint_index = 0;
    size_t segment_start_char_idx = 0;
    float max_total_width = 0.0f;
    glm::vec2 size(0.0f, lineHeight);

    hb_buffer_t* buffer = hb_buffer_create();
	float line_size_x = 0.0;
    while (segment_start_char_idx < string.length())
    {
        size_t line_start_char_idx = segment_start_char_idx;
        size_t line_end_char_idx = string.length();

        bool shortened = shorten_segment_to_escapes(ctx_fn_map, line_start_char_idx, line_end_char_idx);

        auto current_line_text = string.substr(line_start_char_idx, line_end_char_idx - line_start_char_idx);

        if (auto er_ctx_fn_iter = ctx_fn_map.find(codepoint_index); er_ctx_fn_iter != ctx_fn_map.end())
        {
            for (auto& fnPair : er_ctx_fn_iter->second)
            {
                fnPair.second(ctx);
            }
            ctx_fn_map.erase(er_ctx_fn_iter);
        }

        if (breakStyle == enums::EBreakStyle::None || bounds.x <= 0)
        {
            auto [segment_width, glyph_count] = calculateSegmentWidth(current_line_text, ctx.fontSize);
            line_size_x += segment_width;
			size.x = (std::max)(size.x, line_size_x);
            segment_start_char_idx = line_end_char_idx;
            codepoint_index += glyph_count;
            continue;
        }

        float current_line_width = 0.0f;
        size_t current_str_pos = 0;

        while (current_str_pos < current_line_text.length())
        {
            size_t next_delim = current_line_text.find_first_of(" \t\n\r", current_str_pos);
            size_t word_end = (next_delim == std::string_view::npos) ? current_line_text.length() : next_delim;
            std::string_view word = current_line_text.substr(current_str_pos, word_end - current_str_pos);

            auto [word_width, word_glyph_count] = calculateSegmentWidth(word, ctx.fontSize);
			auto sum_glyph_count = word_glyph_count;
            bool has_newline = word.find_first_of("\n\r") != std::string_view::npos;

            if ((current_line_width + word_width > bounds.x && current_line_width > 0) || has_newline)
            {
                max_total_width = std::max(max_total_width, current_line_width);
                size.y += lineHeight;
				line_size_x = 0.0;
                current_line_width = 0.0f;
            }
            current_line_width += word_width;

            if (next_delim != std::string_view::npos)
            {
                size_t space_start = next_delim;
                size_t after_space = current_line_text.find_first_not_of(" \t\n\r", space_start);
                size_t space_end = (after_space == std::string_view::npos) ? current_line_text.length() : after_space;
                std::string_view space = current_line_text.substr(space_start, space_end - space_start);

                auto [space_width, space_glyph_count] = calculateSegmentWidth(space, ctx.fontSize);
				sum_glyph_count += space_glyph_count;
                bool space_newline = space.find_first_of("\n\r") != std::string_view::npos;

                if ((current_line_width + space_width > bounds.x && current_line_width > 0) || space_newline)
                {
                    max_total_width = std::max(max_total_width, current_line_width);
                    int newline_count = std::count(space.begin(), space.end(), '\n');
                    size.y += (newline_count > 0 ? newline_count : 1) * lineHeight;
					line_size_x = 0.0;
                    current_line_width = 0.0f;
                }
                current_line_width += space_width;
                current_str_pos = space_end;
            }
            else
            {
                current_str_pos = word_end;
            }

            codepoint_index += sum_glyph_count;
        }

        segment_start_char_idx = line_end_char_idx;
        size.x = std::max(size.x, current_line_width);
    }

    hb_buffer_destroy(buffer);

    return size;
}
template <typename HostT>
void FreetypeFont::stringToHost(const std::string_view raw_string, glm::vec3 position, glm::quat _rotation,
								glm::vec3 _scale, float fontSize, float& lineHeight, glm::vec2 bounds,
								enums::EBreakStyle breakStyle, HostT& host,
								std::vector<size_t>& existingAndUpdatedGlyphIDs, int64_t cursorIndex, size_t& cursor,
								bool msdf)
{
	auto [string, ctx_fn_map] = fonts::parseFontEscapes(raw_string);
	shaders::RuntimeConstants constants;
	if (msdf)
	{
		constants.push_back("MSDF");
	}
	else
	{
		constants.push_back("TextColor");
	}
	auto& rgy = Registry::GetSingleton();
	auto& scene = rgy.getScene(host.INDEX_STACK);
	auto& face = fontHandlePointer->face;
	if (FT_Set_Pixel_Sizes(face, 0, fontSize))
	{
		throw std::runtime_error("Error: Could not set font pixel size to " + std::to_string(fontSize) + ".");
	}
	hb_ft_font_changed(hbFont);

	lineHeight = static_cast<float>(face->size->metrics.height) / 64.0f;
	if (lineHeight <= 0.0f)
	{
		lineHeight = fontSize * 1.2f;
	}

	hb_buffer_t* hb_buffer = hb_buffer_create();
	if (!hb_buffer)
	{
		throw std::runtime_error("Error: Could not create HarfBuzz buffer.");
	}

	std::vector<size_t> new_glyph_entity_ids_for_string_indices(string.length(), 0);

	glm::vec4 foreground_color(1);
	glm::vec4 background_color(0);
	glm::vec3 current_pen_position = position;
	float start_line_x = position.x;
	FontContext ctx{
        fontSize,
		fontSize,
        lineHeight,
        position.x,
        position.y,
		start_line_x,
        foreground_color,
        background_color,
		foreground_color,
        background_color,
		[&](auto& ctx){
			if (FT_Set_Pixel_Sizes(face, 0, ctx.fontSize))
			{
				throw std::runtime_error("Error: Could not set font pixel size to " + std::to_string(fontSize) + ".");
			}
			hb_ft_font_changed(hbFont);
		}
	};

	size_t segment_start_char_idx = 0;

	size_t codepoint_index = 0;

	while (segment_start_char_idx < string.length())
	{
		size_t line_start_char_idx = segment_start_char_idx;
		float current_line_width_for_wrap = 0.0f;
		size_t line_end_char_idx = line_start_char_idx;

		if (breakStyle == enums::EBreakStyle::None || bounds.x <= 0)
		{
			segment_start_char_idx = line_end_char_idx = string.length();
		}
		else
		{
			size_t temp_segment_char_idx = segment_start_char_idx;
			while (temp_segment_char_idx < string.length())
			{
				size_t next_delimiter_pos = string.find_first_of(" \t\n\r", temp_segment_char_idx);
				size_t word_end_char_pos = (next_delimiter_pos == std::string_view::npos) ? string.length() : next_delimiter_pos;

				std::string_view word_view = string.substr(temp_segment_char_idx, word_end_char_pos - temp_segment_char_idx);
				auto [word_width, word_glyph_count] = calculateSegmentWidth(word_view, fontSize);

				bool has_newline_in_word = (word_view.find('\n') != std::string_view::npos || word_view.find('\r') != std::string_view::npos);

				if ((current_line_width_for_wrap + word_width > bounds.x && current_line_width_for_wrap > 0) || has_newline_in_word)
				{
					break;
				}

				current_line_width_for_wrap += word_width;
				temp_segment_char_idx = word_end_char_pos;

				if (next_delimiter_pos != std::string_view::npos)
				{
					size_t space_start_char_pos = next_delimiter_pos;
					size_t non_space_after_space = string.find_first_not_of(" \t\n\r", space_start_char_pos);
					size_t space_end_char_pos = (non_space_after_space == std::string_view::npos) ? string.length() : non_space_after_space;

					std::string_view space_view = string.substr(space_start_char_pos, space_end_char_pos - space_start_char_pos);
					auto [space_width, space_glyph_count] = calculateSegmentWidth(space_view, fontSize);

					bool has_newline_in_space = (space_view.find('\n') != std::string_view::npos || space_view.find('\r') != std::string_view::npos);

					if ((current_line_width_for_wrap + space_width > bounds.x && current_line_width_for_wrap > 0) || has_newline_in_space)
					{
						break;
					}
					current_line_width_for_wrap += space_width;
					temp_segment_char_idx = space_end_char_pos;
				}
				line_end_char_idx = temp_segment_char_idx;
			}
			segment_start_char_idx = line_end_char_idx;
		}

		bool dont_advance_line = shorten_segment_to_escapes(ctx_fn_map, line_start_char_idx, line_end_char_idx);
		
		auto er_ctx_fn_iter = ctx_fn_map.find(codepoint_index);
		if (er_ctx_fn_iter != ctx_fn_map.end())
		{
			for (auto& fnPair : er_ctx_fn_iter->second)
				fnPair.second(ctx);
			ctx_fn_map.erase(er_ctx_fn_iter);
		}
		segment_start_char_idx = line_end_char_idx;

		auto current_line_text = string.substr(line_start_char_idx, line_end_char_idx - line_start_char_idx);

		hb_buffer_clear_contents(hb_buffer);
		hb_buffer_add_utf8(hb_buffer, current_line_text.data(), static_cast<int>(current_line_text.length()), 0, static_cast<int>(current_line_text.length()));
		hb_buffer_set_direction(hb_buffer, HB_DIRECTION_LTR);
		hb_buffer_set_script(hb_buffer, HB_SCRIPT_LATIN);
		hb_buffer_set_language(hb_buffer, hb_language_from_string("en", -1));
		hb_shape(hbFont, hb_buffer, nullptr, 0);

		unsigned int glyph_count;
		hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(hb_buffer, &glyph_count);
		hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(hb_buffer, &glyph_count);

		for (unsigned int i = 0; i < glyph_count; ++i)
		{
			auto ctx_fn_iter = ctx_fn_map.find(codepoint_index);
			if (ctx_fn_iter != ctx_fn_map.end())
			{
				for (auto& fnPair : ctx_fn_iter->second)
					fnPair.second(ctx);
				ctx_fn_map.erase(ctx_fn_iter);
			}
			auto& c_glyph_info = glyph_info[i];
			FT_UInt glyph_index = c_glyph_info.codepoint;
			unsigned int cluster_in_line = glyph_info[i].cluster;
			unsigned int original_string_cluster_index = line_start_char_idx + cluster_in_line;

			FreetypeCharacter* characterPointer = &getCharacter(glyph_index, ctx.fontSize * ftTextureScale, msdf);
			
			float c_advance = static_cast<float>(characterPointer->advance >> 6);
			float x_offset = ceilf(static_cast<float>(glyph_pos[i].x_offset) / 64.0f);
			float y_offset = ceilf(static_cast<float>(glyph_pos[i].y_offset) / 64.0f);
			float x_advance = static_cast<float>(glyph_pos[i].x_advance) / 64.0f;

			if (characterPointer->texturePointer)
			{
				glm::vec3 characterPosition{0};
				
				characterPosition.x = ctx.x + (characterPointer->bearing.x * _scale.x);
				
				characterPosition.y = ctx.y - (characterPointer->size.y - characterPointer->bearing.y) * _scale.y;
				characterPosition.z = position.z;

				if (original_string_cluster_index < new_glyph_entity_ids_for_string_indices.size())
				{
					size_t entity_id_to_use = 0;
					if (original_string_cluster_index < existingAndUpdatedGlyphIDs.size())
					{
						entity_id_to_use = existingAndUpdatedGlyphIDs[original_string_cluster_index];
					}

					if (!entity_id_to_use)
					{
						auto glyphCreateInfo = entities::PlaneFactory(
							characterPointer->texturePointer, "Glyph_" + std::to_string(original_string_cluster_index),
							characterPosition, _rotation,
							glm::vec3(characterPointer->size, 1.f) * _scale, characterPointer->uv2s, constants,
							entities::PlaneType::XY_BottomLeft
						);
						if (!msdf)
						{
							glyphCreateInfo.runtimeConstantValueShaderSetters = {
								{ "TextColor", ValueSetterPair{
									zg::observable_ptr(new std::any(ctx.foreground_color)),
									[](auto& mesh, auto& shader, const auto& value)
									{
										shader.setBlock("TextColor", mesh, std::any_cast<const glm::vec4&>(value), sizeof(glm::vec4));
									}
								} }
							};
						}
						auto glyph_tuple = host.addEntity(glyphCreateInfo);
						entity_id_to_use = std::get<KEY_ID_VECTOR_ID_INDEX>(glyph_tuple);
						auto& glyph = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(glyph_tuple);
						glyph.VALUE = static_cast<uint64_t>(glyph_index);
					}
					else
					{
						auto& glyph = rgy.getEntity(entity_id_to_use);
						if (glyph.VALUE.has_value() && glyph.template getValue<uint64_t>() != static_cast<uint64_t>(glyph_index))
						{
							glyph.VALUE = static_cast<uint64_t>(glyph_index);
							glyph.setTexture(0, 0, characterPointer->texturePointer);
						}
						if (glyph.position != characterPosition)
						{
							glyph.position = characterPosition;
						}
						if (glyph.scale != glm::vec3(characterPointer->size, 1.f) * _scale)
						{
							glyph.scale = glm::vec3(characterPointer->size, 1.f) * _scale;
						}
						auto& glyph_color = std::any_cast<glm::vec4&>((*glyph.runtimeConstantValueShaderSetters["TextColor"].first));
						if (glyph_color != ctx.foreground_color)
							glyph_color = ctx.foreground_color;
					}
					new_glyph_entity_ids_for_string_indices[original_string_cluster_index] = entity_id_to_use;

					size_t next_cluster_original_index = string.length();
					for (unsigned int j = i + 1; j < glyph_count; ++j)
					{
						if (glyph_info[j].cluster != cluster_in_line)
						{
							next_cluster_original_index = line_start_char_idx + glyph_info[j].cluster;
							break;
						}
					}

					for (size_t char_idx_in_cluster = original_string_cluster_index + 1;
							char_idx_in_cluster < next_cluster_original_index && char_idx_in_cluster < string.length();
							++char_idx_in_cluster)
					{
						new_glyph_entity_ids_for_string_indices[char_idx_in_cluster] = 0;
					}
				}
			}
			ctx.x += (x_advance) * _scale.x;
			codepoint_index++;
		}

		if (dont_advance_line)
			continue;

		ctx.y -= lineHeight;
		ctx.x = start_line_x;
		codepoint_index++;

		if (line_end_char_idx < string.length())
		{
			std::string_view trailing_segment = string.substr(line_end_char_idx, segment_start_char_idx - line_end_char_idx);
			size_t newline_count = 0;
			for (char c : trailing_segment)
			{
				if (c == '\n') newline_count++;
			}
			if (newline_count > 0)
			{
				ctx.y -= (newline_count - 1) * lineHeight;
			}
		}
	}

	for (size_t i = 0; i < existingAndUpdatedGlyphIDs.size(); ++i)
	{
		size_t old_entity_id = existingAndUpdatedGlyphIDs[i];
		size_t new_entity_id = (i < new_glyph_entity_ids_for_string_indices.size()) ? new_glyph_entity_ids_for_string_indices[i] : 0;

		if (old_entity_id != 0 && old_entity_id != new_entity_id)
		{
			host.removeEntity(old_entity_id);
		}
	}
	for (size_t i = new_glyph_entity_ids_for_string_indices.size(); i < existingAndUpdatedGlyphIDs.size(); ++i)
	{
		if (existingAndUpdatedGlyphIDs[i] != 0)
		{
			host.removeEntity(existingAndUpdatedGlyphIDs[i]);
		}
	}
	
	existingAndUpdatedGlyphIDs = new_glyph_entity_ids_for_string_indices;

	hb_buffer_destroy(hb_buffer);
}
template void zg::fonts::freetype::FreetypeFont::stringToHost<zg::Scene>(const std::string_view, glm::vec3,
																		glm::quat, glm::vec3, float, float&, glm::vec2,
																		zg::enums::EBreakStyle, zg::Scene&,
																		std::vector<size_t>&, int64_t, size_t&, bool);
template void zg::fonts::freetype::FreetypeFont::stringToHost<zg::Entity>(const std::string_view, glm::vec3,
																		glm::quat, glm::vec3, float, float&,
																		glm::vec2, zg::enums::EBreakStyle,
																		zg::Entity&, std::vector<size_t>&, int64_t,
																		size_t&, bool);
FreetypeCharacter& FreetypeFont::getCharacter(FT_UInt glyph_index, float fontSize, bool msdf)
{
	auto& map = msdf ? glyphIndexFontSizeMSDFCharacters : glyphIndexFontSizeCharacters;
	auto& fontSizes = map[glyph_index];
	auto iter = fontSizes.find(fontSize);
	if (iter == fontSizes.end())
	{
		auto iter2 = fontSizes.insert({fontSize, {iRenderer, *this, glyph_index, fontSize, msdf}});
		return iter2.first->second;
	}
	return iter->second;
}