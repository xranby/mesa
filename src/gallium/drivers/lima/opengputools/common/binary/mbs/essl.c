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



#include "mbs.h"
#include <ogt/essl.h>
#include <ogt/link.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>



essl_symbol_t* mbs_symbol_to_essl(mbs_chunk_t* chunk, unsigned* offset)
{
	if (!chunk)
		return NULL;

	essl_storage_qualifier_e storage_qualifier;
	if (strncmp(&chunk->ident[1], "UNI", 3) == 0)
		storage_qualifier = essl_storage_qualifier_uniform;
	else if (strncmp(&chunk->ident[1], "VAR", 3) == 0)
		storage_qualifier = essl_storage_qualifier_varying;
	else if (strncmp(&chunk->ident[1], "ATT", 3) == 0)
		storage_qualifier = essl_storage_qualifier_attribute;
	else
	{
		fprintf(stderr, "Error: Invalid MBS symbol chunk.\n");
		return NULL;
	}

	mbs_chunk_t* str
		= (mbs_chunk_t*)((uintptr_t)chunk + sizeof(mbs_chunk_t));
	mbs_symbol_t* s
		= (mbs_symbol_t*)((uintptr_t)chunk
			+ (sizeof(mbs_chunk_t) * 2) + str->size);

	essl_type_e type;
	switch (s->type)
	{
		case mbs_symbol_type_float:
		{
			switch (s->component_count)
			{
				case 1:
					type = essl_type_float;
					break;
				case 2:
					type = essl_type_vec2;
					break;
				case 3:
					type = essl_type_vec3;
					break;
				case 4:
					type = essl_type_vec4;
					break;
				default:
					fprintf(stderr, "Error: MBS component count too high,"
						" cannot represent in ESSL.\n");
					return NULL;
			}
		} break;
		case mbs_symbol_type_int:
		{
			switch (s->component_count)
			{
				case 1:
					type = essl_type_int;
					break;
				case 2:
					type = essl_type_ivec2;
					break;
				case 3:
					type = essl_type_ivec3;
					break;
				case 4:
					type = essl_type_ivec4;
					break;
				default:
					fprintf(stderr, "Error: MBS component count too high,"
						" cannot represent in ESSL.\n");
					return NULL;
			}
		} break;
		case mbs_symbol_type_bool:
		{
			switch (s->component_count)
			{
				case 1:
					type = essl_type_bool;
					break;
				case 2:
					type = essl_type_bvec2;
					break;
				case 3:
					type = essl_type_bvec3;
					break;
				case 4:
					type = essl_type_bvec4;
					break;
				default:
					fprintf(stderr, "Error: MBS component count too high,"
						" cannot represent in ESSL.\n");
					return NULL;
			}
		} break;
		case mbs_symbol_type_matrix:
		{
			switch (s->component_count)
			{
				case 2:
					type = essl_type_mat2;
					break;
				case 3:
					type = essl_type_mat3;
					break;
				case 4:
					type = essl_type_mat4;
					break;
				default:
					fprintf(stderr, "Error: MBS invalid matrix component count,"
						" cannot represent in ESSL.\n");
					return NULL;
			}
		} break;
		case mbs_symbol_type_sampler2D:
			type = essl_type_sampler2D;
			break;
		case mbs_symbol_type_samplerCube:
			type = essl_type_samplerCube;
			break;
		default:
			fprintf(stderr, "Error: MBS symbol type unknown,"
				" cannot represent in ESSL.\n");
			return NULL;
	}

	essl_precision_e precision;
	switch (s->precision)
	{
		case 1:
			precision = essl_precision_low;
			break;
		case 2:
			precision = essl_precision_medium;
			break;
		case 3:
			precision = essl_precision_high;
			break;
		case 4:
			precision = essl_precision_super;
			break;
		default:
			fprintf(stderr, "Error: MBS precision unknown,"
				" cannot represent in ESSL.\n");
			return NULL;
	}

	const char* ident = (const char*)((uintptr_t)str + sizeof(mbs_chunk_t));
	unsigned    ident_len = strnlen(ident, str->size);

	unsigned vcount = 0;
	float*   vlist  = NULL;
	
	if ((storage_qualifier
		== essl_storage_qualifier_uniform)
		&& (chunk->size > (str->size
			+ sizeof(mbs_chunk_t) + sizeof(mbs_symbol_t))))
	{
		unsigned ecount
			= (s->entry_count ? s->entry_count : 1)
				* essl_type_components(type);
		mbs_chunk_t* vini
			= (mbs_chunk_t*)((uintptr_t)s
				+ sizeof(mbs_symbol_t));
		uint32_t* count
			= (uint32_t*)((uintptr_t)vini
				+ sizeof(mbs_chunk_t));

		if ((strncmp(vini->ident, "VINI", 4) == 0)
			&& (*count == ecount))
		{
			storage_qualifier = essl_storage_qualifier_const;
			vlist  = (float*)((uintptr_t)count + 4);
			vcount = ecount;
		}
	}

	essl_symbol_t* sym
		= (essl_symbol_t*)malloc(
			sizeof(essl_symbol_t)
			+ (vcount * sizeof(essl_value_t))
			+ (ident_len + 1));
	if (!sym)
	{
		fprintf(stderr, "Error: Out of memory.\n");
		return NULL;
	}

	sym->value = (essl_value_t*)((uintptr_t)sym
		+ sizeof(essl_symbol_t));
	sym->name
		= (const char*)((uintptr_t)sym->value
			+ (sizeof(essl_value_t) * vcount));
	if (!vlist)
		sym->value = NULL;
	memcpy((void*)sym->name, ident, ident_len);
	((char*)sym->name)[ident_len] = '\0';

	sym->type              = type;
	sym->structure         = NULL;
	sym->storage_qualifier = storage_qualifier;
	sym->precision         = precision;
	sym->invariant         = false;
	sym->count             = s->entry_count;

	if (vlist)
	{
		unsigned i;
		for (i = 0; i < vcount; i++)
			sym->value[i].f = (double)vlist[i];
	}

	if (offset)
	{
		if (storage_qualifier
			!= essl_storage_qualifier_attribute)
		{
			*offset = s->offset;
		} else {
			mbs_attribute_t* a
				= (mbs_attribute_t*)s;
			*offset = a->offset;
		}
	}
	return sym;
}


static bool mbs__symbol_from_essl(
	essl_symbol_t* symbol,
	mbs_symbol_t* mbs_symbol)
{
	if (!symbol)
		return false;

	bool varying
		= (symbol->storage_qualifier
			== essl_storage_qualifier_varying);

	switch (symbol->type)
	{
		case essl_type_bool:
		case essl_type_int:
		case essl_type_float:
			symbol->type = essl_type_float;
			break;
		case essl_type_vec2:
		case essl_type_bvec2:
		case essl_type_ivec2:
			symbol->type = essl_type_vec2;
			break;
		case essl_type_vec3:
		case essl_type_bvec3:
		case essl_type_ivec3:
			symbol->type = essl_type_vec3;
			break;		
		case essl_type_vec4:
		case essl_type_bvec4:
		case essl_type_ivec4:
			symbol->type = essl_type_vec4;
			break;
		case essl_type_mat2:
		case essl_type_mat3:
		case essl_type_mat4:
		case essl_type_sampler2D:
		case essl_type_samplerCube:
			break;
		default:
			essl_symbol_delete(symbol);
			fprintf(stderr,
				"Error: LIMA only supports vectors, matrices and samplers.\n");
			return false;
	}

	switch (symbol->type)
	{
		case essl_type_bool:
		case essl_type_bvec2:
		case essl_type_bvec3:
		case essl_type_bvec4:
			symbol->precision = essl_precision_low;
			break;
		case essl_type_sampler2D:
		case essl_type_samplerCube:
			symbol->precision = essl_precision_medium;
			break;
		default:
			break;
	}

	/* TODO - Only check for fragment shader. */
	/*if (symbol->precision
		> essl_precision_medium)
	{
		essl_symbol_delete(symbol);
		fprintf(stderr,
			"Error: LIMA fragment shaders only support mediump or below.\n");
		return false;
	}
	symbol->precision = essl_precision_medium;*/

	uint8_t type;
	switch (symbol->type)
	{
		case essl_type_int:
		case essl_type_ivec2:
		case essl_type_ivec3:
		case essl_type_ivec4:
			type = mbs_symbol_type_int;
			break;
		case essl_type_bool:
		case essl_type_bvec2:
		case essl_type_bvec3:
		case essl_type_bvec4:
			type = mbs_symbol_type_bool;
			break;
		case essl_type_mat2:
		case essl_type_mat3:
		case essl_type_mat4:
			type = mbs_symbol_type_matrix;
			break;
		case essl_type_sampler2D:
			type = mbs_symbol_type_sampler2D;
			break;
		case essl_type_samplerCube:
			type = mbs_symbol_type_samplerCube;
			break;
		default:
			type = mbs_symbol_type_float;
			break;
	}

	unsigned component_count;
	switch (symbol->type)
	{
		case essl_type_float:
			component_count = 1;
			break;
		case essl_type_vec2:
		case essl_type_mat2:
		case essl_type_sampler2D:
		case essl_type_samplerCube:
			component_count = 2;
			break;
		case essl_type_vec3:
		case essl_type_mat3:
			component_count = 3;
			break;
		default:
			component_count = 4;
			break;
	}

	unsigned component_size;
	unsigned src_stride;
	unsigned dst_stride;
	switch (symbol->type)
	{
		case essl_type_float:
		case essl_type_sampler2D:
		case essl_type_samplerCube:
			component_size = 1;
			src_stride     = 1;
			dst_stride     = 16;
			break;
		case essl_type_vec2:
			component_size = 2;
			src_stride     = 2;
			dst_stride     = 16;
			break;
		default:
			component_size = 4;
			src_stride     = 4;
			dst_stride     = 16;
			break;
	}

	unsigned precision;
	switch (symbol->type)
	{
		case essl_type_sampler2D:
		case essl_type_samplerCube:
			precision = 1;
			break;
		default:
			precision = 1 + (symbol->precision - essl_precision_low);
			break;
	}

	unsigned entry_count = symbol->count;
	if (varying)
	{
		dst_stride
			= (entry_count ? entry_count : 1)
			* component_size * 4;
	}

	mbs_symbol_t msym =
	{
		.unknown_0       = 0x00,
		.type            = type,
		.component_count = component_count,
		.component_size  = component_size,
		.entry_count     = entry_count,
		.src_stride      = src_stride,
		.dst_stride      = dst_stride,
		.precision       = precision,
		.unknown_1       = 0x00000000,
		.offset          = 0x0000,
		.index           = 0xFFFF,
	};

	if (mbs_symbol)
		*mbs_symbol = msym;
	return true;
}

mbs_chunk_t* mbs_symbol_from_essl(essl_symbol_t* symbol, unsigned offset)
{
	mbs_symbol_t s;
	if (!mbs__symbol_from_essl(symbol, &s))
		return NULL;
	
	const char* tname[] =
	{
		"VUNI",
		"VVAR",
		"VATT",
	};

	const char* name;
	switch (symbol->storage_qualifier)
	{
		case essl_storage_qualifier_uniform:
		case essl_storage_qualifier_const:
			name = tname[0];
			break;
		case essl_storage_qualifier_varying:
			name = tname[1];
			break;
		case essl_storage_qualifier_attribute:
			name = tname[2];
			break;
		default:
			return NULL;
	}

	mbs_chunk_t* chunk
		= mbs_chunk_create(name);
	if (!chunk
		|| !mbs_chunk_append_string(
			&chunk, symbol->name))
	{
		free(chunk);
		return NULL;
	}

	if (symbol->storage_qualifier
		== essl_storage_qualifier_attribute)
	{
		mbs_attribute_t a;
		memcpy(&a, &s, sizeof(mbs_attribute_t));
		a.unknown_1 = 0x0000;
		a.offset    = offset;
		if (!mbs_chunk_append_data(
			&chunk, &a, sizeof(mbs_attribute_t)))
		{
			free(chunk);
			return NULL;
		}
	} else {
		s.offset = offset;
		if (!mbs_chunk_append_data(
			&chunk, &s, sizeof(mbs_symbol_t)))
		{
			free(chunk);
			return NULL;
		}

		if (symbol->storage_qualifier
			== essl_storage_qualifier_const)
		{
			if (!symbol->value)
			{
				free(chunk);
				return NULL;
			}

			mbs_chunk_t* vini
				= mbs_chunk_create("VINI");
			if (!vini)
			{
				free(chunk);
				return NULL;
			}

			unsigned vcount
				= essl_type_components(symbol->type)
				* (symbol->count ? symbol->count : 1);
			float values[vcount];
	
			unsigned i;
			for (i = 0; i < vcount; i++)
				values[i] = (float)symbol->value[i].f;

			if (!mbs_chunk_append_word(&vini, vcount)
				|| !mbs_chunk_append_data(
					&vini, values, (vcount * sizeof(float)))
				|| !mbs_chunk_append_chunk(&chunk, vini))
			{
				free(vini);
				free(chunk);
				return NULL;
			}
			free(vini);
		}
	}

	return chunk;
}
