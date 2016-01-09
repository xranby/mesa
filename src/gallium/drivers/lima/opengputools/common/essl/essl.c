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



const char* essl_type_name[] =
{
	"void",
	"bool",
	"int",
	"float",
	"vec2", "vec3", "vec4",
	"bvec2", "bvec3", "bvec4",
	"ivec2", "ivec3", "ivec4",
	"mat2", "mat3", "mat4",
	"sampler2D", "samplerCube",
	NULL
};

const char* essl_storage_qualifier_name[] =
{
	"",
	"const",
	"attribute",
	"uniform",
	"varying",
};

const char* essl_parameter_qualifier_name[] =
{
	"",
	"in",
	"out",
	"inout",
};

const char* essl_precision_name[] =
{
	"lowp",
	"mediump",
	"highp",
	"superp",
	NULL,
};
