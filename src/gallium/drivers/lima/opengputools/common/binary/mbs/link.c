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



#include <ogt/link.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>



#include "mbs.h"

static bool ogt_link_map__import_mbs(
	ogt_link_map_t* map, mbs_chunk_t* chunk, uintptr_t size)
{
	if (!map || !chunk)
		return false;
	
	uintptr_t i = 0;
	while (i < size)
	{
		if ((size - i) < sizeof(mbs_chunk_t))
			return false;

		mbs_chunk_t* c = (mbs_chunk_t*)((uintptr_t)chunk + i);
		if ((size - i) < (sizeof(mbs_chunk_t) + c->size))
			return false;

		if ((strncmp(c->ident, "CFRA", 4) == 0)
			|| (strncmp(c->ident, "CVER", 4) == 0)
			|| (strncmp(c->ident, "MBS1", 4) == 0)
			|| (strncmp(c->ident, "SUNI", 4) == 0)
			|| (strncmp(c->ident, "SVAR", 4) == 0)
			|| (strncmp(c->ident, "SATT", 4) == 0))
		{
			unsigned o;
			if ((strncmp(c->ident, "CFRA", 4) == 0)
				|| (strncmp(c->ident, "CVER", 4) == 0)
				|| (strncmp(c->ident, "SVAR", 4) == 0)
				|| (strncmp(c->ident, "SATT", 4) == 0))
				o = 4;
			else if (strncmp(c->ident, "SUNI", 4) == 0)
				o = 8;

			if (o > c->size)
				return false;
			unsigned size = (c->size - o);

			mbs_chunk_t* child
				 = (mbs_chunk_t*)((uintptr_t)c + sizeof(mbs_chunk_t) + o);
			if (!ogt_link_map__import_mbs(map, child, size))
				return false;
		}
		else if ((strncmp(c->ident, "VUNI", 4) == 0)
			|| (strncmp(c->ident, "VATT", 4) == 0)
			|| (strncmp(c->ident, "VVAR", 4) == 0))
		{
			unsigned offset;
			essl_symbol_t* symbol
				= mbs_symbol_to_essl(c, &offset);
			if (!symbol)
			{
				fprintf(stderr, "Error: Failed to import symbol from MBS.\n");
				return false;
			}

			if(!ogt_link_map_import(map, symbol, offset))
			{
				essl_symbol_delete(symbol);
				return false;
			}
		}

		i += sizeof(mbs_chunk_t) + c->size;
	}

	return true;
}

ogt_link_map_t* ogt_link_map_import_mbs(mbs_chunk_t* chunk)
{
	if (!chunk) return NULL;
	ogt_link_map_t* map
		= ogt_link_map_create();
	mbs_chunk_t* c
		= (mbs_chunk_t*)((uintptr_t)chunk
			+ sizeof(mbs_chunk_t));
	if (!ogt_link_map__import_mbs(map, c, chunk->size))
	{
		ogt_link_map_delete(map);
		return NULL;
	}
	return map;
}



bool ogt_link_map_export_mbs(ogt_link_map_t* map, mbs_chunk_t** table)
{
	if (!map || !table)
		return false;

	mbs_chunk_t* t[3] = { NULL, NULL, NULL };

	if (map->uniform_count)
	{
		t[0] = mbs_chunk_create("SUNI");
		if (!t[0]
			|| !mbs_chunk_append_word(
				&t[0], map->uniform_count)
			|| !mbs_chunk_append_word(
				&t[0], ogt_link_map_uniform_area(map)))
			goto ogt_link_map_export_mbs_fail;

		unsigned i;
		for (i = 0; i < map->uniform_count; i++)
		{
			mbs_chunk_t* s
				= mbs_symbol_from_essl(
					map->uniform[i].symbol,
					map->uniform[i].offset);
			if (!s) goto ogt_link_map_export_mbs_fail;
			bool a = mbs_chunk_append_chunk(&t[0], s);
			free(s);
			if (!a) goto ogt_link_map_export_mbs_fail;
		}
	}

	if (map->attribute_count)
	{
		t[1] = mbs_chunk_create("SATT");
		if (!t[1]
			|| !mbs_chunk_append_word(
				&t[1], map->attribute_count))
			goto ogt_link_map_export_mbs_fail;

		unsigned i;
		for (i = 0; i < map->attribute_count; i++)
		{
			mbs_chunk_t* s
				= mbs_symbol_from_essl(
					map->attribute[i].symbol,
					map->attribute[i].offset);
			if (!s) goto ogt_link_map_export_mbs_fail;
			bool a = mbs_chunk_append_chunk(&t[1], s);
			free(s);
			if (!a) goto ogt_link_map_export_mbs_fail;
		}
	}

	if (map->varying_count)
	{
		t[2] = mbs_chunk_create("SVAR");
		if (!t[2]
			|| !mbs_chunk_append_word(
				&t[2], map->varying_count))
			goto ogt_link_map_export_mbs_fail;

		unsigned i;
		for (i = 0; i < map->varying_count; i++)
		{
			mbs_chunk_t* s
				= mbs_symbol_from_essl(
					map->varying[i].symbol,
					map->varying[i].offset);
			if (!s) goto ogt_link_map_export_mbs_fail;
			bool a = mbs_chunk_append_chunk(&t[2], s);
			free(s);
			if (!a) goto ogt_link_map_export_mbs_fail;
		}
	}

	mbs_chunk_t* atomic
		= mbs_chunk_copy(*table);
	if (!atomic)
		goto ogt_link_map_export_mbs_fail;

	unsigned i;
	for (i = 0; i < 3; i++)
	{
		if (t[i]
			&& !mbs_chunk_append_chunk(&atomic, t[i]))
		{
			free(atomic);
			goto ogt_link_map_export_mbs_fail;
		}
	}

	free(*table);
	*table = atomic;
	return true;

ogt_link_map_export_mbs_fail:
	free(t[0]);
	free(t[1]);
	free(t[2]);
	return false;
}
