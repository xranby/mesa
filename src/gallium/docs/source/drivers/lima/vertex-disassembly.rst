http://limadriver.org/Vertex+Disassembly/

limadriver/ Vertex+Disassembly

Vertex Disassembly

There are 3 types of vertex disassembly to choose from when using MBS dump: 1. Explicit: This mostly just shows all the instructions fields though it clips some fields when they're not used. 2. Simple/Verbose: This shows a more sensible assembler syntax but is very verbose and difficult to extract meaning from, this is the best to read if you want to understand the underlying execution method. 3. Decompile: This attempts (usually successfully) to decompile the assembly into a readable form, this is good for verifying predictions about the architecture but must be used alongside the simple/verbose output when reverse-engineering.

All examples on this page are analysing the compiled form of this shader:

#version 100
#pragma optimize on

attribute vec4  a_position;
attribute float a_count;

varying vec4 v_color;



void main()
{
    float c = 0.0;
    for (int i = 0; i < int(a_count); i++)
        c += 1.5;
    v_color = vec4(c, c, c, 1.0);
    gl_Position = a_position;
}

1. Explicit

mbs_dump --explicit ...

To understand this syntax it's best to read the latest Lima+ISA page, since it's changes whenever we discover something new. This syntax is mostly just a raw dump of the instruction fields omitting those which are known not to be used, sources/destinations are shown as indices rather than being decoded.

Example:

Note: A full example of this would be too long so listed here is just one instruction.

{
    add0 0, -0
    add1 0, -0
    .uniform_in = 0
    .uniform_offset = 7
    .attrib_load = 1
    .register_load = 0
    .acc_opcode = 7
    .complex_opcode = 0
    .mul_opcode = 0
    .pass_op = 2
    .complex_src = 21
    .pass_src = 21
    .unknown_1 = 0
    .branch_target = 0
}

2. Simple/Verbose

mbs_dump --simple ...

This is a TTA style assembler, it's easy enough to read however there's a lot of it so it's difficult to understand what the code is actually doing without a pencil and paper (or decompiler).

The basic syntax is as follows: .(<args...>) Commas ',' separate fields in the same instruction (parallel) while semicolons ';' separate/sequence full instructions.

This is a higher level assembler so sources/destinations are given by name, the following sources/destinations are defined (note: Not all of these are available in every field): // TODO

Uniforms/varyings/attributes can also be referenced by name/index however note that since these are accessed as vec2/vec4's they may show a single name for a collection of 4 floats (or 2 vec2's, etc). This currently can't be helped though we're figuring out a way to make this less ambiguous.

Example:

attrib.load(1), acc[0].max(attrib.x, -attrib.x), acc[1].max(attrib.x, -attrib.x);
mul[1].pass(attrib.x[1]), acc[1].sign(attrib.x[1], unused);
mul[1].pass(acc[1].out[2]), acc[1].floor(acc[0].out[2], unused);
mul[1].mul(acc[1].out[2], acc[1].out[1]), acc[1].sign(mul[1].out[2], unused), reg[0].store(2, acc[1].out, none);
uniform.load(3), mul[1].pass(mul[1].out[2]), acc[1].ge(uniform.x, mul[1].out[1]), complex.pass(uniform.x), reg[0].store(2, none, complex.out);
uniform.load(3), acc[1].floor(mul[1].out[1], unused), complex.pass(uniform.x), reg[1].store(2, complex.out, acc[1].out);
complex.pass(acc[1].out[2]);
pass.pass(ident), control.branch(13, 11);
uniform.load(3), register.load(2), mul[1].mul(varying.x, varying.w), acc[0].add(varying.z, -ident), acc[1].add(varying.y, uniform.y), reg[0].store(2, none, acc[1].out);
acc[1].lt(acc[1].out[1], mul[1].out[1]);
uniform.load(3), acc[1].add(acc[0].out[2], uniform.z), pass.pass(acc[1].out[1]), reg[1].store(2, acc[1].out, none), control.branch(13, 8);
attrib.load(0), mul.complex2(attrib.w, attrib.w, unused, unused), complex.rcp(attrib.w);
mul.complex1(ident, mul[0].out[1], ident, attrib.w[1]);
uniform.load(0), acc[0].add(uniform.y, -ident), acc[1].add(uniform.z, -ident);
uniform.load(2), attrib.load(0), mul[0].mul(attrib.z, acc[1].out[1]), mul[1].mul(attrib.y, acc[0].out[1]), acc[1].add(uniform.z, -ident), pass.clamp(mul[0].out[2]);
uniform.load(0), attrib.load(0), mul[0].mul(attrib.x, uniform.x), mul[1].mul(mul[0].out[1], pass.out[1]);
uniform.load(1), register.load(2), mul[0].mul(mul[1].out[2], pass.out[2]), mul[1].mul(mul[0].out[1], pass.out[2]), acc[0].add(pass.out[2], -ident), acc[1].add(mul[1].out[1], uniform.z), complex.pass(varying.z), pass.pass(acc[1].out[2]), varying[0].store(1, complex.out, complex.out), varying[1].store(1, complex.out, pass.out);
uniform.load(1), mul[1].pass(acc[0].out[1]), acc[0].add(mul[0].out[1], uniform.y), acc[1].add(mul[1].out[1], uniform.x), complex.pass(acc[1].out[1]), varying[0].store(0, acc[1].out, acc[0].out), varying[1].store(0, complex.out, mul[1].out);

3. Decompiler

mbs_dump --decompile ...

The decompiler attempts to output code which is as close to the original compiled code as possible, alongside the simple/verbose or explicit this can be used to gain a high level overview of what the code is doing. If the decompiled code doesn't make sense then it means there's a mistake in our understanding of how the machine works and we can look back to the assembly code to see what's really going on and where we've made an incorrect assumption.

The decompiler doesn't show all the instruction fields, it only shows output instructions and anything they reference so we automatically get dead-code elimination, however this can mean that sometimes the code doesn't contain all that's needed to understand the instruction encoding.

Example:

03: $2.x = acc.sign(a_count);
04: $2.y = 0;
05: $2.z = 0;
05: $2.w = acc.floor(acc.abs(a_count));
07: control.branch((0 >= (acc.sign(a_count) * acc.floor(acc.abs(a_count)))), 13, 0x0B);
08: $2.y = ($2.y + 1);
0A: $2.z = ($2.z + 1.5);
0A: control.branch((($2.y + 1) < ($2.x * $2.w)), 13, 0x08);
10: v_color.x = $2.z;
10: v_color.y = $2.z;
10: v_color.z = $2.z;
10: v_color.w = 1;
11: gl_Position.x = (((a_position.x * gl_mali_ViewportTransform[0].x) * clamp(mul.complex1(complex.rcp(a_position.w), mul[0].complex2(a_position.w, a_position.w), complex.rcp(a_position.w), a_position.w), -1e+10, 1e+10)) + gl_mali_ViewportTransform[0].x);
11: gl_Position.y = (((a_position.y * gl_mali_ViewportTransform[0].y) * clamp(mul.complex1(complex.rcp(a_position.w), mul[0].complex2(a_position.w, a_position.w), complex.rcp(a_position.w), a_position.w), -1e+10, 1e+10)) + gl_mali_ViewportTransform[0].y);
11: gl_Position.z = (((a_position.z * gl_mali_ViewportTransform[0].z) * clamp(mul.complex1(complex.rcp(a_position.w), mul[0].complex2(a_position.w, a_position.w), complex.rcp(a_position.w), a_position.w), -1e+10, 1e+10)) + gl_mali_ViewportTransform[0].z);
11: gl_Position.w = clamp(mul.complex1(complex.rcp(a_position.w), mul[0].complex2(a_position.w, a_position.w), complex.rcp(a_position.w), a_position.w), -1e+10, 1e+10);
