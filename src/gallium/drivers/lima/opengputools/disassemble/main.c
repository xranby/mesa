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



#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

#include <ogt/arch.h>
#include <ogt/asm.h>
#include <ogt/binary.h>



static const ogt_arch_t* arch_parse(const char* name)
{
	const ogt_arch_t* arch_list[] =
	{
		ogt_arch_lima_gp,
		ogt_arch_lima_pp,
		ogt_arch_freedreno,
	};

	unsigned arch_list_count
		= sizeof(arch_list) / sizeof(ogt_arch_t*);

	unsigned i;
	for (i = 0; i < arch_list_count; i++)
	{
		if (strcasecmp(name, arch_list[i]->name) == 0)
			return arch_list[i];
	}

	return NULL;
}

static bool syntax_parse(const char* name, ogt_asm_syntax_e* syntax)
{
	if (!name) return false;

	const char* syntax_list[] =
	{
		"raw", "fields",
		"explicit", "verbose",
		"decompile"
	};

	unsigned syntax_list_count
		= sizeof(syntax_list) / sizeof(const char*);

	unsigned i;
	for (i = 0; i < syntax_list_count; i++)
	{
		if (strcasecmp(name, syntax_list[i]) == 0)
		{
			if (syntax) *syntax = i;
			return true;
		}
	}

	return false;
}

static bool type_parse(const char* name, ogt_asm_type_e* type)
{
	if (!name) return false;

	const char* type_list[] =
	{
		NULL, "geometry", "vertex", "fragment",
		NULL, NULL      , NULL    , "pixel"   ,
	};

	unsigned type_list_count
		= sizeof(type_list) / sizeof(const char*);

	unsigned i;
	for (i = 0; i < type_list_count; i++)
	{
		if (!type_list[i]) continue;
		if (strcasecmp(name, type_list[i]) == 0)
		{
			if (type) *type = (i & 3);
			return true;
		}
	}

	return false;
}



static void usage(FILE* fp)
{
	fprintf(fp, "Usage: disassemble [options] code...\n");

	fprintf(fp, "Options:\n");

	fprintf(fp, "\t-a, --arch <architecture>\n");
	fprintf(fp, "\t\tlima_pp\n");
	fprintf(fp, "\t\tlima_gp\n");
	fprintf(fp, "\t\tfreedreno\n");

	fprintf(fp, "\t-s, --syntax <syntax>\n");
	fprintf(fp, "\t\traw\n");
	fprintf(fp, "\t\tfields\n");
	fprintf(fp, "\t\texplicit\n");
	fprintf(fp, "\t\tverbose\n");
	fprintf(fp, "\t\tdecompile\n");

	fprintf(fp, "\t-t, --type <shader type>.\n");
	fprintf(fp, "\t\tgeometry\n");
	fprintf(fp, "\t\tvertex\n");
	fprintf(fp, "\t\tfragment\n");

	fprintf(fp, "Format:\n");
	fprintf(fp, "\tData is expected as space seperated 32-bit words"
		" or an object file.\n");

	fprintf(fp, "Examples:\n");
	fprintf(fp, "\tdisassemble -a lima_gp -s verbose -t vertex 0xb08b06c0"
		" 0x438002c3 0x40010d00 0x00001c08\n");
	fprintf(fp, "\tdisassemble -a lima_gp -s verbose -t vertex vert.mbs\n");
}



static bool addword(
	uint32_t word,
	uint32_t** code, unsigned* size)
{
	if (!code || !size)
		return false;

	uint32_t* ncode
		= (uint32_t*)realloc(*code,
			(*size + 1) * sizeof(uint32_t));
	if (!ncode) return false;
	*code = ncode;
	(*code)[(*size)++] = word;
	return true;
}

static unsigned readword(
	const char* input,
	uint32_t** code, unsigned* size)
{
	if (!input)
		return 0;

	unsigned i = 0;

	uint32_t w;
	if (strncasecmp((void*)&input[i], "0x", 2) == 0)
	{
		i += 2;
		int n;
		if (sscanf((void*)&input[i],
			"%"SCNx32"%n", &w, &n) <= 0)
			return false;
		i += n;
	}
	else if(isdigit(input[i]))
	{
		int n;
		if (sscanf(&input[i],
			"%"SCNu32"%n", &w, &n) <= 0)
			return false;
		i += n;
	}
	else
	{
		return false;
	}
	if (!addword(w, code, size))
		return false;

	if ((input[i] == ',')
		&& (input[++i] != '\0'))
			return readword(&input[i], code, size);

	return true;
}

static void* readfile(const char* path, unsigned* size)
{
	FILE* fp = fopen(path, "rb");
	if (!fp) return NULL;

	if (fseek(fp, 0, SEEK_END) != 0)
	{
		fclose(fp);
		return NULL;
	}
	long fsize = ftell(fp);
	if ((fsize <= 0)
		|| (fseek(fp, 0, SEEK_SET) != 0))
	{
		fclose(fp);
		return NULL;
	}

	void* data = malloc(fsize);
	if (!data)
	{
		fclose(fp);
		return NULL;
	}
	
	if (fread(data, fsize, 1, fp) != 1)
	{
		fclose(fp);
		free(data);
		return NULL;
	}

	fclose(fp);
	if (size)
		*size = (unsigned)fsize;
	return data;
}



int main(int argc, char** argv)
{
	const ogt_arch_t* arch = NULL;
	ogt_asm_syntax_e syntax
		= ogt_asm_syntax_explicit;
	ogt_asm_type_e type = ogt_asm_type_unknown;

	unsigned i;
	for (i = 1; (i < (unsigned)argc) && (argv[i][0] == '-'); i++)
	{
		if ((strcasecmp(argv[i], "-a") == 0)
			|| (strcasecmp(argv[i], "--arch") == 0))
		{
			if (arch)
			{
				fprintf(stderr, "Error: Architecture must only"
					" be specified once.\n");
				usage(stderr);
				return EXIT_FAILURE;
			}

			if (++i >= (unsigned)argc)
			{
				fprintf(stderr, "Error: Must provide an"
					" architecture after architecture flag.\n");
				usage(stderr);
				return EXIT_FAILURE;
			}

			arch = arch_parse(argv[i]);
			if (!arch)
			{
				fprintf(stderr, "Error: Unrecognized architecture name"
					" '%s'.\n", argv[i]);
				usage(stderr);
				return EXIT_FAILURE;
			}
		}
		else if ((strcasecmp(argv[i], "-s") == 0)
			|| (strcasecmp(argv[i], "--syntax") == 0))
		{
			if (++i >= (unsigned)argc)
			{
				fprintf(stderr, "Error: Must provide a"
					" syntax after syntax flag.\n");
				usage(stderr);
				return EXIT_FAILURE;
			}

			if (!syntax_parse(argv[i], &syntax))
			{
				fprintf(stderr, "Error: Unrecognized syntax name"
					" '%s'.\n", argv[i]);
				usage(stderr);
				return EXIT_FAILURE;
			}
		}
		else if ((strcasecmp(argv[i], "-t") == 0)
			|| (strcasecmp(argv[i], "--type") == 0))
		{
			if (++i >= (unsigned)argc)
			{
				fprintf(stderr, "Error: Must provide a"
					" type after type flag.\n");
				usage(stderr);
				return EXIT_FAILURE;
			}

			if (!type_parse(argv[i], &type))
			{
				fprintf(stderr, "Error: Unrecognized type name"
					" '%s'.\n", argv[i]);
				usage(stderr);
				return EXIT_FAILURE;
			}
		}
		else
		{
			fprintf(stderr, "Error: Invalid flag '%s'.\n", argv[i]);
			usage(stderr);
			return EXIT_FAILURE;
		}
	}

	if (i >= (unsigned)argc)
	{
		fprintf(stderr,
			"Error: No code/data provided.\n");
		usage(stderr);
		return EXIT_FAILURE;
	}

	uint32_t* code = NULL;
	unsigned  size = 0;
	if (readword(argv[i], &code, &size))
	{
		for (i++; i < (unsigned)argc; i++)
		{
			if (!readword(
				argv[i], &code, &size))
			{
				fprintf(stderr,
				"Error: Invalid data word in argument %u.\n", i);
				usage(stderr);
				return EXIT_FAILURE;
			}
		}

		if (!ogt_arch_disassemble(
			arch, code, (size << 2), NULL, type, syntax, 0))
		{
			fprintf(stderr, "Error: Failed to disassemble shader.\n");
			return EXIT_FAILURE;
		}
	}
	else
	{
		/* TODO - Properly detect/select object format. */
		ogt_binary_format_t* format
			= ogt_binary_format_mbs;

		/* Dump object file(s) instead. */
		for (; i < (unsigned)argc; i++)
		{
			unsigned size;
			void* mbs = readfile(argv[i], &size);
			if (!mbs)
			{
				fprintf(stderr, "Error: Failed to open MBS file"
					" '%s'.\n", argv[1]);
				return EXIT_FAILURE;
			}
			ogt_binary_dump(format, (void*)mbs, size, syntax);
			free(mbs);
		}
	}

	return EXIT_SUCCESS;
}
