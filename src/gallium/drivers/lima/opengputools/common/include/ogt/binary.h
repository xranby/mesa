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



#ifndef __include_ogt_binary_h__
#define __include_ogt_binary_h__

#include <ogt/types.h>
#include <ogt/asm.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>



struct ogt_binary_format_s
{
	const char* name;
	bool           (*export_cb)(FILE* stream, ogt_program_t* program);
	ogt_program_t* (*import_cb)(FILE* stream);
	void           (*dump_cb)(
		void* data, uintptr_t size,
		ogt_asm_syntax_e syntax);
};


extern bool ogt_binary_export(
	const ogt_binary_format_t* format,
	FILE* stream, ogt_program_t* program);

extern ogt_program_t* ogt_binary_import(
	const ogt_binary_format_t* format,
	FILE* stream);
extern void ogt_binary_dump(
	const ogt_binary_format_t* format,
	void* data, uintptr_t size,
	ogt_asm_syntax_e syntax);


extern const ogt_binary_format_t* ogt_binary_format_raw;
extern const ogt_binary_format_t* ogt_binary_format_mbs;

#endif

