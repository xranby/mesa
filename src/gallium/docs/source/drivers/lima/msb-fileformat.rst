
limadriver/ MBS+File+Format

    Edit RecentChanges History Preferences 

    Index
    Hardware
    Lima+ISA
    Source
    Source-ng

MBS File Format

chunk {
    char     ident[4];
    uint32_t size;
}

string {
    chunk header;   // ="STRI"
    char  string[]; // Null terminated, Zero-padded to 4-byte boundary.
}

frag_sta {
    chunk    header;    // ="FSTA"
    uint32_t stacksize; // fragment stack size
    uint32_t stackofs;  // starting offset
}

frag_dis {
    chunk    header;    // ="FDIS"
    uint32_t discard;   // 1 if shader has discard instruction
}

frag_buu {
    chunk    header;    // ="FBUU"
    uint8_t reads_color; // gl_FBColor
    uint8_t writes_color; // gl_FragColor
    uint8_t reads_depth; // gl_FBDepth
    uint8_t writes_depth; // ? gl_FragDepth (not supported in GLES2)
    uint8_t reads_stencil; // gl_FBStencil
    uint8_t writes_stencil; // ? gl_FragStencil (not supported in GLES2)
    uint8_t unknown_0;
    uint8_t unknown_1;
}

symbol {
    chunk    header; // ="VUNI"/"VVAR"/"VATT"
    string   symbol;
    uint8_t  unknown_0; // =0x00
    // type: 
    //   0x01 float
    //   0x02 int
    //   0x03 bool 
    //   0x04 matrix
    //   0x05 sampler2D
    //   0x06 samplerCube
    //   0x08 struct
    //   0x09 samplerExternalOES 
    uint8_t  type;      
    uint16_t component_count;
    uint16_t component_size;
    uint16_t entry_count;
    uint16_t src_stride;
    uint8_t  dst_stride;
    uint8_t  precision;
    uint32_t invariant; // 1 if "invariant" keyword specified, otherwise 0
    uint16_t offset;
    uint16_t index; // Usually -1 (0xFFFF) otherwise index of parent struct
}

table {
    chunk    header; // ="SUNI"/"SVAR"/"SATT"
    uint32_t count;
    symbol   symbols[count];
}

dbin {
    chunk    header; // ="DBIN"
    uint32_t code[];
}

frag {
    chunk    header;  // ="CFRA"
    // version (seems _mali_core_type from mali_ioctl.h)
    //   0x05 MALI_200
    //   0x07 MALI_400_PP
    uint32_t version; // =5
    frag_sta sta;
    frag_dis dis;
    frag_buu buu;
    table    uniforms; // ="SUNI"
    table    varyings; // ="SVAR"
    dbin     code;
}

vert_fins {
    chunk header; // ="FINS"
    uint32_t unknown_0;
    uint32_t instructions;
    uint32_t attrib_prefetch;
}

vertex {
    chunk header; // ="CVER"
    // version (seems _mali_core_type from mali_ioctl.h)
    //   0x02 MALI_GP2
    //   0x06 MALI_400_GP
    uint32_t version;
    vert_fins fins;
    table uniforms; // ="SUNI"
    table attributes; // ="SATT"
    table variants; // ="SVAR"
    dbin code;
}

file {
    chunk header; // ="MBS1"
    frag  fragment;
    vert  vertex;
}

Symbols & Symbol layout

Each entry in the symbol tables represents an array of a single base type (vec4, struct, etc.) or just a single base type. Note that, conceptually, variables are laid out on one big array; unlike the ESSL varying and uniform packing algorithm, there is (mostly) no notion of columns and row.

Each storage type (uniform, varying, attribute) has a different alignment and stride (which are usually the same) for each type, due to restrictions on how the storage type in question can be indirectly addressed:

    For varyings, float has an alignment and stride of 1, vec2 has an alignment of 2, and vec3 and vec4 have an alignment of 4 (mat2 has the same alignment as vec2, mat3 the same as vec3, etc.).
    Uniforms in the PP have the same alignment restrictions as varyings. Note that ints and bools have the exact same behavior as floats, and a sampler takes up 1 float of space (WTF?).
    For uniforms in the GP, the stride for everything is 4, but floats, vec2's, and vec3's are fine as long as they fit within a vec4... basically it's the original ESSL packing rules.
    For attributes, there is no packing so everything occupies a separate vec4 variable.
    An array has the same alignment as its base type, and a stride equal to the stride of the base type times the number of elements.
    A structure's alignment is determined by the largest alignment of all its children, and its stride is the smallest multiple of its alignment greater than or equal to its size.

Here is a more detailed description of each field:

    component_count: for base types, the number of components (for example, vec3 has a three components). For structures, the number of elements in the structure. For samplers, it is the number of dimensions of the sampler, sampler2D is 2 and samplerCube is 3 (and presumably sampler3D is 3) (why is this needed??).
    component_size: The total size of each element in the array, or the total size of the symbol if not in an array. For structures, this includes any padding at the end. Normally, this is the same as the stride, except for matrices where component_size is the same as component_count.
    entry_count: The number of elements in the array. 0 indicates that this symbol is not an array.
    src_stride: The stride needed when accessing elements of the array indirectly. When getting the offset of an element in an array, the formula is offset + index * src_stride, where 0 <= index < entry_count. Note that src_stride must be a multiple of the alignment of the symbol type.
    dst_stride: always 16??? except for varyings fed directly into texture fetches, where it's 24
    offset: the base offset of the symbol, relative to the start of the structure or to the start of the array if this symbol is not part of a structure. Note that this must be a multiple of the alignment of this symbol.
    index: -1 if not part of a structure, otherwise the index of the parent structure in the symbol table.

Links: Mali Offline Shader Compiler index lima+assembler
Last edited 1 year and 10 months ago

