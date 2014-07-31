#include <cstdlib>
#include <android_native_app_glue.h>
#include "ncApplication.h"
#include "ncAndroidInputManager.h"
class ncIAppEventHandler;

ncIAppEventHandler* create_apphandler();

/// Process the next input event.
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
	ncAndroidInputManager::ParseEvent(event);
	return 1;
}

/// Process the next main command
static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
	switch (cmd)
	{
		case APP_CMD_SAVE_STATE:
			break;
		case APP_CMD_INIT_WINDOW:
			if (app->window != NULL)
			{
				ncApplication::Init(app, create_apphandler);
				ncApplication::Step();
			}
			break;
		case APP_CMD_TERM_WINDOW:
			ncApplication::Quit();
			break;
		case APP_CMD_GAINED_FOCUS:
			ncAndroidInputManager::EnableAccelerometerSensor();
			ncApplication::SetFocus(true);
			break;
		case APP_CMD_LOST_FOCUS:
			ncAndroidInputManager::DisableAccelerometerSensor();
			ncApplication::SetFocus(false);
			ncApplication::Step();
			break;
	}
}

void android_main(struct android_app* state)
{
	// Make sure glue isn't stripped.
	app_dummy();

	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;

	ncAndroidInputManager::AttachJVM(state->activity->vm);
	ncAndroidInputManager::InitAccelerometerSensor(state);

	while (ncApplication::ShouldQuit() == false)
	{
		int ident;
		int events;
		struct android_poll_source* source;

		while ((ident = ALooper_pollAll(!ncApplication::IsPaused() ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
		{
			if (source != NULL)
				source->process(state, source);

			if (ident == LOOPER_ID_USER)
				ncAndroidInputManager::ParseAccelerometerEvent();

			if (state->destroyRequested)
				ncApplication::Quit();
		}

		if (ncApplication::HasFocus() && !ncApplication::IsPaused())
			ncApplication::Step();
	}

	ncAndroidInputManager::DetachJVM();
	ncApplication::Shutdown();
	ANativeActivity_finish(state->activity);
}
