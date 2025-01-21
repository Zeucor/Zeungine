#include <anex/modules/gl/entities/Input.hpp>
#include <anex/utilities.hpp>
#include <iostream>
using namespace anex::modules::gl::entities;
Input::Input(GLWindow &window,
						 GLScene &scene,
						 glm::vec3 position,
						 glm::vec3 rotation,
						 glm::vec3 scale,
						 glm::vec4 backgroundColor,
						 fonts::freetype::FreetypeFont &font,
             float width,
						 float height,
						 const std::string& placeholderText,
						 float padding,
						 const shaders::RuntimeConstants &constants,
             std::string_view name):
	anex::modules::gl::GLEntity(
		window,
		anex::mergeVectors<std::string_view>({
			{
				"Color", "Position",
				"View", "Projection", "Model", "CameraPosition"
			}
		}, constants),
		6,
		{0, 1, 2,  2, 3, 0},
		4,
		{
			{ 0, -0, 0 }, { 0, -0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }
		},
		position,
		rotation,
		scale,
    name.empty() ? "Input " + std::to_string(++inputsCount) : name
	),
	scene(scene),
	size({ 0, 0 }),
	font(font),
 	width(width),
	height(height),
	NDCWidth((width / window.windowWidth) * 2),
	NDCHeight((height / window.windowHeight) * 2),
	fontSize(height / 1.2f),
	activeColor(std::clamp(backgroundColor[0] * 0.5f, 0.f, 1.f), std::clamp(backgroundColor[1] * 0.5f, 0.f, 1.f), std::clamp(backgroundColor[2] * 0.5f, 0.f, 1.f), backgroundColor[3]),
	inactiveColor(backgroundColor),
	padding(padding),
	NDCPadding(this->padding / window.windowWidth / 2)
{
	updateIndices(indices);
	colors.resize(4);
	setColor(backgroundColor);
	setSize({NDCWidth, NDCHeight});
	float LineHeight = 0;
	auto placeholderTextSize = font.stringSize(placeholderText, fontSize, LineHeight, {0, 0});
	placeholderTextSize.y /= window.windowHeight * 0.5f;
	placeholderTextSize.x /= window.windowWidth * 0.5f;
	placeholderTextView = std::make_shared<TextView>(
	    window,
	    scene,
	    glm::vec3(placeholderTextSize.x / 2 + NDCPadding, -NDCHeight / 2, 0.1f),
	    glm::vec3(0),
	    glm::vec3(1),
		placeholderText,
		placeholderTextSize,
		font,
		fontSize,
		true,
		[&](auto textSize)
		{
			auto &glWindow = ((VAO&)*this).window;
			return glm::vec3(textSize.x / 2 + NDCPadding / 2, -NDCHeight / 2, 0.1f);
		},
		[&](auto textViewSize)
		{
			auto &glWindow = ((VAO&)*this).window;
			glm::vec2 newSize(std::min(textViewSize.x, this->size.x - (NDCPadding)), textViewSize.y);
			return newSize;
		},
		[&]
		{
			auto &glWindow = ((VAO&)*this).window;
			return this->fontSize = ((this->NDCHeight * glWindow.windowHeight / 2) / 1.f);
		});
	placeholderTextView->addToBVH = false;
	auto textSize = font.stringSize(text, fontSize, LineHeight, {0, 0});
	textSize.y /= window.windowHeight * 0.5f;
	textSize.x /= window.windowWidth * 0.5f;
	textView = std::make_shared<TextView>(
		window,
		scene,
		glm::vec3(textSize.x / 2 + NDCPadding, -NDCHeight / 2, 0.1f),
		glm::vec3(0),
		glm::vec3(1),
		text,
		textSize,
		font,
		fontSize,
	    true,
	    [&](auto textSize)
	    {
			auto &glWindow = ((VAO&)*this).window;
			return glm::vec3(textSize.x / 2 + NDCPadding / 2, -NDCHeight / 2, 0.1f);
	    },
	    [&](auto textViewSize)
	    {
			auto &glWindow = ((VAO&)*this).window;
	    	glm::vec2 newSize(std::min(textViewSize.x, this->size.x - (NDCPadding)), textViewSize.y);
			return newSize;
	    },
	    [&]
	    {
			auto &glWindow = ((VAO&)*this).window;
			return this->fontSize = ((this->NDCHeight * glWindow.windowHeight / 2) / 1.f);
	    }
    );
    textView->cursorIndex = 0;
	textView->addToBVH = false;
	showTextView(placeholderTextView);
    cursorPlane = std::make_shared<Plane>(
        window,
        scene,
        glm::vec3(0, -NDCHeight / 2, 0.1),
        glm::vec3(0),
        glm::vec3(1),
        glm::vec2(0.002, NDCHeight),
        glm::vec4(1)
    );
    cursorPlane->addToBVH = false;
	mouseHoverID = addMouseHoverHandler([&, backgroundColor](const auto &hovered)
	{
		this->hovered = hovered;
		if (hovered)
		{
			setColor(this->active ? this->activeColor : glm::vec4( 0.4, 0.4, 0.4, 1 ));
		}
		else
		{
			setColor(this->active ? this->activeColor : inactiveColor);
		}
	});
	mousePressID = addMousePressHandler(0, [&](auto pressed)mutable
	{
		if (!pressed && !this->active)
  		{
			this->active = true;
			setColor(this->activeColor);
			if (activeInput && activeInput != this)
			{
				activeInput->active = false;
				activeInput->setColor(activeInput->inactiveColor);
			}
			activeInput = this;
		}
	});
	anyKeyPressID = window.addAnyKeyPressHandler([&](auto key, auto pressed)
	{
		if (!this->active)
			return;
		if (!pressed)
    		return;
        auto &textViewRef = *textView;
		if (!std::isprint(key))
		{
			switch (key)
			{
			case 8:
                if (textViewRef.cursorIndex > 0)
                {
					--textViewRef.cursorIndex;
					text.erase(text.begin() + textViewRef.cursorIndex);
					if (text.empty())
                    {
                        goto _showPlaceholder;
                    }
					goto _forceUpdateTextView;
                }
				return;
			case 20:
                if (textViewRef.cursorIndex > 0)
                {
					textViewRef.cursorIndex--;
					goto _forceUpdateTextView;
                }
                return;
            case 19:
	            if (textViewRef.cursorIndex < text.size())
	            {
					textViewRef.cursorIndex++;
					goto _forceUpdateTextView;
	            }
				return;
			case 127:
				if (textViewRef.cursorIndex < text.size())
				{
					text.erase(text.begin() + textViewRef.cursorIndex);
					if (text.empty())
                    {
                        goto _showPlaceholder;
                    }
					goto _forceUpdateTextView;
				}
				return;
			case KEYCODE_HOME:
				{
					textViewRef.cursorIndex = 0;
					goto _forceUpdateTextView;
				};
			case KEYCODE_END:
				{
					textViewRef.cursorIndex = text.size();
					goto _forceUpdateTextView;
				};
			default:
    			return;
			}
		}
        {
		    auto shiftPressed = window.mod & 2;
		    text.insert(text.begin() + textViewRef.cursorIndex, getShiftedChar(key, shiftPressed));
			++textViewRef.cursorIndex;
        }
	    if (text.empty())
	    {
_showPlaceholder:
			showTextView(placeholderTextView);
			removeChild(cursorPlane->ID);
	    }
	    else
	    {
			textViewRef.updateText(text);
            goto _updateTextView;
_forceUpdateTextView:
			textViewRef.updateText(text);
			textViewRef.forceUpdate();
            goto _updateCursor;
_updateTextView:
			textViewRef.forceUpdate();
_updateCursor:
			auto &cursorPosition = textViewRef.cursorPosition;
			auto &glWindow = ((VAO&)*this).window;
			auto &cursorPlanePosition = cursorPlane->position;
			auto cursorPositionXBeforeNDC = cursorPosition.x;
			auto textViewSizeBeforeNDC = glm::vec2(textViewRef.actualSizeBeforeNDC);
			auto inputSizeBeforeNDC = glm::vec2(size.x * glWindow.windowWidth / 2, size.y * glWindow.windowHeight / 2);
			float inputWidthNDCMinusPadding = size.x - (NDCPadding);
			auto inputSizeBeforeNDCMinusPadding = glm::vec2(inputWidthNDCMinusPadding * (glWindow.windowWidth / 2), size.y * glWindow.windowHeight / 2);
			float textWidthNDC = textViewRef.actualSize.x;
			float cursorTextCoord = cursorPosition.x;
	    	if (textWidthNDC > inputWidthNDCMinusPadding)
	    	{
			    float halfVisibleWidthNDC = inputWidthNDCMinusPadding / 2.0f;
				float visibleStartNDC = glm::clamp(
					(cursorTextCoord / textViewRef.textSize.x) - (halfVisibleWidthNDC / textWidthNDC),
					0.0f,
					1.0f - (inputWidthNDCMinusPadding / textWidthNDC)
				);
				float normalizedCursor = glm::clamp(
					(cursorTextCoord / textViewRef.textSize.x - visibleStartNDC) / (inputWidthNDCMinusPadding / textWidthNDC),
					0.0f,
					1.0f
				);
				cursorPlanePosition.x = (normalizedCursor * inputWidthNDCMinusPadding) + (NDCPadding / 2);
			}
			else
			{
				cursorPlanePosition.x = (cursorTextCoord / textViewRef.textSize.x) * textWidthNDC + (NDCPadding / 2);
			}
			cursorPlanePosition.x = glm::clamp(cursorPlanePosition.x, 0.0f, inputWidthNDCMinusPadding + (NDCPadding / 2));
			if (!cursorPlane->ID)
			{
			    addChild(cursorPlane);
			}
			auto &uvs = textViewRef.uvs;
			if (textViewSizeBeforeNDC.x <= inputSizeBeforeNDCMinusPadding.x)
			{
			    uvs[0].x = 0.0f;
			    uvs[1].x = 1.0f;
			    uvs[2].x = 1.0f;
			    uvs[3].x = 0.0f;
			}
			else
			{
				float halfVisibleWidth = (int32_t)(inputSizeBeforeNDCMinusPadding.x / 2.0f);
				float visibleRegionStartNDC = glm::clamp(
					cursorPositionXBeforeNDC - halfVisibleWidth,
					0.0f,
					(float)textViewSizeBeforeNDC.x - (float)inputSizeBeforeNDCMinusPadding.x
				);
				float visibleRegionEndNDC = visibleRegionStartNDC + inputSizeBeforeNDCMinusPadding.x;
				float startNormalized = visibleRegionStartNDC / textViewSizeBeforeNDC.x;
				float endNormalized = visibleRegionEndNDC / textViewSizeBeforeNDC.x;
				startNormalized = glm::clamp(startNormalized, 0.0f, 1.0f);
				endNormalized = glm::clamp(endNormalized, 0.0f, 1.0f);
				uvs[0].x = startNormalized;
				uvs[1].x = endNormalized;
				uvs[2].x = endNormalized;
				uvs[3].x = startNormalized;
			}
 			textViewRef.updateElements("UV2", uvs);
			showTextView(textView);
	    }
	});
};
Input::~Input()
{
	removeMouseHoverHandler(mouseHoverID);
	removeMousePressHandler(0, mousePressID);
};
void Input::preRender()
{
	const auto &model = getModelMatrix();
	shader.bind();
	scene.entityPreRender(*this);
	shader.setBlock("Model", model);
	shader.setBlock("View", scene.view.matrix);
	shader.setBlock("Projection", scene.projection.matrix);
	shader.setBlock("CameraPosition", scene.view.position, 16);
	shader.unbind();
};
void Input::setColor(glm::vec4 color)
{
  auto colorsData = colors.data();
	colorsData[0] = color;
  colorsData[1] = color;
  colorsData[2] = color;
  colorsData[3] = color;
	updateElements("Color", colors);
};
void Input::setSize(glm::vec2 newSize)
{
	positions = {
		{ 0, -newSize.y, 0 }, { newSize.x, -newSize.y, 0 }, { newSize.x, 0, 0 }, { 0, 0, 0 }
	};
	updateElements("Position", positions);
	this->size = newSize;
};
void Input::showTextView(const std::shared_ptr<TextView>& showTextView)
{
  if (activeTextView && activeTextView != showTextView)
  {
    removeChild(activeTextView->ID);
  }
  else if (activeTextView)
  {
    return;
  }
  activeTextView = showTextView;
  addChild(activeTextView);
};
char Input::getShiftedChar(const char &key, bool shiftPressed)
{
	static const std::unordered_map<char, char> shiftMap = {
		{'1', '!'}, {'2', '@'}, {'3', '#'}, {'4', '$'}, {'5', '%'},
		{'6', '^'}, {'7', '&'}, {'8', '*'}, {'9', '('}, {'0', ')'},
		{'-', '_'}, {'=', '+'}, {'[', '{'}, {']', '}'}, {'\\', '|'},
		{';', ':'}, {'\'', '"'}, {',', '<'}, {'.', '>'}, {'/', '?'},
    {'`', '~'}
	};
	if(std::isalpha(key))
	{
		if(shiftPressed)
		{
			return std::toupper(key);
		}
		else
		{
			return std::tolower(key);
		}
	}
	if(shiftPressed)
	{
		auto it = shiftMap.find(key);
		if(it != shiftMap.end())
		{
			return it->second;
		}
	}
	return key;
};