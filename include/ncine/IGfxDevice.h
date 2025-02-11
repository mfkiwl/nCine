#ifndef CLASS_NCINE_IGFXDEVICE
#define CLASS_NCINE_IGFXDEVICE

#include "Vector2.h"
#include "Rect.h"
#include "DisplayMode.h"
#include "AppConfiguration.h"

#ifdef __EMSCRIPTEN__
class EmscriptenUiEvent;
class EmscriptenFullscreenChangeEvent;
class EmscriptenFocusEvent;
#endif

namespace ncine {

class Colorf;

/// It represents the interface to the graphics device where everything is rendered
class DLL_PUBLIC IGfxDevice
{
  public:
	/// A structure used to initialize window properties
	struct WindowMode
	{
		WindowMode()
		    : width(0), height(0), isFullScreen(false), isResizable(false) {}
		WindowMode(unsigned int w, unsigned int h, bool fullscreen, bool resizable)
		    : width(w), height(h), isFullScreen(fullscreen), isResizable(resizable) {}

		unsigned int width;
		unsigned int height;
		bool isFullScreen;
		bool isResizable;
	};

	/// A structure representing a supported monitor video mode
	struct VideoMode
	{
		VideoMode()
		    : width(0), height(0), redBits(8), greenBits(8), blueBits(8), refreshRate(60) {}
		inline bool operator==(const VideoMode &mode) const
		{
			return (width == mode.width && height == mode.height &&
			        redBits == mode.redBits && greenBits == mode.greenBits &&
			        blueBits == mode.blueBits && refreshRate == mode.refreshRate);
		}
		inline bool operator!=(const VideoMode &mode) const { return !operator==(mode); }

		unsigned int width;
		unsigned int height;
		unsigned int redBits;
		unsigned int greenBits;
		unsigned int blueBits;
		unsigned int refreshRate;
	};

	/// Contains the attributes to create an OpenGL context
	struct GLContextInfo
	{
		explicit GLContextInfo(const AppConfiguration &appCfg)
		    : majorVersion(appCfg.glMajorVersion()), minorVersion(appCfg.glMinorVersion()),
		      coreProfile(appCfg.glCoreProfile()), forwardCompatible(appCfg.glForwardCompatible()),
		      debugContext(appCfg.withGlDebugContext) {}

		unsigned int majorVersion;
		unsigned int minorVersion;
		bool coreProfile;
		bool forwardCompatible;
		bool debugContext;
	};

	IGfxDevice(const WindowMode &windowMode, const GLContextInfo &glContextInfo, const DisplayMode &displayMode);
	virtual ~IGfxDevice() {}

	/// Sets the number of vertical blanks to occur before a buffer swap
	/*! An interval of `-1` will enable adaptive v-sync if available */
	virtual void setSwapInterval(int interval) = 0;

	/// Sets screen resolution with two integers
	virtual void setResolution(int width, int height) = 0;
	/// Sets screen resolution with a `Vector2<int>` object
	inline void setResolution(Vector2i size) { setResolution(size.x, size.y); }

	/// Returns true if the device renders in full screen
	inline bool isFullScreen() const { return isFullScreen_; }
	/// Sets the full screen flag of the window
	virtual void setFullScreen(bool fullScreen) = 0;

	/// Returns true if the window is resizable
	inline bool isResizable() const { return isResizable_; }

	/// Sets the position of the application window with two integers
	virtual void setWindowPosition(int x, int y) = 0;
	/// Sets the position of the application window with a `Vector2<int>` object
	inline void setWindowPosition(Vector2i position) { setWindowPosition(position.x, position.y); }
	/// Sets the application window title
	virtual void setWindowTitle(const char *windowTitle) = 0;
	/// Sets the application window icon
	virtual void setWindowIcon(const char *iconFilename) = 0;

	/// Returns device width
	inline int width() const { return width_; }
	/// Returns device height
	inline int height() const { return height_; }
	/// Returns device resolution as a `Vector2i` object
	inline Vector2i resolution() const { return Vector2i(width_, height_); }
	/// Returns device resolution as a `Rectf` object
	inline Rectf screenRect() const { return Rectf(0.0f, 0.0f, static_cast<float>(width_), static_cast<float>(height_)); }
	/// Returns device aspect ratio
	inline float aspect() const { return width_ / static_cast<float>(height_); }

	/// Returns window horizontal position
	inline virtual int windowPositionX() const { return 0; }
	/// Returns window vertical position
	inline virtual int windowPositionY() const { return 0; }
	/// Returns window position as a `Vector2i` object
	inline virtual const Vector2i windowPosition() const { return Vector2i(0, 0); }

	/// Highlights the application window to notify the user
	inline virtual void flashWindow() const {}

	/// Returns the OpenGL context creation attributes
	inline const GLContextInfo &glContextInfo() const { return glContextInfo_; }
	/// Returns display mode
	inline const DisplayMode &displayMode() const { return displayMode_; }

	/// Returns the current monitor video mode
	inline virtual const VideoMode &currentVideoMode() const { return currentVideoMode_; }
	/// Returns the number of video modes supported by the monitor
	inline unsigned int numVideoModes() const { return numVideoModes_; }
	/// Returns the specified monitor video mode
	const VideoMode &videoMode(unsigned int index) const;
	/// Sets the specified monitor video mode
	inline virtual bool setVideoMode(unsigned int index) { return true; }
	/// Updates the array of supported monitor video modes
	inline virtual void updateVideoModes() {}

  protected:
	/// Device width
	int width_;
	/// Device height
	int height_;
	/// Whether device rendering occurs in full screen
	bool isFullScreen_;
	/// Whether the window is resizable
	bool isResizable_;
	/// OpenGL context creation attributes
	GLContextInfo glContextInfo_;
	/// Display properties
	DisplayMode displayMode_;

#ifdef __ANDROID__
	static const int MaxVideoModes = 1;
#else
	static const int MaxVideoModes = 128;
#endif
	VideoMode videoModes_[MaxVideoModes];
	unsigned int numVideoModes_;
	mutable VideoMode currentVideoMode_;

  private:
	/// Sets up the initial OpenGL state for the scenegraph
	virtual void setupGL();

	/// Updates the screen swapping back and front buffers
	virtual void update() = 0;

	friend class Application;
#if defined(WITH_SDL)
	friend class PCApplication;
#endif

#ifdef __EMSCRIPTEN__
	static int resize_callback(int eventType, const EmscriptenUiEvent *event, void *userData);
	static int fullscreenchange_callback(int eventType, const EmscriptenFullscreenChangeEvent *event, void *userData);
	static int focus_callback(int eventType, const EmscriptenFocusEvent *event, void *userData);
#endif
};

}

#endif
