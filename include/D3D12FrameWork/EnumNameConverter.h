#pragma once
#include <string>
#include <array>
#include <limits>
#include <unordered_map>
#include <iostream>

namespace EnumNameConverter {

	//çïñÇèpÇ≈ééÇµÇƒÇ›ÇÈ
	template<size_t N>
	struct StaticString
	{

		constexpr StaticString(std::array<char, N>const& s_literal) {
			for (int i = 0; i < N; i++)buf[i] = s_literal[i];
			buf[length] = '\0';
		}

		char buf[N];
		static constexpr size_t length = N - 1;
	};

	template<size_t N, size_t Start, size_t Length>
	constexpr auto Substr(const char(&str)[N]) {
		constexpr size_t end = Start + Length;
		static_assert(end <= N, "Out of range");

		std::array<char, Length + 1> buf{};
		for (size_t i = 0; i < Length; i++) {
			buf[i] = str[Start + i];
		}
		buf[Length] = '\0';
		return buf;
	}


	constexpr auto Find(const char* _str, const char& _c, const size_t start = 0, const size_t end = static_cast<size_t>(-1)) {
		size_t idx = start;
		while (idx < end)
		{
			if (_str[idx] == _c) {
				return idx;
			}
			idx++;
		}
		return static_cast<size_t>(-1);
	}

	constexpr auto FindExcept(const char* _str, const char& _c, const size_t start = 0, const size_t end = static_cast<size_t>(-1)) {
		size_t idx = start;
		while (idx < end)
		{
			if (_str[idx] != _c) {
				return idx;
			}
			idx++;
		}
		return static_cast<size_t>(-1);
	}

	template<typename Enum>
	inline constexpr auto _cdecl GetTypeLength([[maybe_unused]] int = 0) {
#ifdef _MSC_VER
		constexpr char name[] = { __FUNCSIG__ };
		constexpr auto prefix_len = sizeof("auto __cdecl GetTypeLength<") - 1;
		auto opt_len = 0;
		if constexpr (std::string_view(name + prefix_len, name + prefix_len + 4) == std::string_view("enum")) {
			opt_len += sizeof("enum ") - 1;
		}
		constexpr auto suffix_len = sizeof(">(int)") - 1;
		if constexpr (std::string_view(name + sizeof(name) - 1 - 2) == std::string_view("=0")) {
			opt_len += sizeof("=0") - 1;
		}
#else 
		constexpr auto name[] = { __PRETTY_FUNCTION__ };
		constexpr auto prefix_len = sizeof("");
#endif
		return sizeof(name) - prefix_len - suffix_len - opt_len - 1;
	}


	template<typename Enum, Enum E>
	inline constexpr auto _cdecl EnumName([[maybe_unused]] int = 0) {
#ifdef _MSC_VER
		constexpr char name[] = { __FUNCSIG__ };
		constexpr auto prefix_idx = Find(name, ',', 0, sizeof(name));
		constexpr auto enumvalue_start = FindExcept(name, ' ', prefix_idx + 1, sizeof(name));
		constexpr auto enumvalue_end = Find(name, '>', enumvalue_start, sizeof(name)) - 1;

#else 
		constexpr auto name[] = { __PRETTY_FUNCTION__ };
		constexpr auto prefix_len = sizeof("");
#endif
		std::array<char, enumvalue_end - enumvalue_start + 2> ret = {};
		for (int i = 0; i < enumvalue_end - enumvalue_start + 1; i++) {
			ret[i] = name[i + enumvalue_start];
		}
		ret[enumvalue_end - enumvalue_start + 1] = '\0';


		return StaticString(ret);
	}



#define RANGE_MIN 0
	struct range_min :std::integral_constant<int, RANGE_MIN> {};
#define RANGE_MAX 256
	struct range_max :std::integral_constant<int, RANGE_MAX> {};


	template<typename T, typename U = std::underlying_type_t<T>>
	constexpr int Range_Min() noexcept {
		constexpr auto min = (std::numeric_limits<U>::min)();
		if constexpr (min < range_min::value) {
			return range_min::value;
		}
		else {
			return min;
		}
	}
	template<typename T, typename U = std::underlying_type_t<T>>
	constexpr int Range_Max() noexcept {
		constexpr auto max = (std::numeric_limits<U>::max)();
		if constexpr (max > range_max::value) {
			return range_max::value;
		}
		else {
			return max;
		}
	}

	template<int Start, int End, int...Seq>
	constexpr auto GenerateIntSequence() {
		if constexpr (Start <= End) {
			return GenerateIntSequence<Start, End - 1, End, Seq...>();
		}
		else {
			return std::make_integer_sequence<int, Seq...>();
		}
	}



	template<typename E, int V>
	constexpr bool IsValid() {
		constexpr auto val = static_cast<E>(V);
		constexpr auto name = EnumName<E, val>();
		if constexpr (std::is_same_v<decltype(name.buf), char[sizeof(name.buf)]> &&
			name.buf[0] != '(') {
			return true;
		}
		else {
			return false;

		}
	}

#define FOR256(M)\
M(0)M(1)M(2)M(3)M(4)M(5)M(6)M(7)M(8)M(9)\
M( 10)M( 11)M( 12)M( 13)M( 14)M( 15)M( 16)M( 17)M( 18)M( 19)\
M( 20)M( 21)M( 22)M( 23)M( 24)M( 25)M( 26)M( 27)M( 28)M( 29)\
M( 30)M( 31)M( 32)M( 33)M( 34)M( 35)M( 36)M( 37)M( 38)M( 39)\
M( 40)M( 41)M( 42)M( 43)M( 44)M( 45)M( 46)M( 47)M( 48)M( 49)\
M( 50)M( 51)M( 52)M( 53)M( 54)M( 55)M( 56)M( 57)M( 58)M( 59)\
M( 60)M( 61)M( 62)M( 63)M( 64)M( 65)M( 66)M( 67)M( 68)M( 69)\
M( 70)M( 71)M( 72)M( 73)M( 74)M( 75)M( 76)M( 77)M( 78)M( 79)\
M( 80)M( 81)M( 82)M( 83)M( 84)M( 85)M( 86)M( 87)M( 88)M( 89)\
M( 90)M( 91)M( 92)M( 93)M( 94)M( 95)M( 96)M( 97)M( 98)M( 99)\
M( 100)M( 101)M( 102)M( 103)M( 104)M( 105)M( 106)M( 107)M( 108)M( 109)\
M( 110)M( 111)M( 112)M( 113)M( 114)M( 115)M( 116)M( 117)M( 118)M( 119)\
M( 120)M( 121)M( 122)M( 123)M( 124)M( 125)M( 126)M( 127)M( 128)M( 129)\
M( 130)M( 131)M( 132)M( 133)M( 134)M( 135)M( 136)M( 137)M( 138)M( 139)\
M( 140)M( 141)M( 142)M( 143)M( 144)M( 145)M( 146)M( 147)M( 148)M( 149)\
M( 150)M( 151)M( 152)M( 153)M( 154)M( 155)M( 156)M( 157)M( 158)M( 159)\
M( 160)M( 161)M( 162)M( 163)M( 164)M( 165)M( 166)M( 167)M( 168)M( 169)\
M( 170)M( 171)M( 172)M( 173)M( 174)M( 175)M( 176)M( 177)M( 178)M( 179)\
M( 180)M( 181)M( 182)M( 183)M( 184)M( 185)M( 186)M( 187)M( 188)M( 189)\
M( 190)M( 191)M( 192)M( 193)M( 194)M( 195)M( 196)M( 197)M( 198)M( 199)\
M(200)M(201)M(202)M(203)M(204)M(205)M(206)M(207)M(208)M(209)\
M( 210)M( 211)M( 212)M( 213)M( 214)M( 215)M( 216)M( 217)M( 218)M( 219)\
M( 220)M( 221)M( 222)M( 223)M( 224)M( 225)M( 226)M( 227)M( 228)M( 229)\
M( 230)M( 231)M( 232)M( 233)M( 234)M( 235)M( 236)M( 237)M( 238)M( 239)\
M( 240)M( 241)M( 242)M( 243)M( 244)M( 245)M( 246)M( 247)M( 248)M( 249)\
M( 250)M( 251)M( 252)M( 253)M( 254)M( 255)\

	template<typename E, int Start, int Max>
	constexpr void ValidCount(int* _count) {
#define VALIDCOUNT(I)\
	if constexpr(Start+I<Max){\
		if constexpr (IsValid<E, Start + I>()) {\
				(*_count)++; \
		}\
	}

		FOR256(VALIDCOUNT);

		if constexpr (Start + 256 < Max) {
			ValidCount<E, Start + 256, Max>(_count);
		}
#undef VALIDCOUNT
	}

	template<typename E, int Start, int Max>
	constexpr auto ValidCount_v() {
		int count = 0;
		ValidCount<E, Start, Max>(&count);
		return count;
	}

	template<typename E, size_t N, int Start, int RangeMax>
	constexpr void EnumArray_Impl(std::array<E, N>* _enum_array, int _arrayIdx) {
		int idx = _arrayIdx;
#define VALIDCOUNT(I)\
	if constexpr(Start+I<RangeMax){\
		if constexpr (IsValid<E, Start + I>()) {\
			if(idx<N){\
				(*_enum_array)[idx] = static_cast<E>(Start + I); \
				idx++; \
			}\
		}\
	}

		FOR256(VALIDCOUNT);

		if constexpr (Start + 256 < RangeMax) {
			EnumArray_Impl<E, N, Start + 256, RangeMax>(_enum_array, idx);
		}
#undef VALIDCOUNT
	}
	template<typename Enum>
	constexpr auto MakeEnumArray() {
		constexpr auto min = Range_Min<Enum>();
		constexpr auto max = Range_Max<Enum>();
		constexpr auto vcount = ValidCount_v<Enum, min, max>();
		std::array<Enum, vcount> enum_array = {};
		EnumArray_Impl<Enum, vcount, min, max>(&enum_array, 0);

		return enum_array;
	}

	template<typename Enum>
	struct EnumArray {
		static constexpr auto value = MakeEnumArray<Enum>();
	};

	template<typename Enum>
	constexpr auto EnumArray_v() {
		return EnumArray<Enum>::value;
	}

	template<typename Enum, size_t Start, size_t Max>
	constexpr void MakeEnumDict_Impl(
		std::unordered_map<std::string, Enum>* _enum_map) {
		std::string name = {};
#define ADDDICT(I)\
	if constexpr(Start+I<Max){\
		if constexpr (IsValid<Enum, Start + I>()) {\
			constexpr auto val = static_cast<Enum>(Start + I); \
			(*_enum_map)[std::string(EnumName<Enum,val>().buf)]=val;\
		}\
	}

		FOR256(ADDDICT);

		if constexpr (Start + 256 < Max) {
			MakeEnumDict_Impl<Enum, Start + 256, Max>(_enum_map);
		}
#undef ADDDICT
	}

	template<typename Enum>
	constexpr auto MakeEnumDict() {
		std::unordered_map<std::string, Enum> ret = {};
		MakeEnumDict_Impl<Enum, 0, Range_Max<Enum>()>(&ret);
		return ret;
	}

	template<typename Enum>
	struct EnumDict
	{
		static inline const auto value = MakeEnumDict<Enum>();
	};

	template<typename Enum>
	constexpr auto EnumDict_v() {
		return EnumDict<Enum>::value;
	}

	template<typename Enum>
	constexpr bool EnumValue(std::string_view _str, Enum* _out) {
		auto name = std::string(_str.begin(), _str.end());
		if (EnumDict<Enum>::value.count(name)) {
			*_out = EnumDict<Enum>::value.at(name);
			return true;
		}
		return false;
	}

}
