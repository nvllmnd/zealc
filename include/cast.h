#pragma once


#include "intdefs.h"
#define cast(T, _src) ((__typeof__(T))(_src))

#define pcast(T, _ptr) (cast(__typeof__(T*), (_ptr)))



