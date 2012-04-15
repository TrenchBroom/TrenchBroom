/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/

#pragma once
#ifndef GWEN_EXPORTS_H
#define GWEN_EXPORTS_H

//
// GWEN_COMPILE_DLL 
// - We're compiling the Gwen.DLL (or .dylib etc)
//
#if defined(GWEN_COMPILE_DLL)

#ifdef _WIN32
	#if defined(__GNUC__)
		#define GWEN_EXPORT __attribute__((dllexport))
	#else
		#define GWEN_EXPORT __declspec(dllexport)
	#endif
#endif 
//
// GWEN_COMPILE_STATIC
// - We're compiling gwen as a static library
//
#elif defined(GWEN_COMPILE_STATIC)

	#define GWEN_EXPORT

//
// GWEN_DLL
// - We're including gwen using the dll
//
#elif defined( GWEN_DLL )

#ifdef _WIN32
	#ifdef __GNUC__
		#define GWEN_EXPORT __attribute__((dllimport))
	#else
		#define GWEN_EXPORT __declspec(dllimport)
	#endif

	#ifdef _MSC_VER
		#ifndef _DEBUG
			#pragma comment ( lib, "gwen.lib" )
		#else
			#pragma comment ( lib, "gwend.lib" )
		#endif
	#endif
#endif

//
// - We're including gwen using a static library
//
#else

	#define GWEN_EXPORT

#ifdef _WIN32
	#ifdef _MSC_VER
		#ifndef _DEBUG
			#pragma comment ( lib, "gwen_static.lib" )
		#else
			#pragma comment ( lib, "gwend_static.lib" )
		#endif
	#endif
#endif

#endif

#ifndef GWEN_EXPORT 
	#define GWEN_EXPORT
#endif



#endif // GWEN_EXPORTS_H