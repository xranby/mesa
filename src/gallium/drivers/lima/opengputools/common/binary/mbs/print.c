/* Author(s):
 *   Ben Brewer (ben.brewer@codethink.co.uk)
 *   Connor Abbott
 *
 * Copyright (c) 2012
 *   Codethink (http://www.codethink.co.uk)
 *   Connor Abbott (connor@abbott.cx)
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
#include <ogt/link.h>
#include <ogt/arch.h>
#include <ogt/asm.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>



typedef union
__attribute__((__packed__))
{
	const char ident[4];
	uint32_t   magic;
} mbs__ident_t;

static inline mbs__ident_t* mbs__ident(const char* ident)
	{ return ((mbs__ident_t*)ident); }



typedef unsigned (*mbs__chunk_print_f)(
	void* data, uint32_t size, unsigned tabs,
	mbs__ident_t* parent, ogt_asm_syntax_e syntax, ogt_link_map_t* map);

typedef struct
{
	char               ident[4];
	mbs__chunk_print_f func;
} mbs__chunk_print_func_t;



unsigned mbs__chunk_print(
	mbs_chunk_t* chunk, uint32_t size,
	unsigned tabs, mbs__ident_t* parent,
	ogt_asm_syntax_e syntax, ogt_link_map_t* map);

static unsigned mbs__chunk_print_unknown(
	uint32_t* data, uint32_t size, unsigned tabs,
	mbs__ident_t* parent, ogt_asm_syntax_e syntax, ogt_link_map_t* map)
{
	(void) parent; /* Not used. */
	(void) syntax;  /* Not used. */
	(void) map;    /* Not used. */

	unsigned i, o;
	for (i = 0, o = 0; (o + 4) <= size; i++, o += 4)
	{
		ogt_asm_print_tabs(tabs);
		printf(".unknown_%02u = 0x%08"PRIX32",\n", i, data[i]);
	}

	if ((size - o) >= 2)
	{
		ogt_asm_print_tabs(tabs);
		printf(".unknown_%02u = 0x%04"PRIX16",\n", i++,
			*((uint16_t*)((uintptr_t)data + o)));
		o += 2;
	}

	if ((size - o) >= 1)
	{
		ogt_asm_print_tabs(tabs);
		printf(".unknown_%02u = 0x%02"PRIX8",\n", i,
			*((uint8_t*)((uintptr_t)data + o)));
		o += 2;
	}

	return size;
}

static unsigned mbs__chunk_print_chunk_array(
	uint8_t* data, uint32_t size, unsigned tabs,
	mbs__ident_t* parent, ogt_asm_syntax_e syntax, ogt_link_map_t* map)
{
	if (!size)
		return 0;

	uint32_t left = size;
	while (left)
	{
		unsigned csize
			= mbs__chunk_print(
				(mbs_chunk_t*)data, left, tabs, parent, syntax, map);
		if (!csize)
		{
			fprintf(stderr, "ERROR: Failed to print sub-chunk.\n");
			return 0;
		}
		left -= csize;
		data += csize;
	}
	return size;
}

static unsigned mbs__chunk_print_cfra(
	uint32_t* data, uint32_t size, unsigned tabs,
	mbs__ident_t* parent, ogt_asm_syntax_e syntax, ogt_link_map_t* map)
{
	(void) parent;  /* Not used. */

	if (size < 4)
	{
		fprintf(stderr, "Error: Invalid fragment (CFRA) chunk.\n");
		return 0;
	}

	ogt_asm_print_tabs(tabs);
	printf(".version = %"PRIu32",\n", data[0]);
	return 4 + mbs__chunk_print_chunk_array(
		(void*)&data[1], (size - 4), tabs, mbs__ident("CFRA"), syntax, map);
}

static unsigned mbs__chunk_print_cver(
	uint32_t* data, uint32_t size, unsigned tabs,
	mbs__ident_t* parent, ogt_asm_syntax_e syntax, ogt_link_map_t* map)
{
	(void) parent;  /* Not used. */

	if (size < 4)
	{
		fprintf(stderr, "Error: Invalid vertex (CVER) chunk.\n");
		return 0;
	}

	ogt_asm_print_tabs(tabs);
	printf(".version = %"PRIu32",\n", data[0]);
	return 4 + mbs__chunk_print_chunk_array(
		(void*)&data[1], (size - 4), tabs, mbs__ident("CVER"), syntax, map);
}

static unsigned mbs__chunk_print_suni(
	uint32_t* data, uint32_t size, unsigned tabs,
	mbs__ident_t* parent, ogt_asm_syntax_e syntax, ogt_link_map_t* map)
{
	(void) parent;  /* Not used. */

	if (size < 8)
	{
		fprintf(stderr, "Error: Invalid symbol table chunk.\n");
		return 0;
	}

	ogt_asm_print_tabs(tabs);
	printf(".count = %"PRIu32",\n", data[0]);
	ogt_asm_print_tabs(tabs);
	printf(".size  = %"PRIu32",\n", data[1]);
	return 8 + mbs__chunk_print_chunk_array(
		(void*)&data[2], (size - 8), tabs,
		mbs__ident("SUNI"), syntax, map);
}

static unsigned mbs__chunk_print_svar(
	uint32_t* data, uint32_t size, unsigned tabs,
	mbs__ident_t* parent, ogt_asm_syntax_e syntax, ogt_link_map_t* map)
{
	(void) parent;  /* Not used. */

	if (size < 4)
	{
		fprintf(stderr, "Error: Invalid symbol table chunk.\n");
		return 0;
	}

	ogt_asm_print_tabs(tabs);
	printf(".count = %"PRIu32",\n", data[0]);
	return 4 + mbs__chunk_print_chunk_array(
		(void*)&data[1], (size - 4), tabs,
		mbs__ident("SVER"), syntax, map);
}

static unsigned mbs__chunk_print_satt(
	uint32_t* data, uint32_t size, unsigned tabs,
	mbs__ident_t* parent, ogt_asm_syntax_e syntax, ogt_link_map_t* map)
{
	(void) parent;  /* Not used. */

	if (size < 4)
	{
		fprintf(stderr, "Error: Invalid symbol table chunk.\n");
		return 0;
	}

	ogt_asm_print_tabs(tabs);
	printf(".count = %"PRIu32",\n", data[0]);
	return 4 + mbs__chunk_print_chunk_array(
		(void*)&data[1], (size - 4), tabs,
		mbs__ident("SATT"), syntax, map);
}

static unsigned mbs__chunk_print_symbol(
	void* data, uint32_t size, unsigned tabs,
	mbs__ident_t* parent, ogt_asm_syntax_e syntax, ogt_link_map_t* map)
{
	const char* nparent
		= (tolower(parent->ident[1]) == 'v'
			? "VVAR" :
			"VUNI" );

	ogt_asm_print_tabs(tabs); printf(".name            = \"");
	uint32_t ssize = mbs__chunk_print(
		data, size, tabs, mbs__ident(nparent), syntax, map);
	printf("\",\n");
	if (ssize == 0)
	{
		fprintf(stderr, "Error: Invalid symbol, no string found.\n");
		return 0;
	}

	uint32_t left = size - ssize;
	if (left < sizeof(mbs_symbol_t))
	{
		fprintf(stderr, "Error: Invalid symbol chunk size.\n");
		return 0;
	}
	
	mbs_symbol_t* symbol
		= (mbs_symbol_t*)((uintptr_t)data + ssize);
	ogt_asm_print_tabs(tabs); printf(".unknown_0       = "
		"0x%02"PRIX8" (%"PRIu8"),\n", symbol->unknown_0, symbol->unknown_0);

	ogt_asm_print_tabs(tabs); printf(".type            = ");
	switch (symbol->type)
	{
		case mbs_symbol_type_float:
			printf("float");
			break;
		case mbs_symbol_type_int:
			printf("int");
			break;
		case mbs_symbol_type_bool:
			printf("bool");
			break;
		case mbs_symbol_type_matrix:
			printf("matrix");
			break;
		case mbs_symbol_type_sampler2D:
			printf("sampler2D");
			break;
		case mbs_symbol_type_samplerCube:
			printf("samplerCube");
			break;
		default:
			printf("%"PRIu8, symbol->type);
			break;
	}
	printf(" (%"PRIu8")\n", symbol->type);

	ogt_asm_print_tabs(tabs); printf(".component_count = "
		"%"PRIu16",\n", symbol->component_count);
	ogt_asm_print_tabs(tabs); printf(".component_size  = "
		"%"PRIu16",\n", symbol->component_size);
	ogt_asm_print_tabs(tabs); printf(".entry_count     = "
		"%"PRIu16",\n", symbol->entry_count);
	ogt_asm_print_tabs(tabs); printf(".src_stride      = "
		"%"PRIu16",\n", symbol->src_stride);
	ogt_asm_print_tabs(tabs); printf(".dst_stride      = "
		"%"PRIu8",\n", symbol->dst_stride);
	ogt_asm_print_tabs(tabs); printf(".precision       = "
		"%"PRIu8",\n", symbol->precision);
	ogt_asm_print_tabs(tabs); printf(".unknown_1       = "
		"0x%08"PRIX32" (%"PRIu32"),\n", symbol->unknown_1, symbol->unknown_1);
	ogt_asm_print_tabs(tabs); printf(".offset          = "
		"%"PRIu16",\n", symbol->offset);
	ogt_asm_print_tabs(tabs); printf(".index           = "
		"%"PRIu16",\n", symbol->index);
	
	left -= sizeof(mbs_symbol_t);
	ssize += sizeof(mbs_symbol_t);
	
	if (left == 0)
		return size;
	else
	{
		if (left < sizeof(mbs_chunk_t))
		{
			fprintf(stderr, "Error: Invalid symbol chunk size.\n");
			return 0;
		}
		mbs_chunk_t* init_chunk
			= (mbs_chunk_t*)((uintptr_t)data + ssize);
		ogt_asm_print_tabs(tabs); printf(".init            = ");
		mbs__chunk_print(init_chunk, left,
			tabs, mbs__ident(nparent), syntax, map);
		printf(",\n");
	}

	return size;
}

static unsigned mbs__chunk_print_vatt(
	void* data, uint32_t size, unsigned tabs,
	mbs__ident_t* parent, ogt_asm_syntax_e syntax, ogt_link_map_t* map)
{
	(void) parent; /* Not used. */

	ogt_asm_print_tabs(tabs); printf(".name            = \"");
	uint32_t ssize = mbs__chunk_print(
		data, size, tabs, mbs__ident("VATT"), syntax, map);
	printf("\"\n");
	if (ssize == 0)
	{
		fprintf(stderr, "Error: Invalid symbol, no string found.\n");
		return 0;
	}

	uint32_t left = size - ssize;
	if (left != sizeof(mbs_attribute_t))
	{
		fprintf(stderr, "Error: Invalid symbol chunk size.\n");
		return 0;
	}

	mbs_attribute_t* attr
		= (mbs_attribute_t*)((uintptr_t)data + ssize);
	ogt_asm_print_tabs(tabs); printf(".unknown_0       = "
		"0x%02"PRIX8" (%"PRIu8"),\n", attr->unknown_0, attr->unknown_0);

	ogt_asm_print_tabs(tabs); printf(".type            = ");
	switch (attr->type)
	{
		case mbs_symbol_type_float:
			printf("float");
			break;
		case mbs_symbol_type_int:
			printf("int");
			break;
		case mbs_symbol_type_matrix:
			printf("matrix");
			break;
		case mbs_symbol_type_sampler2D:
			printf("sampler2D");
			break;
		case mbs_symbol_type_samplerCube:
			printf("samplerCube");
			break;
		default:
			printf("%"PRIu8, attr->type);
			break;
	}
	printf(" (%"PRIu8")\n", attr->type);

	ogt_asm_print_tabs(tabs); printf(".component_count = "
		"%"PRIu16",\n", attr->component_count);
	ogt_asm_print_tabs(tabs); printf(".component_size  = "
		"%"PRIu16",\n", attr->component_size);
	ogt_asm_print_tabs(tabs); printf(".entry_count     = "
		"%"PRIu16",\n", attr->entry_count);
	ogt_asm_print_tabs(tabs); printf(".src_stride      = "
		"%"PRIu16",\n", attr->src_stride);
	ogt_asm_print_tabs(tabs); printf(".dst_stride      = "
		"%"PRIu8",\n", attr->dst_stride);
	ogt_asm_print_tabs(tabs); printf(".precision       = "
		"%"PRIu8",\n", attr->precision);
	ogt_asm_print_tabs(tabs); printf(".unknown_1       = "
		"0x%04"PRIX16" (%"PRIu16"),\n", attr->unknown_1, attr->unknown_1);
	ogt_asm_print_tabs(tabs); printf(".offset          = "
		"%"PRIu16",\n", attr->offset);
	
	return size;
}

static unsigned mbs__chunk_print_vini(
	uint32_t* data, uint32_t size, unsigned tabs,
	mbs__ident_t* parent, ogt_asm_syntax_e syntax, ogt_link_map_t* map)
{
	(void) parent;
	(void) syntax;
	(void) map;

	uint32_t count = data[0];
	float* components = (float*)&data[1];
	unsigned i;
	for (i = 0; i < count; i++)
	{
		ogt_asm_print_tabs(tabs);
		printf(".component_%u = %f\n", i, components[i]);
	}
	return size;
}

static unsigned mbs__chunk_print_dbin(
	void* data, uint32_t size, unsigned tabs,
	mbs__ident_t* parent, ogt_asm_syntax_e syntax, ogt_link_map_t* map)
{
	if (size & 3)
	{
		fprintf(stderr,
			"Error: Invalid binary (dbin) chunk size,"
			" must be multiple of 4.\n");
		return 0;
	}

	if (!parent
		|| ((strncmp(parent->ident, "CFRA", 4) != 0)
			&& (strncmp(parent->ident, "CVER", 4) != 0))
		|| (syntax == ogt_asm_syntax_raw))
		return mbs__chunk_print_unknown(
			data, size, tabs, mbs__ident("DBIN"), syntax, map);
	
	const ogt_arch_t* arch = NULL;
	ogt_asm_type_e    type = ogt_asm_type_unknown;

	if(strncmp(parent->ident, "CFRA", 4) == 0)
	{
		arch = ogt_arch_lima_pp;
		type = ogt_asm_type_fragment;
	}
	else if(strncmp(parent->ident, "CVER", 4) == 0)
	{
		arch = ogt_arch_lima_gp;
		type = ogt_asm_type_vertex;
	}

	if (!ogt_arch_disassemble(
		arch, data, size, map,
		type, syntax, tabs))
	{
		fprintf(stderr,
			"Error: Failed to disassemble binary"
			" (dbin) chunk.\n");
		return 0;
	}

	return size;
}

static unsigned mbs__chunk_print_fins(
	void* data, uint32_t size, unsigned tabs,
	mbs__ident_t* parent, ogt_asm_syntax_e syntax, ogt_link_map_t* map)
{
	if (size != 12)
		mbs__chunk_print_unknown(
			data, size, tabs,
			parent, syntax, map);

	mbs_fins_t* fins = (mbs_fins_t*)data;

	ogt_asm_print_tabs(tabs); printf(".unknown_0       = "
		"0x%08"PRIX32",\n", fins->unknown_0);
	ogt_asm_print_tabs(tabs); printf(".instructions    = "
		"%"PRIu32",\n", fins->instructions);
	ogt_asm_print_tabs(tabs); printf(".attrib_prefetch = "
		"%"PRIu32"\n", fins->attrib_prefetch);

	return size;
}


static mbs__chunk_print_func_t mbs__chunk_print_funcs[] =
{
	{ "MBS1", (void*)mbs__chunk_print_chunk_array },
	{ "CFRA", (void*)mbs__chunk_print_cfra },
	{ "CVER", (void*)mbs__chunk_print_cver },
	{ "DBIN", (void*)mbs__chunk_print_dbin },
	{ "SUNI", (void*)mbs__chunk_print_suni },
	{ "SVAR", (void*)mbs__chunk_print_svar },
	{ "SATT", (void*)mbs__chunk_print_satt },
	{ "VUNI", (void*)mbs__chunk_print_symbol },
	{ "VVAR", (void*)mbs__chunk_print_symbol },
	{ "VATT", (void*)mbs__chunk_print_vatt },
	{ "VINI", (void*)mbs__chunk_print_vini },
	{ "FINS", (void*)mbs__chunk_print_fins },
	{ ""    , (void*)mbs__chunk_print_unknown }
};



unsigned mbs__chunk_print(
	mbs_chunk_t* chunk, uint32_t size, unsigned tabs,
	mbs__ident_t* parent, ogt_asm_syntax_e syntax, ogt_link_map_t* map)
{
	if (!chunk) return 0;

	void* data
		= (void*)((uintptr_t)chunk + sizeof(mbs_chunk_t));

	if (chunk->size > (size - sizeof(mbs_chunk_t)))
	{
		fprintf(stderr, "Error: Chunk larger than container.\n");
		return 0;
	}

	if (strncmp(chunk->ident, "STRI", 4) == 0)
	{
		printf("%.*s", chunk->size, (char*)data);
	}
	else if (strncmp(chunk->ident, "VINI", 4) == 0)
	{
		unsigned count = ((uint32_t*)data)[0];
		if (count > 1)
			printf("{ ");

		unsigned i;
		float* f;
		for (i = 0, f = &(((float*)data)[1]); i < count; i++, f++)
		{
			if (i) printf(", ");
			printf("%g", *f);
		}

		if (count > 1)
			printf(" }");
	} else {
		ogt_asm_print_tabs(tabs);
		printf("%.*s[%u]\n", 4, chunk->ident, chunk->size);
		ogt_asm_print_tabs(tabs);
		printf("{\n");

		mbs__chunk_print_func_t* func;
		for (func = mbs__chunk_print_funcs;
			(func->ident[0] != '\0')
				&& (strncmp(chunk->ident, func->ident, 4) != 0);
			func++);
		func->func(data, chunk->size, (tabs + 1), parent, syntax, map);

		ogt_asm_print_tabs(tabs);
		printf("}\n");
	}

	return (chunk->size + sizeof(mbs_chunk_t));
}



void mbs_chunk_print(mbs_chunk_t* chunk, uintptr_t size, ogt_asm_syntax_e syntax)
{
	ogt_link_map_t* map
		= ogt_link_map_import_mbs(chunk);
	mbs__chunk_print(chunk, size, 0, NULL, syntax, map);
}
