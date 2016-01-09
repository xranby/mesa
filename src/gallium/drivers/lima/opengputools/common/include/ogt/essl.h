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



#ifndef __essl_h__
#define __essl_h__

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	essl_type_void,
	essl_type_bool,
	essl_type_int,
	essl_type_float,
	essl_type_vec2,
	essl_type_vec3,
	essl_type_vec4,
	essl_type_bvec2,
	essl_type_bvec3,
	essl_type_bvec4,
	essl_type_ivec2,
	essl_type_ivec3,
	essl_type_ivec4,
	essl_type_mat2,
	essl_type_mat3,
	essl_type_mat4,
	essl_type_sampler2D,
	essl_type_samplerCube,
	essl_type_struct,
	essl_type_count
} essl_type_e;

typedef enum
{
	essl_storage_qualifier_none,
	essl_storage_qualifier_const,
	essl_storage_qualifier_attribute,
	essl_storage_qualifier_uniform,
	essl_storage_qualifier_varying,
	essl_storage_qualifier_count
} essl_storage_qualifier_e;

typedef enum
{
	essl_parameter_qualifier_none,
	essl_parameter_qualifier_in,
	essl_parameter_qualifier_out,
	essl_parameter_qualifier_inout,
	essl_parameter_qualifier_count
} essl_parameter_qualifier_e;

typedef enum
{
	essl_precision_low,
	essl_precision_medium,
	essl_precision_high,
	essl_precision_super,
	essl_precision_undefined,
	essl_precision_count
} essl_precision_e;

extern const char* essl_type_name[];
extern const char* essl_storage_qualifier_name[];
extern const char* essl_parameter_qualifier_name[];
extern const char* essl_precision_name[];

typedef union
{
	double   f;
	uint64_t i;
	void*    p;
} essl_value_t;

typedef struct
{
	unsigned                 ref;
	const char*              name;
	essl_type_e              type;
	void*                    structure;
	essl_storage_qualifier_e storage_qualifier;
	essl_precision_e         precision;
	bool                     invariant;
	unsigned                 count;
	essl_value_t*            value;
} essl_symbol_t;

typedef struct
{
	const char*    name;
	unsigned       count;
	essl_symbol_t* member;
	unsigned*      offset;
} essl_struct_t;

typedef struct
{
	essl_symbol_t               function;
	unsigned                    count;
	essl_symbol_t*              parameter;
	essl_parameter_qualifier_e* qualifier;
	unsigned*                   offset;
} essl_function_t;

typedef struct
{
	essl_precision_e intp;
	essl_precision_e floatp;

	unsigned symbol_count;
	unsigned structure_count;
	unsigned function_count;
	essl_symbol_t**   symbol;
	essl_struct_t**   structure;
	essl_function_t** function;
} essl_program_t;



extern unsigned essl_parse_type(const char* src,
	essl_type_e* type);
extern unsigned essl_parse_storage_qualifier(const char* src,
	essl_storage_qualifier_e* storage_qualifier);
extern unsigned essl_parse_parameter_qualifier(const char* src,
	essl_parameter_qualifier_e* parameter_qualifier);
extern unsigned essl_parse_precision(const char* src,
	essl_precision_e* precision);

extern unsigned essl_parse_default_precision_float(const char* src,
	essl_precision_e* precision);
extern unsigned essl_parse_default_precision_int(const char* src,
	essl_precision_e* precision);

extern unsigned essl_parse_symbol(const char* src,
	essl_precision_e floatp,
	essl_precision_e intp,
	essl_symbol_t** symbol);


extern essl_program_t* essl_program_create();
extern void essl_program_delete(
	essl_program_t* program);
extern bool essl_program_add_symbol(
	essl_program_t* program, essl_symbol_t* symbol);
extern bool essl_program_add_structure(
	essl_program_t* program, essl_struct_t* structure);
extern bool essl_program_add_function(
	essl_program_t* program, essl_function_t* function);
extern unsigned essl_program_parse(
	essl_program_t* program, const char* src);

extern essl_symbol_t* essl_symbol_create(
	const char* name,
	essl_storage_qualifier_e storage_qualifier,
	essl_type_e type, essl_precision_e precision,
	bool invariant, unsigned count, essl_value_t* values);
extern void           essl_symbol_delete(essl_symbol_t* symbol);
extern essl_symbol_t* essl_symbol_reference(essl_symbol_t* symbol);
extern essl_symbol_t* essl_symbol_copy(const essl_symbol_t* symbol);
extern unsigned       essl_symbol_component_count(essl_symbol_t* symbol);
extern const char*    essl_symbol_component_name(
	essl_symbol_t* symbol, unsigned component);

extern unsigned essl_type_components(essl_type_e type);
extern bool     essl_type_is_scalar(essl_type_e type);
extern bool     essl_type_is_vector(essl_type_e type);
extern bool     essl_type_is_matrix(essl_type_e type);
extern bool     essl_type_member(
	essl_type_e type, const char* name,
	unsigned* offset, essl_type_e* mtype);


#include <stdio.h>

extern void essl_symbol_print(
	essl_symbol_t* symbol,
	FILE* stream, unsigned tabs);
extern void essl_struct_print(
	essl_struct_t* structure,
	FILE* stream, unsigned tabs);
extern void essl_function_print(
	essl_function_t* function,
	FILE* stream, unsigned tabs);
extern void essl_program_print(
	essl_program_t* program,
	FILE* stream, unsigned tabs);

#endif
