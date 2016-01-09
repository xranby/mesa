/* Author(s):
 *   Ben Brewer (ben.brewer@codethink.co.uk)
 *
 * Copyright (c) 2012 Codethink (http://www.codethink.co.uk)
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



#include <ogt/essl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



essl_program_t* essl_program_create()
{
	essl_program_t* program
		= (essl_program_t*)malloc(
			sizeof(essl_program_t));
	if (!program) return NULL;

	program->intp
		= essl_precision_undefined;
	program->floatp
		= essl_precision_undefined;

	program->symbol_count    = 0;
	program->structure_count = 0;
	program->function_count  = 0;

	program->symbol    = NULL;
	program->structure = NULL;
	program->function  = NULL;

	return program;
}

void essl_program_delete(
	essl_program_t* program)
{
	if (!program)
		return;

	if (program->symbol)
	{
		unsigned i;
		for (i = 0; i < program->symbol_count; i++)
			essl_symbol_delete(program->symbol[i]);
		free(program->symbol);
	}

	if (program->structure)
	{
		unsigned i;
		for (i = 0; i < program->structure_count; i++)
			free(program->structure[i]);
		free(program->structure);
	}

	if (program->function)
	{
		unsigned i;
		for (i = 0; i < program->function_count; i++)
			free(program->function[i]);
		free(program->function);
	}

	free(program);
}

bool essl_program_add_symbol(
	essl_program_t* program, essl_symbol_t* symbol)
{
	if (!program
		|| !symbol)
		return false;

	essl_symbol_t** nsymbol
		= (essl_symbol_t**)realloc(
			program->symbol,
			(program->symbol_count + 1)
				* sizeof(essl_symbol_t*));
	if (!nsymbol) return NULL;
	program->symbol = nsymbol;

	program->symbol[program->symbol_count++]
		= symbol;
	return true;
}

bool essl_program_add_structure(
	essl_program_t* program, essl_struct_t* structure)
{
	if (!program
		|| !structure)
		return false;

	/* TODO - Parse structures. */

	return false;
}

bool essl_program_add_function(
	essl_program_t* program, essl_function_t* function)
{
	if (!program
		|| !function)
		return false;

	/* TODO - Parse functions. */

	return false;
}

unsigned essl_program_parse(
	essl_program_t* program, const char* src)
{
	if (!program)
		return 0;

	unsigned i;
	
	i = essl_parse_default_precision_float(
		src, &program->floatp);
	if (i) return i;

	i = essl_parse_default_precision_int(
		src, &program->intp);
	if (i) return i;


	{
		essl_symbol_t* symbol;
		i = essl_parse_symbol(src,
			program->floatp,
			program->intp,
			&symbol);
		if (!i) return 0;
		if (!symbol) return i;

		if (essl_program_add_symbol(
			program, symbol))
			return i;
		free(symbol);
		return 0;
	}

	/* TODO - Parse structures/functions. */

	return 0;
}
