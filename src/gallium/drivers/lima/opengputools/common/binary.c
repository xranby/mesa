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



#include <ogt/binary.h>



bool ogt_binary_export(
	const ogt_binary_format_t* format,
	FILE* stream, ogt_program_t* program)
{
	if (!format
		|| !format->export_cb)
		return false;
	return format->export_cb(stream, program);
}

ogt_program_t* ogt_binary_import(
	const ogt_binary_format_t* format,
	FILE* stream)
{
	if (!format
		|| !format->import_cb)
		return NULL;
	return format->import_cb(stream);
}

void ogt_binary_dump(
	const ogt_binary_format_t* format,
	void* data, uintptr_t size,
	ogt_asm_syntax_e syntax)
{
	if (!format
		|| !format->dump_cb)
		return;
	return format->dump_cb(data, size, syntax);
}
