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



#include "freedreno.h"

#include <ogt/asm.h>

#include <stdio.h>




static const char* freedreno_vector_op_name[] =
{
	"add" , "mul" , "max"  , "min" ,
	"op04", "op05", "op06" , "op07",
	"op08", "op09", "floor", "mac" ,
	"op12", "op13", "op14" , "dot4",
	"dot3", "op17", "op18" , "op19",
	"op20", "op21", "op22" , "op23",
	"op24", "op25", "op26" , "op27",
	"op28", "op29", "op30" , "op31",
};

static const char* freedreno_scalar_op_name[] =
{
	"op00", "op01" , "mov" , "op03" ,
	"op04", "op05" , "op06", "exp2" ,
	"log2", "rcp"  , "op10", "rsqrt",
	"op12", "psete", "op14", "op15" ,
	"op16", "op17" , "op18", "op19" ,
	"sqrt", "mul"  , "add" , "op23" ,
	"op24", "op25" , "op26", "op27" ,
	"op28", "op29" , "op30", "op31" ,
};



static void freedreno_disassemble_fetch_field(
	freedreno_inst_fetch_t* code, unsigned tabs)
{
	ogt_asm_print_tabs(tabs++);
	printf("fetch {\n");

	ogt_asm_print_tabs(tabs);
	printf(".unknown_0 = 0x%02X\n", code->unknown_0);

	ogt_asm_print_tabs(tabs);
	printf(".src       = $%u\n", code->src);	

	ogt_asm_print_tabs(tabs);
	printf(".unknown_1 = 0x%01X\n", code->unknown_1);

	ogt_asm_print_tabs(tabs);
	printf(".dest      = $%u\n", code->dest);

	ogt_asm_print_tabs(tabs);
	printf(".unknown_2 = 0x%01X\n", code->unknown_2);

	ogt_asm_print_tabs(tabs);
	printf(".constant  = %u\n", code->constant);	

	ogt_asm_print_tabs(tabs);
	printf(".unknown_3 = 0x%02X\n", code->unknown_3);
	ogt_asm_print_tabs(tabs);
	printf(".unknown_4 = 0x%08X\n", code->unknown_4);
	ogt_asm_print_tabs(tabs);
	printf(".unknown_5 = 0x%08X\n", code->unknown_5);

	ogt_asm_print_tabs(--tabs);
	printf("}\n");
}

static void freedreno_disassemble_alu_field(
	freedreno_inst_alu_t* code, unsigned tabs)
{
	ogt_asm_print_tabs(tabs++);
	printf("alu {\n");

	ogt_asm_print_tabs(tabs);
	printf(".vector_dest      = $%u\n", code->vector_dest);

	ogt_asm_print_tabs(tabs);
	printf(".unknown_0        = 0x%01X\n", code->unknown_0);

	ogt_asm_print_tabs(tabs);
	printf(".scalar_dest      = $%u\n", code->scalar_dest);

	ogt_asm_print_tabs(tabs);
	printf(".unknown_1        = 0x%01X\n", code->unknown_1);

	ogt_asm_print_tabs(tabs);
	printf(".export           = %u\n", code->export);

	ogt_asm_print_tabs(tabs);
	printf(".vector_dest_mask = ");
	if (code->vector_dest_mask)
		ogt_asm_print_mask(code->vector_dest_mask);
	else
		printf("Nothing");
	printf("\n");

	ogt_asm_print_tabs(tabs);
	printf(".scalar_dest_mask = ");
	if (code->scalar_dest_mask)
		ogt_asm_print_mask(code->scalar_dest_mask);
	else
		printf("Nothing");
	printf("\n");

	ogt_asm_print_tabs(tabs);
	printf(".unknown_2        = 0x%01X\n", code->unknown_2);

	ogt_asm_print_tabs(tabs);
	printf(".scalar_op        = %s\n",
		freedreno_scalar_op_name[code->scalar_op]);

	ogt_asm_print_tabs(tabs);
	printf(".scalar_src0_com  = %c\n", "xyzw"[code->scalar_src0_com]);

	ogt_asm_print_tabs(tabs);	
	printf(".unknown_3        = 0x%01X\n", code->unknown_3);

	ogt_asm_print_tabs(tabs);
	printf(".scalar_src1_com  = %c\n", "xyzw"[code->scalar_src1_com]);

	ogt_asm_print_tabs(tabs);
	printf(".vector_src1_swiz = ");
	ogt_asm_print_swizzle(code->vector_src1_swiz);
	printf("\n");

	ogt_asm_print_tabs(tabs);
	printf(".vector_src0_swiz = ");
	ogt_asm_print_swizzle(code->vector_src0_swiz);
	printf("\n");

	ogt_asm_print_tabs(tabs);	
	printf(".unknown_4        = 0x%01X\n", code->unknown_4);

	ogt_asm_print_tabs(tabs);
	printf(".vector_src1_neg  = %u\n", code->vector_src1_neg);
	ogt_asm_print_tabs(tabs);
	printf(".vector_src0_neg  = %u\n", code->vector_src0_neg);

	ogt_asm_print_tabs(tabs);
	printf(".predicate_case   = %u\n", code->predicate_case);
	ogt_asm_print_tabs(tabs);
	printf(".predicate        = %u\n", code->predicate);

	ogt_asm_print_tabs(tabs);	
	printf(".unknown_5        = 0x%01X\n", code->unknown_5);

	ogt_asm_print_tabs(tabs);
	printf(".scalar_src2      = $%u\n", code->scalar_src2);
	ogt_asm_print_tabs(tabs);
	printf(".scalar_src2_flag = %u\n", code->scalar_src2_flag);
	ogt_asm_print_tabs(tabs);
	printf(".scalar_src2_abs  = %u\n", code->scalar_src2_abs);

	ogt_asm_print_tabs(tabs);
	printf(".scalar_src1      = $%u\n", code->scalar_src1);
	ogt_asm_print_tabs(tabs);
	printf(".scalar_src1_flag = %u\n", code->scalar_src1_flag);
	ogt_asm_print_tabs(tabs);
	printf(".scalar_src1_abs  = %u\n", code->scalar_src1_abs);

	ogt_asm_print_tabs(tabs);
	printf(".scalar_src0      = $%u\n", code->scalar_src0);
	ogt_asm_print_tabs(tabs);
	printf(".scalar_src0_flag = %u\n", code->scalar_src0_flag);
	ogt_asm_print_tabs(tabs);
	printf(".scalar_src0_abs  = %u\n", code->scalar_src0_abs);

	ogt_asm_print_tabs(tabs);
	printf(".vector_op        = %s\n",
		freedreno_vector_op_name[code->vector_op]);

	ogt_asm_print_tabs(tabs);
	printf(".scalar_src2_bank = %u\n", code->scalar_src2_bank);
	ogt_asm_print_tabs(tabs);
	printf(".vector_src1_bank = %u\n", code->vector_src1_bank);
	ogt_asm_print_tabs(tabs);
	printf(".vector_src0_bank = %u\n", code->vector_src0_bank);

	ogt_asm_print_tabs(--tabs);
	printf("}\n");
}

static void freedreno_disassemble_field(
	freedreno_inst_t* code, unsigned tabs)
{
	if (code->alu.scalar_src2_bank
		|| code->alu.vector_src1_bank
		|| code->alu.vector_src0_bank)
		freedreno_disassemble_alu_field(&code->alu, tabs);
	else
		freedreno_disassemble_fetch_field(&code->fetch, tabs);
}

static bool freedreno_disassemble_fields(
	void* code, unsigned size,
	unsigned tabs)
{
	/* TODO - Handle odd sizes. */
	if (size % sizeof(freedreno_inst_t))
		return false;

	unsigned i;
	for (i = 0; i < size; i += sizeof(freedreno_inst_t))
	{
		freedreno_disassemble_field(
			(void*)((uintptr_t)code + i), tabs);
	}

	return true;
}

bool freedreno_disassemble(
	void* code, unsigned size,
	ogt_link_map_t* map,
	ogt_asm_type_e type,
	ogt_asm_syntax_e syntax,
	unsigned tabs)
{
	(void)type; /* Not used (yet). */

	/* Print symbol table. */
	essl_program_t* program
		= ogt_link_map_export_essl(map);
	if (program)
	{
		essl_program_print(program, stdout, tabs);
		printf("\n");
	}

	switch (syntax)
	{
		case ogt_asm_syntax_fields:
			return freedreno_disassemble_fields(
				code, size, tabs);
		default:
			break;
	}

	return false;
}
