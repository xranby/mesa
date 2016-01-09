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



#include "error.h"
#include <stdlib.h>



static const char* essl_error__preprocessor_string[] =
{
	"Preprocessor syntax error",
	"#error",
	"#extension if a required extension extension_name is not supported, or if all is specified",
	"High Precision not supported",
	"#version must be the 1st directive/statement in a program",
	"#line has wrong parameters",
	"Language version not supported",
};

static const char* essl_error__lexer_string[] =
{
	"Syntax error",
	"Unidentified identifier.",
	"Use of reserved keywords",
};


const char* essl_error_string(essl_error_e error)
{
	unsigned index = (error & 0x3FF);
	if (index-- == 0)
		return NULL;

	switch (error & ~0x3FF)
	{
		case essl_preprocessor_error:
			if (error >= 7)
				return NULL;
			return essl_error__preprocessor_string[index];
		case essl_lexer_error:
			if (error >= 3)
				return NULL;
			return essl_error__lexer_string[index];
		default:
			break;
	}
	return NULL;
}



#include <stdio.h>

void essl_error_print(essl_error_e error, const char* extra)
{
	switch (error & ~0x3FF)
	{
		case essl_preprocessor_error:
			fprintf(stderr, "P%04u: ", (error & 0x3FF));
			break;
		case essl_lexer_error:
		case essl_linker_error:
			fprintf(stderr, "L%04u: ", (error & 0x3FF));
			break;
		case essl_semantic_error:
			fprintf(stderr, "S%04u: ", (error & 0x3FF));
			break;
		default:
			fprintf(stderr, "U%u: ", error);
			break;
	}

	const char* estr
		= essl_error_string(error);
	if (estr)
		fprintf(stderr, "%s\n", estr);
	else
		fprintf(stderr, "Internal error (%s).\n", extra);
}
