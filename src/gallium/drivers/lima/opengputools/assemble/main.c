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
#include <ogt/program.h>



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

static const ogt_binary_format_t* format_parse(const char* name)
{
	const ogt_binary_format_t* format_list[] =
	{
		ogt_binary_format_raw,
		ogt_binary_format_mbs,
	};

	unsigned format_list_count
		= sizeof(format_list) / sizeof(ogt_binary_format_t*);

	unsigned i;
	for (i = 0; i < format_list_count; i++)
	{
		if (strcasecmp(name, format_list[i]->name) == 0)
			return format_list[i];
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
	fprintf(fp, "Usage: assemble [options] [-o output] <input>\n");

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

	fprintf(fp, "\t-f, --format <format>.\n");
	fprintf(fp, "\t\traw\n");
	fprintf(fp, "\t\tmbs\n");

	fprintf(fp, "\t-o, --output Output file.\n");	
	fprintf(fp, "\t\tDefaults to a.out otherwise.\n");

	fprintf(fp, "Format:\n");
	fprintf(fp, "\tData is expected as space seperated 32-bit words"
		" or an object file.\n");

	fprintf(fp, "Examples:\n");
	fprintf(fp, "\tassemble -a lima_gp -s verbose -t vertex"
		" -o vert.mbs vert.glsl\n");
}



static char* readfile(const char* path, unsigned* size)
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

	char* data = (char*)malloc(fsize + 1);
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
	data[fsize] = '\0';

	fclose(fp);
	if (size)
		*size = (unsigned)fsize;
	return data;
}



int main(int argc, char** argv)
{
	const ogt_arch_t* arch = NULL;
	const ogt_binary_format_t* format = NULL;
	ogt_asm_syntax_e syntax
		= ogt_asm_syntax_explicit;
	ogt_asm_type_e type = ogt_asm_type_unknown;
	const char* output = NULL;

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
		else if ((strcasecmp(argv[i], "-f") == 0)
			|| (strcasecmp(argv[i], "--format") == 0))
		{
			if (format)
			{
				fprintf(stderr, "Error: Format must only"
					" be specified once.\n");
				usage(stderr);
				return EXIT_FAILURE;
			}

			if (++i >= (unsigned)argc)
			{
				fprintf(stderr, "Error: Must provide an"
					" format after format flag.\n");
				usage(stderr);
				return EXIT_FAILURE;
			}

			format = format_parse(argv[i]);
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
		else if ((strcasecmp(argv[i], "-o") == 0)
			|| (strcasecmp(argv[i], "--output") == 0))
		{
			if (output)
			{
				fprintf(stderr, "Error: Output must only"
					" be specified once.\n");
				usage(stderr);
				return EXIT_FAILURE;
			}

			if (++i >= (unsigned)argc)
			{
				fprintf(stderr, "Error: Must provide a"
					" filename after output flag.\n");
				usage(stderr);
				return EXIT_FAILURE;
			}

			output = argv[i];
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

	unsigned size;
	char*    source = readfile(argv[i], &size);
	if (!source)
	{
		fprintf(stderr, "Error: Failed to open MBS file"
			" '%s'.\n", argv[1]);
		return EXIT_FAILURE;
	}

	ogt_program_t* program = ogt_arch_assemble(
		arch, source, size, NULL, type, syntax);
	free(source);
	if (!program)
	{
		fprintf(stderr, "Error: Failed to assemble source file.\n");
		return EXIT_FAILURE;
	}

	if (!format)
	{
		format = ogt_arch_binary_format(arch);
		if (!format)
			format = ogt_binary_format_raw;
	}

	if (!output) output = "a.out";
	FILE* fp = fopen(output, "wb");
	if (!fp)
	{
		ogt_program_delete(program);
		fprintf(stderr, "Error: Failed to open output for writing.\n");
		return EXIT_FAILURE;
	}
	bool exported
		= ogt_binary_export(format, fp, program);
	ogt_program_delete(program);
	fclose(fp);
	if (!exported)
	{
		fprintf(stderr, "Error: Failed to encode/write object.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
