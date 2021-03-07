#pragma once

#include <Engine/Resources/RawGeometry.hpp>
#include <Engine/Resources/Resource.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Geometry.hpp>
#include <unordered_map>

namespace DG = Diligent;

namespace Assimp {
	class Importer;
}

struct aiScene;

namespace Morpheus {

	class GeometryResource;

	class GeometryResource : public IResource {
	private:
		DG::IBuffer* mVertexBuffer;
		DG::IBuffer* mIndexBuffer;

		uint mVertexBufferOffset;

		VertexLayout mLayout;

		DG::DrawIndexedAttribs mIndexedAttribs;
		DG::DrawAttribs mUnindexedAttribs;

		BoundingBox mBoundingBox;

		std::unordered_map<std::string, GeometryResource*>::iterator mIterator;

		void InitIndexed(DG::IBuffer* vertexBuffer, DG::IBuffer* indexBuffer,
			uint vertexBufferOffset, 
			const DG::DrawIndexedAttribs& attribs,
			const VertexLayout& layout,
			const BoundingBox& aabb);

		void Init(DG::IBuffer* vertexBuffer,
			uint vertexBufferOffset,
			const DG::DrawAttribs& attribs,
			const VertexLayout& layout,
			const BoundingBox& aabb);

		void LoadAssimp(const aiScene* scene,
			const VertexLayout& vertexLayout);

	public:

		GeometryResource(ResourceManager* manager);
		~GeometryResource();

		inline bool IsReady() const {
			return mVertexBuffer != nullptr;
		}

		inline DG::IBuffer* GetVertexBuffer() {
			return mVertexBuffer;
		}

		inline DG::IBuffer* GetIndexBuffer() {
			return mIndexBuffer;
		}

		inline uint GetVertexBufferOffset() const {
			return mVertexBufferOffset;
		}

		inline const VertexLayout& GetLayout() const {
			return mLayout;
		}

		inline DG::DrawIndexedAttribs GetIndexedDrawAttribs() const {
			return mIndexedAttribs;
		}

		inline DG::DrawAttribs GetDrawAttribs() const {
			return mUnindexedAttribs;
		}

		inline BoundingBox GetBoundingBox() const {
			return mBoundingBox;
		}

		entt::id_type GetType() const override {
			return resource_type::type<GeometryResource>;
		}

		GeometryResource* ToGeometry() override;

		friend class GeometryLoader;
		friend class RawGeometry;
		friend class ResourceCache<GeometryResource>;
	};

	class GeometryLoader {
	public:
		static void Load(DG::IRenderDevice* device, 
			const LoadParams<GeometryResource>& params,
			GeometryResource* loadinto);

		static TaskId LoadAsync(DG::IRenderDevice* device, 
			ThreadPool* pool,
			const LoadParams<GeometryResource>& params,
			const TaskBarrierCallback& callback,
			GeometryResource* loadinto);
	};

	template <>
	class ResourceCache<GeometryResource> : public IResourceCache {
	private:
		std::unordered_map<std::string, GeometryResource*> mResourceMap;
		GeometryLoader mLoader;
		ResourceManager* mManager;

		std::shared_mutex mMutex;

	public:
		ResourceCache(ResourceManager* manager);
		~ResourceCache();

		IResource* Load(const void* params) override;
		TaskId AsyncLoadDeferred(const void* params,
			ThreadPool* threadPool,
			IResource** output,
			const TaskBarrierCallback& callback = nullptr) override;
		void Add(IResource* resource, const void* params) override;
		void Unload(IResource* resource) override;
		void Clear() override;
	};
}