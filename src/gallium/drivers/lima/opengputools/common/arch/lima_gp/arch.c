/* Author(s):
 *   Ben Brewer (ben.brewer@codethink.co.uk)
 *
 * Copyright (c) 2012
 *   Codethink (http://www.codethink.co.uk)
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



#include <ogt/arch.h>
#include <ogt/asm.h>
#include <ogt/binary.h>
#include <stdlib.h>



extern bool lima_gp_disassemble(
	void* code, unsigned size,
	ogt_link_map_t* map,
	ogt_asm_type_e type,
	ogt_asm_syntax_e syntax,
	unsigned tabs);

extern ogt_program_t* lima_gp_assemble(
	const char* code, unsigned size,
	ogt_link_map_t* map,
	ogt_asm_type_e type,
	ogt_asm_syntax_e syntax);

static const ogt_binary_format_t*
	ogt_arch_lima_gp__binary_format(void)
{
	return ogt_binary_format_mbs;
}



static const ogt_arch_t ogt_arch__lima_gp =
{
	.name          = "lima_gp",
	.type          = ogt_arch_type_unified,
	.binary_format = ogt_arch_lima_gp__binary_format,
	.disassemble   = lima_gp_disassemble,
	.assemble      = lima_gp_assemble,
};

const ogt_arch_t* ogt_arch_lima_gp = &ogt_arch__lima_gp;
