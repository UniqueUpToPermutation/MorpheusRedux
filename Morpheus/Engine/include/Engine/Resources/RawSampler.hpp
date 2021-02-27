#pragma once

#include <Engine/Resources/RawTexture.hpp>

#include <vector>
#include <cmath>

namespace DG = Diligent;

namespace Morpheus {
	class RawTexture;

	template <typename ReturnType, typename IndexType>
	class ISurfaceAdaptor {
	public:
		virtual void SampleLinearMip(IndexType x, IndexType dA, uint slice, ReturnType out[]) const = 0;
		virtual void SampleLinearMip(IndexType x, IndexType y, IndexType dA, uint slice, ReturnType out[]) const = 0;
		virtual void SampleLinearMip(IndexType x, IndexType y, IndexType z, IndexType dA, ReturnType out[]) const = 0;
		virtual void SampleLinear(IndexType x, uint mip, uint slice, ReturnType out[]) const = 0;
		virtual void SampleLinear(IndexType x, IndexType y, uint mip, uint slice, ReturnType out[]) const = 0;
		virtual void SampleLinear(IndexType x, IndexType y, IndexType z, uint mip, ReturnType out[]) const = 0;
		virtual void SampleCubeLinearMip(IndexType x, IndexType y, IndexType z, IndexType dA, uint slice, ReturnType out[]) const = 0;
		virtual void SampleCubeLinear(IndexType x, IndexType y, IndexType z, uint mip, uint slice, ReturnType out[]) const = 0;
		virtual void SampleCubeNearest(IndexType x, IndexType y, IndexType z, uint mip, uint slice, ReturnType out[]) const = 0;
		virtual void SampleNearest(IndexType x, uint mip, uint slice, ReturnType out[]) const = 0;
		virtual void SampleNearest(IndexType x, IndexType y, uint mip, uint slice, ReturnType out[]) const = 0;
		virtual void SampleNearest(IndexType x, IndexType y, IndexType z, uint mip, ReturnType out[]) const = 0;
	};

	enum class SurfaceWrapping {
		WRAP,
		CLAMP
	};

	template <typename IndexType>
	void WrapFloat(IndexType& t, SurfaceWrapping wrapType) {
		if (wrapType == SurfaceWrapping::CLAMP) {
			t = std::max<IndexType>(std::min<IndexType>(t, 1.0), 0.0);
		} else {
			t = t - std::floor(t);
		}
	}

	template <typename IndexType>
	struct CubemapHelper {
		constexpr static uint32_t BORDER_TOP = 0;
		constexpr static uint32_t BORDER_BOTTOM = 1;
		constexpr static uint32_t BORDER_LEFT = 2;
		constexpr static uint32_t BORDER_RIGHT = 3;

		constexpr static uint32_t FACE_POSITIVE_X = 0;
		constexpr static uint32_t FACE_NEGATIVE_X = 1;
		constexpr static uint32_t FACE_POSITIVE_Y = 2;
		constexpr static uint32_t FACE_NEGATIVE_Y = 3;
		constexpr static uint32_t FACE_POSITIVE_Z = 4;
		constexpr static uint32_t FACE_NEGATIVE_Z = 5;

		static IndexType Jacobian(IndexType u, IndexType v) {
			u = (u - 0.5) * 2.0;
			v = (v - 0.5) * 2.0;
			IndexType mag = u * u + v * v;
			return 1.0 / (mag * std::sqrt(mag));
		}

		static void ToUV(IndexType x, IndexType y, IndexType z, 
			IndexType* u, IndexType* v, uint* face) {
			IndexType x_abs = std::abs(x);
			IndexType y_abs = std::abs(y);
			IndexType z_abs = std::abs(z);
			IndexType u_;
			IndexType v_;

			if (x_abs >= y_abs && x_abs >= z_abs) {
				u_ = y / x_abs;
				v_ = z / x_abs;
				if (x >= 0.0) {
					*face = FACE_POSITIVE_X;
				} else {
					*face = FACE_NEGATIVE_X;
				}
			}
			else if (y_abs >= x_abs && y_abs >= z_abs) {
				u_ = x / y_abs;
				v_ = z / y_abs;
				if (y >= 0) {
					*face = FACE_POSITIVE_Y;
				} else {
					*face = FACE_NEGATIVE_Y;
				}
			}
			else {
				u_ = x / z_abs;
				v_ = y / z_abs;
				if (z >= 0) {
					*face = FACE_POSITIVE_Z;
				} else {
					*face = FACE_NEGATIVE_Z;
				}
			}

			*u = (u_ + 1.0) * 0.5;
			*v = (v_ + 1.0) * 0.5;
		}
	};

	struct WrapParameters {
		SurfaceWrapping mWrapTypeX;
		SurfaceWrapping mWrapTypeY;
		SurfaceWrapping mWrapTypeZ;

		static WrapParameters Default() {
			return WrapParameters{SurfaceWrapping::WRAP, SurfaceWrapping::WRAP, SurfaceWrapping::WRAP};
		}
	};

	class RawTextureSampler;

	template <typename T>
	ISurfaceAdaptor<T, T>* GetRawAdaptor(RawTextureSampler* sampler);

	template <>
	ISurfaceAdaptor<float, float>* GetRawAdaptor<float>(RawTextureSampler* sampler);
	template <>
	ISurfaceAdaptor<double, double>* GetRawAdaptor<double>(RawTextureSampler* sampler);

	class RawTextureSampler {
	private:
		std::unique_ptr<ISurfaceAdaptor<float, float>> mAdapterF;
		std::unique_ptr<ISurfaceAdaptor<double, double>> mAdapterD;

	public:
		RawTextureSampler(RawTexture* texture, 
			const WrapParameters& wrapping = WrapParameters::Default());

		template <typename T>
		inline void SampleLinearMip(T x, T dA, uint slice, T out[]) const {
			GetRawAdaptor<T>(this)->SampleLinearMip(x, dA, slice, out);
		}
		template <typename T>
		inline void SampleLinearMip(T x, T y, T dA, uint slice, T out[]) const {
			GetRawAdaptor<T>(this)->SampleLinearMip(x, y, dA, slice, out);
		}
		template <typename T>
		inline void SampleLinearMip(T x, T y, T z, T dA, T out[]) const {
			GetRawAdaptor<T>(this)->SampleLinearMip(x, y, z, dA, out);
		}
		template <typename T>
		inline void SampleLinear(T x, uint mip, uint slice, T out[]) const {
			GetRawAdaptor<T>(this)->SampleLinear(x, mip, slice, out);
		}
		template <typename T>
		inline void SampleLinear(T x, T y, uint mip, uint slice, T out[]) const {
			GetRawAdaptor<T>(this)->SampleLinear(x, y, mip, slice, out);
		}
		template <typename T>
		inline void SampleLinear(T x, T y, T z, uint mip, T out[]) const {
			GetRawAdaptor<T>(this)->SampleLinear(x, y, z, mip, out);
		}
		template <typename T>
		inline void SampleCubeLinearMip(T x, T y, T z, T dA, uint slice, T out[]) const {
			GetRawAdaptor<T>(this)->SampleCubeLinearMip(x, y, z, dA, slice, out);
		}
		template <typename T>
		inline void SampleCubeLinear(T x, T y, T z, uint mip, uint slice, T out[]) const {
			GetRawAdaptor<T>(this)->SampleCubeLinear(x, y, z, mip, slice, out);
		}
		template <typename T>
		inline void SampleCubeNearest(T x, T y, T z, uint mip, uint slice, T out[]) const {
			GetRawAdaptor<T>(this)->SampleCubeNearest(x, y, z, mip, slice, out);
		}
		template <typename T>
		inline void SampleNearest(T x, uint mip, uint slice, T out[]) const {
			GetRawAdaptor<T>(this)->SampleNearest(x, mip, slice, out);
		}
		template <typename T>
		inline void SampleNearest(T x, T y, uint mip, uint slice, T out[]) const {
			GetRawAdaptor<T>(this)->SampleNearest(x, y, mip, slice, out);
		}
		template <typename T>
		inline void SampleNearest(T x, T y, T z, uint mip, T out[]) const {
			GetRawAdaptor<T>(this)->SampleNearest(x, y, z, mip, out);
		}
		template <typename T>
		inline void SampleLinearMip(T x, T dA, T out[]) const {
			Sample(x, dA, 0, out);
		}
		template <typename T>
		inline void SampleLinearMip(T x, T y, T dA, T out[]) const {
			Sample(x, y, dA, 0, out);
		}
		template <typename T>
		inline void SampleLinear(T x, uint slice, T out[]) const {
			SampleLinear(x, 0, slice, out);
		}
		template <typename T>
		inline void SampleLinear(T x, T out[]) const {
			SampleLinear(x, 0, 0, out);
		}
		template <typename T>
		inline void SampleLinear(T x, T y, uint slice, T out[]) const {
			SampleLinear(x, y, 0, slice, out);
		}
		template <typename T>
		inline void SampleLinear(T x, T y, T out[]) const {
			SampleLinear(x, y, 0, 0, out);
		}
		template <typename T>
		inline void SampleLinear(T x, T y, T z, T out[]) const {
			SampleLinear(x, y, z, 0, out);
		}
		template <typename T>
		inline void SampleNearest(T x, uint slice, T out[]) const {
			SampleNearest(x, 0, slice, out);
		}
		template <typename T>
		inline void SampleNearest(T x, T y, uint slice, T out[]) const {
			SampleNearest(x, y, 0, slice, out);
		}
		template <typename T>
		inline void SampleNearest(T x, T out[]) const {
			SampleNearest(x, 0, 0, out);
		}
		template <typename T>
		inline void SampleNearest(T x, T y, T out[]) const {
			SampleNearest(x, y, 0, 0, out);
		}
		template <typename T>
		inline void SampleNearest(T x, T y, T z, T out[]) const {
			SampleNearest(x, y, z, 0, out);
		}
		template <typename T>
		inline void SampleCubeLinearMip(T x, T y, T z, T dA, T out[]) const {
			SampleCubeLinearMip(x, y, z, dA, 0, out);
		}
		template <typename T>
		inline void SampleCubeLinear(T x, T y, T z, T out[]) const {
			SampleCubeLinear(x, y, z, 0, 0, out);
		}
		template <typename T>
		inline void SampleCubeNearest(T x, T y, T z, T out[]) const {
			SampleCubeNearest(x, y, z, 0, 0, out);
		}
		template <typename T>
		inline void SampleCubeLinear(T x, T y, T z, uint slice, T out[]) const {
			SampleCubeLinear(x, y, z, 0, slice, out);
		}
		template <typename T>
		inline void SampleCubeNearest(T x, T y, T z, uint slice, T out[]) const {
			SampleCubeNearest(x, y, z, 0, slice, out);
		}
	};
}