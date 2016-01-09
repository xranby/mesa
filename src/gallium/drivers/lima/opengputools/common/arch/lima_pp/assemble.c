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



#include "lima_pp.h"

#include <ogt/bitaddr.h>
#include <ogt/essl.h>
#include <ogt/link.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>



static unsigned _lima_pp_asm_parse_space(const char* src)
{
	unsigned i;
	for (i = 0; isspace(src[i]); i++);
	while (src[i] == '#')
	{
		for (i++; src[i] != '\n'; i++);
		for (i++; isspace(src[i]); i++);
	}
	return i;
}

static unsigned _lima_pp_asm_parse_unsigned(const char* src,
	unsigned bits, unsigned* number)
{
	if (!isdigit(src[0]))
		return 0;
	
	unsigned n, i;
	for (n = 0, i = 0; isdigit(src[i]); i++)
	{
		n = (n * 10) + (src[i] - '0');
		if (n >= (1U << bits))
			return 0;
	}

	if (number)
		*number = n;
	return i;
}

static unsigned _lima_pp_asm_parse_zero(const char* src)
{
	unsigned i;
	for (i = 0; src[i] == '0'; i++);
	if (src[i] == '.')
	{
		if ((i == 0)
			&& src[i + 1] != '0')
			return 0;
		for (i++; src[i] == '0'; i++);
	}
	if (src[i] == 'f')
		i++;
	return i;
}

static unsigned _lima_pp_asm_parse_one(const char* src)
{
	unsigned i;
	for (i = 0; src[i] == '0'; i++);
	if (src[i++] != '1')
		return 0;
	if (src[i] == '.')
	{
		for (i++; src[i] == '0'; i++);
	}
	if (src[i] == 'f')
		i++;
	return i;
}

static unsigned _lima_pp_asm_parse_vec4(
	const char* src, lima_pp_vec4_t* vec4)
{
	if (strncasecmp(src, "vec4", 4) != 0)
		return 0;
	unsigned i = 4;

	i += _lima_pp_asm_parse_space(&src[i]);
	if (src[i++] != '(')
		return 0;
	i += _lima_pp_asm_parse_space(&src[i]);

	unsigned j;
	lima_pp_vec4_t v;
	j = ogt_hfloat_parse(&src[i], &v.x);
	if (!j) return 0;
	i += j;

	i += _lima_pp_asm_parse_space(&src[i]);
	if (src[i++] != ',')
		return 0;
	i += _lima_pp_asm_parse_space(&src[i]);

	j = ogt_hfloat_parse(&src[i], &v.y);
	if (!j) return 0;
	i += j;

	i += _lima_pp_asm_parse_space(&src[i]);
	if (src[i++] != ',')
		return 0;
	i += _lima_pp_asm_parse_space(&src[i]);

	j = ogt_hfloat_parse(&src[i], &v.z);
	if (!j) return 0;
	i += j;

	i += _lima_pp_asm_parse_space(&src[i]);
	if (src[i++] != ',')
		return 0;
	i += _lima_pp_asm_parse_space(&src[i]);

	j = ogt_hfloat_parse(&src[i], &v.w);
	if (!j) return 0;
	i += j;

	i += _lima_pp_asm_parse_space(&src[i]);
	if (src[i++] != ')')
		return 0;

	if (vec4)
		*vec4 = v;
	return i;
}



static unsigned _lima_pp_asm_parse_op(const char* src,
	lima_pp_asm_op_t* table, unsigned length, unsigned* op, bool* arg0)
{
	if (!table)
		return 0;

	char obuff[8];

	unsigned o;
	for (o = 0; o < length; o++)
	{
		const char* name
			= table[o].name;
		if (!name)
		{
			sprintf(obuff, "op%u", o);
			name = obuff;
		}
		unsigned i = strlen(name);

		if (strncasecmp(src, name, i) == 0)
		{
			if (op)
				*op = o;
			if (arg0)
				*arg0 = table[o].arg0;
			i += _lima_pp_asm_parse_space(&src[i]);
			if (src[i++] != '(')
				return 0;
			i += _lima_pp_asm_parse_space(&src[i]);
			return i;
		}
	}

	return 0;
}

static unsigned _lima_pp_asm_parse_op_sym(const char* src,
	lima_pp_asm_op_t* table, unsigned length, unsigned* op, bool* arg0)
{
	if (!table)
		return 0;

	unsigned o;
	for (o = 0; o < length; o++)
	{
		if (!table[o].symbol)
			continue;
		unsigned i = strlen(table[o].symbol);
		if (strncasecmp(src, table[o].symbol, i) == 0)
		{
			i += _lima_pp_asm_parse_space(&src[i]);
			if (op)
				*op = o;
			if (arg0)
				*arg0 = table[o].arg0;
			return i;
		}
	}

	return 0;
}



static unsigned _lima_pp_asm_parse_vec4_component(
	const char* src, unsigned* component)
{
	unsigned c;
	switch (tolower(src[0]))
	{
		case 'x':
		case 'r':
		case 's':
			c = 0;
			break;
		case 'y':
		case 'g':
		case 't':
			c = 1;
			break;
		case 'z':
		case 'b':
		case 'p':
			c = 2;
			break;
		case 'w':
		case 'a':
		case 'q':
			c = 3;
			break;
		default:
			return 0;
	}
	if (component)
		*component = c;
	return 1;
}

static unsigned _lima_pp_asm_parse_vec4_mask(const char* src, uint8_t* mask)
{
	uint8_t m = 0;
	unsigned c;
	unsigned i
		= _lima_pp_asm_parse_vec4_component(src, &c);
	if (!i) return 0;
	m |= (1 << c);

	unsigned j;
	for (j = _lima_pp_asm_parse_vec4_component(&src[i], &c);
		j; j = _lima_pp_asm_parse_vec4_component(&src[i], &c))
	{
		if (m & (1 << c))
			return 0;
		m |= (1 << c);
		i += j;
	}

	if (mask)
		*mask = m;
	return i;
}

static unsigned _lima_pp_asm_parse_vec4_swizzle(
	const char* src, uint8_t* swizzle)
{
	uint8_t s = 0;
	unsigned i = 0, j, c;

	j = _lima_pp_asm_parse_vec4_component(&src[i], &c);
	if (!j) return 0; i += j; s |= (c << 0);
	j = _lima_pp_asm_parse_vec4_component(&src[i], &c);
	if (!j) return 0; i += j; s |= (c << 2);
	j = _lima_pp_asm_parse_vec4_component(&src[i], &c);
	if (!j) return 0; i += j; s |= (c << 4);
	j = _lima_pp_asm_parse_vec4_component(&src[i], &c);
	if (!j) return 0; i += j; s |= (c << 6);

	if (swizzle)
		* swizzle = s;
	return i;
}

static unsigned _lima_pp_asm_parse_reg(const char* src, unsigned* reg)
{
	if ((src[0] != '$')
		|| !isdigit(src[1]))
		return 0;

	unsigned i, r;
	for (i = 1, r = 0; isdigit(src[i]); i++)
		r = (r * 10) + (src[i] - '0');

	if (reg)
		*reg = r;
	return i;
}

static unsigned _lima_pp_asm_parse_reg_special(
	const char* src, const char* special)
{
	if (src[0] != '^')
		return 0;
	unsigned slen = strlen(special);
	if (strncasecmp(&src[1], special, slen) == 0)
		return (slen + 1);
	return 0;
}

static unsigned _lima_pp_asm_parse_modifier_end(
	const char* src,
	lima_pp_outmod_e modifier)
{
	unsigned i
		= _lima_pp_asm_parse_space(src);
	
	switch (modifier)
	{
		case lima_pp_outmod_clamp_fraction:
		{
			if (src[i++] != ',')
				return 0;
			i += _lima_pp_asm_parse_space(&src[i]);

			unsigned j;
			j  = _lima_pp_asm_parse_zero(&src[i]);
			if (!j) return 0;
			i += j;
			i += _lima_pp_asm_parse_space(&src[i]);

			if (src[i++] != ',')
				return 0;
			i += _lima_pp_asm_parse_space(&src[i]);

			j  = _lima_pp_asm_parse_one(&src[i]);
			if (!j) return 0;
			i += j;
			i += _lima_pp_asm_parse_space(&src[i]);
		} break;
		case lima_pp_outmod_clamp_positive:
		case lima_pp_outmod_round:
			break;
		default:
			return 0;
	}

	if (src[i++] != ')')
		return 0;
	return i;
}

static unsigned _lima_pp_asm_parse_dest(
	const char* src, const char* special,
	unsigned* reg,
	unsigned* component,
	uint8_t*  mask,
	lima_pp_outmod_e* modifier)
{
	unsigned r;
	unsigned i;
	if (special)
		i = _lima_pp_asm_parse_reg_special(src, special);
	else
		i = _lima_pp_asm_parse_reg(src, &r);
	if (!i) return 0;

	uint8_t m = 0x0F;
	if (mask)
	{
		if (src[i] == '.')
		{
			i++;
			unsigned j
				= _lima_pp_asm_parse_vec4_mask(
					&src[i], &m);
			if (!j) return 0;
			i += j;
		}
	}

	unsigned c = 0;
	if (component)
	{
		if (src[i++] != '.')
			return 0;
		unsigned j;
		j = _lima_pp_asm_parse_vec4_component(&src[i], &c);
		if (!j) return 0;
		i += j;
	}

	i += _lima_pp_asm_parse_space(&src[i]);
	if (src[i++] != '=')
		return 0;

	lima_pp_outmod_e mod = lima_pp_outmod_none;
	if (modifier)
	{
		i += _lima_pp_asm_parse_space(&src[i]);
		if (strncasecmp(&src[i], "clamp", 5) == 0)
		{
			mod = lima_pp_outmod_clamp_fraction;
			i += 5;
			i += _lima_pp_asm_parse_space(&src[i]);
			if (src[i++] != '(')
				return 0;
		}
		else if (strncasecmp(&src[i], "max", 3) == 0)
		{
			unsigned k = i + 3;
			k += _lima_pp_asm_parse_space(&src[k]);
			if (src[k++] == '(')
			{
				unsigned j;
				k += _lima_pp_asm_parse_space(&src[k]);
				j = _lima_pp_asm_parse_zero(&src[k]);
				k += j;
				k += _lima_pp_asm_parse_space(&src[k]);
				if (j && src[k++] == ',')
				{
					mod = lima_pp_outmod_clamp_positive;
					i = k;
				}
			}
		}
		else if (strncasecmp(&src[i], "round", 5) == 0)
		{
			mod = lima_pp_outmod_round;
			i += 5;
			i += _lima_pp_asm_parse_space(&src[i]);
			if (src[i++] != '(')
				return 0;
		}
	}

	if (reg)
		*reg = r;
	if (component)
		*component = c;
	if (mask)
		*mask = m;
	if (modifier)
		*modifier = mod;
	return i;
}

static unsigned _lima_pp_asm_parse_source(
	const char* src, const char* special, bool src_pipe,
	unsigned* reg, unsigned* component,
	bool* negate, bool* absolute,
	uint8_t* swizzle)
{
	unsigned i = 0, j;
	bool neg = false;
	if (src[i] == '-')
	{
		neg = true;
		i++;
		i += _lima_pp_asm_parse_space(&src[i]);
	}

	bool abs = false;
	if (strncasecmp(&src[i], "abs", 3) == 0)
	{
		i += 3;
		i += _lima_pp_asm_parse_space(&src[i]);
		if (src[i++] != '(')
			return 0;
		abs = true;
		i += _lima_pp_asm_parse_space(&src[i]);
	}

	unsigned r = 0;
	unsigned c = 0;
	if (special)
	{
		j = _lima_pp_asm_parse_reg_special(&src[i], special);
		if (!j) return 0;
		i += j;
	} else {
		j = _lima_pp_asm_parse_reg(&src[i], &r);
		if (!j && src_pipe)
		{
			const char* spec_name[] =
			{
				"const0",
				"const1",
				"texture",
				"uniform",
				NULL
			};
			lima_pp_vec4_reg_e spec_value[] =
			{
				lima_pp_vec4_reg_constant0,
				lima_pp_vec4_reg_constant1,
				lima_pp_vec4_reg_texture,
				lima_pp_vec4_reg_uniform,
			};

			unsigned s;
			for (s = 0; !j && spec_name[s]; s++)
			{
				j = _lima_pp_asm_parse_reg_special(
					&src[i], spec_name[s]);
				r = spec_value[s];
			}
			if (!j) return 0;
		}
		i += j;

		if (component)
		{
			if (src[i++] != '.')
				return 0;
			j = _lima_pp_asm_parse_vec4_component(&src[i], &c);
			if (!j) return 0;
			i += j;
		}
	}

	uint8_t swiz = 0xE4;
	if (swizzle
		&& (src[i] == '.'))
	{
		i++;
		j = _lima_pp_asm_parse_vec4_swizzle(
				&src[i], &swiz);
		if (!j) return 0;
		i += j;
	}

	if (abs)
	{
		i += _lima_pp_asm_parse_space(&src[i]);
		if (src[i++] != ')')
			return 0;
	}

	if (!special && reg)
		*reg = r;
	if (!special && component)
		*component = c;
	if (negate)
		*negate = neg;
	if (absolute)
		*absolute = abs;
	if (swizzle)
		*swizzle = swiz;
	return i;
}

static unsigned _lima_pp_asm_parse_source_symbol(
	const char* src, essl_storage_qualifier_e sq,
	unsigned* symbol, ogt_link_map_t* map)
{
	if (!map
		|| (!isalpha(src[0])
			&& (src[0] != '_')))
		return 0;

	/* TODO - Correctly handle wide (vec4) uniform references. */

	unsigned i;
	for (i = 1; isalnum(src[i]) || src[i] == '_'; i++);
	char name[i + 1];
	memcpy(name, src, i);
	name[i] = '\0';

	unsigned offset;
	essl_symbol_t* s
		= ogt_link_map_find(map, name, &offset);

	if (!s
		|| (s->storage_qualifier != sq))
		return 0;

	if (symbol)
		*symbol = (offset >> 2);
	return i;
}

static unsigned _lima_pp_asm_parse_source_uniform(
	const char* src, unsigned* uniform, ogt_link_map_t* map)
{
	unsigned i
		= _lima_pp_asm_parse_source_symbol(
			src, essl_storage_qualifier_uniform,
			uniform, map);
	if (i) return i;

	if (strncasecmp(src, "uniform", 7) != 0)
		return 0;
	i = 7;
	i += _lima_pp_asm_parse_space(&src[i]);
	if (src[i++] != '[')
		return 0;
	i += _lima_pp_asm_parse_space(&src[i]);

	if (!isdigit(src[i]))
		return 0; 

	unsigned index;
	for (index = 0; isdigit(src[i]); i++)
		index = (index * 10) + (src[i] - '0');

	i += _lima_pp_asm_parse_space(&src[i]);
	if (src[i++] != ']')
		return 0;

	if (uniform)
		*uniform = index;
	return i;
}

static unsigned _lima_pp_asm_parse_source_varying(
	const char* src, unsigned* varying, ogt_link_map_t* map)
{
	unsigned i
		= _lima_pp_asm_parse_source_symbol(
			src, essl_storage_qualifier_varying,
			varying, map);
	if (i) return i;

	if (strncasecmp(src, "varying", 7) != 0)
		return 0;
	i = 7;
	i += _lima_pp_asm_parse_space(&src[i]);
	if (src[i++] != '[')
		return 0;
	i += _lima_pp_asm_parse_space(&src[i]);

	if (!isdigit(src[i]))
		return 0; 
	unsigned index;
	for (index = 0; isdigit(src[i]); i++)
		index = (index * 10) + (src[i] - '0');

	i += _lima_pp_asm_parse_space(&src[i]);
	if (src[i++] != ']')
		return 0;

	if (varying)
		*varying = index;
	return i;
}



static unsigned _lima_pp_asm_parse_field_uniform(
	const char* src, lima_pp_field_uniform_t* field, ogt_link_map_t* map)
{
	lima_pp_field_uniform_t f =
	{
		.source    = lima_pp_uniform_src_uniform,
		.unknown_0 = 0x000,
		.alignment = true,
		.unknown_1 = 0x0000,
		.index     = 0,
	};

	unsigned i = _lima_pp_asm_parse_reg_special(src, "uniform");
	if (!i) return 0;

	unsigned component = 0;
	if (src[i] == '.')
	{
		i++;
		unsigned j
			= _lima_pp_asm_parse_vec4_component(
				&src[i], &component);
		if (!j) return 0;
		i += j;
		f.alignment = false;
	}

	i += _lima_pp_asm_parse_space(&src[i]);
	if (src[i++] != '=')
		return 0;
	i += _lima_pp_asm_parse_space(&src[i]);

	unsigned index;
	unsigned j = _lima_pp_asm_parse_source_uniform(&src[i], &index, map);
	if (!j) return 0;
	i += j;
	if (!f.alignment)
		index = (index << 2) | component;
	f.index = index;

	i += _lima_pp_asm_parse_space(&src[i]);
	if ((src[i] != ',')
		&& (src[i] != ';'))
		return 0;

	if (field)
		*field = f;
	return i;
}

static unsigned _lima_pp_asm_parse_field_varying(
	const char* src, lima_pp_field_varying_t* field, ogt_link_map_t* map)
{
	unsigned dest;
	uint8_t  mask;

	unsigned i;
	i = _lima_pp_asm_parse_dest(src, NULL,
		&dest, NULL, & mask, NULL);
	if (!i) dest = lima_pp_vec4_reg_discard;

	i += _lima_pp_asm_parse_space(&src[i]);

	unsigned index;
	unsigned j
		= _lima_pp_asm_parse_source_varying(
			&src[i], &index, map);

	if (j)
	{
		i += j;

		unsigned alignment = 3;
		if (src[i] == '.')
		{
			i++;
			unsigned c[2];
			j = _lima_pp_asm_parse_vec4_component(&src[i], &c[0]);
			if (!j) return 0; i += j;

			j = _lima_pp_asm_parse_vec4_component(&src[i], &c[1]);
			if (j)
			{
				if ((c[0] & 1)
					|| (c[1] != ((c[0] + 1) & 3)))
					return 0;
				i += j;

				alignment = 1;
				index = (index << 1) | (c[0] >> 1);		
			} else {
				alignment = 0;
				index = (index << 2) | c[0];
			}
		}

		lima_pp_field_varying_t f =
		{
			.imm =
			{
				.perspective = 0,
				.source_type = 0,
				.unknown_0   = 0x0,
				.alignment   = alignment,
				.unknown_1   = 0x078,
				.index       = index,
				.dest        = dest,
				.mask        = mask,
				.unknown_2   = 0x0,
			},
		};

		if (field)
			*field = f;
		return i;
	}

	unsigned reg;
	bool neg, abs;
	uint8_t swiz;
	j = _lima_pp_asm_parse_source(&src[i], NULL, false,
		&reg, NULL, &neg, &abs, &swiz);
	if (!j) return 0;
	i += j;

	/* TODO - Special sources. */

	i += _lima_pp_asm_parse_space(&src[i]);
	if ((src[i] != ',')
		&& (src[i] != ';'))
		return 0;

	lima_pp_field_varying_t f =
	{
		.reg =
		{
			.perspective = 0,
			.source_type = 1,
			.unknown_0   = 0x00,
			.source      = reg,
			.negate      = neg,
			.absolute    = abs,
			.swizzle     = swiz,
			.dest        = dest,
			.mask        = mask,
			.unknown_2   = 0x0
		},
	};

	if (field)
		*field = f;
	return i;
}

static unsigned _lima_pp_asm_parse_field_sampler(
	const char* src, lima_pp_field_sampler_t* field, ogt_link_map_t* map)
{
	(void)map; /* Not used. */

	unsigned i = 0, j;

	j = _lima_pp_asm_parse_dest(&src[i], "texture",
		NULL, NULL, NULL, NULL);
	if (!j) return 0;
	i += j;
	i+= _lima_pp_asm_parse_space(&src[i]);

	if (strncasecmp(&src[i], "sampler", 7) != 0)
		return 0;
	i += 7;
	
	unsigned type;
	if (strncasecmp(&src[i], "2d", 2) == 0)
	{
		type = lima_pp_sampler_type_2d;
		i += 2;
	}
	else if (isdigit(src[i]))
	{
		j  = _lima_pp_asm_parse_unsigned(&src[i], 5, &type);
		if (!j) return 0;
		i += j;
	}
	else if (strncasecmp(&src[i], "cube", 4) == 0)
	{
		i += 4;
		type = lima_pp_sampler_type_cube;
	} else
		return 0;

	i += _lima_pp_asm_parse_space(&src[i]);
	if (src[i++] != '(')
		return 0;
	i += _lima_pp_asm_parse_space(&src[i]);

	unsigned index;
	j = _lima_pp_asm_parse_unsigned(&src[i], 12, &index);
	if (!j) return 0;
	i += j;
	i += _lima_pp_asm_parse_space(&src[i]);

	lima_pp_vec4_reg_e offset_reg = 0;
	unsigned        offset_com = 0;
	bool            offset_en  = false;
	if (src[i] == '+')
	{
		offset_en = true;

		i++;
		i += _lima_pp_asm_parse_space(&src[i]);
		
		j = _lima_pp_asm_parse_source(&src[i], NULL, false,
			&offset_reg, &offset_com, NULL, NULL, NULL);
		if (!j) return 0;
		i += j;

		i += _lima_pp_asm_parse_space(&src[i]);
	}

	lima_pp_vec4_reg_e lod_bias_reg = 0;
	unsigned        lod_bias_com = 0;
	bool            lod_bias_en  = false;
	if (src[i] == ',')
	{
		lod_bias_en = true;

		i++;
		i += _lima_pp_asm_parse_space(&src[i]);
		
		j = _lima_pp_asm_parse_source(&src[i], NULL, false,
			&lod_bias_reg, &lod_bias_com, NULL, NULL, NULL);
		if (!j) return 0;
		i += j;

		i += _lima_pp_asm_parse_space(&src[i]);
	}

	if (src[i++] != ')')
		return 0;

	i += _lima_pp_asm_parse_space(&src[i]);
	if ((src[i] != ',')
		&& (src[i] != ';'))
		return 0;

	lima_pp_field_sampler_t f =
	{
		.lod_bias     = (lod_bias_reg << 2) | lod_bias_com,
		.index_offset = (offset_reg << 2) | offset_com,
		.unknown_0    = 0,
		.lod_bias_en  = lod_bias_en,
		.unknown_1    = 0x00,
		.type         = type,
		.offset_en    = offset_en,
		.index        = index,
		.unknown_2    = 0x39001,
	};

	if (field)
		*field = f;
	return i;
}

static unsigned _lima_pp_asm_parse_field_alu(const char* src,
	lima_pp_asm_op_t* asm_op, unsigned asm_op_count, unsigned asm_op_mov,
	unsigned* op, unsigned op_shift,
	lima_pp_vec4_reg_e* dst_reg, unsigned* dst_com, uint8_t* dst_mask,
	bool* dst_en, lima_pp_outmod_e* dst_mod, const char* dst_preg,
	lima_pp_vec4_reg_e* src0_reg, unsigned* src0_com, uint8_t* src0_swiz,
	bool* src0_abs, bool* src0_neg,
	lima_pp_vec4_reg_e* src1_reg, unsigned* src1_com, uint8_t* src1_swiz,
	bool* src1_abs, bool* src1_neg, const char* src1_sreg, bool* src1_special)
{
	unsigned i = 0, j;

	lima_pp_vec4_reg_e _dst_reg  = 0;
	unsigned        _dst_com  = 0;
	uint8_t         _dst_mask = 0x0;
	lima_pp_outmod_e   _dst_mod  = lima_pp_outmod_none;
	bool            _dst_en   = false;

	if (dst_reg)
	{
		j = _lima_pp_asm_parse_dest(&src[i], NULL,
			(dst_reg  ? &_dst_reg  : NULL),
			(dst_com  ? &_dst_com  : NULL),
			(dst_mask ? &_dst_mask : NULL),
			(dst_mod  ? &_dst_mod  : NULL));
		if (j)
		{
			_dst_en = true;
			i += j;
			i += _lima_pp_asm_parse_space(&src[i]);
		}
	}

	if (dst_preg
		&& (_dst_mod == lima_pp_outmod_none))
	{
		j = _lima_pp_asm_parse_dest(&src[i], dst_preg,
			NULL, NULL, NULL,
			(dst_mod  ? &_dst_mod  : NULL));
		if (j)
		{
			i += j;
			i += _lima_pp_asm_parse_space(&src[i]);
		}
	}

	bool arg0_expected = false;
	bool bracket = false;
	unsigned _op = 0;
	if (op)
	{
		j = _lima_pp_asm_parse_op(&src[i],
			asm_op, asm_op_count, &_op, &arg0_expected);
		bracket = (j != 0);
		if (bracket)
			i += j;
	}

	lima_pp_vec4_reg_e _src0_reg  = 0;
	unsigned        _src0_com  = 0;
	uint8_t         _src0_swiz = 0x00;
	bool            _src0_abs  = false;
	bool            _src0_neg  = false;

	bool arg0 = false;
	if (op)
	{
		if (src0_reg)
		{
			if (!bracket
				|| arg0_expected)
			{
				j = _lima_pp_asm_parse_source(&src[i], NULL, true,
					(src0_reg  ? &_src0_reg  : NULL),
					(src0_com  ? &_src0_com  : NULL),
					(src0_neg  ? &_src0_neg  : NULL),
					(src0_abs  ? &_src0_abs  : NULL),
					(src0_swiz ? &_src0_swiz : NULL));
				arg0 = (j != 0);
				if (arg0)
				{
					i += j;
					i += _lima_pp_asm_parse_space(&src[i]);
				}
			}
		}

		if (bracket)
		{
			if (arg0
				&& (src[i++] != ','))
				return 0;
		} else {
			j = _lima_pp_asm_parse_op_sym(&src[i],
				asm_op, asm_op_count, &_op, &arg0_expected);
			if (!j && (asm_op_mov < asm_op_count))
				_op = asm_op_mov;
			else if (arg0 != arg0_expected)
				return 0;
			i += j;
		}
		i += _lima_pp_asm_parse_space(&src[i]);
	}

	lima_pp_vec4_reg_e _src1_reg     = 0;
	unsigned        _src1_com     = 0;
	uint8_t         _src1_swiz    = 0x00;
	bool            _src1_abs     = false;
	bool            _src1_neg     = false;
	bool            _src1_special = false;

	if ((_op == lima_pp_vec4_mul_op_mov)
		&& arg0)
	{
		_src1_reg  = _src0_reg;
		_src1_com  = _src0_com;
		_src1_swiz = _src0_swiz;
		_src1_abs  = _src0_abs;
		_src1_neg  = _src0_neg;
		_src0_reg  = 0;
		_src0_com  = 0;
		_src0_swiz = 0;
		_src0_abs  = 0;
		_src0_neg  = 0;
	} else {
		j = _lima_pp_asm_parse_source(&src[i], NULL, true,
			(src1_reg  ? &_src1_reg  : NULL),
			(src1_com  ? &_src1_com  : NULL),
			(src1_neg  ? &_src1_neg  : NULL),
			(src1_abs  ? &_src1_abs  : NULL),
			(src1_swiz ? &_src1_swiz : NULL));
		if (!j)
		{
			if (!src1_sreg)
				return 0;
			j = _lima_pp_asm_parse_source(&src[i], src1_sreg, true,
				(src1_reg  ? &_src1_reg  : NULL),
				(src1_com  ? &_src1_com  : NULL),
				(src1_neg  ? &_src1_neg  : NULL),
				(src1_abs  ? &_src1_abs  : NULL),
				(src1_swiz ? &_src1_swiz : NULL));
			if (!j) return 0;
			_src1_special = true;
		}
		i += j;
	}

	if (op_shift
		&& (_op == 0))
	{
		i += _lima_pp_asm_parse_space(&src[i]);
		if ((src[i] == '<')
			&& (src[i + 1] == '<'))
		{
			i += 2;
			i += _lima_pp_asm_parse_space(&src[i]);
			if (!isdigit(src[i]))
				return 0;
			unsigned v;
			for (v = 0; isdigit(src[i]); i++)
			{
				v = (v * 10) + (src[i] - '0');
				if (v >= op_shift)
					return 0;
			}
			_op += v;
		}
	}
	
	if (bracket)
	{
		i += _lima_pp_asm_parse_space(&src[i]);
		if (src[i++] != ')')
			return 0;
	}

	if (_dst_mod)
	{
		j = _lima_pp_asm_parse_modifier_end(&src[i], _dst_mod);
		if (!j) return 0;
		i += j;
	}

	i += _lima_pp_asm_parse_space(&src[i]);
	if ((src[i] != ',')
		&& (src[i] != ';'))
		return 0;

	if (op)       *op = _op;
	if (dst_reg)  *dst_reg  = _dst_reg;
	if (dst_com)  *dst_com  = _dst_com;
	if (dst_mask) *dst_mask = _dst_mask;
	if (dst_en)   *dst_en   = _dst_en;
	if (dst_mod)  *dst_mod  = _dst_mod;
	if (src0_reg)  *src0_reg  = _src0_reg;
	if (src0_com)  *src0_com  = _src0_com;
	if (src0_swiz) *src0_swiz = _src0_swiz;
	if (src0_abs)  *src0_abs  = _src0_abs;
	if (src0_neg)  *src0_neg  = _src0_neg;
	if (src1_reg)  *src1_reg  = _src1_reg;
	if (src1_com)  *src1_com  = _src1_com;
	if (src1_swiz) *src1_swiz = _src1_swiz;
	if (src1_abs)  *src1_abs  = _src1_abs;
	if (src1_neg)  *src1_neg  = _src1_neg;
	if (src1_special) *src1_special = _src1_special;
	return i;
}

static unsigned _lima_pp_asm_parse_field_vec4_mul(
	const char* src, lima_pp_field_vec4_mul_t* field, ogt_link_map_t* map)
{
	(void)map; /* Not used. */

	lima_pp_vec4_reg_e dst_reg;
	uint8_t         dst_mask;
	lima_pp_outmod_e   dst_mod;

	lima_pp_vec4_reg_e src0_reg;
	uint8_t         src0_swiz;
	bool            src0_abs;
	bool            src0_neg;

	lima_pp_vec4_reg_e src1_reg;
	uint8_t         src1_swiz;
	bool            src1_abs;
	bool            src1_neg;

	lima_pp_vec4_mul_op_e op;

	unsigned i = _lima_pp_asm_parse_field_alu(src,
		lima_pp_vec4_mul_asm_op, 32, lima_pp_vec4_mul_op_mov, &op, 8,
		&dst_reg, NULL, &dst_mask, NULL, &dst_mod, "vmul",
		&src0_reg, NULL, &src0_swiz, &src0_abs, &src0_neg,
		&src1_reg, NULL, &src1_swiz, &src1_abs, &src1_neg, NULL, NULL);
	if (!i) return 0;
	
	lima_pp_field_vec4_mul_t f =
	{
		.arg1_source   = src1_reg,
		.arg1_swizzle  = src1_swiz,
		.arg1_absolute = src1_abs,
		.arg1_negate   = src1_neg,
		.arg0_source   = src0_reg,
		.arg0_swizzle  = src0_swiz,
		.arg0_absolute = src0_abs,
		.arg0_negate   = src0_neg,
		.dest          = dst_reg,
		.mask          = dst_mask,
		.dest_modifier = dst_mod,
		.op            = op,
	};

	if (field)
		*field = f;
	return i;
}

static unsigned _lima_pp_asm_parse_field_vec4_acc(
	const char* src, lima_pp_field_vec4_acc_t* field, ogt_link_map_t* map)
{
	(void)map; /* Not used. */

	lima_pp_vec4_reg_e dst_reg;
	uint8_t         dst_mask;
	lima_pp_outmod_e   dst_mod;

	lima_pp_vec4_reg_e src0_reg;
	uint8_t         src0_swiz;
	bool            src0_abs;
	bool            src0_neg;

	lima_pp_vec4_reg_e src1_reg;
	uint8_t         src1_swiz;
	bool            src1_abs;
	bool            src1_neg;
	bool            src1_mul;

	lima_pp_vec4_acc_op_e op;

	unsigned i = _lima_pp_asm_parse_field_alu(src,
		lima_pp_vec4_acc_asm_op, 32, lima_pp_vec4_acc_op_mov, &op, 0,
		&dst_reg, NULL, &dst_mask, NULL, &dst_mod, NULL,
		&src0_reg, NULL, &src0_swiz, &src0_abs, &src0_neg,
		&src1_reg, NULL, &src1_swiz, &src1_abs, &src1_neg, "vmul", &src1_mul);
	if (!i) return 0;
	
	lima_pp_field_vec4_acc_t f =
	{
		.arg1_source   = src1_reg,
		.arg1_swizzle  = src1_swiz,
		.arg1_absolute = src1_abs,
		.arg1_negate   = src1_neg,
		.arg0_source   = src0_reg,
		.arg0_swizzle  = src0_swiz,
		.arg0_absolute = src0_abs,
		.arg0_negate   = src0_neg,
		.dest          = dst_reg,
		.mask          = dst_mask,
		.dest_modifier = dst_mod,
		.op            = op,
		.mul_in        = src1_mul,
	};

	if (field)
		*field = f;
	return i;
}

static unsigned _lima_pp_asm_parse_field_float_mul(
	const char* src, lima_pp_field_float_mul_t* field, ogt_link_map_t* map)
{
	(void)map; /* Not used. */

	lima_pp_vec4_reg_e dst_reg;
	unsigned        dst_com;
	lima_pp_outmod_e   dst_mod;
	bool            dst_en;

	lima_pp_vec4_reg_e src0_reg;
	unsigned        src0_com;
	bool            src0_abs;
	bool            src0_neg;

	lima_pp_vec4_reg_e src1_reg;
	unsigned        src1_com;
	bool            src1_abs;
	bool            src1_neg;

	lima_pp_float_mul_op_e op;

	unsigned i = _lima_pp_asm_parse_field_alu(src,
		lima_pp_float_mul_asm_op, 32, lima_pp_float_mul_op_mov, &op, 8,
		&dst_reg, &dst_com, NULL, &dst_en, &dst_mod, "fmul",
		&src0_reg, &src0_com, NULL, &src0_abs, &src0_neg,
		&src1_reg, &src1_com, NULL, &src1_abs, &src1_neg, NULL, NULL);
	if (!i) return 0;

	lima_pp_field_float_mul_t f =
	{
		.arg1_source   = (src1_reg << 2) | src1_com,
		.arg1_absolute = src1_abs,
		.arg1_negate   = src1_neg,
		.arg0_source   = (src0_reg << 2) | src0_com,
		.arg0_absolute = src0_abs,
		.arg0_negate   = src0_neg,
		.dest          = (dst_reg << 2) | dst_com,
		.output_en     = dst_en,
		.dest_modifier = dst_mod,
		.op            = op,
	};

	if (field)
		*field = f;
	return i;
}

static unsigned _lima_pp_asm_parse_field_float_acc(
	const char* src, lima_pp_field_float_acc_t* field, ogt_link_map_t* map)
{
	(void)map; /* Not used. */

	lima_pp_vec4_reg_e dst_reg;
	unsigned        dst_com;
	lima_pp_outmod_e   dst_mod;
	bool            dst_en;

	lima_pp_vec4_reg_e src0_reg;
	unsigned        src0_com;
	bool            src0_abs;
	bool            src0_neg;

	lima_pp_vec4_reg_e src1_reg;
	unsigned        src1_com;
	bool            src1_abs;
	bool            src1_neg;
	bool            src1_mul;

	lima_pp_float_mul_op_e op;

	unsigned i = _lima_pp_asm_parse_field_alu(src,
		lima_pp_float_acc_asm_op, 32, lima_pp_float_acc_op_mov, &op, 0,
		&dst_reg, &dst_com, NULL, &dst_en, &dst_mod, NULL,
		&src0_reg, &src0_com, NULL, &src0_abs, &src0_neg,
		&src1_reg, &src1_com, NULL, &src1_abs, &src1_neg, "fmul", &src1_mul);
	if (!i) return 0;

	lima_pp_field_float_acc_t f =
	{
		.arg1_source   = (src1_reg << 2) | src1_com,
		.arg1_absolute = src1_abs,
		.arg1_negate   = src1_neg,
		.arg0_source   = (src0_reg << 2) | src0_com,
		.arg0_absolute = src0_abs,
		.arg0_negate   = src0_neg,
		.dest          = (dst_reg << 2) | dst_com,
		.output_en     = dst_en,
		.dest_modifier = dst_mod,
		.op            = op,
		.mul_in        = src1_mul,
	};

	if (field)
		*field = f;
	return i;
}

static unsigned _lima_pp_asm_parse_field_combine_ss(
	const char* src, lima_pp_field_combine_t* field)
{
	lima_pp_vec4_reg_e dst_reg = 0;
	unsigned             dst_com = 0;

	lima_pp_vec4_reg_e src0_reg = 0;
	unsigned             src0_com = 0;
	bool                 src0_abs = 0;
	bool                 src0_neg = 0;

	unsigned op;

	unsigned i = _lima_pp_asm_parse_field_alu(src,
		lima_pp_combine_asm_op, 16, lima_pp_combine_scalar_op_mov, &op, 0,
		&dst_reg, &dst_com, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		&src0_reg, &src0_com, NULL, &src0_abs, &src0_neg, NULL, NULL);
	if (!i) return 0;

	lima_pp_field_combine_t f;
	f.scalar.dest_vec      = false;
	f.scalar.arg1_en       = false;
	f.scalar.op            = op;
	f.scalar.arg0_absolute = src0_abs;
	f.scalar.arg0_negate   = src0_neg;
	f.scalar.arg0_src      = (src0_reg << 2) | src0_com;
	f.scalar.arg1_absolute = 0;
	f.scalar.arg1_negate   = 0;
	f.scalar.arg1_src      = 0;
	f.scalar.dest          = (dst_reg << 2) | dst_com;

	if (field)
		*field = f;
	return i;
}

static unsigned _lima_pp_asm_parse_field_combine_vs(
	const char* src, lima_pp_field_combine_t* field)
{
	lima_pp_vec4_reg_e dst_reg;
	uint8_t              dst_mask;

	lima_pp_vec4_reg_e src0_reg = 0;
	unsigned             src0_com = 0;
	bool                 src0_abs = 0;
	bool                 src0_neg = 0;

	unsigned op;

	unsigned i = _lima_pp_asm_parse_field_alu(src,
		lima_pp_combine_asm_op, 16, lima_pp_combine_scalar_op_mov, &op, 0,
		&dst_reg, NULL, &dst_mask, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		&src0_reg, &src0_com, NULL, &src0_abs, &src0_neg, NULL, NULL);
	if (!i) return 0;

	lima_pp_field_combine_t f;
	f.vector.dest_vec  = true;
	f.scalar.arg1_en   = false;
	f.scalar.op        = op;
	f.scalar.arg0_absolute = src0_abs;
	f.scalar.arg0_negate   = src0_neg;
	f.scalar.arg0_src      = (src0_reg << 2) | src0_com;
	f.scalar.arg1_absolute = 0;
	f.scalar.arg1_negate   = 0;
	f.scalar.arg1_src      = 0;
	f.vector.mask      = dst_mask;
	f.vector.dest      = dst_reg;

	if (field)
		*field = f;
	return i;
}

static unsigned _lima_pp_asm_parse_field_combine_sv(
	const char* src, lima_pp_field_combine_t* field)
{
	lima_pp_vec4_reg_e dst_reg = 0;
	unsigned             dst_com = 0;

	lima_pp_vec4_reg_e reg;
	uint8_t              swiz;
	bool                 neg;

	unsigned op;

	lima_pp_asm_op_t combine_atan2_asm_op
		= { "atan2", NULL, 0, 1 };

	unsigned i = _lima_pp_asm_parse_field_alu(src,
		&combine_atan2_asm_op, 1, 1, &op, 0,
		&dst_reg, &dst_com, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		&reg, NULL, &swiz, NULL, &neg, NULL, NULL);
	if (!i) return 0;

	lima_pp_field_combine_t f;
	f.scalar.dest_vec      = false;
	f.vector.arg1_en       = true;
	f.vector.arg1_swizzle  = swiz;
	f.vector.arg1_source   = reg;
	f.scalar.arg0_absolute = false;
	f.scalar.arg0_negate   = neg;
	f.scalar.arg0_src      = 0;
	f.scalar.arg1_absolute = 0;
	f.scalar.arg1_negate   = 0;
	f.scalar.arg1_src      = 0;
	f.scalar.dest         = (dst_reg << 2) | dst_com;

	if (field)
		*field = f;
	return i;
}

static unsigned _lima_pp_asm_parse_field_combine_vv(
	const char* src, lima_pp_field_combine_t* field)
{
	lima_pp_vec4_reg_e dst_reg;
	uint8_t         dst_mask;

	lima_pp_vec4_reg_e src0_reg;
	uint8_t         src0_swiz;
	bool            src0_neg;

	lima_pp_vec4_reg_e src1_reg;
	unsigned        src1_com;
	bool            src1_abs;
	bool            src1_neg;

	unsigned op;

	lima_pp_asm_op_t combine_mul_asm_op
		= { "mul", "*", 1, 1 };

	unsigned i = _lima_pp_asm_parse_field_alu(src,
		&combine_mul_asm_op, 1, 1, &op, 0,
		&dst_reg, NULL, &dst_mask, NULL, NULL, NULL,
		&src0_reg, NULL, &src0_swiz, NULL, &src0_neg,
		&src1_reg, &src1_com, NULL, &src1_abs, &src1_neg, NULL, NULL);
	if (!i) return 0;

	lima_pp_field_combine_t f;
	f.vector.dest_vec      = true;
	f.vector.arg1_en       = true;
	f.vector.arg1_swizzle  = src0_swiz;
	f.vector.arg1_source   = src0_reg;
	f.scalar.arg0_absolute = src1_abs;
	f.scalar.arg0_negate   = (src0_neg ? !src1_neg : src1_neg);
	f.scalar.arg0_src      = (src1_reg << 2) | src1_com;
	f.vector.mask          = dst_mask;
	f.vector.dest          = dst_reg;

	if (field)
		*field = f;
	return i;
}

static unsigned _lima_pp_asm_parse_field_combine(
	const char* src, lima_pp_field_combine_t* field, ogt_link_map_t* map)
{
	(void)map; /* Not used. */

	unsigned i;
	i = _lima_pp_asm_parse_field_combine_ss(src, field);
	if (i) return i;
	i = _lima_pp_asm_parse_field_combine_sv(src, field);
	if (i) return i;
	i = _lima_pp_asm_parse_field_combine_vs(src, field);
	if (i) return i;
	i = _lima_pp_asm_parse_field_combine_vv(src, field);
	return i;
}

static unsigned _lima_pp_asm_parse_field_branch(
	const char* src, lima_pp_field_branch_t* field, ogt_link_map_t* map)
{
	(void)map; /* Not used. */

	unsigned i = 0, j;

	bool cond_lt = true;
	bool cond_eq = true;
	bool cond_gt = true;

	lima_pp_vec4_reg_e src0_reg = 0;
	unsigned        src0_com = 0;
	lima_pp_vec4_reg_e src1_reg = 0;
	unsigned        src1_com = 0;

	if (strncasecmp(&src[i], "if", 2) == 0)
	{
		i += 2;
		i += _lima_pp_asm_parse_space(&src[i]);
		if (src[i++] != '(')
			return 0;
		i += _lima_pp_asm_parse_space(&src[i]);

		j = _lima_pp_asm_parse_zero(&src[i]);
		if (j)
		{
			i += j;
			cond_lt = false;
			cond_eq = false;
			cond_gt = false;
		} else {
			j = _lima_pp_asm_parse_source(&src[i], NULL, true,
				&src0_reg, &src0_com, NULL, NULL, NULL);
			if (!j) return 0;
			i += j;

			i += _lima_pp_asm_parse_space(&src[i]);
			if (strncmp(&src[i], "==", 2) == 0)
			{
				cond_lt = false;
				cond_gt = false;
				i += 2;
			}
			else if ((strncmp(&src[i], "!=", 2) == 0)
				|| (strncmp(&src[i], "<>", 2) == 0))
			{
				cond_eq = false;
				i += 2;
			}
			else if (src[i] == '>')
			{
				cond_lt = false;
				cond_eq = false;
				i++;
			}
			else if (src[i] == '<')
			{
				cond_eq = false;
				cond_gt = false;
				i++;
			}
			else if (strncmp(&src[i], ">=", 2) == 0)
			{
				cond_lt = false;
				i += 2;
			}
			else if (strncmp(&src[i], "<=", 2) == 0)
			{
				cond_gt = false;
				i += 2;
			}
			else
				return 0;
			i += _lima_pp_asm_parse_space(&src[i]);

			j = _lima_pp_asm_parse_source(&src[i], NULL, true,
				&src1_reg, &src1_com, NULL, NULL, NULL);
			if (!j) return 0;
			i += j;
		}

		i += _lima_pp_asm_parse_space(&src[i]);
		if (src[i++] != ')')
			return 0;
		i += _lima_pp_asm_parse_space(&src[i]);
	}

	if ((strncasecmp(&src[i], "jump", 4) != 0)
		&& (strncasecmp(&src[i], "goto", 4) != 0))
		return 0;
	i += 4;


	
	i += _lima_pp_asm_parse_space(&src[i]);
	bool negate  = (src[i] == '-');
	signed max = (1U << 26) - 1;
	if (negate)
	{
		i++;
		i += _lima_pp_asm_parse_space(&src[i]);
		max++;
	}
	else if (src[i] == '+')
	{
		i++;
	}

	if (!isdigit(src[i]))
		return 0;
	signed target;
	for (target = 0; isdigit(src[i]); i++)
	{
		target = (target * 10) + (src[i] - '0');
		if (target > max)
			return 0;
	}
	if (negate)
		target = -target;

	i += _lima_pp_asm_parse_space(&src[i]);
	if ((src[i] != ',')
		&& (src[i] != ';'))
		return 0;

	lima_pp_field_branch_t f =
	{
		.unknown_0   = 0x0,
		.arg1_source = (src1_reg << 2) | src1_com,
		.arg0_source = (src0_reg << 2) | src1_com,
		.cond_gt     = cond_gt,
		.cond_eq     = cond_eq,
		.cond_lt     = cond_lt,
		.unknown_1   = 0x000000,
		.target      = target,
		.unknown_2   = 0x03,
	};

	if (field)
		*field = f;
	return i;
}

static unsigned _lima_pp_asm_parse_field_const0(
	const char* src, lima_pp_vec4_t* field, ogt_link_map_t* map)
{
	(void)map; /* Not used. */

	unsigned i
		= _lima_pp_asm_parse_dest(src, "const0",
			NULL, NULL, NULL, NULL);
	if (!i) return 0;
	i += _lima_pp_asm_parse_space(&src[i]);
	unsigned j
		= _lima_pp_asm_parse_vec4(&src[i], field);
	if (!j) return 0;
	return (i + j);
}

static unsigned _lima_pp_asm_parse_field_const1(
	const char* src, lima_pp_vec4_t* field, ogt_link_map_t* map)
{
	(void)map; /* Not used. */

	unsigned i
		= _lima_pp_asm_parse_dest(src, "const1",
			NULL, NULL, NULL, NULL);
	if (!i) return 0;
	i += _lima_pp_asm_parse_space(&src[i]);
	unsigned j
		= _lima_pp_asm_parse_vec4(&src[i], field);
	if (!j) return 0;
	return (i + j);
}

static unsigned _lima_pp_asm_parse_instruction(
	const char* src, void* data, ogt_link_map_t* map)
{
	if (!src || !data)
		return 0;

	lima_pp_instruction_t inst;
	#ifdef CHECK_ENCODING
	memset(&inst, 0x00, sizeof(lima_pp_instruction_t));
	#endif

	inst.control.stop      = 0;
	inst.control.sync      = 0;
	inst.control.fields    = 0;
	inst.control.sched     = 0;
	inst.control.writeback = 1;
	inst.control.unknown   = 0;

	void* fields[] =
	{
		&inst.varying,
		&inst.sampler,
		&inst.uniform,
		&inst.vec4_mul,
		&inst.float_mul,
		&inst.vec4_acc,
		&inst.float_acc,
		&inst.combine,
		&inst.temp_write,
		&inst.branch,
		&inst.const0,
		&inst.const1,
	};

	void* funcs[] =
	{
		_lima_pp_asm_parse_field_varying,
		_lima_pp_asm_parse_field_sampler,
		_lima_pp_asm_parse_field_uniform,
		_lima_pp_asm_parse_field_vec4_mul,
		_lima_pp_asm_parse_field_float_mul,
		_lima_pp_asm_parse_field_vec4_acc,
		_lima_pp_asm_parse_field_float_acc,
		_lima_pp_asm_parse_field_combine,
		NULL, /* TODO - Parse temp-write field */
		_lima_pp_asm_parse_field_branch,
		_lima_pp_asm_parse_field_const0,
		_lima_pp_asm_parse_field_const1,
	};

	static lima_pp_field_e order [] =
	{
		lima_pp_field_vec4_const_0,
		lima_pp_field_vec4_const_1,
		lima_pp_field_varying,
		lima_pp_field_sampler,
		lima_pp_field_uniform,
		lima_pp_field_vec4_acc,
		lima_pp_field_float_acc,
		lima_pp_field_vec4_mul,
		lima_pp_field_float_mul,
		lima_pp_field_combine,
		lima_pp_field_temp_write,
		lima_pp_field_branch,
	};


	unsigned i = 0, j, f = 0;
	i = _lima_pp_asm_parse_space(src);
	while ((f < lima_pp_field_count)
			&& (src[i] != ';'))
	{
		if (inst.control.fields)
		{
			if (src[i++] != ',')
				return 0;
			i += _lima_pp_asm_parse_space(&src[i]);
		}

		for (j = 0, f = 0; !j && (f < lima_pp_field_count); f++)
		{
			unsigned z = order[f];

			unsigned (*parse)(const char*, void*, ogt_link_map_t*);
			parse = funcs[z];
			j = (parse ? parse(&src[i], fields[z], map) : 0);
			if (j)
			{
				i += j;
				inst.control.fields |= (1 << z);
			}
		}

		if (strncasecmp(&src[i], "stop", 4) == 0)
		{
			i += 4;
			i += _lima_pp_asm_parse_space(&src[i]);
			inst.control.stop = true;
			f = 0;
		}

		if (strncasecmp(&src[i], "sync", 4) == 0)
		{
			i += 4;
			i += _lima_pp_asm_parse_space(&src[i]);
			inst.control.sync = true;
			f = 0;
		}
	}
	if (src[i++] != ';')
		return 0;
	i += _lima_pp_asm_parse_space(&src[i]);

	lima_pp_instruction_encode(&inst, data);

	#ifdef CHECK_ENCODING
	{
		lima_pp_instruction_t check;
		memset(&check, 0x00, sizeof(lima_pp_instruction_t));
		lima_pp_instruction_decode(data, &check);
		if (memcmp(&inst, &check, sizeof(lima_pp_instruction_t)) != 0)
		{
			fprintf(stderr, "Error: Failed to encode instruction.\n");
			printf("\tEncode:\n");
			lima_pp_instruction_print(&inst, true, 2);
			printf("\tDecode:\n");
			lima_pp_instruction_print(&check, true, 2);
			return 0;
		}
	}
	#endif

	return i;
}



static uint32_t* lima_pp__assemble(
	const char* src, unsigned* size,
	essl_program_t* program)
{
	if (!src)
		return NULL;

	unsigned  bsiz = 1024;
	uint32_t* buff = malloc(bsiz << 2);
	if (!buff) return NULL;

	lima_pp_ctrl_t* last = NULL;

	unsigned offset = 0, i = 0, j;
	i += _lima_pp_asm_parse_space(&src[i]);

	if (program)
	{
		do {
			i += _lima_pp_asm_parse_space(&src[i]);
			j = essl_program_parse(
				program, &src[i]);
			i += j;
		} while (j);
	}

	/* TODO - Construct linker map. */
	ogt_link_map_t* map
		= ogt_link_map_import_essl(program);
	/* TODO - Replace program with linker map. */

	while (src[i] != '\0')
	{
		i += _lima_pp_asm_parse_space(&src[i]);

		if ((bsiz - offset) < 32)
		{
			uint32_t* nbuff
				= realloc(buff, (bsiz << 3));
			if (!nbuff)
			{
				free(buff);
				return NULL;
			}
			buff = nbuff;
			bsiz <<= 1;
		}

		j = _lima_pp_asm_parse_instruction(&src[i], &buff[offset], map);
		if (!j) break;
		i += j;
		
		if (last)
		{
			last->sched = lima_pp_instruction_schedule(
				(lima_pp_ctrl_t*)&buff[offset]);
		}
		last = (lima_pp_ctrl_t*)&buff[offset];

		unsigned count
			= ((lima_pp_ctrl_t*)&buff[offset])->count;
		offset += count;
	}
	if (src[i] != '\0')
	{
		free(map);
		free(buff);
		return NULL;
	}

	if (last)
	{
		last->stop = 1;
		last->writeback = 0;
	}

	{
		uint32_t* nbuff
			= realloc(buff, (offset << 2));
		if (nbuff) buff = nbuff;
	}

	free(map);
	if (size)
		*size = (offset * 4);
	return buff;
}



#include <ogt/program.h>
#include <ogt/asm.h>
#include <ogt/arch.h>

ogt_program_t* lima_pp_assemble(
	const char* code, unsigned size,
	ogt_link_map_t* map,
	ogt_asm_type_e type,
	ogt_asm_syntax_e syntax)
{
	/* TODO - Use existing map. */
	if (map) return NULL;

	if ((!code && (size > 0))
		|| (type != ogt_asm_type_vertex)
		|| (syntax != ogt_asm_syntax_verbose))
		return NULL;

	/* TODO - Support multiple syntaxes. */

	ogt_program_t* program
		= ogt_program_create(ogt_arch_lima_pp,
			ogt_asm_type_fragment);;
	if (!program) return NULL;

	if (!program->symbol_table)
	{
		program->symbol_table
			= essl_program_create();
		if (!program->symbol_table)
		{
			ogt_program_delete(program);
			return NULL;
		}
	}

	program->code = lima_pp__assemble(
		code, &program->code_size, program->symbol_table);
	if (!program->code)
	{
		ogt_program_delete(program);
		return NULL;
	}

	return program;
}
