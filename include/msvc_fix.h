#ifndef MSVC_FIX_H

#if defined(_MSC_VER)
	#define GB_PACKED_BEGIN __pragma(pack(push, 1))
	#define GB_PACKED_END   __pragma(pack(pop))
	#define GB_PACKED
#else
	#define GB_PACKED_BEGIN
	#define GB_PACKED_END
	#define GB_PACKED __attribute__((packed))
#endif

#endif