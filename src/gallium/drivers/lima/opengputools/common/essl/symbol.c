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
#include <stdint.h>
#include <string.h>


essl_symbol_t* essl_symbol_create(
	const char* name,
	essl_storage_qualifier_e storage_qualifier,
	essl_type_e type, essl_precision_e precision,
	bool invariant, unsigned count, essl_value_t* values)
{
	if (!name) return NULL;
	if ((storage_qualifier == essl_storage_qualifier_const)
		&& !values)
		return NULL;

	unsigned nlen  = strlen(name) + 1;
	unsigned vsize = (sizeof(essl_value_t*)
		* essl_type_components(type)
		* (count ? count : 1));

	essl_symbol_t* symbol
		= (essl_symbol_t*)malloc(sizeof(essl_symbol_t) + nlen + vsize);
	if (!symbol) return NULL;

	symbol->ref = 0;

	symbol->name  = (const char*)((uintptr_t)symbol + sizeof(essl_symbol_t));
	memcpy((void*)symbol->name, name, nlen);
	symbol->type      = type;
	symbol->structure = NULL;
	symbol->storage_qualifier = storage_qualifier;
	symbol->precision = precision;
	symbol->invariant = invariant;
	symbol->count     = count;

	symbol->value = NULL;
	if (values)
	{
		symbol->value = (essl_value_t*)((uintptr_t)symbol->name + nlen);
		memcpy(symbol->value, values, vsize);
	}

	return symbol;
}

void essl_symbol_delete(essl_symbol_t* symbol)
{
	if (!symbol) return;
	if (symbol->ref)
		symbol->ref--;
	else
		free(symbol);
}

essl_symbol_t* essl_symbol_reference(essl_symbol_t* symbol)
{
	if (!symbol) return NULL;
	symbol->ref++;
	return symbol;
}

essl_symbol_t* essl_symbol_copy(const essl_symbol_t* symbol)
{
	if (!symbol)
		return NULL;
	unsigned nlen = strlen(symbol->name) + 1;
	essl_symbol_t* copy
		= (essl_symbol_t*)malloc(
			sizeof(essl_symbol_t) + nlen);
	if (!copy) return NULL;
	memcpy(copy, symbol, sizeof(essl_symbol_t));

	copy->ref = 0;
	copy->name
		= (const char*)((uintptr_t)copy
			+ sizeof(essl_symbol_t));
	memcpy((char*)copy->name, symbol->name, nlen);
	return copy;
}

unsigned essl_symbol_component_count(essl_symbol_t* symbol)
{
	if (!symbol)
		return 0;
	return essl_type_components(symbol->type);
}

const char* essl_symbol_component_name(essl_symbol_t* symbol, unsigned component)
{
	unsigned m = essl_symbol_component_count(symbol);
	if (m == 0) return NULL;
	component %= m;

	switch (symbol->type)
	{
		case essl_type_vec2:
		case essl_type_vec3:
		case essl_type_vec4:
		case essl_type_bvec2:
		case essl_type_bvec3:
		case essl_type_bvec4:
		case essl_type_ivec2:
		case essl_type_ivec3:
		case essl_type_ivec4:
		{
			static const char* c[]
				= { "x", "y", "z", "w" };
			return c[component];
		} break;
		case essl_type_mat2:
		{
			static const char* c[] =
			{
				"xx", "xy",
				"yx", "yy",
			};
			return c[component];
		} break;
		case essl_type_mat3:
		{
			static const char* c[] =
			{
				"xx", "xy", "xz",
				"yx", "yy", "yz",
				"zx", "zy", "zz",
			};
			return c[component];
		} break;
		case essl_type_mat4:
		{
			static const char* c[] =
			{
				"xx", "xy", "xz", "xw",
				"yx", "yy", "yz", "yw",
				"zx", "zy", "zz", "zw",
				"wx", "wy", "wz", "ww",
			};
			return c[component];
		} break;
		default:
			break;
	}

	return NULL;
}
