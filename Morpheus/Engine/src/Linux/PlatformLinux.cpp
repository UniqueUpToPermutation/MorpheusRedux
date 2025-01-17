
#include <Engine/Linux/PlatformLinux.hpp>

namespace Morpheus
{
#if VULKAN_SUPPORTED
	struct xcb_size_hints_t
	{
		uint32_t flags;                          /** User specified flags */
		int32_t  x, y;                           /** User-specified position */
		int32_t  width, height;                  /** User-specified size */
		int32_t  min_width, min_height;          /** Program-specified minimum size */
		int32_t  max_width, max_height;          /** Program-specified maximum size */
		int32_t  width_inc, height_inc;          /** Program-specified resize increments */
		int32_t  min_aspect_num, min_aspect_den; /** Program-specified minimum aspect ratios */
		int32_t  max_aspect_num, max_aspect_den; /** Program-specified maximum aspect ratios */
		int32_t  base_width, base_height;        /** Program-specified base size */
		uint32_t win_gravity;                    /** Program-specified window gravity */
	};

	enum XCB_SIZE_HINT
	{
		XCB_SIZE_HINT_US_POSITION   = 1 << 0,
		XCB_SIZE_HINT_US_SIZE       = 1 << 1,
		XCB_SIZE_HINT_P_POSITION    = 1 << 2,
		XCB_SIZE_HINT_P_SIZE        = 1 << 3,
		XCB_SIZE_HINT_P_MIN_SIZE    = 1 << 4,
		XCB_SIZE_HINT_P_MAX_SIZE    = 1 << 5,
		XCB_SIZE_HINT_P_RESIZE_INC  = 1 << 6,
		XCB_SIZE_HINT_P_ASPECT      = 1 << 7,
		XCB_SIZE_HINT_BASE_SIZE     = 1 << 8,
		XCB_SIZE_HINT_P_WIN_GRAVITY = 1 << 9
	};
#endif

	typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, int, const int*);

	static constexpr int None = 0;

	static constexpr uint16_t MinWindowWidth  = 320;
	static constexpr uint16_t MinWindowHeight = 240;

	PlatformLinux::PlatformLinux() :
		bQuit(false) {
	}

	int PlatformLinux::HandleXEvent(XEvent* xev) {
		int handled = false;
		for (auto handler : mEventHandlersX) {
			if (!handled || xev->type == ButtonRelease || xev->type == MotionNotify || xev->type == KeyRelease) {
				handled = (*handler)(xev);
			} else {
				break;
			}
		}

		if (!handled || xev->type == ButtonRelease || xev->type == MotionNotify || xev->type == KeyRelease) {
			handled = mInput.HandleXEvent(xev);
		}

		return handled;
	}

#ifdef VULKAN_SUPPORTED
	void PlatformLinux::HandleXCBEvent(xcb_generic_event_t* event) {

		int handled = false;
		for (auto handler : mEventHandlersXCB) {
			if (!handled) {
				handled = (*handler)(event);
			} else {
				break;
			}
		}
		
		auto EventType = event->response_type & 0x7f;
		// Always handle mouse move, button release and key release events
		if (!handled || EventType == XCB_MOTION_NOTIFY || EventType == XCB_BUTTON_RELEASE || EventType == XCB_KEY_RELEASE)
		{
			handled = mInput.HandleXCBEvent(event);
		}
	}
#endif

	void PlatformLinux::HandleWindowResize(uint width, uint height) {
		
		mParams.mWindowWidth = width;
		mParams.mWindowHeight = height;

		for (auto handler : mWindowResizeHandlers) {
			(*handler)(width, height);
		}
	}

#ifdef VULKAN_SUPPORTED
	int PlatformLinux::InitializeVulkan(const PlatformParams& params) {

		int scr         = 0;
		mXCBInfo.connection = xcb_connect(nullptr, &scr);
		if (mXCBInfo.connection == nullptr || xcb_connection_has_error(mXCBInfo.connection))
		{
			std::cerr << "Unable to make an XCB connection\n";
			exit(-1);
		}

		const xcb_setup_t*    setup = xcb_get_setup(mXCBInfo.connection);
		xcb_screen_iterator_t iter  = xcb_setup_roots_iterator(setup);
		while (scr-- > 0)
			xcb_screen_next(&iter);

		auto screen = iter.data;

		mXCBInfo.width  = params.mWindowWidth;
		mXCBInfo.height = params.mWindowHeight;

		uint32_t value_mask, value_list[32];

		mXCBInfo.window = xcb_generate_id(mXCBInfo.connection);

		value_mask    = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
		value_list[0] = screen->black_pixel;
		value_list[1] =
			XCB_EVENT_MASK_KEY_RELEASE |
			XCB_EVENT_MASK_KEY_PRESS |
			XCB_EVENT_MASK_EXPOSURE |
			XCB_EVENT_MASK_STRUCTURE_NOTIFY |
			XCB_EVENT_MASK_POINTER_MOTION |
			XCB_EVENT_MASK_BUTTON_PRESS |
			XCB_EVENT_MASK_BUTTON_RELEASE;

		xcb_create_window(mXCBInfo.connection, XCB_COPY_FROM_PARENT, mXCBInfo.window, screen->root, 0, 0, mXCBInfo.width, mXCBInfo.height, 0,
						XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, value_mask, value_list);

		// Magic code that will send notification when window is destroyed
		xcb_intern_atom_cookie_t cookie = xcb_intern_atom(mXCBInfo.connection, 1, 12, "WM_PROTOCOLS");
		xcb_intern_atom_reply_t* reply  = xcb_intern_atom_reply(mXCBInfo.connection, cookie, 0);

		xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(mXCBInfo.connection, 0, 16, "WM_DELETE_WINDOW");
		mXCBInfo.atom_wm_delete_window       = xcb_intern_atom_reply(mXCBInfo.connection, cookie2, 0);

		xcb_change_property(mXCBInfo.connection, XCB_PROP_MODE_REPLACE, mXCBInfo.window, (*reply).atom, 4, 32, 1,
							&(*mXCBInfo.atom_wm_delete_window).atom);
		free(reply);

		mTitle = params.mWindowTitle;

		// https://stackoverflow.com/a/27771295
		xcb_size_hints_t hints = {};
		hints.flags            = XCB_SIZE_HINT_P_MIN_SIZE;
		hints.min_width        = MinWindowWidth;
		hints.min_height       = MinWindowHeight;
		xcb_change_property(mXCBInfo.connection, XCB_PROP_MODE_REPLACE, mXCBInfo.window, XCB_ATOM_WM_NORMAL_HINTS, XCB_ATOM_WM_SIZE_HINTS,
							32, sizeof(xcb_size_hints_t), &hints);

		xcb_map_window(mXCBInfo.connection, mXCBInfo.window);

		// Force the x/y coordinates to 100,100 results are identical in consecutive
		// runs
		const uint32_t coords[] = {100, 100};
		xcb_configure_window(mXCBInfo.connection, mXCBInfo.window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);
		xcb_flush(mXCBInfo.connection);

		xcb_generic_event_t* e;
		while ((e = xcb_wait_for_event(mXCBInfo.connection)))
		{
			if ((e->response_type & ~0x80) == XCB_EXPOSE) break;
		}

		xcb_change_property(mXCBInfo.connection, XCB_PROP_MODE_REPLACE, mXCBInfo.window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING,
			8, mTitle.length(), mTitle.c_str());

		xcb_flush(mXCBInfo.connection);

		mParams.mDeviceType = DG::RENDER_DEVICE_TYPE_VULKAN;

		mMode = PlatformLinuxMode::XCB;

		mInput.InitXCBKeysms(mXCBInfo.connection);

		return 1;
	}
#endif

	int PlatformLinux::InitializeGL(const PlatformParams& params) {
		mDisplay = XOpenDisplay(0);

		// clang-format off
		static int visual_attribs[] =
		{
			GLX_RENDER_TYPE,    GLX_RGBA_BIT,
			GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
			GLX_DOUBLEBUFFER,   true,

			// The largest available total RGBA color buffer size (sum of GLX_RED_SIZE, 
			// GLX_GREEN_SIZE, GLX_BLUE_SIZE, and GLX_ALPHA_SIZE) of at least the minimum
			// size specified for each color component is preferred.
			GLX_RED_SIZE,       8,
			GLX_GREEN_SIZE,     8,
			GLX_BLUE_SIZE,      8,
			GLX_ALPHA_SIZE,     8,

			// The largest available depth buffer of at least GLX_DEPTH_SIZE size is preferred
			GLX_DEPTH_SIZE,     24,

			GLX_SAMPLE_BUFFERS, 0,

			// Setting GLX_SAMPLES to 1 results in 2x MS backbuffer, which is 
			// against the spec that states:
			//     if GLX SAMPLE BUFFERS is zero, then GLX SAMPLES will also be zero
			// GLX_SAMPLES, 1,

			None
		};
		// clang-format on

		int          fbcount = 0;
		GLXFBConfig* fbc     = glXChooseFBConfig(mDisplay, DefaultScreen(mDisplay), visual_attribs, &fbcount);
		if (!fbc)
		{
			LOG_ERROR_MESSAGE("Failed to retrieve a framebuffer config");
			return 0;
		}

		XVisualInfo* vi = glXGetVisualFromFBConfig(mDisplay, fbc[0]);

		XSetWindowAttributes swa;
		swa.colormap     = XCreateColormap(mDisplay, RootWindow(mDisplay, vi->screen), vi->visual, AllocNone);
		swa.border_pixel = 0;
		swa.event_mask =
			StructureNotifyMask |
			ExposureMask |
			KeyPressMask |
			KeyReleaseMask |
			ButtonPressMask |
			ButtonReleaseMask |
			PointerMotionMask;

		Window mWindow = XCreateWindow(mDisplay, RootWindow(mDisplay, vi->screen), 0, 0, 
			params.mWindowWidth, params.mWindowHeight, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);
		if (!mWindow)
		{
			LOG_ERROR_MESSAGE("Failed to create window.");
			return 0;
		}

		{
			auto SizeHints        = XAllocSizeHints();
			SizeHints->flags      = PMinSize;
			SizeHints->min_width  = MinWindowWidth;
			SizeHints->min_height = MinWindowHeight;
			XSetWMNormalHints(mDisplay, mWindow, SizeHints);
			XFree(SizeHints);
		}

		XMapWindow(mDisplay, mWindow);

		glXCreateContextAttribsARBProc glXCreateContextAttribsARB = nullptr;
		{
			// Create an oldstyle context first, to get the correct function pointer for glXCreateContextAttribsARB
			GLXContext ctx_old         = glXCreateContext(mDisplay, vi, 0, GL_TRUE);
			glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
			glXMakeCurrent(mDisplay, None, NULL);
			glXDestroyContext(mDisplay, ctx_old);
		}

		if (glXCreateContextAttribsARB == nullptr)
		{
			LOG_ERROR("glXCreateContextAttribsARB entry point not found. Aborting.");
			return 0;
		}

		int Flags = GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
	#ifdef _DEBUG
		Flags |= GLX_CONTEXT_DEBUG_BIT_ARB;
	#endif

		int major_version = 4;
		int minor_version = 3;

		static int context_attribs[] =
		{
			GLX_CONTEXT_MAJOR_VERSION_ARB, major_version,
			GLX_CONTEXT_MINOR_VERSION_ARB, minor_version,
			GLX_CONTEXT_FLAGS_ARB, Flags,
			None //
		};

		constexpr int True = 1;
		mGLXContext  = glXCreateContextAttribsARB(mDisplay, fbc[0], NULL, True, context_attribs);
		if (!mGLXContext)
		{
			LOG_ERROR("Failed to create GL context.");
			return 0;
		}
		XFree(fbc);

		glXMakeCurrent(mDisplay, mWindow, mGLXContext);

		mParams.mDeviceType = DG::RENDER_DEVICE_TYPE_GL;
	
		mTitle = params.mWindowTitle;
		XStoreName(mDisplay, mWindow, mTitle.c_str());

		mMode = PlatformLinuxMode::X11;

		return 1;
	}

	void PlatformLinux::Flush() {
#if VULKAN_SUPPORTED
		if (mParams.mDeviceType == DG::RENDER_DEVICE_TYPE_VULKAN) {
			xcb_flush(mXCBInfo.connection);
		}
#endif
	}

	void PlatformLinux::Show() {

	}
	void PlatformLinux::Hide() {
		
	}
	void PlatformLinux::SetCursorVisible(bool value) {
		
	}

	const PlatformParams& PlatformLinux::GetParameters() const {
		return mParams;
	}

	const InputController& PlatformLinux::GetInput() const {
		return mInput;
	}

	void PlatformLinux::AddUserResizeHandler(user_window_resize_t* handler) {
		mWindowResizeHandlers.emplace(handler);
	}
	void PlatformLinux::RemoveUserResizeHandler(user_window_resize_t* handler) {
		mWindowResizeHandlers.erase(handler);
	}
				
	void PlatformLinux::MessagePump() {
		mInput.NewFrame();

		if (mParams.mDeviceType == DG::RENDER_DEVICE_TYPE_GL) {
			XEvent xev;
			// Handle all events in the queue
			while (XCheckMaskEvent(mDisplay, 0xFFFFFFFF, &xev))
			{
				HandleXEvent(&xev);
				switch (xev.type)
				{
					case ConfigureNotify:
					{
						XConfigureEvent& xce = reinterpret_cast<XConfigureEvent&>(xev);
						if (xce.width != 0 && xce.height != 0)
							HandleWindowResize(xce.width, xce.height);
						break;
					}
				}
			}
		}
#if VULKAN_SUPPORTED
		else if (mParams.mDeviceType == DG::RENDER_DEVICE_TYPE_VULKAN) {
			xcb_generic_event_t* event = nullptr;

			while ((event = xcb_poll_for_event(mXCBInfo.connection)) != nullptr)
			{
				HandleXCBEvent(event);
				switch (event->response_type & 0x7f)
				{
					case XCB_CLIENT_MESSAGE:
						if ((*(xcb_client_message_event_t*)event).data.data32[0] ==
							(*mXCBInfo.atom_wm_delete_window).atom)
						{
							bQuit = true;
						}
						break;

					case XCB_DESTROY_NOTIFY:
						bQuit = true;
						break;

					case XCB_CONFIGURE_NOTIFY:
					{
						const auto* cfgEvent = reinterpret_cast<const xcb_configure_notify_event_t*>(event);
						if ((cfgEvent->width != mXCBInfo.width) || (cfgEvent->height != mXCBInfo.height))
						{
							mXCBInfo.width  = cfgEvent->width;
							mXCBInfo.height = cfgEvent->height;
							if ((mXCBInfo.width > 0) && (mXCBInfo.height > 0))
							{
								HandleWindowResize(mXCBInfo.width, mXCBInfo.height);
							}
						}
					}
					break;

					default:
						break;
				}
				free(event);
			}
		}
#endif
	}

	DG::LinuxNativeWindow PlatformLinux::GetNativeWindow() const {
		
		if (mParams.mDeviceType == DG::RENDER_DEVICE_TYPE_GL) {
			DG::LinuxNativeWindow window;
			window.pDisplay = mDisplay;
			window.WindowId = mWindow;
			return window;
		}
#if VULKAN_SUPPORTED
		else if (mParams.mDeviceType == DG::RENDER_DEVICE_TYPE_VULKAN) {
			DG::LinuxNativeWindow window;
			window.pXCBConnection = mXCBInfo.connection;
			window.WindowId = mXCBInfo.window;
			return window;
		}
#endif
		return DG::LinuxNativeWindow();
	}


	int PlatformLinux::Startup(const PlatformParams& params) {

		bIsInitialized = true;
		bool UseVulkan = true;

		switch (params.mDeviceType) {
			case DG::RENDER_DEVICE_TYPE_GL:
				UseVulkan = false;
				break;
#if VULKAN_SUPPORTED
			case DG::RENDER_DEVICE_TYPE_VULKAN:
				UseVulkan = true;
				break;
			case DG::RENDER_DEVICE_TYPE_UNDEFINED:
				UseVulkan = true;
				break;
#else 
			case DG::RENDER_DEVICE_TYPE_UNDEFINED:
				UseVulkan = false;
				break;
#endif
			default:
				throw std::runtime_error("Linux does not support specified renderer backend!");
				break;
		}

#if VULKAN_SUPPORTED
		if (UseVulkan)
		{
			auto ret = InitializeVulkan(params);
			if (ret)
			{
				return ret;
			}
			else
			{
				LOG_ERROR_MESSAGE("Failed to initialize the engine in Vulkan mode. Attempting to use OpenGL");
			}
		}
	#endif

		return InitializeGL(params);
	}

	void PlatformLinux::Shutdown() {
		if (mParams.mDeviceType == DG::RENDER_DEVICE_TYPE_GL) {
			if (mDisplay) {
				glXMakeCurrent(mDisplay, None, NULL);
				glXDestroyContext(mDisplay, mGLXContext);
				XDestroyWindow(mDisplay, mWindow);
				XCloseDisplay(mDisplay);
				mDisplay = nullptr;
			}
		}
#if VULKAN_SUPPORTED
		else if (mParams.mDeviceType == DG::RENDER_DEVICE_TYPE_VULKAN) {
			if (mXCBInfo.connection) {
				xcb_destroy_window(mXCBInfo.connection, mXCBInfo.window);
				xcb_disconnect(mXCBInfo.connection);
				mXCBInfo.connection = nullptr;
			}
		}
#endif
		else {
			throw std::runtime_error("Unknown device type!");
		}
	}

	bool PlatformLinux::IsValid() const {
		return !bQuit;
	}

	PlatformLinux* PlatformLinux::ToLinux() {
		return this;
	}

	PlatformWin32* PlatformLinux::ToWindows() {
		return nullptr;
	}

	IPlatform* CreatePlatform() {
		return new PlatformLinux();
	}
}