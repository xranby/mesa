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
#include <stdio.h>
#include <inttypes.h>



static void _print_tabs(unsigned tabs)
{
	unsigned i;
	for (i = 0; i < tabs; i++)
		printf("\t");
}

static void _essl_symbol_print(essl_symbol_t* symbol,
	essl_parameter_qualifier_e parameter_qualifier,
	FILE* stream, unsigned tabs)
{
	if (!symbol
		|| !symbol->name
		|| (symbol->name[0] == '\0')
		|| (symbol->type >= essl_type_count)
		|| (symbol->storage_qualifier >= essl_storage_qualifier_count)
		|| (symbol->precision >= essl_precision_count))
		return;

	const char* type_name;
	if (symbol->type == essl_type_struct)
	{
		essl_struct_t* structure
			= (essl_struct_t*)symbol->structure;
		if (!structure
			|| !structure->name
			|| (structure->name[0] == '\0'))
			return;
		type_name = structure->name;
	} else {
		type_name = essl_type_name[symbol->type];
	}

	if (symbol->invariant)
		fprintf(stream, "invariant ");

	fprintf(stream, "%-9s",
		essl_storage_qualifier_name[symbol->storage_qualifier]);
	if (parameter_qualifier
		!= essl_parameter_qualifier_none)
		fprintf(stream, " %s",
			essl_parameter_qualifier_name[parameter_qualifier]);

	if (symbol->precision != essl_precision_undefined)
		fprintf(stream, " %-7s",
			essl_precision_name[symbol->precision]);

	fprintf(stream, " %-5s %s",
		type_name,
		symbol->name);

	if (symbol->count)
		fprintf(stream, "[%u]", symbol->count);

	if (symbol->value)
	{
		fprintf(stream, " = ");
		switch (symbol->type)
		{
			case essl_type_bool:
				fprintf(stream, "%s", (symbol->value->i ? "true" : "false"));
				break;
			case essl_type_sampler2D:
			case essl_type_samplerCube:
			case essl_type_int:
				fprintf(stream, "%"PRIi64, symbol->value->i);
				break;
			case essl_type_float:
				fprintf(stream, "%g", symbol->value->f);
				break;
			case essl_type_vec2:
				fprintf(stream,
					"vec2(%g, %g)",
					symbol->value[0].f,
					symbol->value[1].f);
				break;
			case essl_type_vec3:
				fprintf(stream,
					"vec3(%g, %g, %g)",
					symbol->value[0].f,
					symbol->value[1].f,
					symbol->value[2].f);
				break;
			case essl_type_vec4:
				fprintf(stream,
					"vec4(%g, %g, %g, %g)",
					symbol->value[0].f,
					symbol->value[1].f,
					symbol->value[2].f,
					symbol->value[3].f);
				break;
			case essl_type_bvec2:
				fprintf(stream,
					"bvec2(%s, %s)",
					(symbol->value[0].i ? "true" : "false"),
					(symbol->value[1].i ? "true" : "false"));
				break;
			case essl_type_bvec3:
				fprintf(stream,
					"bvec3(%s, %s, %s)",
					(symbol->value[0].i ? "true" : "false"),
					(symbol->value[1].i ? "true" : "false"),
					(symbol->value[2].i ? "true" : "false"));
				break;
			case essl_type_bvec4:
				fprintf(stream,
					"bvec4(%s, %s, %s, %s)",
					(symbol->value[0].i ? "true" : "false"),
					(symbol->value[1].i ? "true" : "false"),
					(symbol->value[2].i ? "true" : "false"),
					(symbol->value[3].i ? "true" : "false"));
				break;
			case essl_type_ivec2:
				fprintf(stream,
					"ivec2(%"PRIi64", %"PRIi64")",
					symbol->value[0].i,
					symbol->value[1].i);
				break;
			case essl_type_ivec3:
				fprintf(stream,
					"ivec3(%"PRIi64", %"PRIi64", %"PRIi64")",
					symbol->value[0].i,
					symbol->value[1].i,
					symbol->value[2].i);
				break;
			case essl_type_ivec4:
				fprintf(stream,
					"ivec4(%"PRIi64", %"PRIi64", %"PRIi64", %"PRIi64")",
					symbol->value[0].i,
					symbol->value[1].i,
					symbol->value[2].i,
					symbol->value[3].i);
				break;
			case essl_type_mat2:
				printf("mat2(\n");
				_print_tabs(tabs);
				printf("\t%g, %g,\n",
					symbol->value[0].f,
					symbol->value[1].f);
				_print_tabs(tabs);
				printf("\t%g, %g)",
					symbol->value[2].f,
					symbol->value[3].f);
				break;
			case essl_type_mat3:
				printf("mat3(\n");
				_print_tabs(tabs);
				printf("\t%g, %g, %g,\n",
					symbol->value[0].f,
					symbol->value[1].f,
					symbol->value[2].f);
				_print_tabs(tabs);
				printf("\t%g, %g, %g,\n",
					symbol->value[3].f,
					symbol->value[4].f,
					symbol->value[5].f);
				_print_tabs(tabs);
				printf("\t%g, %g, %g)",
					symbol->value[6].f,
					symbol->value[7].f,
					symbol->value[8].f);
				break;
			case essl_type_mat4:
				printf("mat4(\n");
				_print_tabs(tabs);
				printf("\t%g, %g, %g, %g,\n",
					symbol->value[ 0].f,
					symbol->value[ 1].f,
					symbol->value[ 2].f,
					symbol->value[ 3].f);
				_print_tabs(tabs);
				printf("\t%g, %g, %g, %g,\n",
					symbol->value[ 4].f,
					symbol->value[ 5].f,
					symbol->value[ 6].f,
					symbol->value[ 7].f);
				_print_tabs(tabs);
				printf("\t%g, %g, %g, %g,\n",
					symbol->value[ 8].f,
					symbol->value[ 9].f,
					symbol->value[10].f,
					symbol->value[11].f);
				_print_tabs(tabs);
				printf("\t%g, %g, %g, %g)",
					symbol->value[12].f,
					symbol->value[13].f,
					symbol->value[14].f,
					symbol->value[15].f);
				break;
			/*case essl_type_struct: TODO*/
			default:
				fprintf(stream, "{0}");
				break;
		};

		// TODO - Print value(s).
	}
}

void essl_symbol_print(essl_symbol_t* symbol,
	FILE* stream, unsigned tabs)
{
	_essl_symbol_print(symbol,
		essl_parameter_qualifier_none, stream, tabs);
	fprintf(stream, ";\n");
}

void essl_struct_print(essl_struct_t* structure,
	FILE* stream, unsigned tabs)
{
	if (!structure
		|| !structure->name
		|| (structure->name[0] == '\0')
		|| !structure->member
		|| (structure->count == 0))
		return;

	fprintf(stream, "struct %s {\n",
		structure->name);
	unsigned i;
	for (i = 0; i < structure->count; i++)
	{
		_print_tabs(tabs);
		essl_symbol_print(&structure->member[i], stream, tabs);
	}
	_print_tabs(tabs);
	fprintf(stream, "};\n");
}

void essl_function_print(essl_function_t* function,
	FILE* stream, unsigned tabs)
{
	if (!function)
		return;
	if ((function->count > 0)
		&& (!function->parameter
			|| !function->qualifier))
		return;

	_essl_symbol_print(&function->function,
		essl_parameter_qualifier_none, stream, tabs);
	printf("(");
	unsigned i;
	for (i = 0; i < function->count; i++)
	{
		if (i) fprintf(stream, ", ");
		_essl_symbol_print(&function->parameter[i],
			function->qualifier[i], stream, tabs);
	}
	printf(");\n");
}

void essl_program_print(
	essl_program_t* program,
	FILE* stream, unsigned tabs)
{
	if (!program)
		return;

	bool newline = false;
	if (program->floatp
		< essl_precision_undefined)
	{
		_print_tabs(tabs);
		fprintf(stream, "precision %s float;\n",
			essl_precision_name[program->floatp]);
		newline = true;
	}
	if (program->intp
		< essl_precision_undefined)
	{
		_print_tabs(tabs);
		fprintf(stream, "precision %s int;\n",
			essl_precision_name[program->intp]);
		newline = true;
	}

	if (program->structure)
	{
		if (newline)
			fprintf(stream, "\n");

		unsigned i;
		for (i = 0; i < program->structure_count; i++)
			essl_struct_print(program->structure[i], stream, (tabs + 1));
		newline = true;
	}

	if (program->symbol)
	{
		if (newline)
			fprintf(stream, "\n");

		unsigned i;
		for (i = 0; i < program->symbol_count; i++)
		{
			if (program->symbol[i]
				&& program->symbol[i]->name
				&& (program->symbol[i]->name[0] == '?'))
				continue;
			_print_tabs(tabs);
			essl_symbol_print(program->symbol[i], stream, (tabs + 1));
		}
		newline = true;
	}

	if (program->function)
	{
		if (newline)
			fprintf(stream, "\n");

		unsigned i;
		for (i = 0; i < program->function_count; i++)
		{
			_print_tabs(tabs);
			essl_function_print(program->function[i], stream, (tabs + 1));
		}
		newline = true;
	}
}
