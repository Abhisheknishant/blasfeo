/**************************************************************************************************
*                                                                                                 *
* This file is part of BLASFEO.                                                                   *
*                                                                                                 *
* BLASFEO -- BLAS For Embedded Optimization.                                                      *
* Copyright (C) 2016-2017 by Gianluca Frison.                                                     *
* Developed at IMTEK (University of Freiburg) under the supervision of Moritz Diehl.              *
* All rights reserved.                                                                            *
*                                                                                                 *
* HPMPC is free software; you can redistribute it and/or                                          *
* modify it under the terms of the GNU Lesser General Public                                      *
* License as published by the Free Software Foundation; either                                    *
* version 2.1 of the License, or (at your option) any later version.                              *
*                                                                                                 *
* HPMPC is distributed in the hope that it will be useful,                                        *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                                  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                                            *
* See the GNU Lesser General Public License for more details.                                     *
*                                                                                                 *
* You should have received a copy of the GNU Lesser General Public                                *
* License along with HPMPC; if not, write to the Free Software                                    *
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA                  *
*                                                                                                 *
* Author: Gianluca Frison, giaf (at) dtu.dk                                                       *
*                          gianluca.frison (at) imtek.uni-freiburg.de                             *
*                                                                                                 *
**************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "../include/blasfeo_block_size.h"
#include "../include/blasfeo_common.h"
#include "../include/blasfeo_d_kernel.h"
#include "../include/blasfeo_d_aux.h"



/****************************
* old interface
****************************/

void dgemm_nt_lib(int m, int n, int k, double alpha, double *pA, int sda, double *pB, int sdb, double beta, double *pC, int sdc, double *pD, int sdd)
	{

	if(m<=0 || n<=0)
		return;
	
	const int bs = 4;

	int i, j, l;

	i = 0;

#if defined(TARGET_X64_INTEL_HASWELL)
	for(; i<m-11; i+=12)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dgemm_nt_12x4_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd);
			}
		if(j<n)
			{
			kernel_dgemm_nt_12x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
			}
		}
	if(m>i)
		{
		if(m-i<=4)
			{
			goto left_4;
			}
		else if(m-i<=8)
			{
			goto left_8;
			}
		else
			{
			goto left_12;
			}
		}
#elif defined(TARGET_X64_INTEL_SANDY_BRIDGE)
	for(; i<m-7; i+=8)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dgemm_nt_8x4_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd);
			}
		if(j<n)
			{
			kernel_dgemm_nt_8x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
			}
		}
	if(m>i)
		{
		if(m-i<=4)
			{
			goto left_4;
			}
		else
			{
			goto left_8;
			}
		}
#elif defined(TARGET_ARMV8A_ARM_CORTEX_A57)
	for(; i<m-7; i+=8)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dgemm_nt_8x4_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd);
			}
		if(j<n)
			{
			kernel_dgemm_nt_4x4_vs_lib4(k, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
			kernel_dgemm_nt_4x4_vs_lib4(k, &alpha, &pA[(i+4)*sda], &pB[j*sdb], &beta, &pC[j*bs+(i+4)*sdc], &pD[j*bs+(i+4)*sdd], m-(i+4), n-j);
			}
		}
	if(m>i)
		{
		if(m-i<=4)
			{
			goto left_4;
			}
		else
			{
			goto left_8;
			}
		}
#else
	for(; i<m-3; i+=4)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dgemm_nt_4x4_lib4(k, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd]);
			}
		if(j<n)
			{
			kernel_dgemm_nt_4x4_vs_lib4(k, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
			}
		}
	if(m>i)
		{
		goto left_4;
		}
#endif

	// common return if i==m
	return;

	// clean up loops definitions

#if defined(TARGET_X64_INTEL_HASWELL)
	left_12:
	j = 0;
	for(; j<n; j+=4)
		{
		kernel_dgemm_nt_12x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
		}
	return;
#endif

#if defined(TARGET_X64_INTEL_HASWELL)
	left_8:
	j = 0;
	for(; j<n-8; j+=12)
		{
		kernel_dgemm_nt_8x8l_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], sdb, &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
		kernel_dgemm_nt_8x8u_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[(j+4)*sdb], sdb, &beta, &pC[(j+4)*bs+i*sdc], sdc, &pD[(j+4)*bs+i*sdd], sdd, m-i, n-(j+4));
		}
	
	if(j<n-4)
		{
		kernel_dgemm_nt_8x8l_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], sdb, &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
		kernel_dgemm_nt_4x4_vs_lib4(k, &alpha, &pA[i*sda], &pB[(j+4)*sdb], &beta, &pC[(j+4)*bs+i*sdc], &pD[(j+4)*bs+i*sdd], m-i, n-(j+4));
		}
	else if(j<n)
		{
		kernel_dgemm_nt_8x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
		}
	return;
#endif
#if defined(TARGET_X64_INTEL_SANDY_BRIDGE)
	left_8:
	j = 0;
	for(; j<n; j+=4)
		{
		kernel_dgemm_nt_8x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
		}
	return;
#endif
#if defined(TARGET_ARMV8A_ARM_CORTEX_A57)
	left_8:
	j = 0;
	for(; j<n; j+=4)
		{
		kernel_dgemm_nt_4x4_vs_lib4(k, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
		kernel_dgemm_nt_4x4_vs_lib4(k, &alpha, &pA[(i+4)*sda], &pB[j*sdb], &beta, &pC[j*bs+(i+4)*sdc], &pD[j*bs+(i+4)*sdd], m-(i+4), n-j);
		}
	return;
#endif

#if defined(TARGET_X64_INTEL_HASWELL)
	left_4:
	j = 0;
	for(; j<n-8; j+=12)
		{
		kernel_dgemm_nt_4x12_vs_lib4(k, &alpha, &pA[i*sda], &pB[j*sdb], sdb, &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
		}
	if(j<n-4)
		{
		kernel_dgemm_nt_4x8_vs_lib4(k, &alpha, &pA[i*sda], &pB[j*sdb], sdb, &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
		}
	else if(j<n)
		{
		kernel_dgemm_nt_4x4_vs_lib4(k, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
		}
	return;
#else
	left_4:
	j = 0;
	for(; j<n; j+=4)
		{
		kernel_dgemm_nt_4x4_vs_lib4(k, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
		}
	return;
#endif

	}



#if 0
void dgemm_nn_lib(int m, int n, int k, double alpha, double *pA, int sda, double *pB, int sdb, double beta, double *pC, int sdc, double *pD, int sdd)
	{

	if(m<=0 || n<=0)
		return;
	
	const int bs = 4;

	int i, j, l;

	i = 0;

#if defined(TARGET_X64_INTEL_HASWELL)
	for(; i<m-11; i+=12)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dgemm_nn_12x4_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*bs], sdb, &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd);
			}
		if(j<n)
			{
			kernel_dgemm_nn_12x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*bs], sdb, &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
			}
		}
	if(m>i)
		{
		if(m-i<=4)
			{
			goto left_4;
			}
		else if(m-i<=8)
			{
			goto left_8;
			}
		else
			{
			goto left_12;
			}
		}
#elif defined(TARGET_X64_INTEL_SANDY_BRIDGE)
	for(; i<m-7; i+=8)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dgemm_nn_8x4_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*bs], sdb, &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd);
			}
		if(j<n)
			{
			kernel_dgemm_nn_8x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*bs], sdb, &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
			}
		}
	if(m>i)
		{
		if(m-i<=4)
			{
			goto left_4;
			}
		else
			{
			goto left_8;
			}
		}
#else
	for(; i<m-3; i+=4)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dgemm_nn_4x4_lib4(k, &alpha, &pA[i*sda], 0, &pB[j*bs], sdb, &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd]);
			}
		if(j<n)
			{
			kernel_dgemm_nn_4x4_vs_lib4(k, &alpha, &pA[i*sda], &pB[j*bs], sdb, &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
			}
		}
	if(m>i)
		{
		goto left_4;
		}
#endif

	// common return if i==m
	return;

	// clean up loops definitions

#if defined(TARGET_X64_INTEL_HASWELL)
	left_12:
	j = 0;
	for(; j<n; j+=4)
		{
		kernel_dgemm_nn_12x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], sdb, &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
		}
	return;
#endif

#if defined(TARGET_X64_INTEL_SANDY_BRIDGE) || defined(TARGET_X64_INTEL_HASWELL)
	left_8:
	j = 0;
	for(; j<n; j+=4)
		{
		kernel_dgemm_nn_8x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*bs], sdb, &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
		}
	return;
#endif

	left_4:
	j = 0;
	for(; j<n; j+=4)
		{
		kernel_dgemm_nn_4x4_vs_lib4(k, &alpha, &pA[i*sda], &pB[j*bs], sdb, &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
		}
	return;

	}
#endif



void dsyrk_nt_l_lib(int m, int n, int k, double alpha, double *pA, int sda, double *pB, int sdb, double beta, double *pC, int sdc, double *pD, int sdd)
	{

	if(m<=0 || n<=0)
		return;

	const int bs = 4;

	int i, j, l;

	i = 0;

#if defined(TARGET_X64_INTEL_HASWELL)
	for(; i<m-11; i+=12)
		{
		j = 0;
		for(; j<i && j<n-3; j+=4)
			{
			kernel_dgemm_nt_12x4_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd);
			}
		if(j<n)
			{
			if(j<i) // dgemm
				{
				kernel_dgemm_nt_12x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
				}
			else // dsyrk
				{
				if(j<n-11)
					{
					kernel_dsyrk_nt_l_12x4_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd);
					kernel_dsyrk_nt_l_8x4_lib4(k, &alpha, &pA[(i+4)*sda], sda, &pB[(j+4)*sdb], &beta, &pC[(j+4)*bs+(i+4)*sdc], sdc, &pD[(j+4)*bs+(i+4)*sdd], sdd);
					kernel_dsyrk_nt_l_4x4_lib4(k, &alpha, &pA[(i+8)*sda], &pB[(j+8)*sdb], &beta, &pC[(j+8)*bs+(i+8)*sdc], &pD[(j+8)*bs+(i+8)*sdd]);
					}
				else
					{
					kernel_dsyrk_nt_l_12x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
					if(j<n-4)
						{
						kernel_dsyrk_nt_l_8x4_vs_lib4(k, &alpha, &pA[(i+4)*sda], sda, &pB[(j+4)*sdb], &beta, &pC[(j+4)*bs+(i+4)*sdc], sdc, &pD[(j+4)*bs+(i+4)*sdd], sdd, m-i-4, n-j-4);
						if(j<n-8)
							{
							kernel_dsyrk_nt_l_4x4_vs_lib4(k, &alpha, &pA[(i+8)*sda], &pB[(j+8)*sdb], &beta, &pC[(j+8)*bs+(i+8)*sdc], &pD[(j+8)*bs+(i+8)*sdd], m-i-8, n-j-8);
							}
						}
					}
				}
			}
		}
	if(m>i)
		{
		if(m-i<=4)
			{
			goto left_4;
			}
		else if(m-i<=8)
			{
			goto left_8;
			}
		else
			{
			goto left_12;
			}
		}
#elif defined(TARGET_X64_INTEL_SANDY_BRIDGE)
	for(; i<m-7; i+=8)
		{
		j = 0;
		for(; j<i && j<n-3; j+=4)
			{
			kernel_dgemm_nt_8x4_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd);
			}
		if(j<n)
			{
			if(j<i) // dgemm
				{
				kernel_dgemm_nt_8x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
				}
			else // dsyrk
				{
				if(j<n-7)
					{
					kernel_dsyrk_nt_l_8x4_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd);
					kernel_dsyrk_nt_l_4x4_lib4(k, &alpha, &pA[(i+4)*sda], &pB[(j+4)*sdb], &beta, &pC[(j+4)*bs+(i+4)*sdc], &pD[(j+4)*bs+(i+4)*sdd]);
					}
				else
					{
					kernel_dsyrk_nt_l_8x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
					if(j<n-4)
						{
						kernel_dsyrk_nt_l_4x4_vs_lib4(k, &alpha, &pA[(i+4)*sda], &pB[(j+4)*sdb], &beta, &pC[(j+4)*bs+(i+4)*sdc], &pD[(j+4)*bs+(i+4)*sdd], m-i-4, n-j-4);
						}
					}
				}
			}
		}
	if(m>i)
		{
		if(m-i<=4)
			{
			goto left_4;
			}
		else
			{
			goto left_8;
			}
		}
#else
	for(; i<m-3; i+=4)
		{
		j = 0;
		for(; j<i && j<n-3; j+=4)
			{
			kernel_dgemm_nt_4x4_lib4(k, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd]);
			}
		if(j<n)
			{
			if(i<j) // dgemm
				{
				kernel_dgemm_nt_4x4_vs_lib4(k, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
				}
			else // dsyrk
				{
				if(j<n-3)
					{
					kernel_dsyrk_nt_l_4x4_lib4(k, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd]);
					}
				else
					{
					kernel_dsyrk_nt_l_4x4_vs_lib4(k, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
					}
				}
			}
		}
	if(m>i)
		{
		goto left_4;
		}
#endif

	// common return if i==m
	return;

	// clean up loops definitions

#if defined(TARGET_X64_INTEL_HASWELL)
	left_12:
	j = 0;
	for(; j<i && j<n-3; j+=4)
		{
		kernel_dgemm_nt_12x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
		}
	if(j<n)
		{
		if(j<i) // dgemm
			{
			kernel_dgemm_nt_12x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
			}
		else // dsyrk
			{
			kernel_dsyrk_nt_l_12x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
			if(j<n-4)
				{
				kernel_dsyrk_nt_l_8x4_vs_lib4(k, &alpha, &pA[(i+4)*sda], sda, &pB[(j+4)*sdb], &beta, &pC[(j+4)*bs+(i+4)*sdc], sdc, &pD[(j+4)*bs+(i+4)*sdd], sdd, m-i-4, n-j-4);
				if(j<n-8)
					{
					kernel_dsyrk_nt_l_4x4_vs_lib4(k, &alpha, &pA[(i+8)*sda], &pB[(j+8)*sdb], &beta, &pC[(j+8)*bs+(i+8)*sdc], &pD[(j+8)*bs+(i+8)*sdd], m-i-8, n-j-8);
					}
				}
			}
		}
	return;
#endif

#if defined(TARGET_X64_INTEL_SANDY_BRIDGE) || defined(TARGET_X64_INTEL_HASWELL)
	left_8:
	j = 0;
	for(; j<i && j<n-3; j+=4)
		{
		kernel_dgemm_nt_8x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
		}
	if(j<n)
		{
		if(j<i) // dgemm
			{
			kernel_dgemm_nt_8x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
			}
		else // dsyrk
			{
			kernel_dsyrk_nt_l_8x4_vs_lib4(k, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
			if(j<n-4)
				{
				kernel_dsyrk_nt_l_4x4_vs_lib4(k, &alpha, &pA[(i+4)*sda], &pB[(j+4)*sdb], &beta, &pC[(j+4)*bs+(i+4)*sdc], &pD[(j+4)*bs+(i+4)*sdd], m-i-4, n-j-4);
				}
			}
		}
	return;
#endif

	left_4:
	j = 0;
	for(; j<i && j<n-3; j+=4)
		{
		kernel_dgemm_nt_4x4_vs_lib4(k, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
		}
	if(j<n)
		{
		if(j<i) // dgemm
			{
			kernel_dgemm_nt_4x4_vs_lib4(k, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
			}
		else // dsyrk
			{
			kernel_dsyrk_nt_l_4x4_vs_lib4(k, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
			}
		}
	return;

	}



void dtrmm_nt_ru_lib(int m, int n, double alpha, double *pA, int sda, double *pB, int sdb, double beta, double *pC, int sdc, double *pD, int sdd)
	{

	if(m<=0 || n<=0)
		return;
	
	const int bs = 4;
	
	int i, j;
	
	i = 0;
// XXX there is a bug here !!!!!!
#if 0//defined(TARGET_X64_INTEL_HASWELL)
	for(; i<m-11; i+=12)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dtrmm_nt_ru_12x4_lib4(n-j, &alpha, &pA[j*bs+i*sda], sda, &pB[j*bs+j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd);
			}
		if(j<n) // TODO specialized edge routine
			{
			kernel_dtrmm_nt_ru_12x4_vs_lib4(n-j, &alpha, &pA[j*bs+i*sda], sda, &pB[j*bs+j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
			}
		}
	if(i<m)
		{
		if(m-i<5)
			{
			goto left_4;
			}
		if(m-i<9)
			{
			goto left_8;
			}
		else
			{
			goto left_12;
			}
		}

#elif defined(TARGET_X64_INTEL_SANDY_BRIDGE) || defined(TARGET_X64_INTEL_HASWELL)
	for(; i<m-7; i+=8)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dtrmm_nt_ru_8x4_lib4(n-j, &alpha, &pA[j*bs+i*sda], sda, &pB[j*bs+j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd);
			}
		if(j<n) // TODO specialized edge routine
			{
			kernel_dtrmm_nt_ru_8x4_vs_lib4(n-j, &alpha, &pA[j*bs+i*sda], sda, &pB[j*bs+j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
			}
		}
	if(i<m)
		{
		if(m-i<5)
			{
			goto left_4;
			}
		else
			{
			goto left_8;
			}
		}

#else
	for(; i<m-3; i+=4)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dtrmm_nt_ru_4x4_lib4(n-j, &alpha, &pA[j*bs+i*sda], &pB[j*bs+j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd]);
			}
		if(j<n) // TODO specialized edge routine
			{
			kernel_dtrmm_nt_ru_4x4_vs_lib4(n-j, &alpha, &pA[j*bs+i*sda], &pB[j*bs+j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
			}
		}
	if(i<m)
		{
		goto left_4;
		}
#endif
	
	// common return
	return;

#if defined(TARGET_X64_INTEL_HASWELL)
	// clean up
	left_12:
	j = 0;
//	for(; j<n-3; j+=4)
	for(; j<n; j+=4)
		{
		kernel_dtrmm_nt_ru_12x4_vs_lib4(n-j, &alpha, &pA[j*bs+i*sda], sda, &pB[j*bs+j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
		}
//	if(j<n) // TODO specialized edge routine
//		{
//		kernel_dtrmm_nt_ru_8x4_vs_lib4(n-j, &pA[j*bs+i*sda], sda, &pB[j*bs+j*sdb], alg, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
//		}
	return;
#endif

#if defined(TARGET_X64_INTEL_SANDY_BRIDGE) || defined(TARGET_X64_INTEL_HASWELL)
	// clean up
	left_8:
	j = 0;
//	for(; j<n-3; j+=4)
	for(; j<n; j+=4)
		{
		kernel_dtrmm_nt_ru_8x4_vs_lib4(n-j, &alpha, &pA[j*bs+i*sda], sda, &pB[j*bs+j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
		}
//	if(j<n) // TODO specialized edge routine
//		{
//		kernel_dtrmm_nt_ru_8x4_vs_lib4(n-j, &pA[j*bs+i*sda], sda, &pB[j*bs+j*sdb], alg, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
//		}
	return;
#endif

	left_4:
	j = 0;
//	for(; j<n-3; j+=4)
	for(; j<n; j+=4)
		{
		kernel_dtrmm_nt_ru_4x4_vs_lib4(n-j, &alpha, &pA[j*bs+i*sda], &pB[j*bs+j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
		}
//	if(j<n) // TODO specialized edge routine
//		{
//		kernel_dtrmm_nt_ru_4x4_vs_lib4(n-j, &pA[j*bs+i*sda], &pB[j*bs+j*sdb], alg, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
//		}
	return;

	}



// D <= B * A^{-T} , with A lower triangular employeing explicit inverse of diagonal
void dtrsm_nt_rl_inv_lib(int m, int n, double *pA, int sda, double *inv_diag_A, double *pB, int sdb, double *pD, int sdd)
	{

	if(m<=0 || n<=0)
		return;
	
	const int bs = 4;
	
	int i, j;
	
	i = 0;

#if defined(TARGET_X64_INTEL_HASWELL)
	for(; i<m-11; i+=12)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dtrsm_nt_rl_inv_12x4_lib4(j, &pD[i*sdd], sdd, &pA[j*sda], &pB[j*bs+i*sdb], sdb, &pD[j*bs+i*sdd], sdd, &pA[j*bs+j*sda], &inv_diag_A[j]);
			}
		if(j<n)
			{
			kernel_dtrsm_nt_rl_inv_12x4_vs_lib4(j, &pD[i*sdd], sdd, &pA[j*sda], &pB[j*bs+i*sdb], sdb, &pD[j*bs+i*sdd], sdd, &pA[j*bs+j*sda], &inv_diag_A[j], m-i, n-j);
			}
		}
	if(m>i)
		{
		if(m-i<=4)
			{
			goto left_4;
			}
		else if(m-i<=8)
			{
			goto left_8;
			}
		else
			{
			goto left_12;
			}
		}
#elif defined(TARGET_X64_INTEL_SANDY_BRIDGE)
	for(; i<m-7; i+=8)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dtrsm_nt_rl_inv_8x4_lib4(j, &pD[i*sdd], sdd, &pA[j*sda], &pB[j*bs+i*sdb], sdb, &pD[j*bs+i*sdd], sdd, &pA[j*bs+j*sda], &inv_diag_A[j]);
			}
		if(j<n)
			{
			kernel_dtrsm_nt_rl_inv_8x4_vs_lib4(j, &pD[i*sdd], sdd, &pA[j*sda], &pB[j*bs+i*sdb], sdb, &pD[j*bs+i*sdd], sdd, &pA[j*bs+j*sda], &inv_diag_A[j], m-i, n-j);
			}
		}
	if(m>i)
		{
		if(m-i<=4)
			{
			goto left_4;
			}
		else
			{
			goto left_8;
			}
		}
#else
	for(; i<m-3; i+=4)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dtrsm_nt_rl_inv_4x4_lib4(j, &pD[i*sdd], &pA[j*sda], &pB[j*bs+i*sdb], &pD[j*bs+i*sdd], &pA[j*bs+j*sda], &inv_diag_A[j]);
			}
		if(j<n)
			{
			kernel_dtrsm_nt_rl_inv_4x4_vs_lib4(j, &pD[i*sdd], &pA[j*sda], &pB[j*bs+i*sdb], &pD[j*bs+i*sdd], &pA[j*bs+j*sda], &inv_diag_A[j], m-i, n-j);
			}
		}
	if(m>i)
		{
		goto left_4;
		}
#endif

	// common return if i==m
	return;

#if defined(TARGET_X64_INTEL_HASWELL)
	left_12:
	j = 0;
	for(; j<n; j+=4)
		{
		kernel_dtrsm_nt_rl_inv_12x4_vs_lib4(j, &pD[i*sdd], sdd, &pA[j*sda], &pB[j*bs+i*sdb], sdb, &pD[j*bs+i*sdd], sdd, &pA[j*bs+j*sda], &inv_diag_A[j], m-i, n-j);
		}
	return;
#endif

#if defined(TARGET_X64_INTEL_SANDY_BRIDGE) || defined(TARGET_X64_INTEL_HASWELL)
	left_8:
	j = 0;
	for(; j<n; j+=4)
		{
		kernel_dtrsm_nt_rl_inv_8x4_vs_lib4(j, &pD[i*sdd], sdd, &pA[j*sda], &pB[j*bs+i*sdb], sdb, &pD[j*bs+i*sdd], sdd, &pA[j*bs+j*sda], &inv_diag_A[j], m-i, n-j);
		}
	return;
#endif

	left_4:
	j = 0;
	for(; j<n; j+=4)
		{
		kernel_dtrsm_nt_rl_inv_4x4_vs_lib4(j, &pD[i*sdd], &pA[j*sda], &pB[j*bs+i*sdb], &pD[j*bs+i*sdd], &pA[j*bs+j*sda], &inv_diag_A[j], m-i, n-j);
		}
	return;

	}



// D <= B * A^{-T} , with A lower triangular with unit diagonal
void dtrsm_nt_rl_one_lib(int m, int n, double *pA, int sda, double *pB, int sdb, double *pD, int sdd)
	{

	if(m<=0 || n<=0)
		return;
	
	const int bs = 4;
	
	int i, j;
	
	i = 0;

#if defined(TARGET_X64_INTEL_HASWELL)
	for(; i<m-11; i+=12)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dtrsm_nt_rl_one_12x4_lib4(j, &pD[i*sdd], sdd, &pA[j*sda], &pB[j*bs+i*sdb], sdb, &pD[j*bs+i*sdd], sdd, &pA[j*bs+j*sda]);
			}
		if(j<n)
			{
			kernel_dtrsm_nt_rl_one_12x4_vs_lib4(j, &pD[i*sdd], sdd, &pA[j*sda], &pB[j*bs+i*sdb], sdb, &pD[j*bs+i*sdd], sdd, &pA[j*bs+j*sda], m-i, n-j);
			}
		}
	if(m>i)
		{
		if(m-i<=4)
			{
			goto left_4;
			}
		else if(m-i<=8)
			{
			goto left_8;
			}
		else
			{
			goto left_12;
			}
		}
#elif defined(TARGET_X64_INTEL_SANDY_BRIDGE)
	for(; i<m-7; i+=8)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dtrsm_nt_rl_one_8x4_lib4(j, &pD[i*sdd], sdd, &pA[j*sda], &pB[j*bs+i*sdb], sdb, &pD[j*bs+i*sdd], sdd, &pA[j*bs+j*sda]);
			}
		if(j<n)
			{
			kernel_dtrsm_nt_rl_one_8x4_vs_lib4(j, &pD[i*sdd], sdd, &pA[j*sda], &pB[j*bs+i*sdb], sdb, &pD[j*bs+i*sdd], sdd, &pA[j*bs+j*sda], m-i, n-j);
			}
		}
	if(m>i)
		{
		if(m-i<=4)
			{
			goto left_4;
			}
		else
			{
			goto left_8;
			}
		}
#else
	for(; i<m-3; i+=4)
		{
		j = 0;
		for(; j<n-3; j+=4)
			{
			kernel_dtrsm_nt_rl_one_4x4_lib4(j, &pD[i*sdd], &pA[j*sda], &pB[j*bs+i*sdb], &pD[j*bs+i*sdd], &pA[j*bs+j*sda]);
			}
		if(j<n)
			{
			kernel_dtrsm_nt_rl_one_4x4_vs_lib4(j, &pD[i*sdd], &pA[j*sda], &pB[j*bs+i*sdb], &pD[j*bs+i*sdd], &pA[j*bs+j*sda], m-i, n-j);
			}
		}
	if(m>i)
		{
		goto left_4;
		}
#endif

	// common return if i==m
	return;

#if defined(TARGET_X64_INTEL_HASWELL)
	left_12:
	j = 0;
	for(; j<n; j+=4)
		{
		kernel_dtrsm_nt_rl_one_12x4_vs_lib4(j, &pD[i*sdd], sdd, &pA[j*sda], &pB[j*bs+i*sdb], sdb, &pD[j*bs+i*sdd], sdd, &pA[j*bs+j*sda], m-i, n-j);
		}
	return;
#endif

#if defined(TARGET_X64_INTEL_SANDY_BRIDGE) || defined(TARGET_X64_INTEL_HASWELL)
	left_8:
	j = 0;
	for(; j<n; j+=4)
		{
		kernel_dtrsm_nt_rl_one_8x4_vs_lib4(j, &pD[i*sdd], sdd, &pA[j*sda], &pB[j*bs+i*sdb], sdb, &pD[j*bs+i*sdd], sdd, &pA[j*bs+j*sda], m-i, n-j);
		}
	return;
#endif

	left_4:
	j = 0;
	for(; j<n; j+=4)
		{
		kernel_dtrsm_nt_rl_one_4x4_vs_lib4(j, &pD[i*sdd], &pA[j*sda], &pB[j*bs+i*sdb], &pD[j*bs+i*sdd], &pA[j*bs+j*sda], m-i, n-j);
		}
	return;

	}



// D <= B * A^{-T} , with A upper triangular employing explicit inverse of diagonal
void dtrsm_nt_ru_inv_lib(int m, int n, double *pA, int sda, double *inv_diag_A, double *pB, int sdb, double *pD, int sdd)
	{

	if(m<=0 || n<=0)
		return;
	
	const int bs = 4;
	
	int i, j, idx;

	int rn = n%4;

	double *dummy;
	
	i = 0;

#if defined(TARGET_X64_INTEL_HASWELL)
	for(; i<m-11; i+=12)
		{
		j = 0;
		// clean at the end
		if(rn>0)
			{
			idx = n-rn;
			kernel_dtrsm_nt_ru_inv_12x4_vs_lib4(0, dummy, 0, dummy, &pB[i*sdb+idx*bs], sdb, &pD[i*sdd+idx*bs], sdd, &pA[idx*sda+idx*bs], &inv_diag_A[idx], m-i, rn);
			j += rn;
			}
		for(; j<n; j+=4)
			{
			idx = n-j-4;
			kernel_dtrsm_nt_ru_inv_12x4_lib4(j, &pD[i*sdd+(idx+4)*bs], sdd, &pA[idx*sda+(idx+4)*bs], &pB[i*sdb+idx*bs], sdb, &pD[i*sdd+idx*bs], sdd, &pA[idx*sda+idx*bs], &inv_diag_A[idx]);
			}
		}
	if(m>i)
		{
		if(m-i<=4)
			{
			goto left_4;
			}
		else if(m-i<=8)
			{
			goto left_8;
			}
		else
			{
			goto left_12;
			}
		}
#elif defined(TARGET_X64_INTEL_SANDY_BRIDGE)
	for(; i<m-7; i+=8)
		{
		j = 0;
		// clean at the end
		if(rn>0)
			{
			idx = n-rn;
			kernel_dtrsm_nt_ru_inv_8x4_vs_lib4(0, dummy, 0, dummy, &pB[i*sdb+idx*bs], sdb, &pD[i*sdd+idx*bs], sdd, &pA[idx*sda+idx*bs], &inv_diag_A[idx], m-i, rn);
			j += rn;
			}
		for(; j<n; j+=4)
			{
			idx = n-j-4;
			kernel_dtrsm_nt_ru_inv_8x4_lib4(j, &pD[i*sdd+(idx+4)*bs], sdd, &pA[idx*sda+(idx+4)*bs], &pB[i*sdb+idx*bs], sdb, &pD[i*sdd+idx*bs], sdd, &pA[idx*sda+idx*bs], &inv_diag_A[idx]);
			}
		}
	if(m>i)
		{
		if(m-i<=4)
			{
			goto left_4;
			}
		else
			{
			goto left_8;
			}
		}
#else
	for(; i<m-3; i+=4)
		{
		j = 0;
		// clean at the end
		if(rn>0)
			{
			idx = n-rn;
			kernel_dtrsm_nt_ru_inv_4x4_vs_lib4(0, dummy, dummy, &pB[i*sdb+idx*bs], &pD[i*sdd+idx*bs], &pA[idx*sda+idx*bs], &inv_diag_A[idx], m-i, rn);
			j += rn;
			}
		for(; j<n; j+=4)
			{
			idx = n-j-4;
			kernel_dtrsm_nt_ru_inv_4x4_lib4(j, &pD[i*sdd+(idx+4)*bs], &pA[idx*sda+(idx+4)*bs], &pB[i*sdb+idx*bs], &pD[i*sdd+idx*bs], &pA[idx*sda+idx*bs], &inv_diag_A[idx]);
			}
		}
	if(m>i)
		{
		goto left_4;
		}
#endif

	// common return if i==m
	return;

#if defined(TARGET_X64_INTEL_HASWELL)
	left_12:
	j = 0;
	// TODO
	// clean at the end
	if(rn>0)
		{
		idx = n-rn;
		kernel_dtrsm_nt_ru_inv_12x4_vs_lib4(0, dummy, 0, dummy, &pB[i*sdb+idx*bs], sdb, &pD[i*sdd+idx*bs], sdd, &pA[idx*sda+idx*bs], &inv_diag_A[idx], m-i, rn);
		j += rn;
		}
	for(; j<n; j+=4)
		{
		idx = n-j-4;
		kernel_dtrsm_nt_ru_inv_12x4_vs_lib4(j, &pD[i*sdd+(idx+4)*bs], sdd, &pA[idx*sda+(idx+4)*bs], &pB[i*sdb+idx*bs], sdb, &pD[i*sdd+idx*bs], sdd, &pA[idx*sda+idx*bs], &inv_diag_A[idx], m-i, 4);
		}
	return;

#endif

#if defined(TARGET_X64_INTEL_SANDY_BRIDGE) || defined(TARGET_X64_INTEL_HASWELL)
	left_8:
	j = 0;
	// TODO
	// clean at the end
	if(rn>0)
		{
		idx = n-rn;
		kernel_dtrsm_nt_ru_inv_8x4_vs_lib4(0, dummy, 0, dummy, &pB[i*sdb+idx*bs], sdb, &pD[i*sdd+idx*bs], sdd, &pA[idx*sda+idx*bs], &inv_diag_A[idx], m-i, rn);
		j += rn;
		}
	for(; j<n; j+=4)
		{
		idx = n-j-4;
		kernel_dtrsm_nt_ru_inv_8x4_vs_lib4(j, &pD[i*sdd+(idx+4)*bs], sdd, &pA[idx*sda+(idx+4)*bs], &pB[i*sdb+idx*bs], sdb, &pD[i*sdd+idx*bs], sdd, &pA[idx*sda+idx*bs], &inv_diag_A[idx], m-i, 4);
		}
	return;

#endif

	left_4:
	j = 0;
	// TODO
	// clean at the end
	if(rn>0)
		{
		idx = n-rn;
		kernel_dtrsm_nt_ru_inv_4x4_vs_lib4(0, dummy, dummy, &pB[i*sdb+idx*bs], &pD[i*sdd+idx*bs], &pA[idx*sda+idx*bs], &inv_diag_A[idx], m-i, rn);
		j += rn;
		}
	for(; j<n; j+=4)
		{
		idx = n-j-4;
		kernel_dtrsm_nt_ru_inv_4x4_vs_lib4(j, &pD[i*sdd+(idx+4)*bs], &pA[idx*sda+(idx+4)*bs], &pB[i*sdb+idx*bs], &pD[i*sdd+idx*bs], &pA[idx*sda+idx*bs], &inv_diag_A[idx], m-i, 4);
		}
	return;

	}



// D <= A^{-1} * B , with A lower triangular with unit diagonal
void dtrsm_nn_ll_one_lib(int m, int n, double *pA, int sda, double *pB, int sdb, double *pD, int sdd)
	{

	if(m<=0 || n<=0)
		return;
	
	const int bs = 4;
	
	int i, j;
	
	i = 0;

#if defined(TARGET_X64_INTEL_HASWELL)
	for( ; i<m-11; i+=12)
		{
		j = 0;
		for( ; j<n-3; j+=4)
			{
			kernel_dtrsm_nn_ll_one_12x4_lib4(i, pA+i*sda, sda, pD+j*bs, sdd, pB+i*sdb+j*bs, sdb, pD+i*sdd+j*bs, sdd, pA+i*sda+i*bs, sda);
			}
		if(j<n)
			{
			kernel_dtrsm_nn_ll_one_12x4_vs_lib4(i, pA+i*sda, sda, pD+j*bs, sdd, pB+i*sdb+j*bs, sdb, pD+i*sdd+j*bs, sdd, pA+i*sda+i*bs, sda, m-i, n-j);
			}
		}
	if(i<m)
		{
		if(m-i<=4)
			goto left_4;
		if(m-i<=8)
			goto left_8;
		else
			goto left_12;
		}
#elif defined(TARGET_X64_INTEL_SANDY_BRIDGE)
	for( ; i<m-7; i+=8)
		{
		j = 0;
		for( ; j<n-3; j+=4)
			{
			kernel_dtrsm_nn_ll_one_8x4_lib4(i, pA+i*sda, sda, pD+j*bs, sdd, pB+i*sdb+j*bs, sdb, pD+i*sdd+j*bs, sdd, pA+i*sda+i*bs, sda);
			}
		if(j<n)
			{
			kernel_dtrsm_nn_ll_one_8x4_vs_lib4(i, pA+i*sda, sda, pD+j*bs, sdd, pB+i*sdb+j*bs, sdb, pD+i*sdd+j*bs, sdd, pA+i*sda+i*bs, sda, m-i, n-j);
			}
		}
	if(i<m)
		{
		if(m-i<=4)
			goto left_4;
		else
			goto left_8;
		}
#else
	for( ; i<m-3; i+=4)
		{
		j = 0;
		for( ; j<n-3; j+=4)
			{
			kernel_dtrsm_nn_ll_one_4x4_lib4(i, pA+i*sda, pD+j*bs, sdd, pB+i*sdb+j*bs, pD+i*sdd+j*bs, pA+i*sda+i*bs);
			}
		if(j<n)
			{
			kernel_dtrsm_nn_ll_one_4x4_vs_lib4(i, pA+i*sda, pD+j*bs, sdd, pB+i*sdb+j*bs, pD+i*sdd+j*bs, pA+i*sda+i*bs, m-i, n-j);
			}
		}
	if(i<m)
		{
		goto left_4;
		}
#endif

	// common return
	return;

#if defined(TARGET_X64_INTEL_HASWELL)
	left_12:
	j = 0;
	for( ; j<n; j+=4)
		{
		kernel_dtrsm_nn_ll_one_12x4_vs_lib4(i, pA+i*sda, sda, pD+j*bs, sdd, pB+i*sdb+j*bs, sdb, pD+i*sdd+j*bs, sdd, pA+i*sda+i*bs, sda, m-i, n-j);
		}
	return;
#endif

#if defined(TARGET_X64_INTEL_SANDY_BRIDGE) || defined(TARGET_X64_INTEL_HASWELL)
	left_8:
	j = 0;
	for( ; j<n; j+=4)
		{
		kernel_dtrsm_nn_ll_one_8x4_vs_lib4(i, pA+i*sda, sda, pD+j*bs, sdd, pB+i*sdb+j*bs, sdb, pD+i*sdd+j*bs, sdd, pA+i*sda+i*bs, sda, m-i, n-j);
		}
	return;
#endif

	left_4:
	j = 0;
	for( ; j<n; j+=4)
		{
		kernel_dtrsm_nn_ll_one_4x4_vs_lib4(i, pA+i*sda, pD+j*bs, sdd, pB+i*sdb+j*bs, pD+i*sdd+j*bs, pA+i*sda+i*bs, m-i, n-j);
		}
	return;

	}



// D <= A^{-1} * B , with A upper triangular employing explicit inverse of diagonal
void dtrsm_nn_lu_inv_lib(int m, int n, double *pA, int sda, double *inv_diag_A, double *pB, int sdb, double *pD, int sdd)
	{

	if(m<=0 || n<=0)
		return;
	
	const int bs = 4;
	
	int i, j, idx;
	double *dummy;
	
	i = 0;
	int rm = m%4;
	if(rm>0)
		{
		// TODO code expliticly the final case
		idx = m-rm; // position of the part to do
		j = 0;
		for( ; j<n; j+=4)
			{
			kernel_dtrsm_nn_lu_inv_4x4_vs_lib4(0, dummy, dummy, 0, pB+idx*sdb+j*bs, pD+idx*sdd+j*bs, pA+idx*sda+idx*bs, inv_diag_A+idx, rm, n-j);
			}
		// TODO
		i += rm;
		}
//	int em = m-rm;
#if defined(TARGET_X64_INTEL_HASWELL)
	for( ; i<m-8; i+=12)
		{
		idx = m-i; // position of already done part
		j = 0;
		for( ; j<n-3; j+=4)
			{
			kernel_dtrsm_nn_lu_inv_12x4_lib4(i, pA+(idx-12)*sda+idx*bs, sda, pD+idx*sdd+j*bs, sdd, pB+(idx-12)*sdb+j*bs, sdb, pD+(idx-12)*sdd+j*bs, sdd, pA+(idx-12)*sda+(idx-12)*bs, sda, inv_diag_A+(idx-12));
			}
		if(j<n)
			{
			kernel_dtrsm_nn_lu_inv_12x4_vs_lib4(i, pA+(idx-12)*sda+idx*bs, sda, pD+idx*sdd+j*bs, sdd, pB+(idx-12)*sdb+j*bs, sdb, pD+(idx-12)*sdd+j*bs, sdd, pA+(idx-12)*sda+(idx-12)*bs, sda, inv_diag_A+(idx-12), 12, n-j);
//			kernel_dtrsm_nn_lu_inv_4x4_vs_lib4(i, pA+(idx-4)*sda+idx*bs, pD+idx*sdd+j*bs, sdd, pB+(idx-4)*sdb+j*bs, pD+(idx-4)*sdd+j*bs, pA+(idx-4)*sda+(idx-4)*bs, inv_diag_A+(idx-4), 4, n-j);
//			kernel_dtrsm_nn_lu_inv_4x4_vs_lib4(i+4, pA+(idx-8)*sda+(idx-4)*bs, pD+(idx-4)*sdd+j*bs, sdd, pB+(idx-8)*sdb+j*bs, pD+(idx-8)*sdd+j*bs, pA+(idx-8)*sda+(idx-8)*bs, inv_diag_A+(idx-8), 4, n-j);
//			kernel_dtrsm_nn_lu_inv_4x4_vs_lib4(i+8, pA+(idx-12)*sda+(idx-8)*bs, pD+(idx-8)*sdd+j*bs, sdd, pB+(idx-12)*sdb+j*bs, pD+(idx-12)*sdd+j*bs, pA+(idx-12)*sda+(idx-12)*bs, inv_diag_A+(idx-12), 4, n-j);
			}
		}
#endif
#if defined(TARGET_X64_INTEL_SANDY_BRIDGE) || defined(TARGET_X64_INTEL_HASWELL)
	for( ; i<m-4; i+=8)
		{
		idx = m-i; // position of already done part
		j = 0;
		for( ; j<n-3; j+=4)
			{
			kernel_dtrsm_nn_lu_inv_8x4_lib4(i, pA+(idx-8)*sda+idx*bs, sda, pD+idx*sdd+j*bs, sdd, pB+(idx-8)*sdb+j*bs, sdb, pD+(idx-8)*sdd+j*bs, sdd, pA+(idx-8)*sda+(idx-8)*bs, sda, inv_diag_A+(idx-8));
			}
		if(j<n)
			{
			kernel_dtrsm_nn_lu_inv_8x4_vs_lib4(i, pA+(idx-8)*sda+idx*bs, sda, pD+idx*sdd+j*bs, sdd, pB+(idx-8)*sdb+j*bs, sdb, pD+(idx-8)*sdd+j*bs, sdd, pA+(idx-8)*sda+(idx-8)*bs, sda, inv_diag_A+(idx-8), 8, n-j);
//			kernel_dtrsm_nn_lu_inv_4x4_vs_lib4(i, pA+(idx-4)*sda+idx*bs, pD+idx*sdd+j*bs, sdd, pB+(idx-4)*sdb+j*bs, pD+(idx-4)*sdd+j*bs, pA+(idx-4)*sda+(idx-4)*bs, inv_diag_A+(idx-4), 4, n-j);
//			kernel_dtrsm_nn_lu_inv_4x4_vs_lib4(i+4, pA+(idx-8)*sda+(idx-4)*bs, pD+(idx-4)*sdd+j*bs, sdd, pB+(idx-8)*sdb+j*bs, pD+(idx-8)*sdd+j*bs, pA+(idx-8)*sda+(idx-8)*bs, inv_diag_A+(idx-8), 4, n-j);
			}
		}
#endif
	for( ; i<m; i+=4)
		{
		idx = m-i; // position of already done part
		j = 0;
		for( ; j<n-3; j+=4)
			{
			kernel_dtrsm_nn_lu_inv_4x4_lib4(i, pA+(idx-4)*sda+idx*bs, pD+idx*sdd+j*bs, sdd, pB+(idx-4)*sdb+j*bs, pD+(idx-4)*sdd+j*bs, pA+(idx-4)*sda+(idx-4)*bs, inv_diag_A+(idx-4));
			}
		if(j<n)
			{
			kernel_dtrsm_nn_lu_inv_4x4_vs_lib4(i, pA+(idx-4)*sda+idx*bs, pD+idx*sdd+j*bs, sdd, pB+(idx-4)*sdb+j*bs, pD+(idx-4)*sdd+j*bs, pA+(idx-4)*sda+(idx-4)*bs, inv_diag_A+(idx-4), 4, n-j);
			}
		}

	// common return
	return;

	}



#if 0
void dlauum_blk_nt_l_lib(int m, int n, int nv, int *rv, int *cv, double *pA, int sda, double *pB, int sdb, int alg, double *pC, int sdc, double *pD, int sdd)
	{

	if(m<=0 || n<=0)
		return;
	
	// TODO remove
	double alpha, beta;
	if(alg==0)
		{
		alpha = 1.0;
		beta = 0.0;
		}
	else if(alg==1)
		{
		alpha = 1.0;
		beta = 1.0;
		}
	else
		{
		alpha = -1.0;
		beta = 1.0;
		}

	// TODO remove
	int k = cv[nv-1];

	const int bs = 4;

	int i, j, l;
	int ii, iii, jj, kii, kiii, kjj, k0, k1;

	i = 0;
	ii = 0;
	iii = 0;

#if defined(TARGET_X64_INTEL_SANDY_BRIDGE) || defined(TARGET_X64_INTEL_HASWELL)
	for(; i<m-7; i+=8)
		{

		while(ii<nv && rv[ii]<i+8)
			ii++;
		if(ii<nv)
			kii = cv[ii];
		else
			kii = cv[ii-1];

		j = 0;
		jj = 0;
		for(; j<i && j<n-3; j+=4)
			{

			while(jj<nv && rv[jj]<j+4)
				jj++;
			if(jj<nv)
				kjj = cv[jj];
			else
				kjj = cv[jj-1];
			k0 = kii<kjj ? kii : kjj;

			kernel_dgemm_nt_8x4_lib4(k0, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd);
			}
		if(j<n)
			{

			while(jj<nv && rv[jj]<j+4)
				jj++;
			if(jj<nv)
				kjj = cv[jj];
			else
				kjj = cv[jj-1];
			k0 = kii<kjj ? kii : kjj;

			if(j<i) // dgemm
				{
				kernel_dgemm_nt_8x4_vs_lib4(k0, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, 8, n-j);
				}
			else // dsyrk
				{
				kernel_dsyrk_nt_l_8x4_vs_lib4(k0, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, 8, n-j);
				if(j<n-4)
					{
					kernel_dsyrk_nt_l_4x4_vs_lib4(k, &alpha, &pA[(i+4)*sda], &pB[(j+4)*sdb], &beta, &pC[(j+4)*bs+(i+4)*sdc], &pD[(j+4)*bs+(i+4)*sdd], 4, n-j-4); // TODO
					}
				}
			}
		}
	if(m>i)
		{
		if(m-i<=4)
			{
			goto left_4;
			}
		else
			{
			goto left_8;
			}
		}
#else
	for(; i<m-3; i+=4)
		{

		while(ii<nv && rv[ii]<i+4)
			ii++;
		if(ii<nv)
			kii = cv[ii];
		else
			kii = cv[ii-1];
//		k0 = kii;
//		printf("\nii %d %d %d %d %d\n", i, ii, rv[ii], cv[ii], kii);

		j = 0;
		jj = 0;
		for(; j<i && j<n-3; j+=4)
			{

			while(jj<nv && rv[jj]<j+4)
				jj++;
			if(jj<nv)
				kjj = cv[jj];
			else
				kjj = cv[jj-1];
			k0 = kii<kjj ? kii : kjj;
//			printf("\njj %d %d %d %d %d\n", j, jj, rv[jj], cv[jj], kjj);

			kernel_dgemm_nt_4x4_lib4(k0, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd]);
			}
		if(j<n)
			{

			while(jj<nv && rv[jj]<j+4)
				jj++;
			if(jj<nv)
				kjj = cv[jj];
			else
				kjj = cv[jj-1];
			k0 = kii<kjj ? kii : kjj;
//			printf("\njj %d %d %d %d %d\n", j, jj, rv[jj], cv[jj], kjj);

			if(i<j) // dgemm
				{
				kernel_dgemm_nt_4x4_vs_lib4(k0, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], 4, n-j);
				}
			else // dsyrk
				{
				kernel_dsyrk_nt_l_4x4_vs_lib4(k0, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], 4, n-j);
				}
			}
		}
	if(m>i)
		{
		goto left_4;
		}
#endif

	// common return if i==m
	return;

	// clean up loops definitions

#if defined(TARGET_X64_INTEL_SANDY_BRIDGE) || defined(TARGET_X64_INTEL_HASWELL)
	left_8:

	kii = cv[nv-1];

	j = 0;
	jj = 0;
	for(; j<i && j<n-3; j+=4)
		{

		while(jj<nv && rv[jj]<j+4)
			jj++;
		if(jj<nv)
			kjj = cv[jj];
		else
			kjj = cv[jj-1];
		k0 = kii<kjj ? kii : kjj;

		kernel_dgemm_nt_8x4_vs_lib4(k0, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
		}
	if(j<n)
		{

		while(jj<nv && rv[jj]<j+4)
			jj++;
		if(jj<nv)
			kjj = cv[jj];
		else
			kjj = cv[jj-1];
		k0 = kii<kjj ? kii : kjj;

		if(j<i) // dgemm
			{
			kernel_dgemm_nt_8x4_vs_lib4(k0, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
			}
		else // dsyrk
			{
			kernel_dsyrk_nt_l_8x4_vs_lib4(k0, &alpha, &pA[i*sda], sda, &pB[j*sdb], &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd, m-i, n-j);
			if(j<n-4)
				{
				kernel_dsyrk_nt_l_4x4_vs_lib4(k, &alpha, &pA[(i+4)*sda], &pB[(j+4)*sdb], &beta, &pC[(j+4)*bs+(i+4)*sdc], &pD[(j+4)*bs+(i+4)*sdd], m-i-4, n-j-4); // TODO
				}
			}
		}
	return;
#endif

	left_4:

	kii = cv[nv-1];

	j = 0;
	jj = 0;
	for(; j<i && j<n-3; j+=4)
		{

		while(jj<nv && rv[jj]<j+4)
			jj++;
		if(jj<nv)
			kjj = cv[jj];
		else
			kjj = cv[jj-1];
		k0 = kii<kjj ? kii : kjj;

		kernel_dgemm_nt_4x4_vs_lib4(k0, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
		}
	if(j<n)
		{

		while(jj<nv && rv[jj]<j+4)
			jj++;
		if(jj<nv)
			kjj = cv[jj];
		else
			kjj = cv[jj-1];
		k0 = kii<kjj ? kii : kjj;

		if(j<i) // dgemm
			{
			kernel_dgemm_nt_4x4_vs_lib4(k0, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
			}
		else // dsyrk
			{
			kernel_dsyrk_nt_l_4x4_vs_lib4(k0, &alpha, &pA[i*sda], &pB[j*sdb], &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd], m-i, n-j);
			}
		}
	return;

	}
#endif



/****************************
* new interface
****************************/



#if defined(LA_HIGH_PERFORMANCE)



// dgemm nt
void dgemm_nt_libstr(int m, int n, int k, double alpha, struct d_strmat *sA, int ai, int aj, struct d_strmat *sB, int bi, int bj, double beta, struct d_strmat *sC, int ci, int cj, struct d_strmat *sD, int di, int dj)
	{

	if(m<=0 | n<=0)
		return;
	
	const int bs = 4;

	int sda = sA->cn;
	int sdb = sB->cn;
	int sdc = sC->cn;
	int sdd = sD->cn;
	int air = ai & (bs-1);
	int bir = bi & (bs-1);
	double *pA = sA->pA + aj*bs + (ai-air)*sda;
	double *pB = sB->pA + bj*bs + (bi-bir)*sda;
	double *pC = sC->pA + cj*bs;
	double *pD = sD->pA + dj*bs;

	if(ai==0 & bi==0 & ci==0 & di==0)
		{
		dgemm_nt_lib(m, n, k, alpha, pA, sda, pB, sdb, beta, pC, sdc, pD, sdd); 
		return;
		}
	
	int ci0 = ci-air;
	int di0 = di-air;
	int offsetC;
	int offsetD;
	if(ci0>=0)
		{
		pC += ci0/bs*bs*sdd;
		offsetC = ci0%bs;
		}
	else
		{
		pC += -4*sdc;
		offsetC = bs+ci0;
		}
	if(di0>=0)
		{
		pD += di0/bs*bs*sdd;
		offsetD = di0%bs;
		}
	else
		{
		pD += -4*sdd;
		offsetD = bs+di0;
		}
	
	int i, j, l;

	int idxB;

	i = 0;
	// clean up at the beginning
	if(air!=0)
		{
#if defined(TARGET_X64_INTEL_HASWELL) || defined(TARGET_X64_INTEL_SANDY_BRIDGE)
		if(m-i>5)
			{
			j = 0;
			idxB = 0;
			// clean up at the beginning
			if(bir!=0)
				{
				kernel_dgemm_nt_8x4_gen_lib4(k, &alpha, &pA[i*sda], sda, &pB[idxB*sdb], &beta, offsetC, &pC[j*bs+i*sdc]-bir*bs, sdc, offsetD, &pD[j*bs+i*sdd]-bir*bs, sdd, air, m-i, bir, n-j);
				j += bs-bir;
				idxB += 4;
				}
			// main loop
			for(; j<n; j+=4)
				{
				kernel_dgemm_nt_8x4_gen_lib4(k, &alpha, &pA[i*sda], sda, &pB[idxB*sdb], &beta, offsetC, &pC[j*bs+i*sdc], sdc, offsetD, &pD[j*bs+i*sdd], sdd, air, m-i, 0, n-j);
				idxB += 4;
				}
			m -= 2*bs-air;
			pA += 2*bs*sda;
			pC += 2*bs*sdc;
			pD += 2*bs*sdd;
			}
		else // m-i<=4
			{
#endif
			j = 0;
			idxB = 0;
			// clean up at the beginning
			if(bir!=0)
				{
				kernel_dgemm_nt_4x4_gen_lib4(k, &alpha, &pA[i*sda], &pB[idxB*sdb], &beta, offsetC, &pC[j*bs+i*sdc]-bir*bs, sdc, offsetD, &pD[j*bs+i*sdd]-bir*bs, sdd, air, m-i, bir, n-j);
				j += bs-bir;
				idxB += 4;
				}
			// main loop
			for(; j<n; j+=4)
				{
				kernel_dgemm_nt_4x4_gen_lib4(k, &alpha, &pA[i*sda], &pB[idxB*sdb], &beta, offsetC, &pC[j*bs+i*sdc], sdc, offsetD, &pD[j*bs+i*sdd], sdd, air, m-i, 0, n-j);
				idxB += 4;
				}
			m -= bs-air;
			pA += bs*sda;
			pC += bs*sdc;
			pD += bs*sdd;
#if defined(TARGET_X64_INTEL_HASWELL) || defined(TARGET_X64_INTEL_SANDY_BRIDGE)
			// nothing more to do
			return;
			}
#endif
		}
	// main loop
#if defined(TARGET_X64_INTEL_HASWELL) || defined(TARGET_X64_INTEL_SANDY_BRIDGE)
	for(; i<m-4; i+=8)
		{
		j = 0;
		idxB = 0;
		// clean up at the beginning
		if(bir!=0)
			{
			kernel_dgemm_nt_8x4_gen_lib4(k, &alpha, &pA[i*sda], sda, &pB[idxB*sdb], &beta, offsetC, &pC[j*bs+i*sdc]-bir*bs, sdc, offsetD, &pD[j*bs+i*sdd]-bir*bs, sdd, 0, m-i, bir, n-j);
			j += bs-bir;
			idxB += 4;
			}
		// main loop
		for(; j<n; j+=4)
			{
			kernel_dgemm_nt_8x4_gen_lib4(k, &alpha, &pA[i*sda], sda, &pB[idxB*sdb], &beta, offsetC, &pC[j*bs+i*sdc], sdc, offsetD, &pD[j*bs+i*sdd], sdd, 0, m-i, 0, n-j);
			idxB += 4;
			}
		}
	if(i<m)
		{
		j = 0;
		idxB = 0;
		// clean up at the beginning
		if(bir!=0)
			{
			kernel_dgemm_nt_4x4_gen_lib4(k, &alpha, &pA[i*sda], &pB[idxB*sdb], &beta, offsetC, &pC[j*bs+i*sdc]-bir*bs, sdc, offsetD, &pD[j*bs+i*sdd]-bir*bs, sdd, 0, m-i, bir, n-j);
			j += bs-bir;
			idxB += 4;
			}
		// main loop
		for(; j<n; j+=4)
			{
			kernel_dgemm_nt_4x4_gen_lib4(k, &alpha, &pA[i*sda], &pB[idxB*sdb], &beta, offsetC, &pC[j*bs+i*sdc], sdc, offsetD, &pD[j*bs+i*sdd], sdd, 0, m-i, 0, n-j);
			idxB += 4;
			}
		}
#else
	for(; i<m; i+=4)
		{
		j = 0;
		idxB = 0;
		// clean up at the beginning
		if(bir!=0)
			{
			kernel_dgemm_nt_4x4_gen_lib4(k, &alpha, &pA[i*sda], &pB[idxB*sdb], &beta, offsetC, &pC[j*bs+i*sdc]-bir*bs, sdc, offsetD, &pD[j*bs+i*sdd]-bir*bs, sdd, 0, m-i, bir, n-j);
			j += bs-bir;
			idxB += 4;
			}
		// main loop
		for(; j<n; j+=4)
			{
			kernel_dgemm_nt_4x4_gen_lib4(k, &alpha, &pA[i*sda], &pB[idxB*sdb], &beta, offsetC, &pC[j*bs+i*sdc], sdc, offsetD, &pD[j*bs+i*sdd], sdd, 0, m-i, 0, n-j);
			idxB += 4;
			}
		}
#endif

	return;

	}



// dgemm nn
void dgemm_nn_libstr(int m, int n, int k, double alpha, struct d_strmat *sA, int ai, int aj, struct d_strmat *sB, int bi, int bj, double beta, struct d_strmat *sC, int ci, int cj, struct d_strmat *sD, int di, int dj)
	{

	if(m<=0 || n<=0)
		return;

	const int bs = 4;

	int sda = sA->cn;
	int sdb = sB->cn;
	int sdc = sC->cn;
	int sdd = sD->cn;

	double *pA = sA->pA + aj*bs + ai/bs*bs*sda;
	double *pB = sB->pA + bj*bs + bi/bs*bs*sdb;
	double *pC = sC->pA + cj*bs;
	double *pD = sD->pA + dj*bs;

	int offsetB = bi%bs;

	int ci0 = ci-ai%bs;
	int di0 = di-ai%bs;
	int offsetC;
	int offsetD;
	if(ci0>=0)
		{
		pC += ci0/bs*bs*sdd;
		offsetC = ci0%bs;
		}
	else
		{
		pC += -4*sdc;
		offsetC = bs+ci0;
		}
	if(di0>=0)
		{
		pD += di0/bs*bs*sdd;
		offsetD = di0%bs;
		}
	else
		{
		pD += -4*sdd;
		offsetD = bs+di0;
		}
	
	int i, j, l;

	// clean up at the beginning
	if(ai%bs!=0)
		{
#if defined(TARGET_X64_INTEL_SANDY_BRIDGE) || defined(TARGET_X64_INTEL_HASWELL)
		if(m>5)
			{
			j = 0;
			for(; j<n; j+=4)
				{
				kernel_dgemm_nn_8x4_gen_lib4(k, &alpha, &pA[0], sda, offsetB, &pB[j*bs], sdb, &beta, offsetC, &pC[j*bs], sdc, offsetD, &pD[j*bs], sdd, ai%bs, m, 0, n-j);
				}
			m -= 2*bs-ai%bs;
			pA += 2*bs*sda;
			pC += 2*bs*sda;
			pD += 2*bs*sda;
			}
		else // m-i<=4
			{
#endif
			j = 0;
			for(; j<n; j+=4)
				{
				kernel_dgemm_nn_4x4_gen_lib4(k, &alpha, &pA[0], offsetB, &pB[j*bs], sdb, &beta, offsetC, &pC[j*bs], sdc, offsetD, &pD[j*bs], sdd, ai%bs, m, 0, n-j);
				}
			m -= 2*bs-ai%bs;
			pA += 2*bs*sda;
			pC += 2*bs*sda;
			pD += 2*bs*sda;
#if defined(TARGET_X64_INTEL_SANDY_BRIDGE) || defined(TARGET_X64_INTEL_HASWELL)
			// nothing more to do
			return;
			}
#endif
		}
	// main loop
	i = 0;
	if(offsetC==0 & offsetD==0)
		{
#if defined(TARGET_X64_INTEL_HASWELL)
		for(; i<m-11; i+=12)
			{
			j = 0;
			for(; j<n-3; j+=4)
				{
				kernel_dgemm_nn_12x4_lib4(k, &alpha, &pA[i*sda], sda, offsetB, &pB[j*bs], sdb, &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd);
				}
			if(j<n)
				{
//				kernel_dgemm_nn_12x4_gen_lib4(k, &alpha, &pA[i*sda], sda, offsetB, &pB[j*bs], sdb, &beta, 0, &pC[j*bs+i*sdc], sdc, 0, &pD[j*bs+i*sdd], sdd, m-i, n-j);
				kernel_dgemm_nn_8x4_gen_lib4(k, &alpha, &pA[i*sda], sda, offsetB, &pB[j*bs], sdb, &beta, 0, &pC[j*bs+i*sdc], sdc, 0, &pD[j*bs+i*sdd], sdd, 0, m-i, 0, n-j);
				kernel_dgemm_nn_4x4_gen_lib4(k, &alpha, &pA[(i+8)*sda], offsetB, &pB[j*bs], sdb, &beta, 0, &pC[j*bs+(i+8)*sdc], sdc, 0, &pD[j*bs+(i+8)*sdd], sdd, 0, m-(i+8), 0, n-j);
				}
			}
		if(m>i)
			{
			if(m-i<=4)
				{
				goto left_4;
				}
			else if(m-i<=8)
				{
				goto left_8;
				}
			else
				{
				goto left_12;
				}
			}
#elif defined(TARGET_X64_INTEL_SANDY_BRIDGE)
		for(; i<m-7; i+=8)
			{
			j = 0;
			for(; j<n-3; j+=4)
				{
				kernel_dgemm_nn_8x4_lib4(k, &alpha, &pA[i*sda], sda, offsetB, &pB[j*bs], sdb, &beta, &pC[j*bs+i*sdc], sdc, &pD[j*bs+i*sdd], sdd);
				}
			if(j<n)
				{
				kernel_dgemm_nn_8x4_gen_lib4(k, &alpha, &pA[i*sda], sda, offsetB, &pB[j*bs], sdb, &beta, 0, &pC[j*bs+i*sdc], sdc, 0, &pD[j*bs+i*sdd], sdd, 0, m-i, 0, n-j);
				}
			}
		if(m>i)
			{
			if(m-i<=4)
				{
				goto left_4;
				}
			else
				{
				goto left_8;
				}
			}
#else
		for(; i<m-3; i+=4)
			{
			j = 0;
			for(; j<n-3; j+=4)
				{
				kernel_dgemm_nn_4x4_lib4(k, &alpha, &pA[i*sda], offsetB, &pB[j*bs], sdb, &beta, &pC[j*bs+i*sdc], &pD[j*bs+i*sdd]);
				}
			if(j<n)
				{
				kernel_dgemm_nn_4x4_gen_lib4(k, &alpha, &pA[i*sda], offsetB, &pB[j*bs], sdb, &beta, 0, &pC[j*bs+i*sdc], sdc, 0, &pD[j*bs+i*sdd], sdd, 0, m-i, 0, n-j);
				}
			}
		if(m>i)
			{
			goto left_4;
			}
#endif
		}
	else
		{
// TODO 12x4
#if defined(TARGET_X64_INTEL_HASWELL) || defined(TARGET_X64_INTEL_SANDY_BRIDGE)
		for(; i<m-4; i+=8)
			{
			j = 0;
			for(; j<n; j+=4)
				{
				kernel_dgemm_nn_8x4_gen_lib4(k, &alpha, &pA[i*sda], sda, offsetB, &pB[j*bs], sdb, &beta, offsetC, &pC[j*bs+i*sdc], sdc, offsetD, &pD[j*bs+i*sdd], sdd, 0, m-i, 0, n-j);
				}
			}
		if(m>i)
			{
			goto left_4;
			}
#else
		for(; i<m; i+=4)
			{
			j = 0;
			for(; j<n; j+=4)
				{
				kernel_dgemm_nn_4x4_gen_lib4(k, &alpha, &pA[i*sda], offsetB, &pB[j*bs], sdb, &beta, offsetC, &pC[j*bs+i*sdc], sdc, offsetD, &pD[j*bs+i*sdd], sdd, 0, m-i, 0, n-j);
				}
			}
#endif
		}

	// common return if i==m
	return;

	// clean up loops definitions

#if defined(TARGET_X64_INTEL_HASWELL)
	left_12:
	j = 0;
	for(; j<n; j+=4)
		{
//		kernel_dgemm_nn_12x4_gen_lib4(k, &alpha, &pA[i*sda], sda, offsetB, &pB[j*sdb], sdb, &beta, offsetC, &pC[j*bs+i*sdc], sdc, offsetD, &pD[j*bs+i*sdd], sdd, 0, m-i, 0, n-j);
		kernel_dgemm_nn_8x4_gen_lib4(k, &alpha, &pA[i*sda], sda, offsetB, &pB[j*sdb], sdb, &beta, offsetC, &pC[j*bs+i*sdc], sdc, offsetD, &pD[j*bs+i*sdd], sdd, 0, m-i, 0, n-j);
		kernel_dgemm_nn_4x4_gen_lib4(k, &alpha, &pA[(i+8)*sda], offsetB, &pB[j*sdb], sdb, &beta, offsetC, &pC[j*bs+(i+8)*sdc], sdc, offsetD, &pD[j*bs+(i+8)*sdd], sdd, 0, m-(i+8), 0, n-j);
		}
	return;
#endif

#if defined(TARGET_X64_INTEL_SANDY_BRIDGE) || defined(TARGET_X64_INTEL_HASWELL)
	left_8:
	j = 0;
	for(; j<n; j+=4)
		{
		kernel_dgemm_nn_8x4_gen_lib4(k, &alpha, &pA[i*sda], sda, offsetB, &pB[j*bs], sdb, &beta, offsetC, &pC[j*bs+i*sdc], sdc, offsetD, &pD[j*bs+i*sdd], sdd, 0, m-i, 0, n-j);
		}
	return;
#endif

	left_4:
	j = 0;
	for(; j<n; j+=4)
		{
		kernel_dgemm_nn_4x4_gen_lib4(k, &alpha, &pA[i*sda], offsetB, &pB[j*bs], sdb, &beta, offsetC, &pC[j*bs+i*sdc], sdc, offsetD, &pD[j*bs+i*sdd], sdd, 0, m-i, 0, n-j);
		}
	return;

	return;
	}
	


// dtrsm_nn_llu
void dtrsm_llnu_libstr(int m, int n, double alpha, struct d_strmat *sA, int ai, int aj, struct d_strmat *sB, int bi, int bj, struct d_strmat *sD, int di, int dj)
	{
	if(ai!=0 | bi!=0 | di!=0 | alpha!=1.0)
		{
		printf("\ndtrsm_llnu_libstr: feature not implemented yet: ai=%d, bi=%d, di=%d, alpha=%f\n", ai, bi, di, alpha);
		exit(1);
		}
	const int bs = 4;
	// TODO alpha
	int sda = sA->cn;
	int sdb = sB->cn;
	int sdd = sD->cn;
	double *pA = sA->pA + aj*bs;
	double *pB = sB->pA + bj*bs;
	double *pD = sD->pA + dj*bs;
	dtrsm_nn_ll_one_lib(m, n, pA, sda, pB, sdb, pD, sdd); 
	return;
	}



// dtrsm_nn_lun
void dtrsm_lunn_libstr(int m, int n, double alpha, struct d_strmat *sA, int ai, int aj, struct d_strmat *sB, int bi, int bj, struct d_strmat *sD, int di, int dj)
	{
	if(ai!=0 | bi!=0 | di!=0 | alpha!=1.0)
		{
		printf("\ndtrsm_lunn_libstr: feature not implemented yet: ai=%d, bi=%d, di=%d, alpha=%f\n", ai, bi, di, alpha);
		exit(1);
		}
	const int bs = 4;
	// TODO alpha
	int sda = sA->cn;
	int sdb = sB->cn;
	int sdd = sD->cn;
	double *pA = sA->pA + aj*bs;
	double *pB = sB->pA + bj*bs;
	double *pD = sD->pA + dj*bs;
	double *dA = sA->dA;
	int ii;
	if(ai==0 & aj==0)
		{
		if(sA->use_dA!=1)
			{
			ddiaex_lib(n, 1.0, ai, pA, sda, dA);
			for(ii=0; ii<n; ii++)
				dA[ii] = 1.0 / dA[ii];
			sA->use_dA = 1;
			}
		}
	else
		{
		ddiaex_lib(n, 1.0, ai, pA, sda, dA);
		for(ii=0; ii<n; ii++)
			dA[ii] = 1.0 / dA[ii];
		sA->use_dA = 0;
		}
	dtrsm_nn_lu_inv_lib(m, n, pA, sda, dA, pB, sdb, pD, sdd); 
	return;
	}



// dtrsm_right_lower_transposed_notunit
void dtrsm_rltn_libstr(int m, int n, double alpha, struct d_strmat *sA, int ai, int aj, struct d_strmat *sB, int bi, int bj, struct d_strmat *sD, int di, int dj)
	{
	if(ai!=0 | bi!=0 | di!=0 | alpha!=1.0)
		{
		printf("\ndtrsm_rltn_libstr: feature not implemented yet: ai=%d, bi=%d, di=%d, alpha=%f\n", ai, bi, di, alpha);
		exit(1);
		}
	const int bs = 4;
	// TODO alpha
	int sda = sA->cn;
	int sdb = sB->cn;
	int sdd = sD->cn;
	double *pA = sA->pA + aj*bs;
	double *pB = sB->pA + bj*bs;
	double *pD = sD->pA + dj*bs;
	double *dA = sA->dA;
	int ii;
	if(ai==0 & aj==0)
		{
		if(sA->use_dA!=1)
			{
			ddiaex_lib(n, 1.0, ai, pA, sda, dA);
			for(ii=0; ii<n; ii++)
				dA[ii] = 1.0 / dA[ii];
			sA->use_dA = 1;
			}
		}
	else
		{
		ddiaex_lib(n, 1.0, ai, pA, sda, dA);
		for(ii=0; ii<n; ii++)
			dA[ii] = 1.0 / dA[ii];
		sA->use_dA = 0;
		}
	dtrsm_nt_rl_inv_lib(m, n, pA, sda, dA, pB, sdb, pD, sdd); 
	return;
	}



// dtrsm_right_lower_transposed_unit
void dtrsm_rltu_libstr(int m, int n, double alpha, struct d_strmat *sA, int ai, int aj, struct d_strmat *sB, int bi, int bj, struct d_strmat *sD, int di, int dj)
	{
	if(ai!=0 | bi!=0 | di!=0 | alpha!=1.0)
		{
		printf("\ndtrsm_rltu_libstr: feature not implemented yet: ai=%d, bi=%d, di=%d, alpha=%f\n", ai, bi, di, alpha);
		exit(1);
		}
	const int bs = 4;
	// TODO alpha
	int sda = sA->cn;
	int sdb = sB->cn;
	int sdd = sD->cn;
	double *pA = sA->pA + aj*bs;
	double *pB = sB->pA + bj*bs;
	double *pD = sD->pA + dj*bs;
	dtrsm_nt_rl_one_lib(m, n, pA, sda, pB, sdb, pD, sdd); 
	return;
	}



// dtrsm_right_upper_transposed_notunit
void dtrsm_rutn_libstr(int m, int n, double alpha, struct d_strmat *sA, int ai, int aj, struct d_strmat *sB, int bi, int bj, struct d_strmat *sD, int di, int dj)
	{
	if(ai!=0 | bi!=0 | di!=0 | alpha!=1.0)
		{
		printf("\ndtrsm_rutn_libstr: feature not implemented yet: ai=%d, bi=%d, di=%d, alpha=%f\n", ai, bi, di, alpha);
		exit(1);
		}
	const int bs = D_BS;
	// TODO alpha
	int sda = sA->cn;
	int sdb = sB->cn;
	int sdd = sD->cn;
	double *pA = sA->pA + aj*bs;
	double *pB = sB->pA + bj*bs;
	double *pD = sD->pA + dj*bs;
	double *dA = sA->dA;
	int ii;
	if(ai==0 & aj==0)
		{
		if(sA->use_dA!=1)
			{
			ddiaex_lib(n, 1.0, ai, pA, sda, dA);
			for(ii=0; ii<n; ii++)
				dA[ii] = 1.0 / dA[ii];
			sA->use_dA = 1;
			}
		}
	else
		{
		ddiaex_lib(n, 1.0, ai, pA, sda, dA);
		for(ii=0; ii<n; ii++)
			dA[ii] = 1.0 / dA[ii];
		sA->use_dA = 0;
		}
	dtrsm_nt_ru_inv_lib(m, n, pA, sda, dA, pB, sdb, pD, sdd); 
	return;
	}



// dtrmm_right_upper_transposed_notunit (B, i.e. the first matrix, is triangular !!!)
void dtrmm_rutn_libstr(int m, int n, double alpha, struct d_strmat *sB, int bi, int bj, struct d_strmat *sA, int ai, int aj, struct d_strmat *sD, int di, int dj)
	{
	if(ai!=0 | bi!=0 | di!=0)
		{
		printf("\ndtrmm_rutn_libstr: feature not implemented yet: ai=%d, bi=%d, di=%d\n", ai, bi, di);
		exit(1);
		}
	const int bs = 4;
	int sda = sA->cn;
	int sdb = sB->cn;
	int sdd = sD->cn;
	double *pA = sA->pA + aj*bs;
	double *pB = sB->pA + bj*bs;
	double *pD = sD->pA + dj*bs;
	dtrmm_nt_ru_lib(m, n, alpha, pA, sda, pB, sdb, 0.0, pD, sdd, pD, sdd); 
	return;
	}



// dtrmm_right_lower_nottransposed_notunit (B, i.e. the first matrix, is triangular !!!)
void dtrmm_rlnn_libstr(int m, int n, double alpha, struct d_strmat *sB, int bi, int bj, struct d_strmat *sA, int ai, int aj, struct d_strmat *sD, int di, int dj)
	{

	const int bs = 4;

	int sda = sA->cn;
	int sdb = sB->cn;
	int sdd = sD->cn;
	double *pA = sA->pA + aj*bs + ai/bs*bs*sda;
	double *pB = sB->pA + bj*bs + bi/bs*bs*sdb;
	double *pD = sD->pA + dj*bs;

	int offsetB = bi%bs;

	int di0 = di-ai%bs;
	int offsetD;
	if(di0>=0)
		{
		pD += di0/bs*bs*sdd;
		offsetD = di0%bs;
		}
	else
		{
		pD += -4*sdd;
		offsetD = bs+di0;
		}
	
	int ii, jj;

	ii = 0;
	if(ai%bs!=0)
		{
		jj = 0;
		for(; jj<n; jj+=4)
			{
			kernel_dtrmm_nn_rl_4x4_gen_lib4(n-jj, &alpha, &pA[ii*sda+jj*bs], offsetB, &pB[jj*sdb+jj*bs], sdb, offsetD, &pD[ii*sdd+jj*bs], sdd, ai%bs, m-ii, 0, n-jj);
			}
		m -= bs-ai%bs;
		pA += bs*sda;
		pD += bs*sdd;
		}
	if(offsetD==0)
		{
#if defined(TARGET_X64_INTEL_HASWELL)
		for(; ii<m-11; ii+=12)
			{
			jj = 0;
			for(; jj<n-5; jj+=4)
				{
				kernel_dtrmm_nn_rl_12x4_lib4(n-jj, &alpha, &pA[ii*sda+jj*bs], sda, offsetB, &pB[jj*sdb+jj*bs], sdb, &pD[ii*sdd+jj*bs], sdd);
				}
			for(; jj<n; jj+=4)
				{
//				kernel_dtrmm_nn_rl_12x4_gen_lib4(n-jj, &alpha, &pA[ii*sda+jj*bs], sda, offsetB, &pB[jj*sdb+jj*bs], sdb, 0, &pD[ii*sdd+jj*bs], sdd, 0, 12, 0, n-jj);
				kernel_dtrmm_nn_rl_8x4_gen_lib4(n-jj, &alpha, &pA[ii*sda+jj*bs], sda, offsetB, &pB[jj*sdb+jj*bs], sdb, 0, &pD[ii*sdd+jj*bs], sdd, 0, 8, 0, n-jj);
				kernel_dtrmm_nn_rl_4x4_gen_lib4(n-jj, &alpha, &pA[(ii+8)*sda+jj*bs], offsetB, &pB[jj*sdb+jj*bs], sdb, 0, &pD[(ii+8)*sdd+jj*bs], sdd, 0, 4, 0, n-jj);
				}
			}
		if(ii<m)
			{
			if(ii<m-8)
				goto left_12;
			else if(ii<m-4)
				goto left_8;
			else
				goto left_4;
			}
#elif defined(TARGET_X64_INTEL_SANDY_BRIDGE)
		for(; ii<m-7; ii+=8)
			{
			jj = 0;
			for(; jj<n-5; jj+=4)
				{
				kernel_dtrmm_nn_rl_8x4_lib4(n-jj, &alpha, &pA[ii*sda+jj*bs], sda, offsetB, &pB[jj*sdb+jj*bs], sdb, &pD[ii*sdd+jj*bs], sdd);
				}
			for(; jj<n; jj+=4)
				{
				kernel_dtrmm_nn_rl_8x4_gen_lib4(n-jj, &alpha, &pA[ii*sda+jj*bs], sda, offsetB, &pB[jj*sdb+jj*bs], sdb, 0, &pD[ii*sdd+jj*bs], sdd, 0, 8, 0, n-jj);
				}
			}
		if(ii<m)
			{
			if(ii<m-4)
				goto left_8;
			else
				goto left_4;
			}
#else
		for(; ii<m-3; ii+=4)
			{
			jj = 0;
			for(; jj<n-5; jj+=4)
				{
				kernel_dtrmm_nn_rl_4x4_lib4(n-jj, &alpha, &pA[ii*sda+jj*bs], offsetB, &pB[jj*sdb+jj*bs], sdb, &pD[ii*sdd+jj*bs]);
				}
			for(; jj<n; jj+=4)
				{
				kernel_dtrmm_nn_rl_4x4_gen_lib4(n-jj, &alpha, &pA[ii*sda+jj*bs], offsetB, &pB[jj*sdb+jj*bs], sdb, 0, &pD[ii*sdd+jj*bs], sdd, 0, 4, 0, n-jj);
				}
			}
		if(ii<m)
			{
			goto left_4;
			}
#endif
		}
	else
		{
#if defined(TARGET_X64_INTEL_HASWELL) || defined(TARGET_X64_INTEL_SANDY_BRIDGE)
		for(; ii<m-4; ii+=8)
			{
			jj = 0;
			for(; jj<n; jj+=4)
				{
				kernel_dtrmm_nn_rl_8x4_gen_lib4(n-jj, &alpha, &pA[ii*sda+jj*bs], sda, offsetB, &pB[jj*sdb+jj*bs], sdb, offsetD, &pD[ii*sdd+jj*bs], sdd, 0, m-ii, 0, n-jj);
				}
			}
		if(ii<m)
			{
			goto left_4;
			}
#else
		for(; ii<m; ii+=4)
			{
			jj = 0;
			for(; jj<n; jj+=4)
				{
				kernel_dtrmm_nn_rl_4x4_gen_lib4(n-jj, &alpha, &pA[ii*sda+jj*bs], offsetB, &pB[jj*sdb+jj*bs], sdb, offsetD, &pD[ii*sdd+jj*bs], sdd, 0, m-ii, 0, n-jj);
				}
			}
#endif
		}

	// common return if i==m
	return;

	// clean up loops definitions

#if defined(TARGET_X64_INTEL_HASWELL)
	left_12:
	jj = 0;
	for(; jj<n; jj+=4)
		{
//		kernel_dtrmm_nn_rl_12x4_gen_lib4(n-jj, &alpha, &pA[ii*sda+jj*bs], sda, offsetB, &pB[jj*sdb+jj*bs], sdb, 0, &pD[ii*sdd+jj*bs], sdd, 0, m-ii, 0, n-jj);
		kernel_dtrmm_nn_rl_8x4_gen_lib4(n-jj, &alpha, &pA[ii*sda+jj*bs], sda, offsetB, &pB[jj*sdb+jj*bs], sdb, 0, &pD[ii*sdd+jj*bs], sdd, 0, 8, 0, n-jj);
		kernel_dtrmm_nn_rl_4x4_gen_lib4(n-jj, &alpha, &pA[(ii+8)*sda+jj*bs], offsetB, &pB[jj*sdb+jj*bs], sdb, 0, &pD[(ii+8)*sdd+jj*bs], sdd, 0, m-(ii+4), 0, n-jj);
		}
	return;
#endif

#if defined(TARGET_X64_INTEL_HASWELL) || defined(TARGET_X64_INTEL_SANDY_BRIDGE)
	left_8:
	jj = 0;
	for(; jj<n; jj+=4)
		{
		kernel_dtrmm_nn_rl_8x4_gen_lib4(n-jj, &alpha, &pA[ii*sda+jj*bs], sda, offsetB, &pB[jj*sdb+jj*bs], sdb, offsetD, &pD[ii*sdd+jj*bs], sdd, 0, m-ii, 0, n-jj);
		}
	return;
#endif

	left_4:
	jj = 0;
	for(; jj<n; jj+=4)
		{
		kernel_dtrmm_nn_rl_4x4_gen_lib4(n-jj, &alpha, &pA[ii*sda+jj*bs], offsetB, &pB[jj*sdb+jj*bs], sdb, offsetD, &pD[ii*sdd+jj*bs], sdd, 0, m-ii, 0, n-jj);
		}
	return;

	}



// dsyrk_lower_nortransposed (allowing for different factors !!!)
void dsyrk_ln_libstr(int m, int n, int k, double alpha, struct d_strmat *sA, int ai, int aj, struct d_strmat *sB, int bi, int bj, double beta, struct d_strmat *sC, int ci, int cj, struct d_strmat *sD, int di, int dj)
	{
	if(ai!=0 | bi!=0 | ci!=0 | di!=0)
		{
		printf("\ndryrk_ln_libstr: feature not implemented yet: ai=%d, bi=%d, ci=%d, di=%d\n", ai, bi, ci, di);
		exit(1);
		}
	const int bs = D_BS;
	int sda = sA->cn;
	int sdb = sB->cn;
	int sdc = sC->cn;
	int sdd = sD->cn;
	double *pA = sA->pA + aj*bs;
	double *pB = sB->pA + bj*bs;
	double *pC = sC->pA + cj*bs;
	double *pD = sD->pA + dj*bs;
	dsyrk_nt_l_lib(m, n, k, alpha, pA, sda, pB, sdb, beta, pC, sdc, pD, sdd);
	return;
	}



#else

#error : wrong LA choice

#endif



