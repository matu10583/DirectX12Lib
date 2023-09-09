#pragma once
#include <stdio.h>
#include <string>
#include <sstream>
#include <tchar.h>
#include <Windows.h>
#include <errno.h>
#include <memory>
#include <cassert>
#include "D3D12FrameWork/DX12Settings.h"
#include <exception>
#include <type_traits>

namespace D3D12FrameWork {

#if defined(DBUG) || defined(_DEBUG)
#define LOG_DEBUG(M,...)\
fprintf(stderr,"[DEBUG] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_ERR(M,...)\
fprintf(stderr,"[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, \
	 ##__VA_ARGS__)
#define THROWIFFAILED(HR)\
	if(FAILED(HR)){\
		throw std::exception();\
	}
#define RETURNIFFAILED(HR)\
	if(FAILED(HR)){\
		throw std::exception();\
		return false;\
	}
#else
#define LOG_ERR(M,...)
#define LOG_DEBUG(M,...)
#define THROWIFFAILED(HR)
#define RETURNIFFAILED(HR)\
	if(FAILED(HR)){\
		return false;\
}
#endif

#define CHECK(COND)\
	do{\
		if (!(COND)) {\
			LOG_ERR("Check failure: %s", #COND); \
			exit(-1);\
		}\
	} while (0);

#define CHECKM(COND,M,...)\
	do{\
		if (!(COND)) {\
			LOG_ERR("Check failure: %s \n" M, #COND, ##__VA_ARGS__); \
			exit(-1);\
		}\
	} while (0);
#define SAFEDELETE(PTR)\
	delete PTR;\
	PTR=nullptr;
#define SAFERELEASE(PTR)\
	PTR->Release();\
	PTR=nullptr;

#define GETSET(TYPE,Mem,mem)\
		TYPE& Get##Mem(){\
			return mem;\
		}\
		void Set##Mem(TYPE const& val){\
			mem =val;\
		}

#define GETSETPTR(TYPE,Mem,mem)\
		TYPE* Get##Mem(){\
			return mem;\
		}\
		void Set##Mem(type* const& val){\
			mem =val;\
		}

#define GETTER(TYPE,Mem,mem)\
		TYPE& Get##Mem()const{\
			return mem;\
		}
#define GETTERCONST(TYPE,Mem,mem)\
		TYPE const& Get##Mem()const{\
			return mem;\
		}

#define GETTERPTR(TYPE,Mem,mem)\
		TYPE* Get##Mem()const{\
			return mem;\
		}

#define SETTER(TYPE,Mem,mem)\
		void Set##Mem(TYPE const& val){\
			mem =val;\
		}

#define SETTERPTR(TYPE,Mem,mem)\
		void Set##Mem(type* const& val){\
			mem =val;\
		}

#define DECLMOVECOPY(T)\
	T(T const&)=default;\
	T& operator=(T const&)=default;\
	T(T&&)noexcept = default;\
	T& operator=(T&&)noexcept=default;

	//çƒãAìIÇ…å^ÇíTÇ∑
	class NULLTYPE {};
	template<typename T, typename Head, typename... Tail>
	struct FindType {
		static const bool value = FindType<T, Tail..., NULLTYPE>::value;
	};
	template<typename T, typename... Tail>
	struct FindType<T, T, Tail...> {
		static const bool value = true;
	};
	template<typename T>
	struct FindType<T, NULLTYPE> {
		static const bool value = false;
	};
}
