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



#include <ogt/arch.h>
#include <stdio.h>



bool ogt_arch_disassemble(
	const ogt_arch_t* arch,
	void* code, unsigned size,
	ogt_link_map_t* map,
	ogt_asm_type_e type,
	ogt_asm_syntax_e syntax,
	unsigned tabs)
{
	if (!arch
		|| (!code && (size > 0))
		|| !arch->disassemble)
		return false;

	switch (arch->type)
	{
		case ogt_arch_type_unified:
			switch (type)
			{
				case ogt_asm_type_geometry:
				case ogt_asm_type_vertex:
				case ogt_asm_type_fragment:
					break;
				default:
					fprintf(stderr, "Error: Shader type not specified"
						" for unified architecture.\n");
					return false;
			}
			break;
		case ogt_arch_type_geometry:
			if (type == ogt_asm_type_unknown)
				type = ogt_asm_type_geometry;
			else if (type != ogt_asm_type_geometry)
			{
				fprintf(stderr, "Error: Shader type doesn't match target"
					" (geometry shader).\n");
				return false;
			}
			break;
		case ogt_arch_type_vertex:
			if (type == ogt_asm_type_unknown)
				type = ogt_asm_type_vertex;
			else if (type != ogt_asm_type_vertex)
			{
				fprintf(stderr, "Error: Shader type doesn't match target"
					" (vertex shader).\n");
				return false;
			}
			break;
		case ogt_arch_type_fragment:
			if (type == ogt_asm_type_unknown)
				type = ogt_asm_type_fragment;
			else if (type != ogt_asm_type_fragment)
			{
				fprintf(stderr, "Error: Shader type doesn't match target"
					" (fragment shader).\n");
				return false;
			}
			break;
		default:
			return false;
	}

	return arch->disassemble(code, size, map, type, syntax, tabs);
}



ogt_program_t* ogt_arch_assemble(
	const ogt_arch_t* arch,
	const char* code, unsigned size,
	ogt_link_map_t* map,
	ogt_asm_type_e type,
	ogt_asm_syntax_e syntax)
{
	if (!arch
		|| (!code && (size > 0))
		|| !arch->assemble)
		return false;

	switch (arch->type)
	{
		case ogt_arch_type_unified:
			switch (type)
			{
				case ogt_asm_type_geometry:
				case ogt_asm_type_vertex:
				case ogt_asm_type_fragment:
					break;
				default:
					fprintf(stderr, "Error: Shader type not specified"
						" for unified architecture.\n");
					return false;
			}
			break;
		case ogt_arch_type_geometry:
			if (type == ogt_asm_type_unknown)
				type = ogt_asm_type_geometry;
			else if (type != ogt_asm_type_geometry)
			{
				fprintf(stderr, "Error: Shader type doesn't match target"
					" (geometry shader).\n");
				return false;
			}
			break;
		case ogt_arch_type_vertex:
			if (type == ogt_asm_type_unknown)
				type = ogt_asm_type_vertex;
			else if (type != ogt_asm_type_vertex)
			{
				fprintf(stderr, "Error: Shader type doesn't match target"
					" (vertex shader).\n");
				return false;
			}
			break;
		case ogt_arch_type_fragment:
			if (type == ogt_asm_type_unknown)
				type = ogt_asm_type_fragment;
			else if (type != ogt_asm_type_fragment)
			{
				fprintf(stderr, "Error: Shader type doesn't match target"
					" (fragment shader).\n");
				return false;
			}
			break;
		default:
			return false;
	}

	return arch->assemble(code, size, map, type, syntax);
}


const ogt_binary_format_t* ogt_arch_binary_format(
	const ogt_arch_t* arch)
{
	if (!arch
		|| !arch->binary_format)
		return NULL;
	return arch->binary_format();
}
