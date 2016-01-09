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



#include <ogt/program.h>
#include <stdlib.h>
#include <string.h>



ogt_program_t* ogt_program_create(
	const ogt_arch_t* arch, ogt_asm_type_e type)
{
	if (!arch) return NULL;

	ogt_program_t* program
		= (ogt_program_t*)malloc(
			sizeof(ogt_program_t));
	if (!program) return NULL;

	program->symbol_table = NULL;
	program->link_map     = NULL;
	program->arch         = arch;
	program->type         = type;
	program->code         = NULL;
	program->code_size    = 0;

	/* TODO - Handle architecture specifics better. */
	program->attrib_prefetch = 0;

	return program;
}

void ogt_program_delete(ogt_program_t* program)
{
	if (!program) return;
	ogt_link_map_delete(program->link_map);
	essl_program_delete(program->symbol_table);
	free(program->code);
	free(program);
}



bool ogt_program_symbol_add(
	ogt_program_t* program, essl_symbol_t* symbol)
{
	if (!program || !symbol)
		return false;

	if (program->link_map)
		return ogt_link_map_place(
			program->link_map, symbol);

	if (!program->symbol_table)
	{
		program->symbol_table
			= essl_program_create();
		if (!program->symbol_table)
			return false;
	}
	return essl_program_add_symbol(
		program->symbol_table, symbol);
}

bool ogt_program_code_add(
	ogt_program_t* program, void* code, unsigned size)
{
	if (size == 0) return true;
	if (!program || !code) return false;

	void* rcode = realloc(program->code,
		(program->code_size + size));
	if (!rcode) return false;

	program->code = rcode;
	memcpy(
		(void*)((uintptr_t)program->code
			+ program->code_size),
		code, size);
	program->code_size += size;
	return true;
}
