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



#ifndef __ogt_binary_mbs_h__
#define __ogt_binary_mbs_h__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>



typedef struct
__attribute__((__packed__))
{
	union
	__attribute__((__packed__))
	{
		char     ident[4];
		uint32_t magic;
	};
	uint32_t size;
} mbs_chunk_t;

typedef struct
{
	uint32_t unknown_0;
	uint32_t instructions;
	uint32_t attrib_prefetch; /* Points instruction after last attribute read */
} mbs_fins_t;

typedef enum
{
	mbs_symbol_qualifier_uniform,
	mbs_symbol_qualifier_varying,
	mbs_symbol_qualifier_attribute,
} mbs_symbol_qualifier_e;

typedef enum
{
	mbs_symbol_type_float       = 1,
	mbs_symbol_type_int         = 2,
	mbs_symbol_type_bool        = 3,
	mbs_symbol_type_matrix      = 4,
	mbs_symbol_type_sampler2D   = 5,
	mbs_symbol_type_samplerCube = 6,
} mbs_symbol_type_e;

typedef struct
__attribute__((__packed__))
{
	uint8_t  unknown_0; // =0x00
	uint8_t  type;
	uint16_t component_count;
	uint16_t component_size;
	uint16_t entry_count;
	uint16_t src_stride;
	uint8_t  dst_stride;
	uint8_t  precision;
	uint32_t unknown_1; // =0x00000000
	uint16_t offset;
	uint16_t index; // Usually -1 (0xFFFF)
} mbs_symbol_t;

typedef struct
__attribute__((__packed__))
{
	uint8_t  unknown_0; // =0x00
	uint8_t  type;
	uint16_t component_count;
	uint16_t component_size;
	uint16_t entry_count;
	uint16_t src_stride;
	uint8_t  dst_stride;
	uint8_t  precision;
	uint16_t unknown_1; // =0x00000000
	uint16_t offset;
} mbs_attribute_t;



extern mbs_chunk_t* mbs_chunk_create(const char* name);
extern mbs_chunk_t* mbs_chunk_copy(mbs_chunk_t* chunk);

extern bool mbs_chunk_append_data(
	mbs_chunk_t** chunk, void* data, unsigned size);
extern bool mbs_chunk_append_word(
	mbs_chunk_t** chunk, uint32_t word);
extern bool mbs_chunk_append_chunk(
	mbs_chunk_t** chunk, mbs_chunk_t* append);
extern bool mbs_chunk_append_string(
	mbs_chunk_t** chunk, const char* string);



#include <ogt/asm.h>

extern void mbs_chunk_print(
	mbs_chunk_t* chunk, uintptr_t size, ogt_asm_syntax_e syntax);



#include <ogt/essl.h>

extern essl_symbol_t* mbs_symbol_to_essl(
	mbs_chunk_t* chunk, unsigned* offset);
extern mbs_chunk_t*   mbs_symbol_from_essl(
	essl_symbol_t* symbol, unsigned offset);



#include <ogt/program.h>

extern bool mbs_export(
	FILE* stream, ogt_program_t* program);



#include <ogt/link.h>

extern bool ogt_link_map_export_mbs(
	ogt_link_map_t* map, mbs_chunk_t** table);
extern ogt_link_map_t* ogt_link_map_import_mbs(
	mbs_chunk_t* chunk);

#endif
