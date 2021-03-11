#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ins.h"

double** read_txt_data(const char *file_name, int row, int col)
{
    FILE *fp;
	int i, j, k=0;
	double **data;
    fp = fopen(file_name, "r");

    if (fp==NULL){
        printf("\n errors when opening 2D geo file! Check the file path! \n");
    }

	data = (double**) calloc(row, sizeof *data);
	for (j=0;j<row;j++)
	{
		data[j] = (double*) calloc(col, sizeof *data[j]);
		for (i=0;i<col;i++)
		{
            k++;
			fscanf(fp, "%lf", &data[j][i]);
		}
	}
	//free(data[0]);
    //free(data);
	return data;
}

void interp_txt_data(double **data, FLOAT x, FLOAT y, FLOAT z, FLOAT *a, 
        int row, int col, FLOAT xllcorner, FLOAT yllcorner,
        FLOAT NODATA_VALUE, FLOAT dx, FLOAT dy)
{
	int i, j, ip1, jp1, i1;
	double a00, a01, a10, a11;
	//double wx, wy, val;
    int interp_points=4;
    double x1[col];
    double y1[row];
    double eps=1e-3;

    //if (y<0)
     //   y = -y;
    
    for (i1=0;i1<col;i1++)
		x1[i1] = i1*dx + xllcorner;

	for (i1=0;i1<row;i1++)
		y1[i1] = i1*dy + yllcorner;
	
	i = (int) (x - xllcorner)/dx;
	j = (int) (y - yllcorner)/dy;

    //phgPrintf("x, y %f %f\n", x, y);
    //phgPrintf("i, j %d %d\n", i, j);

    if (i<col-1 && j<row-1){

        //phgPrintf("i, j %d %d\n", i, j);
		
        a00 = data[row-j-2][i];
	    a01 = data[row-j-1][i];	    
	    a10 = data[row-j-2][i+1];	    
	    a11 = data[row-j-1][i+1];
		
        if (abs(a00-NODATA_VALUE)<eps){		
            a00 = 0;
            interp_points--;
            }
        if (abs(a01-NODATA_VALUE)<eps){		
            a01 = 0;
            interp_points--;
            }
        if (abs(a10-NODATA_VALUE)<eps){		
            a10 = 0;
            interp_points--;
            }
        if (abs(a11-NODATA_VALUE)<eps){
            a11 = 0;
            interp_points--;
            }
        
		//printf("a00 a01 a10 a11: %lf %lf %lf %lf %d %lf %lf\n", a00, a01, a10, a11, interp_points, x, y);
        //if (interp_points==0){
		//	printf("something terrible happened!\n");
		//}	


        if (interp_points > 0 && interp_points < 4){
            a[0] = (a00+a01+a10+a11)/interp_points;
            }
        
        else if (interp_points == 0){
            if (abs(data[row-j-2][i]-NODATA_VALUE)>eps){
                a[0] = data[row-j-2][i];
            }
            else if (abs(data[row-j][i]-NODATA_VALUE)>eps){
                a[0] = data[row-j][i];
            }
            else if (abs(data[row-j-1][i-1]-NODATA_VALUE)>eps){
                a[0] = data[row-j-1][i-1];
            }
            else if (abs(data[row-j-1][i+1]-NODATA_VALUE)>eps){
                a[0] = data[row-j-1][i+1];
            }
            else if (abs(data[row-j-2][i-1]-NODATA_VALUE)>eps){
                a[0] = data[row-j-2][i-1];
            }
            else if (abs(data[row-j-2][i+1]-NODATA_VALUE)>eps){
                a[0] = data[row-j-2][i+1];
            }
            else if (abs(data[row-j][i-1]-NODATA_VALUE)>eps){
                a[0] = data[row-j][i-1];
            }
            else if (abs(data[row-j][i+1]-NODATA_VALUE)>eps){
                a[0] = data[row-j][i+1];
            }
        }
        
        else{
            a[0]=1.0/((x1[i+1]-x1[i])*(y1[j]-y1[j+1]))*(a00*(x1[i+1]-x)*(y1[j]-y)+\
                    a10*(x-x1[i])*(y1[j]-y)+a01*(x1[i+1]-x)*(y-y1[j+1])+a11*(x-x1[i])*(y-y1[j+1]));
            }
    }
    else{
        if (j==row){
            a[0] = data[row-j][i];
        }
        else if (i==col){
            a[0] = data[row-j-1][i-1];
        }
        else{ 
            a[0] = data[row-j-1][i];
        }
    }
	
}	

double* read_txt_data_1D(char *file_name, int len)
{
    FILE *fp;
    double *data;
    int i;
    fp = fopen(file_name, "r");
    
    if (fp==NULL){
        printf("errors when opening 1D geo file!\n");
    }

    data = (double*) calloc(len, sizeof (*data));
    for (i=0; i<len; i++){
        fscanf(fp, "%lf", &data[i]);
    }

    fclose(fp);

    return data;
}


void interp_txt_data_1D(double *data, FLOAT x, FLOAT y, FLOAT z, FLOAT *a, 
        FLOAT x_start, FLOAT x_end, FLOAT dx, int len)
{
    int i;

    if (x==0){
        a[0] = data[0];
    }
    if (x>x_start && x<x_end){
        i = x/dx+1;
        a[0] = data[i] + (data[i+1]-data[i])*(x-dx*i)/dx;
    }
    if (x==x_end){
        a[0]=data[len-1];
    }


}
