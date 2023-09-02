#pragma once
#include <DirectXMath.h>
#include <limits>

namespace D3D12FrameWork {
	template<typename T>
	static inline bool AlmEqual(const T a, const T b) {
		assert(std::is_floating_point<T>());
		constexpr auto eps = std::numeric_limits<T>::epsilon();
		auto diff = (a - b) > (T)0 ? (a - b) : (b - a);
		return diff <= eps;
	}

	class Vector3 : public DirectX::XMFLOAT3
	{
	public:
		inline Vector3(float _x, float _y, float _z)
			:XMFLOAT3(_x, _y, _z) {
		}
		inline Vector3(float _num)
			: XMFLOAT3(_num, _num, _num) {
		}
		inline ~Vector3() {}

		inline static Vector3 One() {
			return Vector3(1, 1, 1);
		}
		inline static Vector3 Zero() {
			return Vector3(0, 0, 0);
		}
		inline static Vector3 Up() {
			return Vector3(0, 1, 0);
		}
		inline static Vector3 Right() {
			return Vector3(1, 0, 0);
		}
		inline static Vector3 Forward() {
			return Vector3(0, 0, 1);
		}

		inline float SqrMagnitude()const {
			return this->x * this->x + this->y * this->y + this->z * this->z;
		}
		inline float Magnitude()const {
			return sqrtf(this->SqrMagnitude());
		}
		inline Vector3& normalize() {
			float mag = this->Magnitude();
			*this = Vector3(
				this->x / mag, this->y / mag, this->z / mag
			);
			return *this;
		}
		inline Vector3 normal() {
			float mag = this->Magnitude();
			return Vector3(
				this->x / mag, this->y / mag, this->z / mag
			);
		}

		inline Vector3 operator-()const {
			return Vector3(
				-this->x,
				-this->y,
				-this->z
			);
		}

		inline Vector3& operator+=(Vector3 const& o) {
			*this = Vector3(
				this->x + o.x,
				this->y + o.y,
				this->z + o.z
			);
			return *this;
		}
		inline Vector3& operator-=(Vector3 const& o) {
			*this +=(-o);
			return *this;
		}
		inline Vector3& operator*=(Vector3 const& o) {
			*this = Vector3(
				this->x * o.x,
				this->y * o.y,
				this->z * o.z
			);
			return *this;
		}
		inline Vector3& operator/=(Vector3 const& o) {
			*this = Vector3(
				this->x / o.x,
				this->y / o.y,
				this->z / o.z
			);
			return *this;
		}

	private:

	};
}

inline D3D12FrameWork::Vector3 operator+(D3D12FrameWork::Vector3 const& v, D3D12FrameWork::Vector3 const& o) {
	return D3D12FrameWork::Vector3(v.x + o.x,
		v.y + o.y,
		v.z + o.z);
}
inline D3D12FrameWork::Vector3 operator-(D3D12FrameWork::Vector3 const& v, D3D12FrameWork::Vector3 const& o) {
	return D3D12FrameWork::Vector3(v.x - o.x,
		v.y - o.y,
		v.z - o.z);
}
inline D3D12FrameWork::Vector3 operator*(D3D12FrameWork::Vector3 const& v, D3D12FrameWork::Vector3 const& o) {
	return D3D12FrameWork::Vector3(v.x * o.x,
		v.y * o.y,
		v.z * o.z);
}
inline D3D12FrameWork::Vector3 operator/(D3D12FrameWork::Vector3 const& v, D3D12FrameWork::Vector3 const& o) {
	return D3D12FrameWork::Vector3(v.x / o.x,
		v.y / o.y,
		v.z / o.z);
}
inline D3D12FrameWork::Vector3 operator*(D3D12FrameWork::Vector3 const& v, float const& o) {
	return D3D12FrameWork::Vector3(v.x * o,
		v.y * o,
		v.z * o);
}
inline D3D12FrameWork::Vector3 operator/(D3D12FrameWork::Vector3 const& v, float const& o) {
	return D3D12FrameWork::Vector3(v.x / o,
		v.y / o,
		v.z / o);
}


inline bool operator==(D3D12FrameWork::Vector3 const& v, D3D12FrameWork::Vector3 const& o) {
	return D3D12FrameWork::AlmEqual<float>(v.x, o.x)
		&& D3D12FrameWork::AlmEqual<float>(v.y, v.y)
		&& D3D12FrameWork::AlmEqual<float>(v.z, v.z);
}

namespace D3D12FrameWork{
	class Vector4 : public DirectX::XMFLOAT4
	{
	public:
		inline Vector4(float _x, float _y, float _z, float _w)
			:XMFLOAT4(_x, _y, _z, _w) {
		}
		inline Vector4(float _num)
			: XMFLOAT4(_num, _num, _num, _num) {
		}
		inline ~Vector4() {}

		inline static Vector4 One() {
			return Vector4(1, 1, 1, 1);
		}
		inline static Vector4 Zero() {
			return Vector4(0, 0, 0, 0);
		}

		inline float SqrMagnitude()const {
			return this->x * this->x + this->y * this->y + this->z * this->z+ this->w * this->w;
		}
		inline float Magnitude()const {
			return sqrtf(this->SqrMagnitude());
		}
		inline Vector4& normalize() {
			float mag = this->Magnitude();
			*this = Vector4(
				this->x / mag, this->y / mag, this->z / mag, this->w / mag
			);
			return *this;
		}
		inline Vector4 normal() {
			float mag = this->Magnitude();
			return Vector4(
				this->x / mag, this->y / mag, this->z / mag, this->w / mag
			);
		}

		inline Vector4 operator -() const{
			return Vector4(
				-this->x,
				-this->y,
				-this->z,
				-this->w
			);
		}

		inline Vector4& operator+=(Vector4 const& o) {
			*this = Vector4(
				this->x + o.x,
				this->y + o.y,
				this->z + o.z,
				this->w + o.w
			);
			return *this;
		}
		inline Vector4& operator-=(Vector4 const& o) {
			*this += (-o);
			return *this;
		}
		inline Vector4& operator*=(Vector4 const& o) {
			*this = Vector4(
				this->x * o.x,
				this->y * o.y,
				this->z * o.z,
				this->w * o.w
			);
			return *this;
		}
		inline Vector4& operator/=(Vector4 const& o) {
			*this = Vector4(
				this->x / o.x, this->y / o.y, this->z / o.z, this->w / o.w
			);
			return *this;
		}

	private:

	};

}



inline D3D12FrameWork::Vector4 operator+(D3D12FrameWork::Vector4 const& v, D3D12FrameWork::Vector4 const& o) {
	return D3D12FrameWork::Vector4(v.x + o.x,
		v.y + o.y,
		v.z + o.z,
		v.w + o.w);
}
inline D3D12FrameWork::Vector4 operator-(D3D12FrameWork::Vector4 const& v, D3D12FrameWork::Vector4 const& o) {
	return D3D12FrameWork::Vector4(v.x - o.x,
		v.y - o.y,
		v.z - o.z,
		v.w - o.w);
}
inline D3D12FrameWork::Vector4 operator*(D3D12FrameWork::Vector4 const& v, D3D12FrameWork::Vector4 const& o) {
	return D3D12FrameWork::Vector4(v.x * o.x,
		v.y * o.y,
		v.z * o.z,
		v.w * o.w);
}
inline D3D12FrameWork::Vector4 operator/(D3D12FrameWork::Vector4 const& v, D3D12FrameWork::Vector4 const& o) {
	return D3D12FrameWork::Vector4(v.x / o.x,
		v.y / o.y,
		v.z / o.z,
		v.w / o.w);
}
inline D3D12FrameWork::Vector4 operator*(D3D12FrameWork::Vector4 const& v, float const& o) {
	return D3D12FrameWork::Vector4(v.x * o,
		v.y * o,
		v.z * o,
		v.w * o);
}
inline D3D12FrameWork::Vector4 operator/(D3D12FrameWork::Vector4 const& v, float const& o) {
	return D3D12FrameWork::Vector4(v.x / o,
		v.y / o,
		v.z / o,
		v.w / o);
}


inline bool operator==(D3D12FrameWork::Vector4 const& v, D3D12FrameWork::Vector4 const& o) {
	return D3D12FrameWork::AlmEqual<float>(v.x, o.x)
		&& D3D12FrameWork::AlmEqual<float>(v.y, v.y)
		&& D3D12FrameWork::AlmEqual<float>(v.z, v.z)
		&& D3D12FrameWork::AlmEqual<float>(v.w, v.w);
}
