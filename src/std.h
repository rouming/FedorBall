/**************************************************************************
 * Some generic defines
 **************************************************************************/
#ifndef STD_H
#define STD_H

#define __CONCAT_NAME(x,y) x##y
#define CONCAT_NAME(x,y) __CONCAT_NAME(x,y)

/* Validates constant statement in compile time */
#define BUILD_BUG_ON(condition)											\
	extern void __attribute__((unused))									\
		CONCAT_NAME(__build_bug_on_dummy,								\
					__LINE__)(char a[1 - 2*!!(condition)])

#endif //STD_H
