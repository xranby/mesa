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

extern int essl_yylex(void);
int essl_yyparse(void);


 
void essl_yyerror(const char *str)
{
	fprintf(stderr,"L0001: Syntax error\n%s\n",str);
}

%}

%token ATTRIBUTE CONST BOOL FLOAT INT
%token BREAK CONTINUE DO ELSE FOR IF DISCARD RETURN
%token BVEC2 BVEC3 BVEC4 IVEC2 IVEC3 IVEC4 VEC2 VEC3 VEC4
%token MAT2 MAT3 MAT4 IN OUT INOUT UNIFORM VARYING SAMPLER2D SAMPLERCUBE
%token STRUCT VOID WHILE

%token IDENTIFIER TYPE_NAME FLOATCONSTANT INTCONSTANT BOOLCONSTANT
%token FIELD_SELECTION
%token LEFT_OP RIGHT_OP
%token INC_OP DEC_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP XOR_OP MUL_ASSIGN DIV_ASSIGN ADD_ASSIGN
%token MOD_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN XOR_ASSIGN OR_ASSIGN
%token SUB_ASSIGN

%token LEFT_PAREN RIGHT_PAREN LEFT_BRACKET RIGHT_BRACKET LEFT_BRACE RIGHT_BRACE DOT
%token COMMA COLON EQUAL SEMICOLON BANG DASH TILDE PLUS STAR SLASH PERCENT
%token LEFT_ANGLE RIGHT_ANGLE VERTICAL_BAR CARET AMPERSAND QUESTION

%token INVARIANT
%token SUPER_PRECISION HIGH_PRECISION MEDIUM_PRECISION LOW_PRECISION PRECISION

%start translation_unit

%%

variable_identifier:
	IDENTIFIER
	;

primary_expression:
	variable_identifier
	|
	INTCONSTANT
	|
	FLOATCONSTANT
	|
	BOOLCONSTANT
	|
	LEFT_PAREN expression RIGHT_PAREN
	;

postfix_expression:
	primary_expression
	|
	postfix_expression LEFT_BRACKET integer_expression RIGHT_BRACKET
	|
	function_call
	|
	postfix_expression DOT FIELD_SELECTION
	|
	postfix_expression INC_OP
	|
	postfix_expression DEC_OP
	;

integer_expression:
	expression
	;

function_call:
	function_call_generic
	;

function_call_generic:
	function_call_header_with_parameters RIGHT_PAREN
	|
	function_call_header_no_parameters RIGHT_PAREN
	;

function_call_header_no_parameters:
	function_call_header VOID
	|
	function_call_header
	;

function_call_header_with_parameters:
	function_call_header assignment_expression
	|
	function_call_header_with_parameters COMMA assignment_expression
	;

function_call_header:
	function_identifier LEFT_PAREN
	;

function_identifier:
	constructor_identifier
	|
	IDENTIFIER
	;

constructor_identifier:
	FLOAT
	|
	INT
	|
	BOOL
	|
	VEC2
	|
	VEC3
	|
	VEC4
	|
	BVEC2
	|
	BVEC3
	|
	BVEC4
	|
	IVEC2
	|
	IVEC3
	|
	IVEC4
	|
	MAT2
	|
	MAT3
	|
	MAT4
	|
	TYPE_NAME

unary_expression:
	postfix_expression
	|
	INC_OP unary_expression
	|
	DEC_OP unary_expression
	|
	unary_operator unary_expression
	;

unary_operator:
	PLUS
	|
	DASH
	|
	BANG
	|
	TILDE
	;

multiplicative_expression:
	unary_expression
	|
	multiplicative_expression STAR unary_expression
	|
	multiplicative_expression SLASH unary_expression
	|
	multiplicative_expression PERCENT unary_expression
	;

additive_expression:
	multiplicative_expression
	|
	additive_expression PLUS multiplicative_expression
	|
	additive_expression DASH multiplicative_expression
	;

shift_expression:
	additive_expression
	|
	shift_expression LEFT_OP additive_expression
	|
	shift_expression RIGHT_OP additive_expression
	;

relational_expression:
	shift_expression
	|
	relational_expression LEFT_ANGLE shift_expression
	|
	relational_expression RIGHT_ANGLE shift_expression
	|
	relational_expression LE_OP shift_expression
	|
	relational_expression GE_OP shift_expression
	;

equality_expression:
	relational_expression
	|
	equality_expression EQ_OP relational_expression
	|
	equality_expression NE_OP relational_expression
	;

and_expression:
	equality_expression
	|
	and_expression AMPERSAND equality_expression
	;

exclusive_or_expression:
	and_expression
	|
	exclusive_or_expression CARET and_expression
	;

inclusive_or_expression:
	exclusive_or_expression
	|
	inclusive_or_expression VERTICAL_BAR exclusive_or_expression
	;

logical_and_expression:
	inclusive_or_expression
	|
	logical_and_expression AND_OP inclusive_or_expression
	;

logical_xor_expression:
	logical_and_expression
	|
	logical_xor_expression XOR_OP logical_and_expression
	;

logical_or_expression:
	logical_xor_expression
	|
	logical_or_expression OR_OP logical_xor_expression
	;

conditional_expression:
	logical_or_expression
	|
	logical_or_expression QUESTION expression COLON assignment_expression
	;

assignment_expression:
	conditional_expression
	|
	unary_expression assignment_operator assignment_expression
	;

assignment_operator:
	EQUAL
	|
	MUL_ASSIGN
	|
	DIV_ASSIGN
	|
	MOD_ASSIGN
	|
	ADD_ASSIGN
	|
	SUB_ASSIGN
	|
	LEFT_ASSIGN
	|
	RIGHT_ASSIGN
	|
	AND_ASSIGN
	|
	XOR_ASSIGN
	|
	OR_ASSIGN
	;

expression:
	assignment_expression
	|
	expression COMMA assignment_expression
	;

constant_expression:
	conditional_expression;

declaration:
	function_prototype SEMICOLON
	|
	init_declarator_list SEMICOLON
	|
	PRECISION precision_qualifier type_specifier_no_prec SEMICOLON
	;

function_prototype:
	function_declarator RIGHT_PAREN
	;

function_declarator:
	function_header
	|
	function_header_with_parameters
	;

function_header_with_parameters:
	function_header parameter_declaration
	|
	function_header_with_parameters COMMA parameter_declaration
	;

function_header:
	fully_specified_type IDENTIFIER LEFT_PAREN
	;

parameter_declarator:
	type_specifier IDENTIFIER
	|
	type_specifier IDENTIFIER LEFT_BRACKET constant_expression RIGHT_BRACKET
	;

parameter_declaration:
	type_qualifier parameter_qualifier parameter_declarator
	|
	parameter_qualifier parameter_declarator
	|
	type_qualifier parameter_qualifier parameter_type_specifier
	|
	parameter_qualifier parameter_type_specifier
	|
	type_qualifier parameter_declarator
	|
	parameter_declarator
	|
	type_qualifier parameter_type_specifier
	|
	parameter_type_specifier
	;

parameter_qualifier:
	IN
	|
	OUT
	|
	INOUT
	;

parameter_type_specifier:
	type_specifier
	|
	type_specifier LEFT_BRACKET constant_expression RIGHT_BRACKET
	;

init_declarator_list:
	single_declaration
	|
	init_declarator_list COMMA IDENTIFIER
	|
	init_declarator_list COMMA IDENTIFIER LEFT_BRACKET constant_expression RIGHT_BRACKET
	|
	init_declarator_list COMMA IDENTIFIER EQUAL initializer
	;

single_declaration:
	fully_specified_type
	|
	fully_specified_type IDENTIFIER
	|
	fully_specified_type IDENTIFIER LEFT_BRACKET constant_expression RIGHT_BRACKET
	|
	fully_specified_type IDENTIFIER EQUAL initializer
	|
	INVARIANT IDENTIFIER
	;

fully_specified_type:
	type_specifier
	|
	type_qualifier type_specifier
	;

type_qualifier:
	CONST
	|
	ATTRIBUTE
	|
	VARYING
	|
	INVARIANT VARYING
	|
	UNIFORM
	;

type_specifier:
	type_specifier_no_prec
	|
	precision_qualifier type_specifier_no_prec
	;

type_specifier_no_prec:
	VOID
	|
	FLOAT
	|
	INT
	|
	BOOL
	|
	VEC2
	|
	VEC3
	|
	VEC4
	|
	BVEC2
	|
	BVEC3
	|
	BVEC4
	|
	IVEC2
	|
	IVEC3
	|
	IVEC4
	|
	MAT2
	|
	MAT3
	|
	MAT4
	|
	SAMPLER2D
	|
	SAMPLERCUBE
	|
	struct_specifier
	|
	TYPE_NAME
	;

precision_qualifier:
	SUPER_PRECISION
	|
	HIGH_PRECISION
	|
	MEDIUM_PRECISION
	|
	LOW_PRECISION
	;

struct_specifier:
	STRUCT IDENTIFIER LEFT_BRACE struct_declaration_list RIGHT_BRACE
	|
	STRUCT LEFT_BRACE struct_declaration_list RIGHT_BRACE
	;

struct_declaration_list:
	struct_declaration
	|
	struct_declaration_list struct_declaration
	;

struct_declaration:
	type_specifier struct_declarator_list SEMICOLON
	;

struct_declarator_list:
	struct_declarator
	|
	struct_declarator_list COMMA struct_declarator
	;

struct_declarator:
	IDENTIFIER
	|
	IDENTIFIER LEFT_BRACKET constant_expression RIGHT_BRACKET
	;

initializer:
	assignment_expression
	;

declaration_statement:
	declaration
	;

statement_no_new_scope:
	compound_statement_with_scope
	|
	simple_statement
	;

simple_statement:
	declaration_statement
	|
	expression_statement
	|
	selection_statement
	|
	iteration_statement
	|
	jump_statement
	;

compound_statement_with_scope:
	LEFT_BRACE RIGHT_BRACE
	|
	LEFT_BRACE statement_list RIGHT_BRACE
	;

statement_with_scope:
	compound_statement_no_new_scope
	|
	simple_statement
	;

compound_statement_no_new_scope:
	LEFT_BRACE RIGHT_BRACE
	|
	LEFT_BRACE statement_list RIGHT_BRACE
	;

statement_list:
	statement_no_new_scope
	|
	statement_list statement_no_new_scope
	;

expression_statement:
	SEMICOLON
	|
	expression SEMICOLON
	;

selection_statement:
	IF LEFT_PAREN expression RIGHT_PAREN selection_rest_statement
	;

selection_rest_statement:
	statement_with_scope ELSE statement_with_scope
	|
	statement_with_scope
	;

condition:
	expression
	|
	fully_specified_type IDENTIFIER EQUAL initializer
	;

iteration_statement:
	WHILE LEFT_PAREN condition RIGHT_PAREN statement_no_new_scope
	|
	DO statement_with_scope WHILE LEFT_PAREN expression RIGHT_PAREN SEMICOLON
	|
	FOR LEFT_PAREN for_init_statement for_rest_statement RIGHT_PAREN statement_no_new_scope
	;

for_init_statement:
	expression_statement
	|
	declaration_statement
	;

for_rest_statement:
	condition SEMICOLON
	|
	condition SEMICOLON expression
	|
	SEMICOLON
	|
	SEMICOLON expression
	;

jump_statement:
	CONTINUE SEMICOLON
	|
	BREAK SEMICOLON
	|
	RETURN SEMICOLON
	|
	RETURN expression SEMICOLON
	|
	DISCARD SEMICOLON
	;

translation_unit:
	external_declaration
	|
	translation_unit external_declaration
	;

external_declaration:
	function_definition
	|
	declaration
	;

function_definition:
	function_prototype compound_statement_no_new_scope
	;

