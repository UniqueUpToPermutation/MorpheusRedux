#include <Engine/Resources/Geometry.hpp>

namespace matball {
	#include <embed/matballmesh.hpp>
}

namespace box {
	#include <embed/boxmesh.hpp>
}

namespace bunny {
	#include <embed/bunnymesh.hpp>
}

namespace monkey {
	#include <embed/monkeymesh.hpp>
}

namespace plane {
	#include <embed/planemesh.hpp>
}

namespace sphere {
	#include <embed/spheremesh.hpp>
}

namespace torus {
	#include <embed/torusmesh.hpp>
}

namespace teapot {
	#include <embed/teapotmesh.hpp>
}

namespace Morpheus {

	Geometry Geometry::Prefabs::MaterialBall(const VertexLayout& layout) {
		return Geometry(layout, 
			matball::mVertexCount,
			matball::mIndexCount,
			matball::mIndices,
			matball::mPositions,
			matball::mUVs,
			matball::mNormals,
			matball::mTangents,
			matball::mBitangents);
	}

	Geometry Geometry::Prefabs::Box(const VertexLayout& layout) {
		return Geometry(layout, 
			box::mVertexCount,
			box::mIndexCount,
			box::mIndices,
			box::mPositions,
			box::mUVs,
			box::mNormals,
			box::mTangents,
			box::mBitangents);
	}

	Geometry Geometry::Prefabs::Sphere(const VertexLayout& layout) {
		return Geometry(layout, 
			sphere::mVertexCount,
			sphere::mIndexCount,
			sphere::mIndices,
			sphere::mPositions,
			sphere::mUVs,
			sphere::mNormals,
			sphere::mTangents,
			sphere::mBitangents);
	}

	Geometry Geometry::Prefabs::BlenderMonkey(const VertexLayout& layout) {
		return Geometry(layout, 
			monkey::mVertexCount,
			monkey::mIndexCount,
			monkey::mIndices,
			monkey::mPositions,
			monkey::mUVs,
			monkey::mNormals,
			monkey::mTangents,
			monkey::mBitangents);
	}

	Geometry Geometry::Prefabs::Torus(const VertexLayout& layout) {
		return Geometry (layout, 
			torus::mVertexCount,
			torus::mIndexCount,
			torus::mIndices,
			torus::mPositions,
			torus::mUVs,
			torus::mNormals,
			torus::mTangents,
			torus::mBitangents);
	}

	Geometry Geometry::Prefabs::Plane(const VertexLayout& layout) {
		return Geometry(layout, 
			plane::mVertexCount,
			plane::mIndexCount,
			plane::mIndices,
			plane::mPositions,
			plane::mUVs,
			plane::mNormals,
			plane::mTangents,
			plane::mBitangents);
	}

	Geometry Geometry::Prefabs::StanfordBunny(const VertexLayout& layout) {
		return Geometry(layout, 
			bunny::mVertexCount,
			bunny::mIndexCount,
			bunny::mIndices,
			bunny::mPositions,
			bunny::mUVs,
			bunny::mNormals,
			bunny::mTangents,
			bunny::mBitangents);
	}

	Geometry Geometry::Prefabs::UtahTeapot(const VertexLayout& layout) {
		return Geometry(layout, 
			teapot::mVertexCount,
			teapot::mIndexCount,
			teapot::mIndices,
			teapot::mPositions,
			teapot::mUVs,
			teapot::mNormals,
			teapot::mTangents,
			teapot::mBitangents);
	}
}