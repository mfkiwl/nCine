﻿#include <ncine/imgui.h>

#include <nctl/algorithms.h> // for clamping values
#include "apptest_loading.h"
#include <ncine/Application.h>
#include <ncine/Texture.h>
#include <ncine/Sprite.h>
#include <ncine/AudioBuffer.h>
#include <ncine/AudioBufferPlayer.h>
#include <ncine/AudioStreamPlayer.h>
#include <ncine/TextNode.h>
#include <ncine/LuaStateManager.h>
#include <ncine/IFile.h>
#include <ncine/Colorf.h>
#include "apptest_datapath.h"

#define DEFAULT_CONSTRUCTORS (0)
#define LOADING_FAILURES (0)

namespace {

#ifdef __ANDROID__
const char *TextureFiles[MyEventHandler::NumTextures] = { "texture1_ETC2.ktx", "texture2_ETC2.ktx", "texture3_ETC2.ktx", "texture4_ETC2.ktx" };
#else
const char *TextureFiles[MyEventHandler::NumTextures] = { "texture1.png", "texture2.png", "texture3.png", "texture4.png" };
#endif
nctl::StaticArray<nctl::UniquePtr<uint8_t[]>, MyEventHandler::NumTextures> textureBuffers;
nctl::StaticArray<unsigned long int, MyEventHandler::NumTextures> textureBufferSizes;

const char *SoundFiles[MyEventHandler::NumSounds] = { "explode.wav", "waterdrop.wav", "coins.wav", "c64.ogg", "chiptune_loop.ogg" };
nctl::StaticArray<nctl::UniquePtr<uint8_t[]>, MyEventHandler::NumSounds> soundBuffers;
nctl::StaticArray<unsigned long int, MyEventHandler::NumSounds> soundBufferSizes;

const char *FontFiles[MyEventHandler::NumFonts] = { "DroidSans32_256.fnt", "NotoSans-Regular32_256.fnt", "NotoSerif-Regular32_256.fnt", "OpenSans-Regular32_256.fnt", "Roboto-Regular32_256.fnt" };
#ifdef __ANDROID__
const char *FontTexFiles[MyEventHandler::NumFonts] = { "DroidSans32_256_ETC2.ktx", "NotoSans-Regular32_256_ETC2.ktx", "NotoSerif-Regular32_256_ETC2.ktx", "OpenSans-Regular32_256_ETC2.ktx", "Roboto-Regular32_256_ETC2.ktx" };
#else
const char *FontTexFiles[MyEventHandler::NumFonts] = { "DroidSans32_256.png", "NotoSans-Regular32_256.png", "NotoSerif-Regular32_256.png", "OpenSans-Regular32_256.png", "Roboto-Regular32_256.png" };
#endif
nctl::StaticArray<nctl::UniquePtr<uint8_t[]>, MyEventHandler::NumFonts> fontBuffers;
nctl::StaticArray<unsigned long int, MyEventHandler::NumFonts> fontBufferSizes;

const char *ScriptFiles[MyEventHandler::NumScripts] = { "init.lua", "reload.lua", "script.lua" };
nctl::StaticArray<nctl::UniquePtr<char[]>, MyEventHandler::NumScripts> scriptBuffers;
nctl::StaticArray<unsigned long int, MyEventHandler::NumScripts> scriptBufferSizes;

#if LOADING_FAILURES
const unsigned long int randomBufferLength = 1024;
uint8_t randomBuffer[randomBufferLength];
#endif

nctl::String auxString(256);
int selectedTextureObject = -1;
int selectedTexture = -1;
nc::Colorf texelsColor;
nc::Recti texelsRegion;

int selectedSound = -1;
int sineWaveFrequency = 440;
float sineWaveDuration = 2.0f;

int selectedFont = -1;

int selectedScript = -1;
int loadedScript = -1;

const char *audioPlayerStateToString(nc::IAudioPlayer::PlayerState state)
{
	switch (state)
	{
		case nc::IAudioPlayer::PlayerState::INITIAL: return "Initial";
		case nc::IAudioPlayer::PlayerState::PLAYING: return "Playing";
		case nc::IAudioPlayer::PlayerState::PAUSED: return "Paused";
		case nc::IAudioPlayer::PlayerState::STOPPED: return "Stopped";
	}

	return "Unknown";
}

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

	for (unsigned int i = 0; i < NumTextures; i++)
	{
		nctl::UniquePtr<nc::IFile> textureFile = nc::IFile::createFileHandle(prefixDataPath("textures", TextureFiles[i]).data());
		textureFile->open(nc::IFile::OpenMode::READ);
		textureBufferSizes.pushBack(textureFile->size());
		textureBuffers.pushBack(nctl::makeUnique<uint8_t[]>(textureBufferSizes[i]));
		textureFile->read(textureBuffers[i].get(), textureBufferSizes[i]);
		textureFile->close();
	}

	for (unsigned int i = 0; i < NumSounds; i++)
	{
		nctl::UniquePtr<nc::IFile> soundFile = nc::IFile::createFileHandle(prefixDataPath("sounds", SoundFiles[i]).data());
		soundFile->open(nc::IFile::OpenMode::READ);
		soundBufferSizes.pushBack(soundFile->size());
		soundBuffers.pushBack(nctl::makeUnique<uint8_t[]>(soundBufferSizes[i]));
		soundFile->read(soundBuffers[i].get(), soundBufferSizes[i]);
		soundFile->close();
	}

	for (unsigned int i = 0; i < NumFonts; i++)
	{
		nctl::UniquePtr<nc::IFile> fontFile = nc::IFile::createFileHandle(prefixDataPath("fonts", FontFiles[i]).data());
		fontFile->open(nc::IFile::OpenMode::READ);
		fontBufferSizes.pushBack(fontFile->size());
		fontBuffers.pushBack(nctl::makeUnique<uint8_t[]>(fontBufferSizes[i]));
		fontFile->read(fontBuffers[i].get(), fontBufferSizes[i]);
		fontFile->close();
	}

	for (unsigned int i = 0; i < NumScripts; i++)
	{
		nctl::UniquePtr<nc::IFile> scriptFile = nc::IFile::createFileHandle(prefixDataPath("scripts", ScriptFiles[i]).data());
		scriptFile->open(nc::IFile::OpenMode::READ);
		scriptBufferSizes.pushBack(scriptFile->size());
		scriptBuffers.pushBack(nctl::makeUnique<char[]>(scriptBufferSizes[i]));
		scriptFile->read(scriptBuffers[i].get(), scriptBufferSizes[i]);
		scriptFile->close();
	}

#if DEFAULT_CONSTRUCTORS
	for (unsigned int i = 0; i < NumTextures; i++)
		textures_.pushBack(nctl::makeUnique<nc::Texture>());
	audioBuffer_ = nctl::makeUnique<nc::AudioBuffer>();
	streamPlayer_ = nctl::makeUnique<nc::AudioStreamPlayer>();
	font_ = nctl::makeUnique<nc::Font>();
#else
	for (unsigned int i = 0; i < NumTextures; i++)
		textures_.pushBack(nctl::makeUnique<nc::Texture>((prefixDataPath("textures", TextureFiles[i])).data()));
	audioBuffer_ = nctl::makeUnique<nc::AudioBuffer>((prefixDataPath("sounds", SoundFiles[0])).data());
	streamPlayer_ = nctl::makeUnique<nc::AudioStreamPlayer>((prefixDataPath("sounds", SoundFiles[3])).data());
	font_ = nctl::makeUnique<nc::Font>((prefixDataPath("fonts", FontFiles[0])).data());
#endif
	luaState_ = nctl::makeUnique<nc::LuaStateManager>(nc::LuaStateManager::ApiType::EDIT_ONLY,
	                                                  nc::LuaStateManager::StatisticsTracking::DISABLED,
	                                                  nc::LuaStateManager::StandardLibraries::NOT_LOADED);

#if LOADING_FAILURES
	// Loading from non-existent files
	for (unsigned int i = 0; i < NumTextures; i++)
		textures_[i]->loadFromFile("NonExistent.png");
	audioBuffer_->loadFromFile("NonExistent.wav");
	streamPlayer_->loadFromFile("NonExistent.ogg");
	font_->loadFromFile("NonExistent.fnt");
	luaState_->loadFromFile("NonExistent.lua");

	// Loading from uninitialized memory buffers
	for (unsigned int i = 0; i < NumTextures; i++)
		textures_[i]->loadFromMemory("NonExistent.png", randomBuffer, randomBufferLength);
	audioBuffer_->loadFromMemory("NonExistent.wav", randomBuffer, randomBufferLength);
	streamPlayer_->loadFromMemory("NonExistent.ogg", randomBuffer, randomBufferLength);
	font_->loadFromMemory("NonExistent.fnt", randomBuffer, randomBufferLength, "NonExistent.png");
	luaState_->loadFromMemory("NonExistent.lua", reinterpret_cast<const char *>(randomBuffer), randomBufferLength);
#endif

	const float width = nc::theApplication().width();
	const float height = nc::theApplication().height();
	for (unsigned int i = 0; i < NumSprites; i++)
	{
		nc::Texture *texture = textures_[i % NumTextures].get();
		const nc::Vector2f position(width / (NumSprites + 3) * (i + 2), height * 0.5f);
		sprites_.pushBack(nctl::makeUnique<nc::Sprite>(&rootNode, texture, position));
	}

	bufferPlayer_ = nctl::makeUnique<nc::AudioBufferPlayer>(audioBuffer_.get());
	textNode_ = nctl::makeUnique<nc::TextNode>(&rootNode, font_.get());
	textNode_->setPosition(width * 0.5f, height * 0.75f);
	textNode_->setString("apptest_loading");
}

void MyEventHandler::onFrameStart()
{
	ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::Begin("apptest_loading");
	if (ImGui::CollapsingHeader("Texture"))
	{
		if (ImGui::TreeNode("Texture Objects"))
		{
			for (int i = 0; i < NumTextures; i++)
			{
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				if (i == selectedTextureObject)
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				ImGui::TreeNodeEx(textures_[i].get(), nodeFlags, "Texture #%u", i);
				if (ImGui::IsItemClicked())
				{
					selectedTextureObject = i;
					texelsRegion.set(0, 0, textures_[i]->width(), textures_[i]->height());
				}
			}
			ImGui::TreePop();
		}

		if (selectedTextureObject >= 0 && selectedTextureObject < NumTextures)
		{
			bool textureHasChanged = false;
			nc::Texture &tex = *textures_[selectedTextureObject];
			ImGui::Text("Name: \"%s\"", tex.name().data());
			ImGui::Text("Size: %d x %d, Channels: %u", tex.width(), tex.height(), tex.numChannels());

			if (ImGui::TreeNode("Load from File or Memory"))
			{
				for (int i = 0; i < NumTextures; i++)
				{
					ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
					if (i == selectedTexture)
						nodeFlags |= ImGuiTreeNodeFlags_Selected;
					ImGui::TreeNodeEx(TextureFiles[i], nodeFlags);
					if (ImGui::IsItemClicked())
						selectedTexture = i;
				}
				if (ImGui::Button("Load from File") && selectedTexture >= 0 && selectedTexture < NumTextures)
				{
					const bool hasLoaded = tex.loadFromFile((prefixDataPath("textures", TextureFiles[selectedTexture])).data());
					if (hasLoaded == false)
						LOGW_X("Cannot load from file \"%s\"", TextureFiles[selectedTexture]);
					textureHasChanged = hasLoaded;
				}
				ImGui::SameLine();
				if (ImGui::Button("Load from Memory") && selectedTexture >= 0 && selectedTexture < NumTextures)
				{
					const bool hasLoaded = tex.loadFromMemory(TextureFiles[selectedTexture],
					                                          textureBuffers[selectedTexture].get(), textureBufferSizes[selectedTexture]);
					if (hasLoaded == false)
						LOGW_X("Cannot load from memory \"%s\"", TextureFiles[selectedTexture]);
					else
					{
						auxString.format("Memory file \"%s\"", TextureFiles[selectedTexture]);
						tex.setName(auxString.data());
					}
					textureHasChanged = hasLoaded;
				}
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Load from Texels"))
			{
				ImGui::DragInt2("Offset", &texelsRegion.x);
				ImGui::DragInt2("Size", &texelsRegion.w);

				texelsRegion.x = nctl::clamp(texelsRegion.x, 0, tex.width());
				texelsRegion.y = nctl::clamp(texelsRegion.y, 0, tex.height());
				texelsRegion.w = nctl::clamp(texelsRegion.w, 0, tex.width() - texelsRegion.x);
				texelsRegion.h = nctl::clamp(texelsRegion.h, 0, tex.height() - texelsRegion.y);

				ImGui::ColorEdit4("Color", texelsColor.data());
				if (ImGui::Button("Load##Texels"))
				{
					const int w = tex.width();
					const int h = tex.height();
					const uint32_t a = static_cast<uint8_t>(texelsColor.a() * 255.0f) << 24;
					const uint32_t b = static_cast<uint8_t>(texelsColor.b() * 255.0f) << 16;
					const uint32_t g = static_cast<uint8_t>(texelsColor.g() * 255.0f) << 8;
					const uint32_t r = static_cast<uint8_t>(texelsColor.r() * 255.0f);
					const uint32_t color = a + b + g + r;
					nctl::UniquePtr<uint32_t[]> pixels = nctl::makeUnique<uint32_t[]>(w * h);
					for (int i = 0; i < w * h; i++)
						pixels[i] = color;

					const bool hasLoaded = tex.loadFromTexels(reinterpret_cast<uint8_t *>(pixels.get()), texelsRegion);
					if (hasLoaded == false)
						LOGW("Cannot load from texels");
					else
					{
						auxString.format("Raw Texels (0x%x)", color);
						tex.setName(auxString.data());
					}

					// When loading from texels the format and the size does not change
					// and the `setTexture` method does not need to be called
				}
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Re-initialize"))
			{
				static int size[2] = { 256, 256 };
				ImGui::SliderInt2("Size", size, 8, 256);
				ImGui::SameLine();
				if (ImGui::Button("Current"))
				{
					size[0] = tex.width();
					size[1] = tex.height();
				}
				size[0] = nctl::clamp(size[0], 8, 256);
				size[1] = nctl::clamp(size[1], 8, 256);

				const char *formats[] = { "R", "RG", "RGB", "RGBA" };
				static int selectedFormat = 3;
				ImGui::Combo("Format", &selectedFormat, formats, IM_ARRAYSIZE(formats));

				if (ImGui::Button("Init"))
				{
					tex.init("Initialized", nc::Texture::Format(selectedFormat), size[0], size[1]);
					textureHasChanged = true;
				}
				ImGui::TreePop();
			}

			if (textureHasChanged)
			{
				texelsRegion.set(0, 0, tex.width(), tex.height());
				for (unsigned int i = 0; i < NumSprites; i++)
				{
					if (sprites_[i]->texture() == &tex)
						sprites_[i]->setTexture(&tex);
				}
			}
		}
		else
			ImGui::TextUnformatted("Select a texture object from the list");
	}

	if (ImGui::CollapsingHeader("Audio"))
	{
		ImGui::Text("Buffer Name: \"%s\"", audioBuffer_->name().data());
		ImGui::Text("Frequency: %d, Channels: %d, Size: %lu", audioBuffer_->frequency(), audioBuffer_->numChannels(), audioBuffer_->bufferSize());

		if (ImGui::Button("Play##Buffer"))
			bufferPlayer_->play();
		ImGui::SameLine();
		if (ImGui::Button("Pause##Buffer"))
			bufferPlayer_->pause();
		ImGui::SameLine();
		if (ImGui::Button("Stop##Buffer"))
			bufferPlayer_->stop();
		ImGui::SameLine();
		bool bufferIsLooping = bufferPlayer_->isLooping();
		ImGui::Checkbox("Looping##Buffer", &bufferIsLooping);
		bufferPlayer_->setLooping(bufferIsLooping);
		ImGui::SameLine();
		ImGui::Text("State: %s", audioPlayerStateToString(bufferPlayer_->state()));

		if (bufferPlayer_->state() != nc::IAudioPlayer::PlayerState::INITIAL)
		{
			int sampleOffset = bufferPlayer_->sampleOffset();
			if (ImGui::SliderInt("Seek##Buffer", &sampleOffset, 0, bufferPlayer_->numSamples()))
			{
				sampleOffset = nctl::clamp(sampleOffset, 0, static_cast<int>(bufferPlayer_->numSamples()));
				bufferPlayer_->setSampleOffset(sampleOffset);
			}
		}

		ImGui::Separator();
		ImGui::Text("Stream Name: \"%s\"", streamPlayer_->name().data());
		ImGui::Text("Frequency: %d, Channels: %d, Size: %lu", streamPlayer_->frequency(), streamPlayer_->numChannels(), streamPlayer_->bufferSize());

		if (ImGui::Button("Play##Stream"))
			streamPlayer_->play();
		ImGui::SameLine();
		if (ImGui::Button("Pause##Stream"))
			streamPlayer_->pause();
		ImGui::SameLine();
		if (ImGui::Button("Stop##Stream"))
			streamPlayer_->stop();
		ImGui::SameLine();
		bool streamIsLooping = streamPlayer_->isLooping();
		ImGui::Checkbox("Looping##Stream", &streamIsLooping);
		streamPlayer_->setLooping(streamIsLooping);
		ImGui::SameLine();
		ImGui::Text("State: %s", audioPlayerStateToString(streamPlayer_->state()));

		if (streamPlayer_->state() != nc::IAudioPlayer::PlayerState::INITIAL)
		{
			int sampleOffset = streamPlayer_->sampleOffset();
			if (ImGui::SliderInt("Seek##Stream", &sampleOffset, 0, streamPlayer_->numStreamSamples()))
			{
				sampleOffset = nctl::clamp(sampleOffset, 0, static_cast<int>(streamPlayer_->numStreamSamples()));
				streamPlayer_->setSampleOffset(sampleOffset);
			}
		}

		ImGui::Separator();
		if (ImGui::TreeNode("Load from File or Memory"))
		{
			for (int i = 0; i < NumSounds; i++)
			{
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				if (i == selectedSound)
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				ImGui::TreeNodeEx(SoundFiles[i], nodeFlags);
				if (ImGui::IsItemClicked())
					selectedSound = i;
			}
			if (ImGui::Button("Load Buffer from File") && selectedSound >= 0 && selectedSound < NumSounds)
			{
				bufferPlayer_->stop();
				const bool hasLoaded = audioBuffer_->loadFromFile((prefixDataPath("sounds", SoundFiles[selectedSound])).data());
				if (hasLoaded == false)
					LOGW_X("Cannot load from file \"%s\"", SoundFiles[selectedSound]);
			}
			ImGui::SameLine();
			if (ImGui::Button("Load Buffer from Memory") && selectedSound >= 0 && selectedSound < NumSounds)
			{
				bufferPlayer_->stop();
				const bool hasLoaded = audioBuffer_->loadFromMemory(SoundFiles[selectedSound],
				                                                    soundBuffers[selectedSound].get(), soundBufferSizes[selectedSound]);
				if (hasLoaded == false)
					LOGW_X("Cannot load from memory \"%s\"", SoundFiles[selectedSound]);
				else
				{
					auxString.format("Memory file \"%s\"", SoundFiles[selectedSound]);
					audioBuffer_->setName(auxString.data());
				}
			}

			if (ImGui::Button("Load Stream from File") && selectedSound >= 0 && selectedSound < NumSounds)
			{
				streamPlayer_->stop();
				const bool hasLoaded = streamPlayer_->loadFromFile((prefixDataPath("sounds", SoundFiles[selectedSound])).data());
				if (hasLoaded == false)
					LOGW_X("Cannot load from file \"%s\"", SoundFiles[selectedSound]);
			}
			ImGui::SameLine();
			if (ImGui::Button("Load Stream from Memory") && selectedSound >= 0 && selectedSound < NumSounds)
			{
				streamPlayer_->stop();
				const bool hasLoaded = streamPlayer_->loadFromMemory(SoundFiles[selectedSound],
				                                                     soundBuffers[selectedSound].get(), soundBufferSizes[selectedSound]);
				if (hasLoaded == false)
					LOGW_X("Cannot load from memory \"%s\"", SoundFiles[selectedSound]);
				else
				{
					auxString.format("Memory file \"%s\"", SoundFiles[selectedSound]);
					streamPlayer_->setName(auxString.data());
				}
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Load from Samples"))
		{
			ImGui::SliderInt("Frequency", &sineWaveFrequency, 110, 7040);
			sineWaveFrequency = nctl::clamp(sineWaveFrequency, 110, 7040);

			ImGui::SliderFloat("Duration", &sineWaveDuration, 0.1f, 5.0f);
			sineWaveDuration = nctl::clamp(sineWaveDuration, 0.1f, 5.0f);

			if (ImGui::Button("Load Buffer from Samples"))
			{
				bufferPlayer_->stop();
				bool hasLoaded = false;

				const unsigned int samplesPerSecond = audioBuffer_->numChannels() * audioBuffer_->frequency();
				const unsigned int numSamples = static_cast<unsigned int>(samplesPerSecond * sineWaveDuration);
				if (audioBuffer_->bytesPerSample() == 1)
				{
					nctl::UniquePtr<int8_t[]> audioSamples = nctl::makeUnique<int8_t[]>(numSamples);
					for (unsigned int i = 0; i < numSamples; i++)
						audioSamples[i] = static_cast<int8_t>(sinf(2.0f * nc::fPi * i * sineWaveFrequency / samplesPerSecond) * 128);
					hasLoaded = audioBuffer_->loadFromSamples((unsigned char *)(audioSamples.get()), numSamples * sizeof(int8_t));
				}
				else
				{
					nctl::UniquePtr<int16_t[]> audioSamples = nctl::makeUnique<int16_t[]>(numSamples);
					for (unsigned int i = 0; i < numSamples; i++)
						audioSamples[i] = static_cast<int16_t>(sinf(2.0f * nc::fPi * i * sineWaveFrequency / samplesPerSecond) * 32767);
					hasLoaded = audioBuffer_->loadFromSamples((unsigned char *)(audioSamples.get()), numSamples * sizeof(int16_t));
				}

				if (hasLoaded == false)
					LOGW("Cannot load from samples");
				else
				{
					auxString.format("Raw Samples (%dHz for %.2fs)", sineWaveFrequency, sineWaveDuration);
					audioBuffer_->setName(auxString.data());
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Re-initialize"))
		{
			const char *frequencyStrings[3] = { "44100Hz", "22050Hz", "11025Hz" };
			static const int frequencies[3] = { 44100, 22050, 11025 };
			static int selectedFrequency = 0;
			ImGui::Combo("Frequency", &selectedFrequency, frequencyStrings, IM_ARRAYSIZE(frequencyStrings));

			const char *formats[4] = { "Mono 8bit", "Stereo 8bit", "Mono 16bit", "Stereo 16bit" };
			static int selectedFormat = 0;
			ImGui::Combo("Format", &selectedFormat, formats, IM_ARRAYSIZE(formats));

			if (ImGui::Button("Init Buffer"))
				audioBuffer_->init("Initialized", nc::AudioBuffer::Format(selectedFormat), frequencies[selectedFrequency]);

			ImGui::TreePop();
		}
	}

	if (ImGui::CollapsingHeader("Font"))
	{
		ImGui::Text("Name: \"%s\"", font_->name().data());
		ImGui::Text("Glyphs: %u, Kerning pairs: %u", font_->numGlyphs(), font_->numKernings());
		ImGui::Text("Line height: %u, Base: %u", font_->lineHeight(), font_->base());

		bool fontHasChanged = false;
		if (ImGui::TreeNode("Load from File or Memory"))
		{
			for (int i = 0; i < NumFonts; i++)
			{
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				if (i == selectedFont)
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				ImGui::TreeNodeEx(FontFiles[i], nodeFlags);
				if (ImGui::IsItemClicked())
					selectedFont = i;
			}
			if (ImGui::Button("Load from File") && selectedFont >= 0 && selectedFont < NumFonts)
			{
				const bool hasLoaded = font_->loadFromFile((prefixDataPath("fonts", FontFiles[selectedFont])).data());
				if (hasLoaded == false)
					LOGW_X("Cannot load from file \"%s\"", FontFiles[selectedFont]);
				fontHasChanged = hasLoaded;
			}
			ImGui::SameLine();
			if (ImGui::Button("Load from Memory") && selectedFont >= 0 && selectedFont < NumFonts)
			{
				const bool hasLoaded = font_->loadFromMemory(FontFiles[selectedFont], fontBuffers[selectedFont].get(), fontBufferSizes[selectedFont],
				                                             (prefixDataPath("fonts", FontTexFiles[selectedFont])).data());
				if (hasLoaded == false)
					LOGW_X("Cannot load from memory \"%s\"", FontFiles[selectedFont]);
				else
				{
					auxString.format("Memory file \"%s\"", FontFiles[selectedFont]);
					audioBuffer_->setName(auxString.data());
				}
				fontHasChanged = hasLoaded;
			}
			ImGui::TreePop();
		}

		if (fontHasChanged)
			textNode_->setFont(font_.get());
	}

	if (ImGui::CollapsingHeader("Lua Script"))
	{
		if (loadedScript >= 0 && loadedScript < NumScripts)
			ImGui::Text("Name: \"%s\"", ScriptFiles[loadedScript]);
		else
			ImGui::TextUnformatted("No script loaded");

		bool scriptHasChanged = false;
		if (ImGui::TreeNode("Load from File or Memory"))
		{
			for (int i = 0; i < NumScripts; i++)
			{
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				if (i == selectedScript)
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				ImGui::TreeNodeEx(ScriptFiles[i], nodeFlags);
				if (ImGui::IsItemClicked())
					selectedScript = i;
			}
			if (ImGui::Button("Load from File") && selectedScript >= 0 && selectedScript < NumScripts)
			{
				const bool hasLoaded = luaState_->loadFromFile((prefixDataPath("scripts", ScriptFiles[selectedScript])).data(), ScriptFiles[selectedScript]);
				if (hasLoaded == false)
					LOGW_X("Cannot load from file \"%s\"", ScriptFiles[selectedScript]);
				scriptHasChanged = hasLoaded;
			}
			ImGui::SameLine();
			if (ImGui::Button("Load from Memory") && selectedScript >= 0 && selectedScript < NumScripts)
			{
				const bool hasLoaded = luaState_->loadFromMemory(ScriptFiles[selectedScript], scriptBuffers[selectedScript].get(), scriptBufferSizes[selectedScript]);
				if (hasLoaded == false)
					LOGW_X("Cannot load from memory \"%s\"", ScriptFiles[selectedScript]);
				scriptHasChanged = hasLoaded;
			}
			ImGui::TreePop();
		}

		if (scriptHasChanged)
		{
			loadedScript = selectedScript;
			luaState_ = nctl::makeUnique<nc::LuaStateManager>(nc::LuaStateManager::ApiType::EDIT_ONLY,
			                                                  nc::LuaStateManager::StatisticsTracking::DISABLED,
			                                                  nc::LuaStateManager::StandardLibraries::NOT_LOADED);
		}
	}

	ImGui::End();
}

void MyEventHandler::onKeyReleased(const nc::KeyboardEvent &event)
{
	if (event.sym == nc::KeySym::ESCAPE || event.sym == nc::KeySym::Q)
		nc::theApplication().quit();
}
