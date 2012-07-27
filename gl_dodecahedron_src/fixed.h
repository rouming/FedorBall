/*******************************************************************************
 * Simple implementation of fixed point math
 ******************************************************************************/
#ifndef FIXED_H
#define FIXED_H

#include <stdint.h>

typedef int32_t    fp_t;
#define FPP        16
#define FPMUL(x,y)	((fp_t)(((int64_t)(x) * (int64_t)(y)) >> FPP))
#define FPDIV(x,y)	((fp_t)(((int64_t)(x) << FPP) / (y)))
#define FTOFP(x)    ((fp_t)((x) * ((fp_t)1<<FPP) + ((x) >= 0 ? 0.5 : -0.5)))
#define FPTOF(x)    ((float)(x) / ((fp_t)1<<FPP))
#define FPONE       ((fp_t)1<<FPP)
#define FPZERO      ((fp_t)0)

#endif //FIXED_H
