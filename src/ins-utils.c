/*
 *  Usefull subroutines.
 *
 *
 *
 *  */
#include "ins.h"


/* Print Macro defination */
void 
NsSolver_Options() 
{
    /* Information of solver */
    phgPrintf("\n===============================================\n");
    phgPrintf(" Incompressible Navier-stokes Solver options:  \n\n");
#if STEADY_STATE
#warning NS: steady state
    phgPrintf("    STEADY-STATE case   \n");
#elif TIME_DEP_NON
#warning NS: time depentdent
    phgPrintf("    TIME-DEPENDENT case   \n");
#endif	/* STEADY_STATE */
    phgPrintf("   Problem: "NS_PROBLEM"\n");
    phgPrintf("===============================================\n\n");

    phgPrintf("Scaling:\n");
    phgPrintf("   equ : %e\n", EQU_SCALING);
    phgPrintf("   len : %e\n", LEN_SCALING);
    phgPrintf("   pres: %e\n", PRES_SCALING);

#warning netCDF disabled
    //nc_data_scaling = LEN_SCALING;

#if USE_SLIDING_BC
    phgPrintf("Sliding BC.\n");
#else
    phgPrintf("No sliding BC.\n");
#endif


#if USE_SYMETRIC
    assert(ns_params->use_symetric);
#else
    assert(!ns_params->use_symetric);
#endif

    return;
}


double
elapsed_time(GRID *g, BOOLEAN flag, double mflops)
/* returns and prints elapsed time since last call to this function */
{
    static double t0 = 0.0;
    double et, tt[3];
    size_t mem;

    phgGetTime(tt);
    et = tt[2] - t0;
    t0 = tt[2];

    mem = phgMemoryUsage(g, NULL);

    if (flag) {
	if (mflops <= 0)
	    phgPrintf("[%0.4lgMB %0.4lfs]\n", mem / (1024.0 * 1024.0), et);
	else
	    phgPrintf("[%0.4lgMB %0.4lfs %0.4lgGF]\n",
		      mem / (1024.0 * 1024.0), et, mflops * 1e-3);
    }

    return et;
}


/* Dof norm for curved elements */
FLOAT
dofNormL2(DOF *dof)
{
    GRID *g = dof->g;
    SIMPLEX *e;
    FLOAT norm = 0;
    FLOAT a = 0, b = 0;
    int i, dim = DofDim(dof);
    
    ForAllElements(g, e) {
	QUAD *quad = phgQuadGetQuad3D(2*DofTypeOrder(dof, e));
	FLOAT vol, det, v[dim];
	const FLOAT *p, *w;
	int q;
	
	p = quad->points;
 	w = quad->weights;
	for (q = 0; q < quad->npoints; q++) {
	    phgGeomGetCurvedJacobianAtLambda(g, e, p, &det);
	    vol = fabs(det / 6.);
	    phgDofEval(dof, e, p, v);
	    for (i = 0; i < dim; i++)
		norm += vol*(*w) * (v[i] * v[i]);
	    w++; p += Dim+1;
	}
    }

    a = norm;
    MPI_Allreduce(&a, &b, 1, MPI_DOUBLE, MPI_SUM, g->comm);
    norm = sqrt(b);
    
    return norm;
}


#define MAX_QUAD_ORDER 12
void
dof_norm_L2(DOF *dof)
{
    GRID *g = dof->g;
    SIMPLEX *e;
    int i, dim = DofDim(dof);
    FLOAT a[dim], b[dim], norm[dim];

    Bzero(norm);
    ForAllElements(g, e) {
	QUAD *quad;
	FLOAT vol, det, v[dim];
	const FLOAT *p, *w;
	int q, order = DofTypeOrder(dof, e) * 2;
	
	if (order > MAX_QUAD_ORDER)
	    order = MAX_QUAD_ORDER;
	quad = phgQuadGetQuad3D(order);
	p = quad->points;
 	w = quad->weights;
	for (q = 0; q < quad->npoints; q++) {
	    phgGeomGetCurvedJacobianAtLambda(g, e, p, &det);
	    vol = fabs(det / 6.);
	    phgDofEval(dof, e, p, v);
	    for (i = 0; i < dim; i++)
		norm[i] += vol*(*w) * (v[i] * v[i]);
	    w++; p += Dim+1;
	}
    }

    memcpy(a, norm, dim * sizeof(FLOAT));
    MPI_Allreduce(&a, &b, dim, MPI_DOUBLE, MPI_SUM, g->comm);

    phgPrintf("   Dof %s normL2\n", dof->name);
    for (i = 0; i < dim; i++) {
	norm[i] = sqrt(b[i]);
	phgPrintf("      %12.4E\n", norm[i]);
    }
    
    return;
}

void dof_range(DOF *u)
{
    GRID *g = u->g;
    int dim = u->dim;
    INT i, k, N;
    FLOAT max[dim*2], min[dim], tmp[2*dim];
    const FLOAT *v;

    for (k = 0; k < dim; k++) {
	max[k] = -1e32;
	min[k] = 1e32;
    }
    
    v = DofData(u);
    N = DofGetDataCount(u);
    for (i = 0; i < N / dim; i++) {
	for (k = 0; k < dim; k++) {
	    max[k] = MAX(max[k], *v);
	    min[k] = MIN(min[k], *v);
	    v++;
	}
    }

    for (k = 0; k < dim; k++) 
	max[k + dim] = - min[k];
    memcpy(tmp, max, 2 * dim * sizeof(FLOAT));
    MPI_Allreduce(&tmp, &max, 2 * dim, MPI_DOUBLE, MPI_MAX, g->comm);
    for (k = 0; k < dim; k++) 
	min[k]  = - max[k + dim];
    

    if (dim == 1) {
 	phgPrintf("   %-5s: [%24.12e, %24.12e]\n", 
		  u->name, min[0], max[0]); 
    } else {
	for (k = 0; k < dim; k++) {
	    if (k == 0)
		phgPrintf("   %-5s[0]: [%24.12e, %24.12e]\n", 
			  u->name, min[k], max[k]); 
	    else
		phgPrintf("          [%d]: [%24.12e, %24.12e]\n", 
			  k, min[k], max[k]); 
	}
    }
}



/* Symetric check:
 * Use random vector x, y check whether Mat and PC is symetric,
 *   if they are, then x'*A*y == y'*A*x.
 *   This is NOT a good way, since rounding error may be not small.
 * */
void sym_check(NSSolver *ns)
{
    if (0 && ns_params->use_symetric) {
	VEC *xup = phgMapCreateVec(ns->solver_u->rhs->map, 1);
	VEC *yup, *zup;
	FLOAT product[2], p0, test;
		
	yup = phgVecCopy(xup, NULL);
	zup = phgVecCopy(xup, NULL);
	phgVecRandomize(xup, 0);
	phgVecRandomize(yup, 0);
		
	phgMatVec(MAT_OP_N, 1.0, ns->solver_u->mat, xup, 0., &zup);
	phgVecDot(yup, 0, zup, 0, product);
	phgMatVec(MAT_OP_N, 1.0, ns->solver_u->mat, yup, 0., &zup);
	phgVecDot(xup, 0, zup, 0, product+1);
		
	if (fabs(product[0]) > 1e-10) 
	    p0 = product[0];
	else if (fabs(product[1]) > 1e-10) 
	    p0 = product[1];
	else
	    p0 = 1.;
	test = fabs((product[0] - product[1]) / p0);
	if (test > 1e-10) {
	    phgError(-1, "Solver matrix may be unsymetric!!!\n random vec test:(%E, %E), diff:%E\n", 
		     product[0], product[1], test);
	}
		
	if (ns_params->use_PCD) {
	    pc_proc(ns->solver_u, xup, &zup);
	    phgVecDot(yup, 0, zup, 0, product);
	    pc_proc(ns->solver_u, yup, &zup);
	    phgVecDot(xup, 0, zup, 0, product+1);
		
	    if (fabs(product[0]) > 1e-10) 
		p0 = product[0];
	    else if (fabs(product[1]) > 1e-10) 
		p0 = product[1];
	    else
		p0 = 1.;
	    test = fabs((product[0] - product[1]) / p0);
	    if (test > 1e-10) {
		phgPrintf("Dumping sym mat vec\n");
		printf("p:%E, %E\n", product[0], product[1]);
		phgVecDumpMATLAB(xup, "vx", "vx_.m");
		phgVecDumpMATLAB(yup, "vy", "vy_.m");
		phgVecDumpMATLAB(ns->pcd->rhsScale, "vs", "vs_.m");
		phgMatDumpMATLAB(ns->matF, "F", "F_.m");
		phgMatDumpMATLAB(ns->pcd->matQp, "Qp", "Qp_.m");
		phgMatDumpMATLAB(ns->pcd->matQp, "Qp", "Qp_.m");
		phgMatDumpMATLAB(ns->pcd->matAp, "Ap", "Ap_.m");
		phgMatDumpMATLAB(ns->pcd->matFp, "Fp", "Fp_.m");
		if (ns_params->use_symetric)
		    phgMatDumpMATLAB(ns->pcd->matFu, "Fu", "Fu_.m");
		phgError(-1, "PC matrix may be unsymetric!!!\n random vec test:(%E, %E), diff:%E\n", 
			 product[0], product[1], test);
	    }
	}
    }
}


/***************************/
/* Check all kinds of bdry */
/***************************/

void
checkBdry(GRID *g)
{
    SIMPLEX *e;
    int s;
    double a[8];

    Bzero(a);
    ForAllElements(g, e) {
        a[7] += phgGeomGetVolume(g, e);
        for (s = 0; s < NFace; s++) {
            FLOAT area = phgGeomGetFaceArea(g, e, s);
            if (e->bound_type[s] & BDRY_MASK) {
                if (e->bound_type[s] & INFLOW)
                    a[0] += area;
                if (e->bound_type[s] & OUTFLOW)
                    a[1] += area;
            }
            else {
                a[6] += area;
            }
        }
    }

#if USE_MPI
    {
        double b[8];
        MPI_Reduce(a, b, 8, MPI_DOUBLE, MPI_SUM, 0, g->comm);
        memcpy(a, b, sizeof(b));
    }
#endif

    phgPrintf("Boundary types check:\n");
    phgPrintf("\tInflow : %20.10e\n\tOutflow: %20.10e\n\tNoflow : %20.10e\n"
              "\tsphere : %20.10e\n\tCyl    : %20.10e\n"
              "\tOther  : %20.10e\n\tAll    : %20.10e\n\tVolume : %20.10e \n",
              a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);

    return;
}


void 
getPecletNum(GRID *g, DOF *u, FLOAT nu, int order)	
/* 
 * Mesh Peclet number:
 * Pe := U * h / nu,
 * where, U = ||u||, h = diam of circumsphere of tetra as h.
 * This suggest that the wind direction coincides with the direction
 * of max length of the element.
 * 
 * */
{
    SIMPLEX *e;
    int q;
    FLOAT pe, pe0, diam, maxu, tmp;

    assert(u->dim == Dim);
    pe0 = -1;
    ForAllElements(g, e) {
	QUAD *quad;
	const FLOAT *vu;
	
	diam = phgGeomGetDiameter(g, e);
	quad = phgQuadGetQuad3D(order);
	vu = phgQuadGetDofValues(e, u, quad);
	maxu = -1;
	for (q = 0; q < quad->npoints; q++) {
	    tmp = INNER_PRODUCT(vu, vu);
	    maxu = MAX(maxu, tmp);
	    vu += Dim;
	}
	tmp = sqrt(maxu) * diam;
	pe0 = MAX(pe0, tmp);
    }

    MPI_Allreduce(&pe0, &pe, 1, MPI_DOUBLE, MPI_MAX, g->comm);
    pe /= nu;
    phgPrintf("* Mesh Peclet num: %16.8E\n", pe);
    if (pe > 2.)
	phgPrintf("* Pe num %16.8E > 2, FEM may get unstable!\n", pe);

    return;
}





FLOAT get_ice_volume(GRID *g)
{
    SIMPLEX *e;

    FLOAT Vol = 0;
    ForAllElements(g, e)
    {
        Vol += phgGeomGetVolume(g, e);
    }
    FLOAT tmp = Vol;
    MPI_Allreduce(&tmp, &Vol, 1, MPI_DOUBLE, MPI_SUM, g->comm);

    return Vol;
}


void get_avg_gu(NSSolver *ns)
{
    GRID *g = ns->g;
    SIMPLEX *e;
    int i, j, k, l, s, q;

    DOF *gu[DDim];
    MAP *map_gu[DDim];
    VEC *vec_gu[DDim];

    DOF *u_P1 = phgDofCopy(ns->u[1], NULL, DOF_P1, NULL);
    phgExportVTK(g, "u_P1.vtk", u_P1, ns->p[1], NULL);

    DOF *gu_P0 = phgDofGradient(u_P1, NULL, NULL, NULL);

    DOF *gu_all = phgDofNew(g, DOF_P1, DDim, "gradu_all", DofInterpolation);

    for (k = 0; k <DDim; k++)
    {
        gu[k] = phgDofNew(g, DOF_P1, 1, "gradu", DofNoAction);
        map_gu[k] = phgMapCreate(gu[k], NULL);
        vec_gu[k] = phgMapCreateVec(map_gu[k], 1);
        phgVecDisassemble(vec_gu[k]);
    }


    DOF *count = phgDofNew(g, DOF_P1, 1, "count", DofNoAction);
    MAP *map_count = phgMapCreate(count, NULL);
    VEC *vec_count = phgMapCreateVec(map_count, 1);
    phgVecDisassemble(vec_count);

    ForAllElements(g, e)
    {
        int M = count->type->nbas;	
        FLOAT rhs[DDim][M], rhs1[M];
        INT local_map_idx[M];
        INT local_map_idx1[M];

        memset(rhs, 0, sizeof rhs); 
        memset(rhs1, 0, sizeof rhs1); 

        FLOAT *gue = DofElementData(gu_P0, e->index);

        for (i = 0; i < M; i++)
            rhs1[i] += 1;

        for (k = 0; k < DDim; k++)
        {
            for (i = 0; i < M; i++)
            {
                rhs[k][i] += gue[k];
            }
        }


        for (i = 0; i < M; i++)
        {
            local_map_idx[i] = phgMapE2L(map_gu[0], 0, e, i);
            local_map_idx1[i] = phgMapE2L(map_count, 0, e, i);
            
        }


        for (k = 0; k < DDim; k++)
            phgVecAddEntries(vec_gu[k], 0, M, local_map_idx, &rhs[k][0]);

        phgVecAddEntries(vec_count, 0, M, local_map_idx1, &rhs1[0]);

    }

    for (k = 0; k < DDim; k++)
    {
        phgVecAssemble(vec_gu[k]);
        phgMapLocalDataToDof(map_gu[k], 1, &gu[k], vec_gu[k]->data);
    }

    phgVecAssemble(vec_count);
    phgMapLocalDataToDof(map_count, 1, &count, vec_count->data);
    
    FLOAT *data_all = DofData(gu_all);
    FLOAT n;
    FLOAT *data[DDim], *data1;
    INT len, len1;

    for (k = 0; k < DDim; k++)
    {
        data[k] = DofData(gu[k]);
        data1 = DofData(count);
        len = DofGetDataCount(gu[k]);
        len1 = DofGetDataCount(count);
        assert(len == len1);

        for (i = 0; i < len; i++)
        {

            n = data1[i];

            data[k][i] = data[k][i]/n;

            //data_all[i*DDim+k] = data[k][i];

        }
    }

    ForAllElements(g, e)
    {
        for (i = 0; i < Dim+1; i++)
        {

            INT local_idx = e->verts[i];
            for (k = 0; k < DDim; k++)
            DofVertexData(gu_all, local_idx)[k] = 
                DofVertexData(gu[k], local_idx)[0];
        }
    }
   
    //phgMapDestroy(&map_gu);
    //phgVecDestroy(&vec_gu);

    //phgExportVTK(g, "avg_gu.vtk", gu[0], gu[1], gu[2], gu[3], gu[4], gu[5], gu[6], gu[7], gu[8], NULL);
	//phgDofCopy(gu, &ns->gu, NULL, "gu");
    phgDofCopy(gu_all, &ns->avg_gu, NULL, "avg_gu");

    for (k = 0; k < DDim; k++)
    {
        phgDofFree(&gu[k]);
        phgMapDestroy(&map_gu[k]);
        phgVecDestroy(&vec_gu[k]);
    }
    phgDofFree(&u_P1);
    phgDofFree(&gu_P0);
    phgDofFree(&gu_all);
    phgDofFree(&count);
    phgMapDestroy(&map_count);
    phgVecDestroy(&vec_count);

}



DOF * compare_two_dofs(DOF *a, DOF *b)
{
    
    DOF *c = phgDofCopy(a, NULL, NULL, NULL);

    FLOAT *va = DofData(a);
    FLOAT *vb = DofData(b);
    FLOAT *vc = DofData(c);

    INT len = DofGetDataCount(a);
    INT i;

    for (i = 0; i < len; i++)
    {
        if (va[i] != 0)
            vc[i] = (va[i] - vb[i])/va[i];
        else
            vc[i] = 0;
    }

    return c;

}

DOF * get_dof_component(GRID *g, DOF *a, DOF_TYPE *dof_type, INT dim_all, INT dim_asked)
{
    INT i, k;

    FLOAT *v = DofData(a);
    INT len = DofGetDataCount(a)/dim_all;

    DOF *cmpn = phgDofNew(g, dof_type, 1, "cmpn", DofNoAction);
    FLOAT *v1 = DofData(cmpn);

    for (i = 0; i < len; i++)
    {
        for (k = 0; k < dim_all; k++)
        {
            if (k == dim_asked)
                v1[i] = v[i*dim_all+k];
        }
    }

    return cmpn;
}



void get_avg_n(GRID *g, DOF *sur_normal)
{
    SIMPLEX *e;
    int i, j, k, l, s, q;
    int i_face, j_face;

    MAP *map_n = phgMapCreate(sur_normal, NULL);
    VEC *vec_n = phgMapCreateVec(map_n, Dim);
    phgVecDisassemble(vec_n);

    ForAllElements(g, e)
    {
        int M = sur_normal->type->nbas;	
        FLOAT rhs[M][Dim];
        BOOLEAN btype[M][Dim];
        INT local_map_idx[M][Dim];

        memset(rhs, 0, sizeof rhs); 
        memset(btype, 0, sizeof btype); 

        for (s = 0; s < NFace; s++)
        {
            int M_face = 3*(sur_normal->type->np_vert + sur_normal->type->np_edge);
            SHORT bases[M_face];
            const FLOAT *normal;

            if (e->bound_type[s] & BC_TOP || e->bound_type[s] & BC_BOTTOM)
            {
                phgDofGetBasesOnFace(sur_normal, e, s, bases);

                normal = phgGeomGetFaceOutNormal(g, e, s);

                for (i = 0; i < M_face; i++)
                {
                    i_face = bases[i];

                    for (k = 0; k < Dim; k++)
                    {
                        rhs[i_face][k] += normal[k]; 
                    }

                }

            }
        }

        for (i = 0; i < M; i++) 
        {
            for (k = 0; k < Dim; k++)
            {
            btype[i][k] = phgDofGetElementBoundaryType(sur_normal, e, i*Dim+k);
            if (!(btype[i][k] & BC_TOP) && !(btype[i][k] & BC_BOTTOM)) 
            {
                rhs[i][k] = 0.;
            } 
            }
        }


        for (i = 0; i < M; i++)
        for (k = 0; k < Dim; k++)
        {
            local_map_idx[i][k] = phgMapE2L(map_n, 0, e, i*Dim+k);
        }
        phgVecAddEntries(vec_n, 0, M * Dim, local_map_idx[0], &rhs[0][0]);
    }

    phgVecAssemble(vec_n);
    phgMapLocalDataToDof(map_n, 1, &sur_normal, vec_n->data);

    
    FLOAT *data = DofData(sur_normal);
    INT len = DofGetDataCount(sur_normal)/3.;
    FLOAT nx, ny, nz, val;

    for (i = 0; i < len; i++)
    {

        nx = data[Dim*i];
        ny = data[Dim*i+1];
        nz = data[Dim*i+2];

        val = sqrt(nx*nx+ny*ny+nz*nz);

        if (val != 0)
        {
        nx = nx/val;ny = ny/val; nz = nz/val;
        }
        else
        {
            nx=0;ny=0;nz=0;
        }

        data[Dim*i+0] = nx;
        data[Dim*i+1] = ny;
        data[Dim*i+2] = nz;
    }
   

    phgMapDestroy(&map_n);
    phgVecDestroy(&vec_n);


    //phgExportVTK(g, "avg_n.vtk", sur_normal, NULL);
}

INT check_surf_convergence(NSSolver *ns, DOF *dH_last, FLOAT tol)
{
    INT dH_convergence;
    DOF *pdH = phgDofCopy(ns->dH, NULL, NULL, "plus_dH");
    DOF *mdH = phgDofCopy(ns->dH, NULL, NULL, "plus_dH");
    phgDofAXPY(1, dH_last, &pdH);
    phgDofAXPY(-1, dH_last, &mdH);
    FLOAT relative_error_norm_dH = 2.0*phgDofNormL2(mdH)/phgDofNormL2(pdH);
    phgPrintf("relative_error_norm dH: %f \n", relative_error_norm_dH);
    phgDofFree(&pdH);
    phgDofFree(&mdH);

    if (relative_error_norm_dH < tol)
    {
        dH_convergence = 1;
    }
    else
    {
        dH_convergence = 0;
    }

    return dH_convergence;
}

INT check_u_convergence0(NSSolver *ns, DOF *u, DOF *p, DOF *u_last, DOF *p_last, FLOAT utol, FLOAT ptol)
{

    DOF *pu, *pp;
    INT u_convergence;

    pu = phgDofCopy(u, NULL, NULL, "plus_velocity");
    pp = phgDofCopy(p, NULL, NULL, "plus_pressure");
    phgDofAXPY(1, u_last, &pu);
    phgDofAXPY(1, p_last, &pp);
    FLOAT relative_error_norm_u = 2.0*phgDofNormL2(ns->du)/phgDofNormL2(pu);
    FLOAT relative_error_norm_p = 2.0*phgDofNormL2(ns->dp)/phgDofNormL2(pp);
    phgPrintf("relative_error_norm (u, p): %f %f\n", relative_error_norm_u, relative_error_norm_p);
    phgDofFree(&pu);
    phgDofFree(&pp);

    if (relative_error_norm_u < utol && relative_error_norm_p < ptol)
    {
        u_convergence = 1;
    }
    else
    {
        u_convergence = 0;
    }

    return u_convergence;
}

INT check_u_convergence(NSSolver *ns, DOF *u, DOF *p, DOF *u_last, DOF *p_last, FLOAT utol, FLOAT ptol)
{

    DOF *pu, *pp;
    INT u_convergence;

    pu = phgDofCopy(u, NULL, NULL, "plus_velocity");
    pp = phgDofCopy(p, NULL, NULL, "plus_pressure");
    phgDofAXPY(1, u_last, &pu);
    phgDofAXPY(1, p_last, &pp);
    FLOAT relative_error_norm_u = 2.0*phgDofNormL2(ns->du)/phgDofNormL2(pu);
    FLOAT relative_error_norm_p = 2.0*phgDofNormL2(ns->dp)/phgDofNormL2(pp);
    phgPrintf("relative_error_norm (u, p): %f %f\n", relative_error_norm_u, relative_error_norm_p);
    phgDofFree(&pu);
    phgDofFree(&pp);

    if (relative_error_norm_u < utol && relative_error_norm_p < ptol)
    {
        u_convergence = 1;
    }
    else
    {
        u_convergence = 0;
    }

    return u_convergence;
}

void get_viscosity_field(GRID *g, DOF *strain_rate_e, DOF *viscosity)
{
    FLOAT temperature = 271.15;
    
    INT i;
    FLOAT A;

    FLOAT n = 3.0;

    //A = A_0_h*exp(-Q_h/(R*temperature));
    A = 1e-25*SEC_PER_YEAR;

    for (i = 0; i < DofGetDataCount(strain_rate_e); i++)
    {
        //printf("%f\n", strain_rate_e->data[i]);
        viscosity->data[i] = 0.5*pow(A, -1.0/n)*pow(strain_rate_e->data[i]+1e-30, -1.0+1.0/n);
        if (viscosity->data[i] > 1.0e20)
            viscosity->data[i] = 1.0e20;
    }
}


void get_effective_strain_rate_field(GRID *g, DOF *strain_rate, DOF *strain_rate_e)
{

    INT i;

    FLOAT *eps = strain_rate->data;

    for (i = 0; i < DofGetDataCount(strain_rate)/(Dim*Dim); i++)
    {
        strain_rate_e->data[i] = sqrt(0.5*(SQUARE(eps[0+i*Dim*Dim])
                                          +SQUARE(eps[4+i*Dim*Dim])
                                          +SQUARE(eps[8+i*Dim*Dim])
                                          +2*SQUARE(eps[1+i*Dim*Dim])
                                          +2*SQUARE(eps[2+i*Dim*Dim])
                                          +2*SQUARE(eps[5+i*Dim*Dim])
                                          ));
    }
}


void get_strain_rate_field(GRID *g, DOF *velocity_grad, DOF *strain_rate)
{

    INT i;

    for (i = 0; i < DofGetDataCount(velocity_grad)/(Dim*Dim); i++)
    {
        strain_rate->data[0+i*Dim*Dim] = velocity_grad->data[0+i*Dim*Dim];
        strain_rate->data[1+i*Dim*Dim] = 0.5*(velocity_grad->data[1+i*Dim*Dim]+velocity_grad->data[3+i*Dim*Dim]);
        strain_rate->data[2+i*Dim*Dim] = 0.5*(velocity_grad->data[2+i*Dim*Dim]+velocity_grad->data[6+i*Dim*Dim]);
        strain_rate->data[3+i*Dim*Dim] = strain_rate->data[1+i*Dim*Dim];
        strain_rate->data[4+i*Dim*Dim] = velocity_grad->data[4+i*Dim*Dim];
        strain_rate->data[5+i*Dim*Dim] = 0.5*(velocity_grad->data[5+i*Dim*Dim]+velocity_grad->data[7+i*Dim*Dim]);
        strain_rate->data[6+i*Dim*Dim] = strain_rate->data[2+i*Dim*Dim];
        strain_rate->data[7+i*Dim*Dim] = strain_rate->data[5+i*Dim*Dim];
        strain_rate->data[8+i*Dim*Dim] = velocity_grad->data[8+i*Dim*Dim];
    }
}
