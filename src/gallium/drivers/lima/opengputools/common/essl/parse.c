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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



static unsigned _essl_parse_space(const char* src)
{
	if (!src)
		return 0;

	unsigned i = 0;
	while (isspace(src[i])
		|| (strncmp(&src[i], "//", 2) == 0)
		|| (strncmp(&src[i], "/*", 2) == 0))
	{
		if (isspace(src[i]))
		{
			for (i = 0; isspace(src[i]); i++);
		}
		else if (src[i + 1] == '/')
		{
			for (i += 2; src[i] != '\n'; i++);
		}
		else
		{
			for (i += 2; src[i] != '\0'; i++)
			{
				if (strncmp(&src[i], "*/", 2) == 0)
				{
					i += 2;
					break;
				}
			}
		}
	}

	return i;
}

static unsigned _essl_parse_ident(const char* src)
{
	if (!src)
		return 0;
	if (!isalpha(src[0])
		&& (src[0] != '_'))
		return 0;

	unsigned i;
	for (i = 0; isalnum(src[i])
		|| (src[i] == '_'); i++);
	return i;
}

static unsigned _essl_parse_index(const char* src, unsigned* index)
{
	if (!src
		|| !isdigit(src[0]))
		return 0;
	
	unsigned i, v;
	for (i = 0, v = 0; isdigit(src[i]); i++)
	{
		if (v > (((1ULL << (sizeof(unsigned) << 3)) - 1) / 10))
			return 0;
		v *= 10;
		v += (src[i] - '0');
	}

	if (index)
		*index = v;
	return i;
}

static unsigned _essl_parse_strlist(const char* src,
	const char* list[], unsigned count,
	unsigned* value)
{
	if (!src)
		return 0;

	unsigned i;
	for (i = count; i--; )
	{
		const char* n = list[i];
		if (!n || (n[0] == '\0'))
			continue;

		unsigned l = strlen(n);
		if (strncasecmp(src, n, l) == 0)
		{
			if (value)
				*value = i;
			return l;
		}
	}

	return 0;
}



unsigned essl_parse_type(const char* src,
	essl_type_e* type)
{
	if (!src)
		return 0;

	unsigned i
		= _essl_parse_strlist(src,
			essl_type_name, essl_type_count,
			type);
	if (i) return i;

	/* TODO - Parse structures. */

	return 0;
}

unsigned essl_parse_storage_qualifier(const char* src,
	essl_storage_qualifier_e* storage_qualifier)
{
	return _essl_parse_strlist(src,
		essl_storage_qualifier_name,
		essl_storage_qualifier_count,
		storage_qualifier);
}

unsigned essl_parse_parameter_qualifier(const char* src,
	essl_parameter_qualifier_e* parameter_qualifier)
{
	return _essl_parse_strlist(src,
		essl_parameter_qualifier_name,
		essl_parameter_qualifier_count,
		parameter_qualifier);
}

unsigned essl_parse_precision(const char* src,
	essl_precision_e* precision)
{
	return _essl_parse_strlist(src,
		essl_precision_name,
		essl_precision_count,
		precision);
}



static unsigned _essl_parse_default_precision_type(const char* src,
	essl_type_e type, essl_precision_e* precision)
{
	if (!src)
		return 0;

	unsigned i = 0, j;	
	if (strncasecmp(&src[i], "precision", 9) != 0)
		return 0;
	i += 9;
	j = _essl_parse_space(&src[i]);
	if (!j) return 0; i += j;

	essl_precision_e p;
	j = essl_parse_precision(&src[i], &p);
	if (!j) return 0; i += j;
	j = _essl_parse_space(&src[i]);
	if (!j) return 0; i += j;

	essl_type_e t;
	j = essl_parse_type(&src[i], &t);
	if (t != type) return 0;
	if (!j) return 0; i += j;
	
	i += _essl_parse_space(&src[i]);
	if (src[i++] != ';')
		return 0;

	if (precision)
		*precision = p;
	return i;
}

unsigned essl_parse_default_precision_float(const char* src,
	essl_precision_e* precision)
{
	return _essl_parse_default_precision_type(src,
		essl_type_float, precision);
}
unsigned essl_parse_default_precision_int(const char* src,
	essl_precision_e* precision)
{
	return _essl_parse_default_precision_type(src,
		essl_type_int, precision);
}



static unsigned _essl_parse_symbol(const char* src,
	essl_precision_e floatp,
	essl_precision_e intp,
	essl_symbol_t** symbol,
	essl_parameter_qualifier_e* parameter_qualifier)
{
	if (!src)
		return 0;

	unsigned i = 0, j;

	bool invariant = false;
	if (strncasecmp(&src[i], "invariant", 9) == 0)
	{
		invariant = true;
		i += 9;
		j = _essl_parse_space(&src[i]);
		if (!j) return 0; i += j;
	}

	essl_storage_qualifier_e storage_qualifier;
	j = essl_parse_storage_qualifier(
		src, &storage_qualifier);
	if (!j) return 0; i += j;
	j = _essl_parse_space(&src[i]);
	if (!j) return 0; i += j;

	if (parameter_qualifier)
	{
		j = essl_parse_parameter_qualifier(
			&src[i], parameter_qualifier);
		if (!j)
			*parameter_qualifier= essl_parameter_qualifier_none;
		i += j;
	}

	essl_precision_e precision;
	j = essl_parse_precision(
		src, &precision);
	if (!j)
	{
		precision = essl_precision_undefined;
	} else {
		i += j;
		j = _essl_parse_space(&src[i]);
		if (!j) return 0; i += j;
	}

	essl_type_e type;
	j = essl_parse_type(
		&src[i], &type);
	if (!j) return 0; i += j;
	if (type == essl_type_void)
		return 0;
	j = _essl_parse_space(&src[i]);
	if (!j) return 0; i += j;

	if (precision == essl_precision_undefined)
	{
		switch (type)
		{
			case essl_type_void:
			case essl_type_sampler2D:
			case essl_type_samplerCube:
			case essl_type_struct:
				break;
			case essl_type_bool:
			case essl_type_bvec2:
			case essl_type_bvec3:
			case essl_type_bvec4:
				precision = essl_precision_low;
				break;
			case essl_type_float:
			case essl_type_vec2:
			case essl_type_vec3:
			case essl_type_vec4:
			case essl_type_mat2:
			case essl_type_mat3:
			case essl_type_mat4:
				precision = floatp;
				break;
			case essl_type_int:
			case essl_type_ivec2:
			case essl_type_ivec3:
			case essl_type_ivec4:
				precision = intp;
				break;
			default:
				return 0;
		}
		switch (type)
		{
			case essl_type_void:
			case essl_type_sampler2D:
			case essl_type_samplerCube:
			case essl_type_struct:
				break;
			default:
				if (precision
					== essl_precision_undefined)
				return 0;
		}
	}

	const char* ident = &src[i];
	unsigned    ident_len = _essl_parse_ident(&src[i]);
	if (!ident_len) return 0;
	i += ident_len;
	i += _essl_parse_space(&src[i]);

	unsigned count = 0;
	bool     array = (src[i] == '[');
	if (array)
	{
		i++;
		i += _essl_parse_space(&src[i]);

		i += _essl_parse_index(&src[i], &count);

		i += _essl_parse_space(&src[i]);
		if (src[i++] != ']')
			return 0;
		i += _essl_parse_space(&src[i]);
	}

	/* TODO - Parse initialization. */

	if (src[i++] != ';')
		return 0;

	if (symbol)
	{
		if (array
			&& (count == 0))
		{
			*symbol = NULL;
			return i;
		}

		essl_symbol_t* sym
			= (essl_symbol_t*)malloc(
				sizeof(essl_symbol_t)
				/* + (count * sizeof(essl_value_t)) */
				+ (ident_len + 1));
		if (!sym)
			return 0;

		sym->name = (const char*)((uintptr_t)sym
			+ sizeof(essl_symbol_t));
		memcpy((void*)sym->name, ident, ident_len);
		((char*)sym->name)[ident_len] = '\0';

		sym->type              = type;
		sym->structure         = NULL;
		sym->storage_qualifier = storage_qualifier;
		sym->precision         = precision;
		sym->invariant         = invariant;
		sym->count             = count;
		sym->value             = NULL;

		*symbol = sym;
	}

	return i;
}

unsigned essl_parse_symbol(const char* src,
	essl_precision_e floatp,
	essl_precision_e intp,
	essl_symbol_t** symbol)
{
	return _essl_parse_symbol(src,
		floatp, intp, symbol,
		NULL);
}
