#ifndef RTYPE_EXPORT_HPP
#define RTYPE_EXPORT_HPP

/**
 * @file Export.hpp
 * @brief Macro for cross-platform symbol exportation and importation.
 *
 * When building a DLL/Shared Library, RTYPE_BUILD_DLL should be defined.
 * When consuming the DLL, it should NOT be defined.
 */

#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef RTYPE_BUILD_DLL
#define RTYPE_API __declspec(dllexport)
#else
#define RTYPE_API __declspec(dllimport)
#endif
#else
#if __GNUC__ >= 4
#define RTYPE_API __attribute__((visibility("default")))
#else
#define RTYPE_API
#endif
#endif

#endif  // RTYPE_EXPORT_HPP
