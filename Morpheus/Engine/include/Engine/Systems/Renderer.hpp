#pragma once

#include <Engine/Systems/System.hpp>

namespace Morpheus {

	class IRenderer;

	enum class MaterialType {
		COOK_TORRENCE,
		LAMBERT,
		CUSTOM
	};

	struct MaterialDesc {
		MaterialType mType = MaterialType::COOK_TORRENCE;

		Handle<Texture> mAlbedo;
		Handle<Texture> mNormal;
		Handle<Texture> mRoughness;
		Handle<Texture> mMetallic;
		Handle<Texture> mDisplacement;

		DG::float3 mAlbedoFactor 		= DG::float3(1.0f, 1.0f, 1.0f);
		DG::float3 mRoughnessFactor 	= DG::float3(1.0f, 1.0f, 1.0f);
		DG::float3 mMetallicFactor 		= DG::float3(1.0f, 1.0f, 1.0f);
		DG::float3 mDisplacementFactor 	= DG::float3(1.0f, 1.0f, 1.0f);
	};

	typedef entt::entity MaterialId;

	class Material {
	private:
		IRenderer* mRenderer;
		MaterialId mId;

	public:
		inline Material() : mRenderer(nullptr), mId(entt::null) {
		}

		inline Material(IRenderer* renderer, MaterialId id);
		inline ~Material();

		inline Material(const Material& mat);
		inline Material(Material&& mat);

		inline Material& operator=(const Material& mat);
		inline Material& operator=(Material&& mat);
	};

	class IRenderer {
	public:		
		// Must be called from main thread!
		virtual MaterialId CreateUnmanagedMaterial(const MaterialDesc& desc) = 0;

		// Thread Safe
		virtual void AddMaterialRef(MaterialId id) = 0;
		// Thread Safe
		virtual void ReleaseMaterial(MaterialId id) = 0;

		// Must be called from main thread!
		inline Material CreateMaterial(const MaterialDesc& desc) {
			MaterialId id = CreateUnmanagedMaterial(desc);
			Material mat(this, id);
			ReleaseMaterial(id);
			return mat;
		}
	};

	Material::Material(IRenderer* renderer, MaterialId id) :
		mRenderer(renderer), mId(id) {
		renderer->AddMaterialRef(id);
	}
	Material::~Material() {
		if (mId != entt::null) {
			mRenderer->ReleaseMaterial(mId);
		}
	}

	Material::Material(const Material& mat) :
		mRenderer(mat.mRenderer), mId(mat.mId) {
		mRenderer->AddMaterialRef(mId);
	}

	Material::Material(Material&& mat) : 
		mRenderer(mat.mRenderer),
		mId(mat.mId) {
		mat.mId = entt::null;
	}

	Material& Material::operator=(const Material& mat) {
		mRenderer = mat.mRenderer;
		mId = mat.mId;
		mRenderer->AddMaterialRef(mId);
		return *this;
	}

	Material& Material::operator=(Material&& mat) {
		std::swap(mat.mRenderer, mRenderer);
		std::swap(mat.mId, mId);
		return *this;
	}
}