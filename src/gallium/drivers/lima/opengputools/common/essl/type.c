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

unsigned essl_type_components(essl_type_e type)
{
	switch (type)
	{
		case essl_type_bool:
		case essl_type_int:
		case essl_type_float:
		case essl_type_sampler2D:
		case essl_type_samplerCube:
			return 1;
		case essl_type_bvec2:
		case essl_type_ivec2:
		case essl_type_vec2:
			return 2;
		case essl_type_bvec3:
		case essl_type_ivec3:
		case essl_type_vec3:
			return 3;
		case essl_type_bvec4:
		case essl_type_ivec4:
		case essl_type_vec4:
		case essl_type_mat2:
			return 4;
		case essl_type_mat3:
			return 9;
		case essl_type_mat4:
			return 16;
		default:
			break;
	}
	return 0;
}

bool essl_type_is_scalar(essl_type_e type)
{
	switch (type)
	{
		case essl_type_bool:
		case essl_type_int:
		case essl_type_float:
			return true;
		default:
			break;
	}
	return false;
}

bool essl_type_is_vector(essl_type_e type)
{
	switch (type)
	{
		case essl_type_bvec2:
		case essl_type_ivec2:
		case essl_type_vec2:
		case essl_type_bvec3:
		case essl_type_ivec3:
		case essl_type_vec3:
		case essl_type_bvec4:
		case essl_type_ivec4:
			return true;
		default:
			break;
	}
	return false;
}

bool essl_type_is_matrix(essl_type_e type)
{
	switch (type)
	{
		case essl_type_mat2:
		case essl_type_mat3:
		case essl_type_mat4:
			return true;
		default:
			break;
	}
	return false;
}



static essl_type_e essl_type__member_type(essl_type_e type)
{
	switch (type)
	{
		case essl_type_bvec2:
		case essl_type_bvec3:
		case essl_type_bvec4:
			return essl_type_bool;
		case essl_type_ivec2:
		case essl_type_ivec3:
		case essl_type_ivec4:
			return essl_type_int;
		case essl_type_vec2:
		case essl_type_vec3:
		case essl_type_vec4:
			return essl_type_float;
		default:
			break;
	}
	return essl_type_void;
}

static bool essl_type__member_offset(char c, unsigned* offset)
{
	unsigned o;
	switch (c)
	{
		case 'x':
		case 'r':
		case 's':
			o = 0;
			break;
		case 'y':
		case 'g':
		case 't':
			o = 1;
			break;
		case 'z':
		case 'b':
		case 'p':
			o = 2;
			break;
		case 'w':
		case 'a':
		case 'q':
			o = 3;
			break;
		default:
			return false;
	}

	if (offset) *offset = o;
	return true;
}

bool essl_type_member(
	essl_type_e type, const char* name,
	unsigned* offset, essl_type_e* mtype)
{
	if (essl_type_is_vector(type))
	{
		if (name[1] != '\0')
			return false;
		
		if (mtype) *mtype = essl_type__member_type(type);
		return essl_type__member_offset(name[0], offset);
	}
	else if (essl_type_is_matrix(type))
	{
		unsigned size = 2 + (type - essl_type_mat2);

		if (name[1] == '\0')
		{
			unsigned row;
			if (!essl_type__member_offset(name[0], &row))
				return false;
			if (offset) *offset = (row * size);
			if (mtype)  *mtype  = essl_type_vec2 + (type - essl_type_mat2);
			return true;
		}

		if (name[2] != '\0')
			return false;

		unsigned row, col;
		if (!essl_type__member_offset(name[0], &row)
			|| !essl_type__member_offset(name[1], &col))
			return false;
		
		if ((row >= size)
			|| (col >= size))
			return false;

		if (*offset) *offset = ((row * size) + col);
		if (*mtype)  *mtype  = essl_type_float;
		return true;
	}

	/* TODO - Get member of structures. */

	return false;
}
