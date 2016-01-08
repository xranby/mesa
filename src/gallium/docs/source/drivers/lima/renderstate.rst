http://limadriver.org/Render_State/

limadriver/ Render State

Render state

The Mali render state is a record of 16 32-bit words (64 bytes). It consists of mainly rasterizer state. When queuing a draw command an address of such a structure is passed with LIMA_PLBU_CMD_RSW_VERTEX_ARRAY (see vs_commands_draw_add in the Lima source).

0x00 [0] blend color
  00000000 00000000 00000000 11111111 blendColor blue component
  00000000 11111111 00000000 00000000 blendColor green component  

0x04 [1] blend color
  00000000 00000000 00000000 11111111 blendColor red component
  00000000 11111111 00000000 00000000 blendColor alpha component

0x08 [2] alpha blend
  00000000 00000000 00000000 00000111 modeRGB (BlendEquation)
  00000000 00000000 00000000 00111000 modeAlpha (BlendEquation)
  00000000 00000000 00000111 11000000 srcRGB (ColorBlendFunc)
  00000000 00000000 11111000 00000000 dstRGB (ColorBlendFunc)
  00000000 00001111 00000000 00000000 srcAlpha (AlphaBlendFunc)
  00000000 11110000 00000000 00000000 dstAlpha (AlphaBlendFunc)
  ???????? 00000000 00000000 00000000 always 11111100? (TODO: check whether this is GLES1 glAlphaFunc)

0x0C [3] depth test
  00000000 00000000 00000000 00000001 GL_DEPTH_TEST
  00000000 00000000 00000000 00001110 depthFunc (CompareFunc)
  00000000 11111111 00000000 00000000 polygonOffset factor
  11111111 00000000 00000000 00000000 polygonOffset units

0x10 [4] depth range
  11111111 11111111 00000000 00000000 max(nearVal, farVal)
  00000000 00000000 11111111 11111111 min(nearVal, farVal)

0x14 [5] stencil GL_FRONT
  00000000 00000000 00000000 00000111 func (CompareFunc)
  00000000 00000000 00000000 00111000 sfail (StencilOp)
  00000000 00000000 00000001 11000000 dpfail (StencilOp)
  00000000 00000000 00001110 00000000 dppass (StencilOp)
  00000000 11111111 00000000 00000000 ref
  11111111 00000000 00000000 00000000 mask

0x18 [6] stencil GL_BACK
  00000000 00000000 00000000 00000111 func (CompareFunc)
  00000000 00000000 00000000 00111000 sfail (StencilOp)
  00000000 00000000 00000001 11000000 dpfail (StencilOp)
  00000000 00000000 00001110 00000000 dppass (StencilOp)
  00000000 11111111 00000000 00000000 ref
  11111111 00000000 00000000 00000000 mask

0x1C [7] stencil test
  00000000 00000000 11111111 11111111 GL_STENCIL_TEST (either all bits are set or not)
  00000000 11111111 00000000 00000000 glAlphaFunc reference value: 0.5 = 0x80, 1.0 = 0xFF.

0x20 [8] multisample
  00000000 00000000 00000000 00000111 always set? could be another CompareFunc
  00000000 00000000 00000000 01101000 (0x00006800 "4x MSAA" in lima)
  00000000 00000000 00000000 10000000 GL_SAMPLE_ALPHA_TO_COVERAGE
  00000000 00000000 00000001 00000000 GL_SAMPLE_ALPHA_TO_ONE
  00000000 00000000 11110000 00000000 sampleCoverage (SampleCoverage)
  00000000 11000000 00000000 00000000 vertex selector? (00 GL_POINTS 01 GL_LINE* 10 GL_TRIANGLE*)

  00000000 00000000 11110000 00000111 (default in GLES2) 
  00000000 00000000 11111000 00000111 (default in lima)

0x24 [9] shader address

  11111111 11111111 11111111 11100000 Fragment shader address
  00000000 00000000 00000000 00011111 Size of first instruction

0x28 [10] varying types

0x2C [11] uniforms address (16-aligned)

0x30 [12] textures address (16-aligned)

0x34 [13] ?
  00000000 00000000 00000001 00000000 ? usually 1
  00000000 00000000 00000010 00000000 Enable early Z
  00000000 00000000 00010000 00000000 Enable pixel kill

0x38 [14] dither etc
  00000000 00000000 00010000 00000000 glFrontFace (0=GL_CCW, 1=GL_CW)
  00000000 00000000 00100000 00000000 GL_DITHER
  00000000 00000001 00000000 00000000 set if(uniform_size) in Lima

0x3C [15] varyings address (16-aligned)

Bitfields

CompareFunc:
    000 GL_NEVER
    001 GL_LESS
    010 GL_EQUAL
    011 GL_LEQUAL
    100 GL_GREATER
    101 GL_NOTEQUAL
    110 GL_GEQUAL
    111 GL_ALWAYS

StencilOp:
    000 GL_KEEP
    001 GL_REPLACE
    010 GL_ZERO
    011 GL_INVERT
    100 GL_INCR_WRAP
    101 GL_DECR_WRAP
    110 GL_INCR
    111 GL_DECR

BlendEquation:
    000 GL_FUNC_SUBTRACT
    001 GL_FUNC_REVERSE_SUBTRACT
    010 GL_FUNC_ADD
    100 GL_MIN_EXT
    101 GL_MAX_EXT

ColorBlendFunc:
    00000 GL_SRC_COLOR
    00001 GL_DST_COLOR
    00010 GL_CONSTANT_COLOR
    00011 GL_ZERO
    00111 GL_SRC_ALPHA_SATURATE
    01000 GL_ONE_MINUS_SRC_COLOR
    01001 GL_ONE_MINUS_DST_COLOR
    01010 GL_ONE_MINUS_CONSTANT_COLOR
    01011 GL_ONE
    10000 GL_SRC_ALPHA
    10001 GL_DST_ALPHA
    11000 GL_ONE_MINUS_SRC_ALPHA
    11001 GL_ONE_MINUS_DST_ALPHA
    10010 GL_CONSTANT_ALPHA
    11010 GL_ONE_MINUS_CONSTANT_ALPHA

AlphaBlendFunc is the same as ColorBlendFunc, except that the upper bit is missing.
  This can be the case because the upper bit determines _ALPHA or _COLOR, and for the the alpha factor
  these are equivalent.

SampleCoverage:
    0000 value=0.00 inverted=FALSE
    0001 value=0.25 inverted=FALSE
    0011 value=0.50 inverted=FALSE
    0111 value=0.75 inverted=FALSE
    1111 value=1.0  inverted=FALSE
    1111 value=0.00 inverted=TRUE
    1110 value=0.25 inverted=TRUE
    1100 value=0.50 inverted=TRUE
    1000 value=0.75 inverted=TRUE
    0000 value=1.00 inverted=TRUE

