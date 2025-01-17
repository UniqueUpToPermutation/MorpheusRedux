#pragma once

#include "GraphicsTypes.h"

#include <Engine/Defines.hpp>
#include <Engine/InputController.hpp>

#include <functional>

namespace DG = Diligent;

#if PLATFORM_WIN32
#define MAIN() int __stdcall WinMain( \
	HINSTANCE hInstance, \
	HINSTANCE hPrevInstance, \
	LPSTR     lpCmdLine, \
	int       nShowCmd) 
#elif PLATFORM_LINUX
#define MAIN() int main(int argc, char** argv) 
#endif

namespace Morpheus {

	
	typedef std::function<void(double, double)> update_callback_t;

	struct EngineParams;

	struct PlatformParams {
		std::string mWindowTitle 	= "Morpheus";
		uint mWindowWidth 			= 1024;
		uint mWindowHeight   		= 756;
		bool bFullscreen 			= false;
		bool bShowOnCreation		= true;

		// Device type for use by Graphics class
		DG::RENDER_DEVICE_TYPE   mDeviceType 		= DG::RENDER_DEVICE_TYPE_UNDEFINED;
	};

	typedef std::function<void(uint, uint)> user_window_resize_t;

	class IPlatform {
	public:
		virtual ~IPlatform() {
		}

		virtual int Startup(const PlatformParams& params = PlatformParams()) = 0;
		virtual void Shutdown() = 0;
		virtual bool IsValid() const = 0;
		virtual void MessagePump() = 0;
		virtual void Flush() = 0;
		virtual void Show() = 0;
		virtual void Hide() = 0;
		virtual void SetCursorVisible(bool value) = 0;
		virtual const PlatformParams& GetParameters() const = 0;
		virtual const InputController& GetInput() const = 0;

		// Adds a delegate that will be called when the window is resized.
		virtual void AddUserResizeHandler(user_window_resize_t* handler) = 0;
		// Removes a delegate that is called when the window is resized.
		virtual void RemoveUserResizeHandler(user_window_resize_t* handler) = 0;

		virtual PlatformLinux* ToLinux() = 0;
		virtual PlatformWin32* ToWindows() = 0;
	};

	IPlatform* CreatePlatform();

	class Platform {
	private:
		IPlatform* mPlatform = nullptr;

	public:
		inline Platform() : mPlatform(CreatePlatform()) {
		}

		inline ~Platform() {
			if (mPlatform) {
				mPlatform->Shutdown();
				delete mPlatform;
			}
		}

		inline IPlatform* operator->() {
			return mPlatform;
		}

		inline operator IPlatform*() {
			return mPlatform;
		}

		Platform(const Platform& plat) = delete;
		Platform& operator=(const Platform& plat) = delete;

		inline Platform(Platform&& plat) : mPlatform(plat.mPlatform) {
			plat.mPlatform = nullptr;
		}

		inline Platform& operator=(Platform&& plat) {
			mPlatform = plat.mPlatform;
			plat.mPlatform = nullptr;
			return *this;
		}

		inline int Startup(const PlatformParams& params = PlatformParams()) {
			return mPlatform->Startup(params);
		}
		inline void Shutdown() {
			return mPlatform->Shutdown();
		}
		inline bool IsValid() const {
			return mPlatform->IsValid();
		}
		inline void MessagePump() {
			mPlatform->MessagePump();
		}
		inline void Flush() {
			mPlatform->Flush();
		}
		inline void Show() {
			mPlatform->Show();
		}
		inline void Hide() {
			mPlatform->Hide();
		}
		inline void SetCursorVisible(bool value) {
			mPlatform->SetCursorVisible(value);
		}
		inline const PlatformParams& GetParameters() const {
			return mPlatform->GetParameters();
		}
		inline const InputController& GetInput() const {
			return mPlatform->GetInput();
		}

		// Adds a delegate that will be called when the window is resized.
		inline void AddUserResizeHandler(user_window_resize_t* handler) {
			mPlatform->AddUserResizeHandler(handler);
		}
		// Removes a delegate that is called when the window is resized.
		inline void RemoveUserResizeHandler(user_window_resize_t* handler) {
			mPlatform->RemoveUserResizeHandler(handler);
		}

		inline PlatformLinux* ToLinux() {
			return mPlatform->ToLinux();
		}
		inline PlatformWin32* ToWindows() {
			return mPlatform->ToWindows();
		}
	};
}