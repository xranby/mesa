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



static void ogt_link__dimensions(essl_symbol_t* symbol,
	unsigned* width, unsigned* height, unsigned* stride)
{
	if (!symbol)
		return;

	unsigned w, h, s;
	switch (symbol->type)
	{
		case essl_type_bool:
		case essl_type_int:
		case essl_type_float:
			w = 1;
			h = 1;
			s = 1;
			break;
		case essl_type_bvec2:
		case essl_type_ivec2:
		case essl_type_vec2:
		case essl_type_sampler2D:
		case essl_type_samplerCube:
			w = 2;
			h = 1;
			s = 2;
			break;
		case essl_type_bvec3:
		case essl_type_ivec3:
		case essl_type_vec3:
			w = 3;
			h = 1;
			s = 3;
			break;
		case essl_type_bvec4:
		case essl_type_ivec4:
		case essl_type_vec4:
			w = 4;
			h = 1;
			s = 4;
			break;
		case essl_type_mat2:
			w = 4; /* As per the GLES 2.0 Shading Language Specification. */
			h = 2;
			s = 4;
			break;
		case essl_type_mat3:
			w = 3;
			h = 3;
			s = 4;
			break;
		case essl_type_mat4:
			w = 4;
			h = 4;
			s = 4;
			break;
		default:
			return;
	}

	if (symbol->count)
		h *= symbol->count;

	if (width ) *width  = w;
	if (height) *height = h;
	if (stride) *stride = s;
}

static unsigned ogt_link__sizeof(essl_symbol_t* symbol)
{
	if (!symbol)
		return 0;
	
	unsigned width = 0, height = 0, stride = 0;
	ogt_link__dimensions(symbol,
		&width, &height, &stride);
	return (height > 1 ? stride : width) * height;
}

static unsigned ogt_link__alignof(essl_symbol_t* symbol)
{
	unsigned size
		= ogt_link__sizeof(symbol);
	if (!size) return 0;

	switch (size)
	{
		case 0:
		case 1:
			return 1;
		case 2:
			return 2;
		default:
			break;
	}
	return 4;
}

static unsigned ogt_link__sort_priority(essl_symbol_t* symbol)
{
	if (!symbol)
		return 0;

	switch (symbol->type)
	{
		case essl_type_sampler2D:
		case essl_type_samplerCube:
			return 256; /* Ensure samplers come first. */
		default:
			break;
	}
	return ogt_link__alignof(symbol);
}



static void ogt_link__symbol_list_sort_by_align(
	essl_symbol_t** list, unsigned count)
{
	unsigned i;
	for (i = 0; i < count; i++)
	{
		unsigned align
			= ogt_link__sort_priority(list[i]);
		unsigned j;
		for (j = (i + 1); j < count; j++)
		{
			unsigned sort
				= ogt_link__sort_priority(list[j]);

			if (sort > align)
			{
				essl_symbol_t* swap = list[i];
				list[i] = list[j];
				list[j] = swap;
				align = sort;
			}
		}
	}
}

static ogt_link_t* ogt_link__collision(
	ogt_link_t* list, unsigned count,
	unsigned offset, unsigned width, unsigned height, unsigned stride)
{
	if (!list || !count)
		return NULL;

	unsigned size
		= (height > 1 ? stride : width) * height;

	unsigned k;
	for (k = 0; k < count; k++)
	{
		unsigned o = list[k].offset;
		if (o >= (offset + size))
			continue;
		unsigned sz = ogt_link__sizeof(list[k].symbol);
		if ((o + sz) <= offset)
			continue;

		unsigned w = 0, s = 0;
		ogt_link__dimensions(list[k].symbol, &w, NULL, &s);
		if ((w == s)
			&& (width == stride))
			return &list[k];

		/* Point-wise collision. */
		unsigned j, js;
		for (j = 0, js = offset; j < height; j++, js += stride)
		{
			if ((js + stride) <= o) continue;
			if (js >= (o + sz)) break;
			unsigned i, is;
			for (i = 0, is = js; i < width; i++, is++)
			{
				if (is < o) continue;
				if (is >= (o + sz)) break;

				/* Check if the cells collide. */
				unsigned x = (is - o) % s;
				if (x < w) return &list[k];
			}
		}
	}
	return NULL;
}



ogt_link_map_t* ogt_link_map_create()
{
	ogt_link_map_t* map
		= (ogt_link_map_t*)malloc(
			sizeof(ogt_link_map_t));
	if (!map) return NULL;
	map->uniform_count   = 0;
	map->attribute_count = 0;
	map->varying_count   = 0;
	map->uniform   = NULL;
	map->attribute = NULL;
	map->varying   = NULL;
	return map;
}

ogt_link_map_t* ogt_link_map_import_essl(essl_program_t* program)
{
	if (!program)
		return NULL;

	ogt_link_map_t* map
		= ogt_link_map_create();
	if (!map) return NULL;

	unsigned i, uc, ac, vc;
	for (i = 0, uc = 0, ac = 0, vc = 0;
		i < program->symbol_count; i++)
	{
		essl_symbol_t* s = program->symbol[i];
		switch (s->storage_qualifier)
		{
			case essl_storage_qualifier_uniform:
			case essl_storage_qualifier_const:
				uc++; break;
			case essl_storage_qualifier_attribute:
				ac++; break;
			case essl_storage_qualifier_varying:
				vc++; break;
			default:
				break;
		}
	}

	essl_symbol_t* uniform[uc];
	essl_symbol_t* attribute[ac];
	essl_symbol_t* varying[vc];

	unsigned u, a, v;
	for (i = 0, u = 0, a = 0, v = 0;
		i < program->symbol_count; i++)
	{
		essl_symbol_t* s = program->symbol[i];
		switch (s->storage_qualifier)
		{
			case essl_storage_qualifier_uniform:
			case essl_storage_qualifier_const:
				uniform[u++] = s;
				break;
			case essl_storage_qualifier_attribute:
				attribute[a++] = s;
				break;
			case essl_storage_qualifier_varying:
				varying[v++] = s;
				break;
			default:
				break;
		}
	}

	ogt_link__symbol_list_sort_by_align(uniform, uc);
	ogt_link__symbol_list_sort_by_align(attribute, ac);
	ogt_link__symbol_list_sort_by_align(varying, vc);

	for (i = 0; i < uc; i++)
	{
		essl_symbol_t* symbol
			= essl_symbol_reference(uniform[i]);
		if (!symbol
			|| !ogt_link_map_place(map, symbol))
		{
			essl_symbol_delete(symbol);
			ogt_link_map_delete(map);
			return NULL;
		}
	}

	for (i = 0; i < ac; i++)
	{
		essl_symbol_t* symbol
			= essl_symbol_reference(attribute[i]);
		if (!symbol
			|| !ogt_link_map_place(map, symbol))
		{
			essl_symbol_delete(symbol);
			ogt_link_map_delete(map);
			return NULL;
		}
	}

	for (i = 0; i < vc; i++)
	{
		essl_symbol_t* symbol
			= essl_symbol_reference(varying[i]);
		if (!symbol
			|| !ogt_link_map_place(map, symbol))
		{
			essl_symbol_delete(symbol);
			ogt_link_map_delete(map);
			return NULL;
		}
	}

	return map;
}



unsigned ogt_link_map_uniform_area(ogt_link_map_t* map)
{
	if (!map) return 0;

	unsigned i;
	unsigned s = 0;
	for (i = 0; i < map->uniform_count; i++)
	{
		unsigned ns
			= map->uniform[i].offset
				+ ogt_link__sizeof(map->uniform[i].symbol);
		if (ns > s)
			s = ns;
	}
	if (s & 3)
		s += (4 - (s & 3));
	return s;
}

essl_program_t* ogt_link_map_export_essl(ogt_link_map_t* map)
{
	if (!map) return NULL;

	essl_program_t* program
		= essl_program_create();
	if (!program)
	{
		fprintf(stderr, "Error: Failed to create program, out of memory.\n");
		return NULL;
	}

	unsigned t;
	for (t = 0; t < 3; t++)
	{
		unsigned     count;
		ogt_link_t* table;
		switch (t)
		{
			default:
				count = map->uniform_count;
				table = map->uniform;
				break;
			case 1:
				count = map->attribute_count;
				table = map->attribute;
				break;
			case 2:
				count = map->varying_count;
				table = map->varying;
				break;
		}

		unsigned i;
		for (i = 0; i < count; i++)
		{
			essl_symbol_t* symbol
				= essl_symbol_reference(table[i].symbol);

			if (!symbol
				|| !essl_program_add_symbol(
					program, symbol))
			{
				essl_symbol_delete(symbol);
				essl_program_delete(program);
				fprintf(stderr, "Error: Failed to copy/append symbol.\n");
				return NULL;
			}
		}
	}

	return program;
}



bool ogt_link_map_import(
	ogt_link_map_t* map, essl_symbol_t* symbol, unsigned offset)
{
	if (!map || !symbol)
		return false;

	ogt_link_t** list;
	unsigned*     count;
	switch (symbol->storage_qualifier)
	{
		case essl_storage_qualifier_uniform:
		case essl_storage_qualifier_const:
			list  = &map->uniform;
			count = &map->uniform_count;
			break;
		case essl_storage_qualifier_attribute:
			list  = &map->attribute;
			count = &map->attribute_count;
			break;
		case essl_storage_qualifier_varying:
			list  = &map->varying;
			count = &map->varying_count;
			break;
		default:
			return false;
	}

	unsigned width = 0, height = 0, stride = 0;
	ogt_link__dimensions(symbol, &width, &height, &stride);
	ogt_link_t* collide
		= ogt_link__collision(
			*list, *count,
			offset, width, height, stride);
	if (collide)
	{
		fprintf(stderr,
			"Error: Import failed, %s, collides with %s.\n",
			symbol->name, collide->symbol->name);
		return false;
	}

	ogt_link_t* nlist
		= (ogt_link_t*)realloc(*list,
			((*count + 1) * sizeof(ogt_link_t)));
	if (!nlist) return false;
	*list = nlist;

	(*list)[*count].symbol = symbol;
	(*list)[*count].offset = offset;
	*count += 1;
	return true;
}

bool ogt_link_map_place(ogt_link_map_t* map, essl_symbol_t* symbol)
{
	if (!map || !symbol)
		return false;

	ogt_link_t* list;
	unsigned     count;
	switch (symbol->storage_qualifier)
	{
		case essl_storage_qualifier_uniform:
		case essl_storage_qualifier_const:
			list  = map->uniform;
			count = map->uniform_count;
			break;
		case essl_storage_qualifier_attribute:
			list  = map->attribute;
			count = map->attribute_count;
			break;
		case essl_storage_qualifier_varying:
			list  = map->varying;
			count = map->varying_count;
			break;
		default:
			return false;
	}

	unsigned align
		= ogt_link__alignof(symbol);
	if (!align) return false;

	unsigned width = 0, height = 0, stride = 0;
	ogt_link__dimensions(symbol, &width, &height, &stride);

	unsigned offset;
	for (offset = 0;
		ogt_link__collision(
			list, count, offset,
			width, height, stride);
			offset += align);

	return ogt_link_map_import(map, symbol, offset);
}



essl_symbol_t* ogt_link_map_reference(
	ogt_link_map_t* map,
	essl_storage_qualifier_e sq, unsigned offset,
	unsigned* index, unsigned* component)
{
	if (!map)
		return NULL;

	ogt_link_t* list;
	unsigned     count;
	switch (sq)
	{
		case essl_storage_qualifier_uniform:
		case essl_storage_qualifier_const:
			list  = map->uniform;
			count = map->uniform_count;
			break;
		case essl_storage_qualifier_attribute:
			list  = map->attribute;
			count = map->attribute_count;
			break;
		case essl_storage_qualifier_varying:
			list  = map->varying;
			count = map->varying_count;
			break;
		default:
			return NULL;
	}

	ogt_link_t* link
		= ogt_link__collision(
			list, count, offset, 1, 1, 1);
	if (!link) return NULL;

	if (index || component)
	{
		unsigned o = (offset - link->offset);
		unsigned w = 0, h = 0, s = 0;
		ogt_link__dimensions(
			link->symbol, &w, &h, &s);
		if (!w || !h || !s)
		{
			if (index    ) *index     = 0;
			if (component) *component = 0;
		} else {
			unsigned sz = (h > 1 ? s : w) * h;

			if (index)
				*index = (o / sz);
			if (component)
			{
				o %= sz;
				*component = (o % s)
					+ ((o / s) * w);
			}
		}
	}

	return link->symbol;
}

essl_symbol_t* ogt_link_map_find(
	ogt_link_map_t* map, const char* name, unsigned* offset)
{
	if (!map)
		return NULL;

	unsigned i;
	for (i = 0; i < map->uniform_count; i++)
	{
		if (strcmp(
			map->uniform[i].symbol->name,
			name) == 0)
		{
			if (offset)
				*offset = map->uniform[i].offset;
			return map->uniform[i].symbol;
		}
	}
	for (i = 0; i < map->attribute_count; i++)
	{
		if (strcmp(
			map->attribute[i].symbol->name,
			name) == 0)
		{
			if (offset)
				*offset = map->attribute[i].offset;
			return map->attribute[i].symbol;
		}
	}
	for (i = 0; i < map->varying_count; i++)
	{
		if (strcmp(
			map->varying[i].symbol->name,
			name) == 0)
		{
			if (offset)
				*offset = map->varying[i].offset;
			return map->varying[i].symbol;
		}
	}

	return NULL;
}



void ogt_link_map_delete(ogt_link_map_t* map)
{
	if (!map)
		return;

	if (map->uniform)
	{
		unsigned i;
		for (i = 0; i < map->uniform_count; i++)
			essl_symbol_delete(map->uniform[i].symbol);
		free(map->uniform);
	}

	if (map->attribute)
	{
		unsigned i;
		for (i = 0; i < map->attribute_count; i++)
			essl_symbol_delete(map->attribute[i].symbol);
		free(map->attribute);
	}

	if (map->varying)
	{
		unsigned i;
		for (i = 0; i < map->varying_count; i++)
			essl_symbol_delete(map->varying[i].symbol);
		free(map->varying);
	}

	free(map);
}



#include <stdio.h>

void ogt_link_map_print(ogt_link_map_t* map)
{
	if (!map)
		return;

	if (map->uniform)
	{
		unsigned i;
		for (i = 0; i < map->uniform_count; i++)
		{
			unsigned size
				= ogt_link__sizeof(map->uniform[i].symbol);
			printf("uniform[%u:%u] %s\n",
				map->uniform[i].offset, size,
				map->uniform[i].symbol->name);
		}
	}

	if (map->attribute)
	{
		unsigned i;
		for (i = 0; i < map->attribute_count; i++)
		{
			unsigned size
				= ogt_link__sizeof(map->attribute[i].symbol);
			printf("attribute[%u:%u] %s\n",
				map->attribute[i].offset, size,
				map->attribute[i].symbol->name);
		}
	}

	if (map->varying)
	{
		unsigned i;
		for (i = 0; i < map->varying_count; i++)
		{
			unsigned size
				= ogt_link__sizeof(map->varying[i].symbol);
			printf("varying[%u:%u] %s\n",
				map->varying[i].offset, size,
				map->varying[i].symbol->name);
		}
	}
}
