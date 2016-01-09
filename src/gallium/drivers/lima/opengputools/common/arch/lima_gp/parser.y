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



%{
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "assemble.h"
#include <ogt/essl.h>
#include <ogt/program.h>
#include <ogt/arch.h>
#include <ogt/types.h>
#include <stdbool.h>

ogt_program_t* lima_gp_yyprogram = NULL;

extern int lima_gp_yylex(void);
int lima_gp_yyparse(void);

int lima_gp_yydebug = 1;
 
void lima_gp_yyerror(const char *str)
{
	fprintf(stderr,"lima_gp syntax error\n%s\n", str);
}

%}

%union {
    double      f;
	int64_t     i;
	bool        b;
	const char* s;
	double      v[4];

	essl_storage_qualifier_e sq;
	essl_precision_e         prec;
	essl_type_e              type;
	essl_symbol_t*           symbol;

	lima_gp_fu_e  fu;
	lima_gp_fu_t  unit;
	lima_gp_src_t source;
	lima_gp_op_e  op;
	lima_gp_reg_e reg;
	lima_gp_instruction_partial_t part;
	ogt_program_t* prog;
}

%token <sq> ATTRIBUTE UNIFORM VARYING CONST
%token <prec> PRECISION_SUPER PRECISION_HIGH PRECISION_MEDIUM PRECISION_LOW
%token <type> BOOL FLOAT INT
%token <type> BVEC2 BVEC3 BVEC4 IVEC2 IVEC3 IVEC4 VEC2 VEC3 VEC4
%token <type> MAT2 MAT3 MAT4 SAMPLER2D SAMPLERCUBE
%token INVARIANT PRECISION

%type <sq> storage_qualifier
%type <prec> precision
%type <type> scalar_type vec2_type vec3_type vec4_type type
%type <symbol> declaration


%token LEFT_PAREN RIGHT_PAREN LEFT_BRACKET RIGHT_BRACKET
%token DOT COMMA ASSIGN MINUS COLON SEMICOLON

%token <s> IDENTIFIER
%token <i> CONST_INT
%token <f> CONST_FLOAT
%token <b> CONST_BOOL


%token ACC MUL PASS COMPLEX
%token TEMPORARY REGISTER STORE
%token BRANCH
%token UNUSED IDENT

%token <op> OP_NEG OP_SUB OP_ABS OP_NABS
%token <op> OP_ADD OP_FLOOR OP_SIGN
%token <op> OP_GE OP_LT OP_GT OP_LE
%token <op> OP_MIN OP_MAX
%token <op> OP_COMPLEX1 OP_COMPLEX2 OP_SELECT
%token <op> OP_EXP2 OP_LOG2 OP_RSQRT OP_RCP
%token <op> OP_STA0 OP_STA1 OP_STA2 OP_STA3
%token <op> OP_CLAMP
%token <op> OP_LOAD OP_STORE
%token <reg> REG_X REG_Y REG_Z REG_W REG_OUT REG_ADDR


%type <s> label
%type <i> const_index
%type <f> scalar_const
%type <v> vec2_const vec3_const vec4_const

%type <fu> vtta_alu_fu vtta_fu
%type <unit> vtta_alu_unit vtta_unit
%type <source> vtta_source
%type <op> unary_op binary_op ternary_op quaternary_op store_op
%type <reg> vtta_reg
%type <part> vtta_field vtta_field_list statement
%type <prog> program


%error-verbose

%start complete_program

%%

const_index:
	  LEFT_BRACKET CONST_INT RIGHT_BRACKET
	{ $$ = $2; }
	;

vtta_alu_fu:
		  ACC
	{ $$ = lima_gp_fu_accumulate; }
	| MUL
	{ $$ = lima_gp_fu_multiply; }
	| PASS
	{ $$ = lima_gp_fu_pass; }
	| COMPLEX
	{ $$ = lima_gp_fu_complex; }
	;

vtta_fu:
	  UNIFORM
	{ $$ = lima_gp_fu_uniform; }
	| TEMPORARY
	{ $$ = lima_gp_fu_temporary; }
	| ATTRIBUTE
	{ $$ = lima_gp_fu_attribute; }
	| REGISTER
	{ $$ = lima_gp_fu_register; }
	| STORE
	{ $$ = lima_gp_fu_store; }
	;

vtta_alu_unit:
	  vtta_alu_fu
	{ $$.unit = $1; $$.index = 0; }
	| vtta_alu_fu const_index
	{ $$.unit = $1; $$.index = $2; }
	;

vtta_unit:
	  vtta_alu_unit
	| vtta_fu
	{ $$.unit = $1; $$.index = 0; }
	| vtta_fu const_index
	{ $$.unit = $1; $$.index = $2; }
	;

vtta_reg:
	  REG_X
	| REG_Y
	| REG_Z
	| REG_W
	| REG_OUT
	| REG_ADDR
	;
	
vtta_source:
	  UNUSED
	{
		$$.unit.unit  = 0;
		$$.unit.index = 0;
		$$.reg = lima_gp_reg_unused;
		$$.time = 0;
		$$.neg = false;
	}
	| IDENT
	{
		$$.unit.unit  = 0;
		$$.unit.index = 0;
		$$.reg = lima_gp_reg_ident;
		$$.time = 0;
		$$.neg = false;
	}
	| vtta_unit DOT vtta_reg
	{ $$.unit = $1; $$.reg = $3; $$.time = 0; $$.neg = false; }
	| vtta_unit DOT vtta_reg const_index
	{ $$.unit = $1; $$.reg = $3; $$.time = $4; $$.neg = false; }
	| MINUS vtta_source
	{ $$ = $2; $$.neg = !$$.neg; }
	;

unary_op:
	  PASS { $$ = lima_gp_op_pass; }
	| OP_NEG
	| OP_ABS
	| OP_NABS
	| OP_FLOOR
	| OP_SIGN
	| OP_EXP2
	| OP_LOG2
	| OP_RSQRT
	| OP_RCP
	| OP_STA0
	| OP_STA1
	| OP_STA2
	| OP_STA3
	| OP_CLAMP
	;

binary_op:
	  OP_SUB
	| OP_ADD
	| OP_GE
	| OP_LT
	| OP_GT
	| OP_LE
	| OP_MIN
	| OP_MAX
	| MUL { $$ = lima_gp_op_mul; }
	;

ternary_op:
	  OP_SELECT
	;

quaternary_op:
	  OP_COMPLEX1
	| OP_COMPLEX2
	;

store_op:
	  REGISTER  { $$ = lima_gp_op_store_register; }
	| VARYING   { $$ = lima_gp_op_store_varying; }
	| TEMPORARY { $$ = lima_gp_op_store_temporary; }
	;

vtta_field:
	  vtta_alu_unit DOT unary_op LEFT_PAREN vtta_source RIGHT_PAREN
	{
		lima_gp_op_t op = { $3, false };
		if (!lima_gp_assemble_field(
			$1, op, &$5, NULL, NULL, NULL, &$$))
			lima_gp_yyerror("Failed to assemble unary operation.");
	}
	| MINUS vtta_alu_unit DOT unary_op LEFT_PAREN vtta_source RIGHT_PAREN
	{
		lima_gp_op_t op = { $4, true };
		if (!lima_gp_assemble_field(
			$2, op, &$6, NULL, NULL, NULL, &$$))
			lima_gp_yyerror("Failed to assemble negated unary operation.");
	}
	| vtta_alu_unit DOT binary_op LEFT_PAREN vtta_source COMMA vtta_source RIGHT_PAREN
	{
		lima_gp_op_t op = { $3, false };
		if (!lima_gp_assemble_field(
			$1, op, &$5, &$7, NULL, NULL, &$$))
			lima_gp_yyerror("Failed to assemble binary operation.");
	}
	| MINUS vtta_alu_unit DOT binary_op LEFT_PAREN vtta_source COMMA vtta_source RIGHT_PAREN
	{
		lima_gp_op_t op = { $4, true };
		if (!lima_gp_assemble_field(
			$2, op, &$6, &$8, NULL, NULL, &$$))
			lima_gp_yyerror("Failed to assemble negated binary operation.");
	}
	| vtta_alu_unit DOT ternary_op LEFT_PAREN vtta_source COMMA vtta_source COMMA vtta_source RIGHT_PAREN
	{
		lima_gp_op_t op = { $3, false };
		if (!lima_gp_assemble_field(
			$1, op, &$5, &$7, &$9, NULL, &$$))
			lima_gp_yyerror("Failed to assemble ternary operation.");
	}
	| vtta_alu_unit DOT quaternary_op LEFT_PAREN vtta_source COMMA vtta_source COMMA vtta_source COMMA vtta_source RIGHT_PAREN
	{
		lima_gp_op_t op = { $3, false };
		if (!lima_gp_assemble_field(
			$1, op, &$5, &$7, &$9, &$11, &$$))
			lima_gp_yyerror("Failed to assemble quaternary operation.");
	}
	| BRANCH LEFT_PAREN CONST_INT RIGHT_PAREN
	{
		if (!lima_gp_assemble_field_branch($3, &$$))
			lima_gp_yyerror("Failed to assemble branch.");
	}
	| STORE const_index DOT store_op LEFT_PAREN CONST_INT COMMA vtta_source COMMA vtta_source RIGHT_PAREN
	{
		lima_gp_op_t op = { $4, false };
		if (!lima_gp_assemble_field_store(
			$2, op, $6, &$8, &$10, &$$))
			lima_gp_yyerror("Failed to assemble store.");
	}
	| UNIFORM DOT OP_LOAD LEFT_PAREN CONST_INT RIGHT_PAREN
	{
		if (!lima_gp_assemble_field_load(
			$5, NULL, &$$))
			lima_gp_yyerror("Failed to assemble non-indexed uniform load.");
	}
	| UNIFORM DOT OP_LOAD LEFT_PAREN CONST_INT COMMA vtta_source RIGHT_PAREN
	{ 
		if (!lima_gp_assemble_field_load(
			$5, &$7, &$$))
			lima_gp_yyerror("Failed to assemble indexed uniform load.");
	}
	| TEMPORARY DOT OP_LOAD LEFT_PAREN CONST_INT RIGHT_PAREN
	{
		if (!lima_gp_assemble_field_load(
			$5, NULL, &$$))
			lima_gp_yyerror("Failed to assemble non-indexed temporary load.");
	}
	| TEMPORARY DOT OP_LOAD LEFT_PAREN CONST_INT COMMA vtta_source RIGHT_PAREN
	{ 
		if (!lima_gp_assemble_field_load(
			$5, &$7, &$$))
			lima_gp_yyerror("Failed to assemble indexed temporary load.");
	}
	| ATTRIBUTE DOT OP_LOAD LEFT_PAREN CONST_INT RIGHT_PAREN
	{
		if (!lima_gp_assemble_field_register(
			0, $5, true, &$$))
			lima_gp_yyerror("Failed to assemble attribute load.");
	}
	| REGISTER const_index DOT OP_LOAD LEFT_PAREN CONST_INT RIGHT_PAREN
	{
		if (!lima_gp_assemble_field_register(
			$2, $6, false, &$$))
			lima_gp_yyerror("Failed to assemble register load.");
	}
	;

vtta_field_list:
	  vtta_field
	| vtta_field_list COMMA vtta_field
	{
		if (!lima_gp_instruction_partial_merge(
			&$$, $1, $3))
			lima_gp_yyerror("Fields clash in statement.");
	}
	;

statement:
	  vtta_field_list SEMICOLON
	;

label:
	  IDENTIFIER COLON
	;

storage_qualifier:
	  UNIFORM
	| ATTRIBUTE
	| VARYING
	;

precision:
	  PRECISION_LOW
	| PRECISION_MEDIUM
	| PRECISION_HIGH
	| PRECISION_SUPER
	;

scalar_type:
	  BOOL
	| INT
	| FLOAT
	;

vec2_type:
	  BVEC2
	| IVEC2
	| VEC2
	;

vec3_type:
	  BVEC3
	| IVEC3
	| VEC3
	;

vec4_type:
	  BVEC4
	| IVEC4
	| VEC4
	;

type:
	  scalar_type
	| vec2_type
	| vec3_type
	| vec4_type
	| MAT2
	| MAT3
	| MAT4
	;

scalar_const:
	  CONST_INT
	{ $$ = (double)$1; }
	| CONST_FLOAT
	| CONST_BOOL
	{ $$ = ($1 ? 1.0 : 0.0); }
	;

vec2_const:
	  vec2_type LEFT_PAREN scalar_const COMMA scalar_const RIGHT_PAREN
	{ $$[0] = $3; $$[1] = $5; }
	;

vec3_const:
	  vec3_type LEFT_PAREN scalar_const COMMA scalar_const COMMA scalar_const RIGHT_PAREN
	{ $$[0] = $3; $$[1] = $5; $$[2] = $7; }
	| vec3_type LEFT_PAREN vec2_const COMMA scalar_const RIGHT_PAREN
	{ $$[0] = $3[0]; $$[1] = $3[1]; $$[2] = $5; }
	| vec3_type LEFT_PAREN scalar_const COMMA vec2_const RIGHT_PAREN
	{ $$[0] = $3; $$[1] = $5[0]; $$[2] = $5[1]; }
	;

vec4_const:
	  vec4_type LEFT_PAREN scalar_const COMMA scalar_const COMMA scalar_const COMMA scalar_const RIGHT_PAREN
	{ $$[0] = $3; $$[1] = $5; $$[2] = $7; $$[3] = $9; }
	| vec4_type LEFT_PAREN vec2_const COMMA vec2_const RIGHT_PAREN
	{ $$[0] = $3[0]; $$[1] = $3[1]; $$[2] = $5[0]; $$[3] = $5[1]; }
	| vec4_type LEFT_PAREN vec2_const COMMA scalar_const COMMA scalar_const RIGHT_PAREN
	{ $$[0] = $3[0]; $$[1] = $3[1]; $$[2] = $5; $$[3] = $7; }
	| vec4_type LEFT_PAREN scalar_const COMMA vec2_const COMMA scalar_const RIGHT_PAREN
	{ $$[0] = $3; $$[1] = $5[0]; $$[2] = $5[1]; $$[3] = $7; }
	| vec4_type LEFT_PAREN scalar_const COMMA scalar_const COMMA vec2_const RIGHT_PAREN
	{ $$[0] = $3; $$[1] = $5; $$[2] = $7[0]; $$[3] = $7[1]; }
	| vec4_type LEFT_PAREN vec3_const COMMA scalar_const RIGHT_PAREN
	{ $$[0] = $3[0]; $$[1] = $3[1]; $$[2] = $3[2]; $$[3] = $5; }
	| vec4_type LEFT_PAREN scalar_const COMMA vec3_const RIGHT_PAREN
	{ $$[0] = $3; $$[1] = $5[0]; $$[2] = $5[1]; $$[3] = $5[2]; }
	;

declaration:
	  storage_qualifier precision type IDENTIFIER SEMICOLON
	{ $$ = essl_symbol_create($4, $1, $3, $2, false, 0, NULL); }
	| storage_qualifier type IDENTIFIER SEMICOLON
	{ $$ = essl_symbol_create($3, $1, $2, essl_precision_undefined, false, 0, NULL); }
	| storage_qualifier precision type IDENTIFIER const_index SEMICOLON
	{ $$ = essl_symbol_create($4, $1, $3, $2, false, $5, NULL); }
	| storage_qualifier type IDENTIFIER const_index SEMICOLON
	{ $$ = essl_symbol_create($3, $1, $2, essl_precision_undefined, false, $4, NULL); }
	| CONST precision scalar_type IDENTIFIER ASSIGN scalar_const SEMICOLON
	{
		essl_value_t v;
		v.f = $6;
		$$ = essl_symbol_create(
			$4, essl_storage_qualifier_const, $3, $2, false, 0, &v);
	}
	| CONST precision vec2_type IDENTIFIER ASSIGN vec2_const SEMICOLON
	{
		essl_value_t v[2];
		v[0].f = $6[0];
		v[1].f = $6[1];
		$$ = essl_symbol_create(
			$4, essl_storage_qualifier_const, $3, $2, false, 0, v);
	}
	| CONST precision vec3_type IDENTIFIER ASSIGN vec3_const SEMICOLON
	{
		essl_value_t v[3];
		v[0].f = $6[0];
		v[1].f = $6[1];
		v[2].f = $6[2];
		$$ = essl_symbol_create(
			$4, essl_storage_qualifier_const, $3, $2, false, 0, v);
	}
	| CONST precision vec4_type IDENTIFIER ASSIGN vec4_const SEMICOLON
	{
		essl_value_t v[4];
		v[0].f = $6[0];
		v[1].f = $6[1];
		v[2].f = $6[2];
		v[3].f = $6[3];
		$$ = essl_symbol_create(
			$4, essl_storage_qualifier_const, $3, $2, false, 0, v);
	}
	;

program:
	{ $$ = ogt_program_create(ogt_arch_lima_gp, ogt_asm_type_vertex); }
	| program declaration
	{
		if (!ogt_program_symbol_add($1, $2))
			lima_gp_yyerror("Failed to insert symbol.");
		$$ = $1;
	}
	| program label
	{
		lima_gp_yyerror("Labels not yet supported.");
	}
	| program statement
	{
		if ($2.inst.register0_attribute)
			$1->attrib_prefetch
				= ($1->code_size / sizeof(lima_gp_instruction_t));
		if (!ogt_program_code_add($1, &$2.inst, sizeof(lima_gp_instruction_t)))
			lima_gp_yyerror("Failed to append statement to program.");
	}
	;

complete_program:
	  program
	{ lima_gp_yyprogram = $1; }

%%
