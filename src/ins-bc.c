/*
 *  Boundary map & boundary condition.
 *
 *
 *
 *
 *  */
#include "ins.h"
#include "mat_op3.h"

/*********************/
/* static parameters */
/*********************/
static FLOAT Time, Re, nu;
static FLOAT Re = 1.0;
static FLOAT nu = 1.0;
static FLOAT Time0;
FLOAT nu_max;
FLOAT nu_min;



/* 
 * Wrapper for func w.r.t time.
 * */
#if USE_MOC
# define FUNC_T_WRAPPER(func_xyz)					\
    void								\
    func_xyz##_t(FLOAT x, FLOAT y, FLOAT z, FLOAT t, FLOAT *values)	\
    {									\
	FLOAT t_save = Time;						\
	setFuncTime(t);							\
	func_xyz(x, y, z, values);					\
	setFuncTime(t_save);						\
    }

FUNC_T_WRAPPER(func_u)
FUNC_T_WRAPPER(func_gradu)
FUNC_T_WRAPPER(func_p)
FUNC_T_WRAPPER(func_f)
//FUNC_T_WRAPPER(func_wind)
#undef FUNC_T_WRAPPER
#endif	/* USE_MOC */


void setFlowParameter(FLOAT Re_in, FLOAT nu_in, FLOAT time_in)
{
    Re = Re_in;
    nu = nu_in;
    Time = time_in;
}

void setFuncTime(FLOAT time_in)
{
    Time = time_in;
}

void adjust_time(FLOAT delta_time)
{
    Time0 = Time; 
    Time += delta_time;
    phgInfo(2, "* Adjust static Time: %E\n", Time);
}

void restore_time()
{
    Time = Time0;
    phgInfo(2, "* Restore static Time: %E\n", Time);
}

void									
func_xyz_(FLOAT x, FLOAT y, FLOAT z, FLOAT *values)	   
{
    *(values++) = x;
    *(values++) = y;
    *(values++) = z; 
}




#if TEST_CASE == LAS 
#  include "LAS.c"

#else
#  error Test case wrong!!!
#endif	/* TEST_CASE */
