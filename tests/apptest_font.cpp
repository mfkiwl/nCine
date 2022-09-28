#include <ncine/config.h>

#include "apptest_font.h"
#include <ncine/Application.h>
#include <ncine/IAppEventHandler.h>
#include <ncine/TextNode.h>
#include "apptest_datapath.h"

#if NCINE_WITH_IMGUI
	#include <ncine/imgui.h>
	#include <nctl/CString.h>
	#include <nctl/StaticString.h>
	#include <ncine/FileSystem.h>
#endif

namespace {

#ifdef __ANDROID__
const char *Font1TextureFile = "DroidSans32_256_ETC2.ktx";
const char *Font2TextureFile = "NotoSerif-Regular32_256_ETC2.ktx";
const char *Font3TextureFile = "Roboto-Regular32_256_ETC2.ktx";
#else
const char *Font1TextureFile = "DroidSans32_256.png";
const char *Font2TextureFile = "NotoSerif-Regular32_256.png";
const char *Font3TextureFile = "Roboto-Regular32_256.png";
#endif
const char *Font1FntFile = "DroidSans32_256.fnt";
const char *Font2FntFile = "NotoSerif-Regular32_256.fnt";
const char *Font3FntFile = "Roboto-Regular32_256.fnt";

#if NCINE_WITH_IMGUI
const char *AlignmentLabels[] = { "Left", "Center", "Right" };
nctl::StaticString<128> auxString;
nctl::StaticString<256> comboString;
nctl::String textNodeString(nc::TextNode::DefaultStringLength);
bool showImGui = true;
#endif

}

nctl::UniquePtr<nc::IAppEventHandler> createAppEventHandler()
{
	return nctl::makeUnique<MyEventHandler>();
}

void MyEventHandler::onPreInit(nc::AppConfiguration &config)
{
	setDataPath(config);
}

void MyEventHandler::onInit()
{
	nc::SceneNode &rootNode = nc::theApplication().rootNode();

	fonts_.pushBack(nctl::makeUnique<nc::Font>((prefixDataPath("fonts", Font1FntFile)).data(),
	                                           (prefixDataPath("fonts", Font1TextureFile)).data()));
	fonts_.pushBack(nctl::makeUnique<nc::Font>((prefixDataPath("fonts", Font2FntFile)).data(),
	                                           (prefixDataPath("fonts", Font2TextureFile)).data()));
	fonts_.pushBack(nctl::makeUnique<nc::Font>((prefixDataPath("fonts", Font3FntFile)).data(),
	                                           (prefixDataPath("fonts", Font3TextureFile)).data()));

	const char testString[] = "WAY.P.ATAV";
	float textHeight = nc::theApplication().height() * 0.8f;
	const float screenWidth = nc::theApplication().width();
	texts_.pushBack(nctl::makeUnique<nc::TextNode>(&rootNode, fonts_[0].get()));
	texts_[0]->setScale(2.0f);
	texts_[0]->setString(testString);
	textHeight -= texts_[0]->height() * 0.5f;
	texts_[0]->setPosition(screenWidth * 0.5f, textHeight);
	texts_[0]->setColor(255, 0, 0, 255);

	texts_.pushBack(nctl::makeUnique<nc::TextNode>(&rootNode, fonts_[0].get()));
	texts_[1]->setScale(2.0f);
	texts_[1]->setString(testString);
	textHeight -= texts_[0]->height();
	texts_[1]->setPosition(screenWidth * 0.5f, textHeight);
	texts_[1]->enableKerning(false);
	texts_[1]->setColor(255, 0, 0, 128);

	texts_.pushBack(nctl::makeUnique<nc::TextNode>(&rootNode, fonts_[1].get()));
	texts_[2]->setScale(2.0f);
	texts_[2]->setString(testString);
	textHeight -= texts_[1]->height();
	texts_[2]->setPosition(screenWidth * 0.5f, textHeight);
	texts_[2]->setColor(0, 255, 0, 255);

	texts_.pushBack(nctl::makeUnique<nc::TextNode>(&rootNode, fonts_[1].get()));
	texts_[3]->setScale(2.0f);
	texts_[3]->setString(testString);
	textHeight -= texts_[2]->height();
	texts_[3]->setPosition(screenWidth * 0.5f, textHeight);
	texts_[3]->enableKerning(false);
	texts_[3]->setColor(0, 255, 0, 128);

	texts_.pushBack(nctl::makeUnique<nc::TextNode>(&rootNode, fonts_[2].get()));
	texts_[4]->setScale(2.0f);
	texts_[4]->setString(testString);
	textHeight -= texts_[3]->height();
	texts_[4]->setPosition(screenWidth * 0.5f, textHeight);
	texts_[4]->setColor(0, 0, 255, 255);

	texts_.pushBack(nctl::makeUnique<nc::TextNode>(&rootNode, fonts_[2].get()));
	texts_[5]->setScale(2.0f);
	texts_[5]->setString(testString);
	textHeight -= texts_[4]->height();
	texts_[5]->setPosition(screenWidth * 0.5f, textHeight);
	texts_[5]->enableKerning(false);
	texts_[5]->setColor(0, 0, 255, 128);

#if NCINE_WITH_IMGUI
	comboString.clear();
	for (unsigned int i = 0; i < NumFonts; i++)
	{
		comboString.formatAppend("%s", nc::fs::baseName(fonts_[i]->name()).data());
		comboString.setLength(comboString.length() + 1);
	}
	comboString.setLength(comboString.length() + 1);
	// Append a second '\0' to signal the end of the combo item list
	comboString[comboString.length() - 1] = '\0';
#endif
}

void MyEventHandler::onFrameStart()
{
#if NCINE_WITH_IMGUI
	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(50, 140), ImGuiCond_FirstUseEver);
	if (showImGui)
	{
		if (ImGui::Begin("apptest_font", &showImGui))
		{
			for (unsigned int i = 0; i < NumTexts; i++)
			{
				auxString.format("Text #%d", i);
				if (ImGui::TreeNode(auxString.data()))
				{
					const nc::Font *font = texts_[i]->font();
					if (font)
					{
						if (ImGui::TreeNode("Font information"))
						{
							ImGui::Text("Line height: %d", font->lineHeight());
							ImGui::Text("Base: %d", font->base());
							ImGui::Text("Texture size: %d x %d", font->textureSize().x, font->textureSize().y);
							ImGui::Text("Number of glyphs: %d", font->numGlyphs());
							ImGui::Text("Number of kerning pairs: %d", font->numKernings());

							ImGui::TreePop();
						}
					}

					int currentComboFont = -1;
					for (unsigned int i = 0; i < NumFonts; i++)
					{
						if (font == fonts_[i].get())
						{
							currentComboFont = i;
							break;
						}
					}
					if (ImGui::Combo("Font", &currentComboFont, comboString.data()))
						texts_[i]->setFont(fonts_[currentComboFont].get());

					char textBuffer[nc::TextNode::DefaultStringLength];
					nctl::strncpy(textBuffer, texts_[i]->string().data(), texts_[i]->string().length() + 1);
					if (ImGui::InputTextMultiline("Text string", textBuffer, nc::TextNode::DefaultStringLength, ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 3.0f)))
						texts_[i]->setString(textBuffer);

					int currentAlignment = static_cast<int>(texts_[i]->alignment());
					if (ImGui::Combo("Alignment", &currentAlignment, AlignmentLabels, IM_ARRAYSIZE(AlignmentLabels)))
						texts_[i]->setAlignment(static_cast<nc::TextNode::Alignment>(currentAlignment));

					nc::Colorf color(texts_[i]->color());
					if (ImGui::ColorEdit4("Color", color.data()))
						texts_[i]->setColor(color);

					bool withKerning = texts_[i]->withKerning();
					if (ImGui::Checkbox("Kerning", &withKerning))
						texts_[i]->enableKerning(withKerning);

					ImGui::TreePop();
				}
			}
		}
		ImGui::End();
	}
#endif
}

void MyEventHandler::onKeyReleased(const nc::KeyboardEvent &event)
{
	if (event.sym == nc::KeySym::ESCAPE)
		nc::theApplication().quit();
#if NCINE_WITH_IMGUI
	else if (event.mod & nc::KeyMod::CTRL && event.sym == nc::KeySym::H)
		showImGui = !showImGui;
#endif
}
