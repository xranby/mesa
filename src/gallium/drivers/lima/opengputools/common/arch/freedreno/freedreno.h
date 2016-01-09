/* Author(s):
 *   Ben Brewer (ben.brewer@codethink.co.uk)
 *   Rob Clark
 *
 * Copyright (c) 2012
 *   Codethink (http://www.codethink.co.uk)
 *   Rob Clark (robclark@gmail.com)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */



#ifndef __ogt_arch_freedreno_h__
#define __ogt_arch_freedreno_h__

#include <stdbool.h>
#include <stdint.h>



typedef enum
{
	freedreno_component_x = 0,
	freedreno_component_y = 1,
	freedreno_component_z = 2,
	freedreno_component_w = 3,
} freedreno_component_e;

typedef enum
{
	freedreno_vector_op_add   =  0,
	freedreno_vector_op_mul   =  1,
	freedreno_vector_op_max   =  2,
	freedreno_vector_op_min   =  3,
	freedreno_vector_op_floor = 10,
	freedreno_vector_op_mac   = 11,
	freedreno_vector_op_dot4  = 15,
	freedreno_vector_op_dot3  = 16,
} freedreno_vector_op_e;

typedef enum
{
	freedreno_scalar_op_mov   =  2,
	freedreno_scalar_op_exp2  =  7,
	freedreno_scalar_op_log2  =  8,
	freedreno_scalar_op_rcp   =  9,
	freedreno_scalar_op_rsqrt = 11,
	freedreno_scalar_op_psete = 13,
	freedreno_scalar_op_sqrt  = 20,
	freedreno_scalar_op_mul   = 21,
	freedreno_scalar_op_add   = 22,
} freedreno_scalar_op_e;



typedef struct
__attribute__((__packed__))
{
	unsigned unknown_0 :  5;
	unsigned src       :  5;
	unsigned unknown_1 :  3;
	unsigned dest      :  5;
	unsigned unknown_2 :  3;
	unsigned constant  :  4;
	unsigned unknown_3 :  8;
	unsigned unknown_4 : 32;
	unsigned unknown_5 : 32;
} freedreno_inst_fetch_t;

typedef struct
__attribute__((__packed__))
{
	unsigned              vector_dest      : 5;
	unsigned              unknown_0        : 3;
	unsigned              scalar_dest      : 5;
	unsigned              unknown_1        : 2;
	bool                  export           : 1;
	unsigned              vector_dest_mask : 4;
	unsigned              scalar_dest_mask : 4;
	unsigned              unknown_2        : 3;
	freedreno_scalar_op_e scalar_op        : 5;
	freedreno_component_e scalar_src0_com  : 2;
	unsigned              unknown_3        : 4;
	freedreno_component_e scalar_src1_com  : 2;
	unsigned              vector_src1_swiz : 8;
	unsigned              vector_src0_swiz : 8;
	unsigned              unknown_4        : 1;
	bool                  vector_src1_neg  : 1;
	bool                  vector_src0_neg  : 1;
	bool                  predicate_case   : 1;
	bool                  predicate        : 1;
	unsigned              unknown_5        : 3;
	unsigned              scalar_src2      : 5;
	unsigned              scalar_src2_flag : 2;
	bool                  scalar_src2_abs  : 1;
	unsigned              scalar_src1      : 5;
	unsigned              scalar_src1_flag : 2;
	bool                  scalar_src1_abs  : 1;
	unsigned              scalar_src0      : 5;
	unsigned              scalar_src0_flag : 2;
	bool                  scalar_src0_abs  : 1;
	freedreno_vector_op_e vector_op        : 5;
	unsigned              scalar_src2_bank : 1;
	unsigned              vector_src1_bank : 1;
	unsigned              vector_src0_bank : 1;
} freedreno_inst_alu_t;

typedef union
__attribute__((__packed__))
{
	freedreno_inst_fetch_t fetch;
	freedreno_inst_alu_t   alu;
} freedreno_inst_t;

#endif
