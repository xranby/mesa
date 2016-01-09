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



#ifndef __ogt_tgsi_h__
#define __ogt_tgsi_h__

#include <stdint.h>
#include <stdbool.h>



typedef struct
__attribute__((__packed__))
{
	unsigned major   :  8;
	unsigned minor   :  8;
	unsigned padding : 16;
} tgsi_version_t;

static const tgsi_version_t
	tgsi_version_default =
{
	.major   = 1,
	.minor   = 1,
	.padding = 0
};

typedef struct
__attribute__((__packed__))
{
	unsigned header_size :  8;
	unsigned body_size   : 24;
} tgsi_header_t;

static const tgsi_header_t
	tgsi_header_default =
{
	.header_size = 2,
	.body_size   = 0
};

typedef enum
{
	tgsi_processor_fragment = 0,
	tgsi_processor_vertex   = 1,
	tgsi_processor_geometry = 2,
} tgsi_processor_e;

typedef struct
__attribute__((__packed__))
{
	tgsi_processor_e processor :  4;
	unsigned         padding   : 28;
} tgsi_processor_t;

static const tgsi_processor_t
	tgsi_processor_default =
{
	.processor = tgsi_processor_fragment,
	.padding   = 0
};



typedef enum
{
	tgsi_token_type_declaration = 0,
	tgsi_token_type_immediate   = 1,
	tgsi_token_type_instruction = 2,
	tgsi_token_type_instruction_ext_nv        = 0,
	tgsi_token_type_instruction_ext_label     = 1,
	tgsi_token_type_instruction_ext_texture   = 2,
	tgsi_token_type_instruction_ext_predicate = 3,
	tgsi_token_type_src_register_ext_swz = 0,
	tgsi_token_type_src_register_ext_mod = 1,
	tgsi_token_type_dst_register_ext_concode   = 0,
	tgsi_token_type_dst_register_ext_modulate  = 1,
	tgsi_token_type_dst_register_ext_predicate = 2,
} tgsi_token_type_e;

typedef struct
__attribute__((__packed__))
{
	tgsi_token_type_e type     :  4;
	unsigned          size     :  8;
	unsigned          data     : 19;
	bool              extended :  1;
} tgsi_token_t;



typedef enum
{
	tgsi_file_null      = 0,
	tgsi_file_constant  = 1,
	tgsi_file_input     = 2,
	tgsi_file_output    = 3,
	tgsi_file_temporary = 4,
	tgsi_file_sampler   = 5,
	tgsi_file_address   = 6,
	tgsi_file_immediate = 7,
} tgsi_file_e;

typedef enum
{
	tgsi_declare_range = 0,
	tgsi_declare_mask  = 1,
} tgsi_declare_e;

typedef struct
__attribute__((__packed__))
{
	tgsi_token_type_e type        :  4;
	unsigned          size        :  8;
	tgsi_file_e       file        :  4;
	tgsi_declare_e    declare     :  4;
	bool              interpolate :  1;
	unsigned          padding     : 10;
	bool              extended    :  1;
} tgsi_declaration_t;

static const tgsi_declaration_t
	tgsi_declaration_default =
{
	.type        = tgsi_token_type_declaration,
	.size        = 2,
	.file        = tgsi_file_null,
	.declare     = tgsi_declare_range,
	.interpolate = false,
	.padding     = 0,
	.extended    = false
};

typedef struct
__attribute__((__packed__))
{
	unsigned first : 16;
	unsigned last  : 16;
} tgsi_declaration_range_t;

static  const tgsi_declaration_range_t
	tgsi_declaration_range_default =
{
	.first = 0,
	.last  = 0
};

typedef uint32_t tgsi_declaration_mask_t;

static tgsi_declaration_mask_t
	tgsi_declaration_mask_default = 0;

typedef enum
{
	tgsi_interpolate_constant    = 0,
	tgsi_interpolate_linear      = 1,
	tgsi_interpolate_perspective = 2,
} tgsi_interpolate_e;

typedef struct
__attribute__((__packed__))
{
	tgsi_interpolate_e interpolate :  4;
	unsigned           padding     : 28;
} tgsi_declaration_interpolation_t;

static const tgsi_declaration_interpolation_t
	tgsi_declaration_interpolation_default =
{
	.interpolate = tgsi_interpolate_constant,
	.padding     = 0
};



typedef enum
{
	tgsi_imm_float32 = 0,
} tgsi_imm_e;

typedef struct
__attribute__((__packed__))
{
	tgsi_token_type_e type      :  4;
	unsigned          size      :  8;
	tgsi_imm_e        data_type :  4;
	unsigned          padding   : 15;
	bool              extended  :  1;
} tgsi_immediate_t;

static const tgsi_immediate_t
	tgsi_immediate_default =
{
	.type      = tgsi_token_type_immediate,
	.size      = 1;
	.data_type = tgsi_imm_float32,
	.padding   = 0,
	.extended  = false
};



typedef enum
{
	tgsi_opcode_arl   =  0,
	tgsi_opcode_mov   =  1,
	tgsi_opcode_lit   =  2,
	tgsi_opcode_rcp   =  3,
	tgsi_opcode_rsq   =  4,
	tgsi_opcode_exp   =  5,
	tgsi_opcode_log   =  6,
	tgsi_opcode_mul   =  7,
	tgsi_opcode_add   =  8,
	tgsi_opcode_dp3   =  9,
	tgsi_opcode_dp4   = 10,
	tgsi_opcode_dst   = 11,
	tgsi_opcode_min   = 12,
	tgsi_opcode_max   = 13,
	tgsi_opcode_slt   = 14,
	tgsi_opcode_sge   = 15,
	tgsi_opcode_mad   = 16,
	tgsi_opcode_sub   = 17,
	tgsi_opcode_lerp  = 18,
	tgsi_opcode_cnd   = 19,
	tgsi_opcode_cnd0  = 20,
	tgsi_opcode_dot2add = 21,
	tgsi_opcode_index = 22,
	tgsi_opcode_negate = 23,
	tgsi_opcode_frac  = 24,
	tgsi_opcode_clamp = 25,
	tgsi_opcode_floor = 26,
	tgsi_opcode_round = 27,
	tgsi_opcode_expbase2 = 28,
	tgsi_opcode_logbase2 = 29,
	tgsi_opcode_power = 30,
	tgsi_opcode_crossproduct = 31,
	tgsi_opcode_multiplymatrix = 32,
	tgsi_opcode_abs   = 33,
	tgsi_opcode_rcc   = 34,
	tgsi_opcode_dph   = 35,
	tgsi_opcode_cos   = 36,
	tgsi_opcode_ddx   = 37,
	tgsi_opcode_ddy   = 38,
	tgsi_opcode_kilp  = 39,
	tgsi_opcode_pk2h  = 40,
	tgsi_opcode_pk2us = 41,
	tgsi_opcode_pk4b  = 42,
	tgsi_opcode_pk4ub = 43,
	tgsi_opcode_rfl   = 44,
	tgsi_opcode_seq   = 45,
	tgsi_opcode_sfl   = 46,
	tgsi_opcode_sgt   = 47,
	tgsi_opcode_sin   = 48,
	tgsi_opcode_sle   = 49,
	tgsi_opcode_sne   = 50,
	tgsi_opcode_str   = 51,
	tgsi_opcode_tex   = 52,
	tgsi_opcode_txd   = 53,
	tgsi_opcode_txp   = 54,
	tgsi_opcode_up2h  = 55,
	tgsi_opcode_up2us = 56,
	tgsi_opcode_up4b  = 57,
	tgsi_opcode_up4ub = 58,
	tgsi_opcode_x2d   = 59,
	tgsi_opcode_ara   = 60,
	tgsi_opcode_arr   = 61,
	tgsi_opcode_bra   = 62,
	tgsi_opcode_cal   = 63,
	tgsi_opcode_ret   = 64,
	tgsi_opcode_ssg   = 65,
	tgsi_opcode_cmp   = 66,
	tgsi_opcode_scs   = 67,
	tgsi_opcode_txb   = 68,
	tgsi_opcode_nrm   = 69,
	tgsi_opcode_div   = 70,
	tgsi_opcode_dp2   = 71,
	tgsi_opcode_txl   = 72,
	tgsi_opcode_brk   = 73,
	tgsi_opcode_if    = 74,
	tgsi_opcode_loop  = 75,
	tgsi_opcode_rep   = 76,
	tgsi_opcode_else  = 77,
	tgsi_opcode_endif = 78,
	tgsi_opcode_endloop = 79,
	tgsi_opcode_endrep = 80,
	tgsi_opcode_pusha = 81,
	tgsi_opcode_popa  = 82,
	tgsi_opcode_ceil  = 83,
	tgsi_opcode_i2f   = 84,
	tgsi_opcode_not   = 85,
	tgsi_opcode_trunc = 86,
	tgsi_opcode_shl   = 87,
	tgsi_opcode_shr   = 88,
	tgsi_opcode_and   = 89,
	tgsi_opcode_or    = 90,
	tgsi_opcode_mod   = 91,
	tgsi_opcode_xor   = 92,
	tgsi_opcode_sad   = 93,
	tgsi_opcode_txf   = 94,
	tgsi_opcode_txq   = 95,
	tgsi_opcode_cont  = 96,
	tgsi_opcode_emit  = 97,
	tgsi_opcode_endprim = 98,
	tgsi_opcode_bgnloop2 = 99,
	tgsi_opcode_bgnsub = 100,
	tgsi_opcode_endloop2 = 101,
	tgsi_opcode_endsub = 102,
	tgsi_opcode_noise1 = 103,
	tgsi_opcode_noise2 = 104,
	tgsi_opcode_noise3 = 105,
	tgsi_opcode_noise4 = 106,
	tgsi_opcode_nop    = 107,
	tgsi_opcode_m4x3   = 108,
	tgsi_opcode_m3x4   = 109,
	tgsi_opcode_m3x3   = 110,
	tgsi_opcode_m3x2   = 111,
	tgsi_opcode_nrm4   = 112,
	tgsi_opcode_callnz = 113,
	tgsi_opcode_ifc    = 114,
	tgsi_opcode_breakc = 115,
	tgsi_opcode_kil    = 116,
	tgsi_opcode_end    = 117,
	tgsi_opcode_swz    = 118,
	tgsi_opcode_count  = 119,
} tgsi_opcode_e;

typedef enum
{
	tgsi_sat_none           = 0,
	tgsi_sat_zero_one       = 1,
	tgsi_sat_minus_plus_one = 2,
} tgsi_sat_e;

typedef struct
__attribute__((__packed__))
{
	tgsi_token_type_e type         : 4;
	unsigned          size         : 8;
	tgsi_opcode_e     opcode       : 8;
	tgsi_sat_e        saturate     : 2;
	unsigned          num_dst_regs : 2;
	unsigned          num_src_regs : 4;
	unsigned          padding      : 3;
	bool              extended     : 1;
} tgsi_instruction_t;

typedef enum
{
	tgsi_swizzle_x = 0,
	tgsi_swizzle_y = 1,
	tgsi_swizzle_z = 2,
	tgsi_swizzle_w = 3,
} tgsi_swizzle_e;

typedef enum
{
	tgsi_precision_default = 0,
	tgsi_precision_float32 = 1,
	tgsi_precision_float16 = 2,
	tgsi_precision_fixed12 = 3,
} tgsi_precision_e;

typedef struct
__attribute__((__packed__))
{
	tgsi_token_type_e type             : 4;
	tgsi_precision_e  precision        : 4;
	unsigned          cond_dst_index   : 4;
	unsigned          cond_flow_index  : 4;
	unsigned          cond_mask        : 4;
	tgsi_swizzle_e    cond_swizzle_x   : 2;
	tgsi_swizzle_e    cond_swizzle_y   : 2;
	tgsi_swizzle_e    cond_swizzle_z   : 2;
	tgsi_swizzle_e    cond_swizzle_w   : 2;
	bool              cond_dst_update  : 1;
	bool              cond_flow_enable : 1;
	unsigned          padding          : 1;
	bool              extended         : 1;
} tgsi_instruction_ext_nv_t;

typedef struct
__attribute__((__packed__))
{
	tgsi_token_type_e type     :  4;
	unsigned          label    : 24;
	bool              target   :  1;
	unsigned          padding  :  2;
	bool              extended :  1;
} tgsi_instruction_ext_label_t;

typedef enum
{
	tgsi_texture_unknown    = 0,
	tgsi_texture_1d         = 1,
	tgsi_texture_2d         = 2,
	tgsi_texture_3d         = 3,
	tgsi_texture_cube       = 4,
	tgsi_texture_rect       = 5,
	tgsi_texture_shadow1d   = 6,
	tgsi_texture_shadow2d   = 7,
	tgsi_texture_shadowrect = 8,
} tgsi_texture_e;

typedef struct
__attribute__((__packed__))
{
	tgsi_token_type_e type     :  4;
	tgsi_texture_e    texture  :  8;
	unsigned          padding  : 19;
	bool              extended :  1;
} tgsi_instruction_ext_texture_t;



typedef struct
__attribute__((__packed__))
{
	bool     indirect  :  1;
	bool     dimension :  1;
	unsigned padding   : 13;
	unsigned index     : 16;
	bool     extended  :  1;
} tgsi_dimension_t;



typedef struct
__attribute__((__packed__))
{
	tgsi_file_e    file      :  4;
	tgsi_swizzle_e swizzle_x :  2;
	tgsi_swizzle_e swizzle_y :  2;
	tgsi_swizzle_e swizzle_z :  2;
	tgsi_swizzle_e swizzle_w :  2;
	bool           negate    :  1;
	bool           indirect  :  1;
	bool           dimension :  1;
	unsigned       index     : 16;
	bool           extended  :  1;
} tgsi_src_register_t;

typedef enum
{
	tgsi_extswizzle_x    = 0,
	tgsi_extswizzle_y    = 1,
	tgsi_extswizzle_z    = 2,
	tgsi_extswizzle_w    = 3,
	tgsi_extswizzle_zero = 4,
	tgsi_extswizzle_one  = 5,
} tgsi_extswizzle_e;

typedef struct
__attribute__((__packed__))
{
	tgsi_token_type_e type          : 4;
	tgsi_extswizzle_e ext_swizzle_x : 4;
	tgsi_extswizzle_e ext_swizzle_y : 4;
	tgsi_extswizzle_e ext_swizzle_z : 4;
	tgsi_extswizzle_e ext_swizzle_w : 4;
	bool              negate_x      : 1;
	bool              negate_y      : 1;
	bool              negate_z      : 1;
	bool              negate_w      : 1;
	tgsi_extswizzle_e ext_divide    : 4;
	unsigned          padding       : 3;
	bool              extended      : 1;
} tgsi_src_register_ext_swz_t;

typedef struct
__attribute__((__packed__))
{
	tgsi_token_type_e type       :  4;
	bool              compliment :  1;
	bool              bias       :  1;
	bool              scale_2x   :  1;
	bool              absolute   :  1;
	bool              negate     :  1;
	unsigned          padding    : 22;
	bool              extended   :  1;
} tgsi_src_register_ext_mod_t;



typedef struct
__attribute__((__packed__))
{
	tgsi_file_e    file       :  4;
	unsigned       write_mask :  4;
	bool           indirect   :  1;
	bool           dimension  :  1;
	unsigned       index      : 16;
	unsigned       padding    :  5;
	bool           extended   :  1;
} tgsi_dst_register_t;

typedef enum
{
	tgsi_cc_gt = 0,
	tgsi_cc_eq = 1,
	tgsi_cc_lt = 2,
	tgsi_cc_ge = 3,
	tgsi_cc_le = 4,
	tgsi_cc_ne = 5,
	tgsi_cc_tr = 6,
	tgsi_cc_fl = 7,
} tgsi_cc_e;

typedef struct
__attribute__((__packed__))
{
	tgsi_token_type_e type           :  4;
	tgsi_cc_e         cond_mask      :  4;
	tgsi_swizzle_e    cond_swizzle_x :  2;
	tgsi_swizzle_e    cond_swizzle_y :  2;
	tgsi_swizzle_e    cond_swizzle_z :  2;
	tgsi_swizzle_e    cond_swizzle_w :  2;
	unsigned          cond_src_index :  4;
	unsigned          padding        : 11;
	bool              extended       :  1;
} tgsi_dst_register_ext_concode_t;

typedef enum
{
	tgsi_modulate_1x      = 0,
	tgsi_modulate_2x      = 1,
	tgsi_modulate_4x      = 2,
	tgsi_modulate_8x      = 3,
	tgsi_modulate_half    = 4,
	tgsi_modulate_quarter = 5,
	tgsi_modulate_eighth  = 6,
	tgsi_modulate_count   = 7,
} tgsi_modulate_e;

typedef struct
__attribute__((__packed__))
{
	tgsi_token_type_e type     :  4;
	tgsi_modulate_e   modulate :  4;
	unsigned          padding  : 23;
	bool              extended :  1;
} tgsi_dst_register_ext_modulate_t;

#endif
