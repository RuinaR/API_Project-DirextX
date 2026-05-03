#pragma once

#if defined(ENGINEFRAMEWORK_DLL)
#if defined(ENGINEFRAMEWORK_EXPORTS)
#define ENGINEFRAMEWORK_API __declspec(dllexport)
#else
#define ENGINEFRAMEWORK_API __declspec(dllimport)
#endif
#else
#define ENGINEFRAMEWORK_API
#endif
