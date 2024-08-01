// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

// Set new base
//#pragma comment(linker, "/BASE:0x13140000")

#include <common/IPrefix.h>
#include <algorithm>
#include <intrin.h>

using namespace std;

#define XC_DECLARE_CONSTRUCTOR_HOOK(Class) \
	static Class *__ctor__(void *Instance) \
	{ \
		return new (Instance) Class(); \
	} \
	\
	static Class *__dtor__(Class *Thisptr, unsigned __int8) \
	{ \
		Thisptr->~Class(); \
		return Thisptr; \
	}

// thread-safe template versions of fastCall()

template<typename TR>
__forceinline TR fastCall(size_t reloff) noexcept
{
	return ((TR(__fastcall*)())(reloff))();
}

template<typename TR, typename T1>
__forceinline TR fastCall(size_t reloff, T1 a1) noexcept
{
	return ((TR(__fastcall*)(T1))(reloff))(a1);
}

template<typename TR, typename T1, typename T2>
__forceinline TR fastCall(size_t reloff, T1 a1, T2 a2) noexcept
{
	return ((TR(__fastcall*)(T1, T2))(reloff))(a1, a2);
}

template<typename TR, typename T1, typename T2, typename T3>
__forceinline TR fastCall(size_t reloff, T1 a1, T2 a2, T3 a3) noexcept
{
	return ((TR(__fastcall*)(T1, T2, T3))(reloff))(a1, a2, a3);
}

template<typename TR, typename T1, typename T2, typename T3, typename T4>
__forceinline TR fastCall(size_t reloff, T1 a1, T2 a2, T3 a3, T4 a4) noexcept
{
	return ((TR(__fastcall*)(T1, T2, T3, T4))(reloff))(a1, a2, a3, a4);
}

template<typename TR, typename T1, typename T2, typename T3, typename T4, typename T5>
__forceinline TR fastCall(size_t reloff, T1 a1, T2 a2, T3 a3, T4 a4, T5 a5) noexcept
{
	return ((TR(__fastcall*)(T1, T2, T3, T4, T5))(reloff))(a1, a2, a3, a4, a5);
}

template<typename TR, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
__forceinline TR fastCall(size_t reloff, T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6) noexcept
{
	return ((TR(__fastcall*)(T1, T2, T3, T4, T5, T6))(reloff))(a1, a2, a3, a4, a5, a6);
}

template<typename TR, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
__forceinline TR fastCall(size_t reloff, T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7) noexcept
{
	return ((TR(__fastcall*)(T1, T2, T3, T4, T5, T6, T7))(reloff))(a1, a2, a3, a4, a5, a6, a7);
}

template<typename TR, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
__forceinline TR fastCall(size_t reloff, T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8) noexcept
{
	return ((TR(__fastcall*)(T1, T2, T3, T4, T5, T6, T7, T8))(reloff))(a1, a2, a3, a4, a5, a6, a7, a8);
}

// thread-safe template versions of thisVirtualCall()

template<typename TR>
__forceinline TR thisVirtualCall(size_t reloff, const void* ths) {
	return (*(TR(__fastcall**)(const void*))(*(__int64*)ths + reloff))(ths);
}

template<typename TR, typename T1>
__forceinline TR thisVirtualCall(size_t reloff, const void* ths, T1 a1) {
	return (*(TR(__fastcall**)(const void*, T1))(*(__int64*)ths + reloff))(ths, a1);
}

template<typename TR, typename T1, typename T2>
__forceinline TR thisVirtualCall(size_t reloff, const void* ths, T1 a1, T2 a2) {
	return (*(TR(__fastcall**)(const void*, T1, T2))(*(__int64*)ths + reloff))(ths, a1, a2);
}

template<typename TR, typename T1, typename T2, typename T3>
__forceinline TR thisVirtualCall(size_t reloff, const void* ths, T1 a1, T2 a2, T3 a3) {
	return (*(TR(__fastcall**)(const void*, T1, T2, T3))(*(__int64*)ths + reloff))(ths, a1, a2, a3);
}

template<typename TR, typename T1, typename T2, typename T3, typename T4>
__forceinline TR thisVirtualCall(size_t reloff, const void* ths, T1 a1, T2 a2, T3 a3, T4 a4) {
	return (*(TR(__fastcall**)(const void*, T1, T2, T3, T4))(*(__int64*)ths + reloff))(ths, a1, a2, a3, a4);
}