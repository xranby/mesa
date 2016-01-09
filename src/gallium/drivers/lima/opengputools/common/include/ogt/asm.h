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



#ifndef __ogt_asm_h__
#define __ogt_asm_h__

#include <stdbool.h>
#include <stdint.h>

#include <ogt/link.h>
#include <ogt/types.h>



typedef enum
{
	ogt_asm_syntax_raw = 0,
	ogt_asm_syntax_fields,
	ogt_asm_syntax_explicit,
	ogt_asm_syntax_verbose,
	ogt_asm_syntax_decompile,
	ogt_asm_syntax_count,
} ogt_asm_syntax_e;

typedef enum
{
	ogt_asm_type_unknown = 0,
	ogt_asm_type_geometry,
	ogt_asm_type_vertex,
	ogt_asm_type_fragment,
	ogt_asm_type_count,
} ogt_asm_type_e;


typedef bool (*ogt_asm_dsm_cb)(
	void* code, unsigned size,
	ogt_link_map_t* map,
	ogt_asm_type_e type,
	ogt_asm_syntax_e syntax,
	unsigned tabs);

typedef ogt_program_t* (*ogt_asm_asm_cb)(
	const char* code, unsigned size,
	ogt_link_map_t* map,
	ogt_asm_type_e type,
	ogt_asm_syntax_e syntax);


extern void ogt_asm_print_tabs(unsigned tabs);
extern void ogt_asm_print_mask(unsigned mask);
extern void ogt_asm_print_swizzle(unsigned swizzle);
extern void ogt_asm_print_bits(unsigned bits, unsigned count);

#endif
