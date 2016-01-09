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



#ifndef __ogt_link_h__
#define __ogt_link_h__

#include <ogt/essl.h>

typedef struct
{
	essl_symbol_t* symbol;
	unsigned       offset;
} ogt_link_t;

typedef struct
{
	unsigned uniform_count;
	unsigned attribute_count;
	unsigned varying_count;
	ogt_link_t* uniform;
	ogt_link_t* attribute;
	ogt_link_t* varying;
} ogt_link_map_t;



extern ogt_link_map_t* ogt_link_map_create();
extern void             ogt_link_map_delete(ogt_link_map_t* map);

extern ogt_link_map_t* ogt_link_map_import_essl(essl_program_t* program);
extern essl_program_t* ogt_link_map_export_essl(ogt_link_map_t* map);


extern bool ogt_link_map_import(
	ogt_link_map_t* map, essl_symbol_t* symbol, unsigned offset);
extern bool ogt_link_map_place(ogt_link_map_t* map, essl_symbol_t* symbol);

extern essl_symbol_t* ogt_link_map_reference(
	ogt_link_map_t* map, essl_storage_qualifier_e sq, unsigned offset,
	unsigned* index, unsigned* component);
extern essl_symbol_t* ogt_link_map_find(
	ogt_link_map_t* map, const char* name, unsigned* offset);

extern unsigned ogt_link_map_uniform_area(ogt_link_map_t* map);

extern void ogt_link_map_print(ogt_link_map_t* map);

#endif
