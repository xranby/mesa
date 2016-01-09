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



#ifndef __ogt_program_h__
#define __ogt_program_h__

#include <ogt/essl.h>
#include <ogt/link.h>
#include <ogt/types.h>
#include <ogt/asm.h>



struct ogt_program_s
{
	essl_program_t* symbol_table;
	ogt_link_map_t* link_map;
	ogt_arch_t*     arch;
	ogt_asm_type_e  type;
	void*           code;
	unsigned        code_size;
	uint32_t        attrib_prefetch;
};


extern ogt_program_t* ogt_program_create(
	const ogt_arch_t* arch, ogt_asm_type_e type);
extern void           ogt_program_delete(
	ogt_program_t* program);

extern bool ogt_program_symbol_add(
	ogt_program_t* program, essl_symbol_t* symbol);
extern bool ogt_program_code_add(
	ogt_program_t* program, void* code, unsigned size);

#endif
