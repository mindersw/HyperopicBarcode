#pragma once

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

/* C++ has a "bool" type built in. */
#ifndef __cplusplus
#ifndef HAVE_BOOL
#define HAVE_BOOL 1
typedef int bool;
#endif
#endif
