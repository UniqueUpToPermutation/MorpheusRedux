#pragma once

#include <Engine/SceneHeirarchy.hpp>

#include <entt/entt.hpp>

#include "BasicMath.hpp"

namespace DG = Diligent;

namespace Morpheus {
	class EditorCameraController {
	private:
		float mElevation = 	0.0f;
		float mAzimuth = 	0.0f;

		float mMouseRotationSpeedX = 0.004f;
		float mMouseRotationSpeedY = 0.004f;

		float mMousePanSpeedX = 0.01f;
		float mMousePanSpeedY = 0.01f;

		float mKeyPanSpeedZ = 10.0f;
		float mKeyPanSpeedX = 10.0f;

		SceneHeirarchy* mScene;

		DG::Quaternion GetViewQuat() const;
		DG::float3 GetViewVector() const;

	public:
		void OnUpdate(const UpdateEvent& e);

		EditorCameraController(SceneHeirarchy* scene);
		EditorCameraController(const EditorCameraController& other);
		~EditorCameraController();
	};
}