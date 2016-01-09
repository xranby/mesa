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



#include <ogt/asm.h>
#include <stdio.h>



void ogt_asm_print_tabs(unsigned tabs)
{
	unsigned i;
	for (i = 0; i < tabs; i++)
		printf("\t");
}

void ogt_asm_print_mask(unsigned mask)
{
	if (mask & 1) printf("x");
	if (mask & 2) printf("y");
	if (mask & 4) printf("z");
	if (mask & 8) printf("w");
}

void ogt_asm_print_swizzle(unsigned swizzle)
{
	printf("%c", "xyzw"[(swizzle >> 0) & 0x3]);
	printf("%c", "xyzw"[(swizzle >> 2) & 0x3]);
	printf("%c", "xyzw"[(swizzle >> 4) & 0x3]);
	printf("%c", "xyzw"[(swizzle >> 6) & 0x3]);
}

void ogt_asm_print_bits(unsigned bits, unsigned count)
{
	unsigned i = count;
	while (i--)
		printf("%u", (bits >> i) & 1);
}
