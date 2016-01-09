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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>



mbs_chunk_t* mbs_chunk_create(const char* name)
{
	if (!name)
		return NULL;

	mbs_chunk_t* chunk
		= (mbs_chunk_t*)malloc(sizeof(mbs_chunk_t));
	if (!chunk) return NULL;

	unsigned i;
	for (i = 0; (i < 4) && (name[i] != '\0'); i++)
		chunk->ident[i] = toupper(name[i]);
	for(; i < 4; i++)
		chunk->ident[i] = '\0';
	chunk->size = 0;
	return chunk;
}

mbs_chunk_t* mbs_chunk_copy(mbs_chunk_t* chunk)
{
	if (!chunk) return NULL;
	unsigned size = chunk->size + sizeof(mbs_chunk_t);
	mbs_chunk_t* copy = malloc(size);
	if (!copy) return NULL;
	memcpy(copy, chunk, size);
	return copy;
}



bool mbs_chunk_append_data(mbs_chunk_t** chunk, void* data, unsigned size)
{
	if (!chunk
		|| !*chunk)
		return false;
	if (!size) return true;
	if (!data) return false;

	mbs_chunk_t* nchunk
		= (mbs_chunk_t*)realloc(
			*chunk, ((*chunk)->size + size + sizeof(mbs_chunk_t)));
	if (!nchunk)
		return false;

	*chunk = nchunk;
	memcpy(
		(void*)((uintptr_t)nchunk + nchunk->size + sizeof(mbs_chunk_t)),
		data, size);
	nchunk->size += size;
	return true;
}

bool mbs_chunk_append_word(mbs_chunk_t** chunk, uint32_t word)
{
	return mbs_chunk_append_data(chunk, &word, 4);
}

bool mbs_chunk_append_chunk(mbs_chunk_t** chunk, mbs_chunk_t* append)
{
	if (!append)
		return NULL;
	return mbs_chunk_append_data(chunk, append,
		(sizeof(mbs_chunk_t) + append->size));
}

bool mbs_chunk_append_string(mbs_chunk_t** chunk, const char* string)
{
	unsigned slen = strlen(string);
	unsigned plen = slen + 1;
	if (plen & 3)
		plen += (4 - (plen & 3));
	char str[plen];
	memcpy(str, string, slen);
	memset(&str[slen], 0x00, (plen - slen));

	mbs_chunk_t* schunk
		= mbs_chunk_create("STRI");
	if (!schunk) return false;
	if (!mbs_chunk_append_data(&schunk, str, plen))
	{
		free(schunk);
		return false;
	}
	bool ret = mbs_chunk_append_chunk(chunk, schunk);
	free(schunk);
	return ret;
}



static mbs_chunk_t* mbs_cxxx_create(
	ogt_program_t* program)
{
	if (!program) return NULL;

	const char* ctyp[] =
	{
		NULL, "CGEO", "CVER", "CFRA",
	};
	unsigned cversion[]
		= { 0, 0, 2, 5, };

	mbs_chunk_t* cxxx
		= mbs_chunk_create(ctyp[program->type]);
	if (!cxxx) return NULL;
	if (!mbs_chunk_append_word(&cxxx,
		cversion[program->type]))
	{
		free(cxxx);
		return NULL;
	}

	bool success = true;

	if (program->type == ogt_asm_type_fragment)
	{
		uint32_t fsta_data[2] = { 1, 1 };
		mbs_chunk_t* fsta
			= mbs_chunk_create("FSTA");
		success = success
			&& mbs_chunk_append_data(
				&fsta, fsta_data, sizeof(fsta_data));
		success = success
			&& mbs_chunk_append_chunk(&cxxx, fsta);
		free(fsta);

		uint32_t fdis_data[1] = { 0 };
		mbs_chunk_t* fdis
			= mbs_chunk_create("FDIS");
		success = success
			&& mbs_chunk_append_data(
				&fdis, fdis_data, sizeof(fdis_data));
		success = success
			&& mbs_chunk_append_chunk(&cxxx, fdis);
		free(fdis);

		uint32_t fbuu_data[2] = { 256, 0 };
		mbs_chunk_t* fbuu
			= mbs_chunk_create("FBUU");
		success = success
			&& mbs_chunk_append_data(
				&fbuu, fbuu_data, sizeof(fbuu_data));
		success = success
			&& mbs_chunk_append_chunk(&cxxx, fbuu);
		free(fbuu);
	}

	if (program->type == ogt_asm_type_vertex)
	{
		mbs_fins_t fins_data =
		{
			.unknown_0       = 0x00000000,
			.instructions    = program->code_size,
			.attrib_prefetch = program->attrib_prefetch,
		};
		mbs_chunk_t* fins
			= mbs_chunk_create("FINS");
		success = success
			&& mbs_chunk_append_data(
				&fins, &fins_data, sizeof(mbs_fins_t));
		success = success
			&& mbs_chunk_append_chunk(&cxxx, fins);
		free(fins);
	}

	if (program->symbol_table)
	{
		if (!program->link_map)
			program->link_map 
				= ogt_link_map_import_essl(
					program->symbol_table);
		success = success
			&& ogt_link_map_export_mbs(
				program->link_map, &cxxx);
	}

	if (program->code_size > 0)
	{
		mbs_chunk_t* dbin
			= mbs_chunk_create("DBIN");
		success = success
			&& mbs_chunk_append_data(&dbin,
				program->code, program->code_size);
		success = success
			&& mbs_chunk_append_chunk(&cxxx, dbin);
		free(dbin);
	}

	if (!success)
	{
		free(cxxx);
		return NULL;
	}

	return cxxx;
}



bool mbs_export(FILE* stream, ogt_program_t* program)
{
	if (!program) return false;

	mbs_chunk_t* cxxx
		= mbs_cxxx_create(program);
	if (!cxxx)
	{
		fprintf(stderr, "Error: Failed to export cxxx chunk.\n");
		return false;
	}

	mbs_chunk_t* mbs1
		= mbs_chunk_create("MBS1");
	if (!mbs1)
	{
		free(cxxx);
		return false;
	}

	bool success
		= mbs_chunk_append_chunk(&mbs1, cxxx);
	free(cxxx);
	if (!success)
	{
		free(mbs1);
		return false;
	}

	success = (fwrite(mbs1,
		(mbs1->size + sizeof(mbs_chunk_t)),
		1, stream) == 1);
	free(mbs1);
	return success;
}



#include <ogt/binary.h>

static const ogt_binary_format_t ogt_binary_format__mbs =
{
	"mbs",
	mbs_export,
	NULL,
	(void*)mbs_chunk_print,
};

const ogt_binary_format_t* ogt_binary_format_mbs
	= &ogt_binary_format__mbs;
