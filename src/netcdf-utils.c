#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <netcdf.h>
#include "netcdf-utils.h"
//#include "ins.h"

#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); abort();}

#define NX 1200
#define NY 1200
#define NZ 11

typedef struct NC_DATA_ {
    float x1[NX];
    float y1[NY];
    float usrf[NY][NX];
    float dhdt[NY][NX];
    float surfvelmag[NY][NX];
    float lat[NY][NX];
    float balvelmag[NY][NX];
    float bheatflx[NY][NX];
    float presprcp[NY][NX];
    float lon[NY][NX];
    float surfvely[NY][NX];
    float surfvelx[NY][NX];
    float topg[NY][NX];
    float thk[NY][NX];
    float runoff[NY][NX];
    float smb[NY][NX];
    float airtemp2m[NY][NX];
    float surftemp[NY][NX];
    float temp[NY][NX];
    float *list[100];
} NC_DATA;

static NC_DATA data;
static float *active_data;
static float *x1, *y1, dx, dy;

#define READ_DATA_1D(var) {					\
	if ((retval = nc_inq_varid(ncid, #var, &varid)))	\
	    ERR(retval);					\
	if ((retval = nc_get_var_float(ncid, varid,		\
				       &data.var[0]))) {	\
	    ERR(retval);					\
	} else	{						\
	    printf("   Read %-15s done.\n", #var);		\
	    data.list[data_index_##var] = &data.var[0];		\
	}							\
    }

#define READ_DATA_2D(var) {					\
	if ((retval = nc_inq_varid(ncid, #var, &varid)))	\
	    ERR(retval);					\
	if ((retval = nc_get_var_float(ncid, varid,		\
				       &data.var[0][0]))) {	\
	    ERR(retval);					\
	} else	{						\
	    printf("   Read %-15s done.\n", #var);		\
	    data.list[data_index_##var] = &data.var[0][0];	\
	}							\
    }

#define READ_DATA_3D(var) {					\
	if ((retval = nc_inq_varid(ncid, #var, &varid)))	\
	    ERR(retval);					\
	if ((retval = nc_get_var_float(ncid, varid,		\
				       &data.var[0][0][0]))) {	\
	    ERR(retval);					\
	} else	{						\
	    printf("   Read %-15s done.\n", #var);		\
	    data.list[data_index_##var] = &data.var[0][0][0];	\
	}							\
    }



void
read_nc_data(char *file_name)
{
    int retval;
    int ncid, varid;

    /* Open the file. */
    if ((retval = nc_open(file_name, NC_NOWRITE, &ncid)))
	ERR(retval);

    READ_DATA_1D(x1); x1 = data.x1;
    READ_DATA_1D(y1); y1 = data.y1;
    dx = (x1[1] - x1[0]);
    dy = (y1[1] - y1[0]);

    /* READ_DATA_2D(dhdt); */
    /* READ_DATA_2D(surfvelmag); */
    /* READ_DATA_2D(lat); */
    /* READ_DATA_2D(balvelmag); */
    /* READ_DATA_2D(bheatflx); */
    /* READ_DATA_2D(presprcp); */
    /* READ_DATA_2D(lon); */
     READ_DATA_2D(usrf); 
    /* READ_DATA_2D(surfvely); */
    /* READ_DATA_2D(surfvelx); */
    READ_DATA_2D(topg);
    READ_DATA_2D(thk);
    /* READ_DATA_2D(runoff); */
    /* READ_DATA_2D(smb); */
    /* READ_DATA_2D(airtemp2m); */
    /* READ_DATA_2D(surftemp); */
    READ_DATA_2D(temp);

    if ((retval = nc_close(ncid)))
	ERR(retval);
     
    printf("*** SUCCESS reading netcdf data file: %s.\n", 
	   file_name);
    return;
}

void 
nc_data_set_active(int var_index) 
{
    assert(var_index <= 18 && var_index >= 1);
    active_data = data.list[var_index];
    return;
}


/*
 * Bilinear interpolation of netCDF data.
 * Note: input coord is of computaional region,
 *       output data is scaled.
 * */
double nc_data_scaling = 1.;
double nc_length_scale = 1.;
void
interp_nc_data_3D(double x, double y, double z, int level, double *a)
{
    int i, j;
    double a00, a01, a10, a11;
    double d00, d01, d10, d11;
    double wx, wy;

    x *= nc_length_scale;
    y *= nc_length_scale;
    z *= nc_length_scale;

#if 0
#else
    /* relative position */
    i = (int) (x-x1[0]) / dx ;
    j = (int) (y-y1[0]) / dy;

    /* only interior region, no interp near margin */
    assert(i < NX-1);
    assert(j < NY-1);
#endif

    /* Bilinear interpolation */
    a00 = active_data[level*NX*NY + j*NX + i];
    a01 = active_data[level*NX*NY + (j+1)*NX + i];
    a10 = active_data[level*NX*NY + j*NX + (i+1)];
    a11 = active_data[level*NX*NY + (j+1)*NX + (i+1)];

#if 0
    dx = x[i+1] - x[i];
    dy = y[j+1] - y[j];
#endif


#if 0
    /* check */
    a00 = (x) + 2*(y);
    a10 = (x+dx) + 2*(y);
    a01 = (x) + 2*(y+dy);
    a11 = (x+dx) + 2*(y+dy);
#endif

    /*
    wx = (x - (x1[i] - x1[0])) / dx;
    wy = (y - (y1[j] - y1[0])) / dy;
    a[0] = a00 * (1-wx) * (1-wy) 
	+ a01 * (1-wx) * wy
	+ a10 * wx * (1-wy)
	+ a11 * wx * wy;
    */

    d00 = sqrt((x-x1[i])*(x-x1[i])+(y-y1[j])*(y-y1[j]));
    d01 = sqrt((x-x1[i])*(x-x1[i])+(y-y1[j+1])*(y-y1[j+1]));
    d10 = sqrt((x-x1[i+1])*(x-x1[i+1])+(y-y1[j])*(y-y1[j]));
    d11 = sqrt((x-x1[i+1])*(x-x1[i+1])+(y-y1[j+1])*(y-y1[j+1]));

    a[0] = (a00/d00+a01/d01+a10/d10+a11/d11)/(1/d00+1/d01+1/d10+1/d11);

    a[0] *= nc_data_scaling;
    return;
}
