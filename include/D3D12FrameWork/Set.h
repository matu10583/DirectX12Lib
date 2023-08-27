#pragma once
#include <concepts>
#include <type_traits>
#include "D3D12FrameWork/ShaderSet.h"

namespace D3D12FrameWork {
	template<class T>
	concept ConvertibleAsSet = std::is_enum_v<T>;
	template<class T>
	class Set
	{
		using U = typename std::underlying_type<T>::type;
	public:
		Set(ConvertibleAsSet auto const val)noexcept
			:m_val(static_cast<U>(val))
		{}
		~Set() = default;

		//グローバルな演算子のオーバーロード
		friend constexpr Set operator|(const Set lhs, const T rhs) { return cast(lhs.val() | cast(rhs)); }
		friend constexpr Set operator|(const T lhs, const Set rhs) { return cast(lhs | rhs.val()); }
		friend constexpr Set operator|(const Set lhs, const Set rhs) { return cast(lhs.val() | rhs.val()); }
		friend constexpr Set operator&(const Set lhs, const T rhs) { return cast(lhs.val() & cast(rhs)); }
		friend constexpr Set operator&(const T lhs, const Set rhs) { return cast(lhs & rhs.val()); }
		friend constexpr Set operator&(const Set lhs, const Set rhs) { return cast(lhs.val() & rhs.val()); }
		friend constexpr Set operator^(const Set lhs, const T rhs) { return cast(lhs.val() ^ cast(rhs)); }
		friend constexpr Set operator^(const T lhs, const Set rhs) { return cast(lhs ^ rhs.val()); }
		friend constexpr Set operator^(const Set lhs, const Set rhs) { return cast(lhs.val() ^ rhs.val()); }
		friend constexpr Set operator==(const Set lhs, const T rhs) { return cast(lhs.val() == cast(rhs)); }
		friend constexpr Set operator==(const T lhs, const Set rhs) { return cast(lhs == rhs.val()); }
		friend constexpr Set operator==(const Set lhs, const Set rhs) { return cast(lhs.val() == rhs.val()); }
		friend constexpr Set operator!=(const Set lhs, const T rhs) { return cast(lhs.val() != cast(rhs)); }
		friend constexpr Set operator!=(const T lhs, const Set rhs) { return cast(lhs != rhs.val()); }
		friend constexpr Set operator!=(const Set lhs, const Set rhs) { return cast(lhs.val() != rhs.val()); }

		//メンバオーバーロード
		constexpr Set operator~()const { return Set(cast(~m_val)); }
		constexpr T get()const { return cast(m_val); }

		Set& operator|=(const T rhs) { return m_val |= cast(rhs), *this; }
		Set& operator&=(const T rhs) { return m_val &= cast(rhs), *this; }
		Set& operator^=(const T rhs) { return m_val ^= cast(rhs), *this; }

		bool IsEmpty()const { return m_val == 0; }

	private:
		U m_val;

		constexpr U val()const { return static_cast<U>(m_val); }
		static constexpr U cast(const T v) { return static_cast<U>(v); }
		static constexpr T cast(const U v) { return static_cast<T>(v); }

	};


	template<ConvertibleAsSet T>
	constexpr Set<T> operator|(const T lhs, const T rhs) {
		using U = typename std::underlying_type<T>::type;
		return Set<T>(static_cast<T>(static_cast<U>(lhs) | static_cast<U>(rhs)));
	}
	template<ConvertibleAsSet T>
	constexpr Set<T> operator&(const T lhs, const T rhs) {
		using U = typename std::underlying_type<T>::type;
		return Set<T>(static_cast<T>(static_cast<U>(lhs) & static_cast<U>(rhs)));
	}
	template<ConvertibleAsSet T>
	constexpr Set<T> operator^(const T lhs, const T rhs) {
		using U = typename std::underlying_type<T>::type;
		return Set<T>(static_cast<T>(static_cast<U>(lhs) ^ static_cast<U>(rhs)));
	}
	template<ConvertibleAsSet T>
	constexpr Set<T> operator~(const T v) {
		using U = typename std::underlying_type<T>::type;
		return Set<T>(static_cast<T>(~static_cast<U>(v)));
	}
}


