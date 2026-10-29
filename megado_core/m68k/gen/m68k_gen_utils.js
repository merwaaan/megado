module.exports =
{
    bit: (value, bit) => (value >> bit) & 1,
    bits: (value, bit_from, bit_to) => (value >> bit_to) & ~(0xFFFFFFFF << (bit_from - bit_to + 1)), // from > to

    // For sizes: byte is 0, word is 1, long is 2
    size_masks: ['0xFF', '0xFFFF', '0xFFFFFFFF'],
    size_antimasks: ['0xFFFFFF00', '0xFFFF0000', '0'],
    size_types: ['uint8_t', 'uint16_t', 'uint32_t'],
    size_enum: ['Byte', 'Word', 'Long'],
    size_values: [8, 16, 32],
    size_bytes: [1, 2, 4],
    read_funcs: ['m68k_read_b', 'm68k_read_w', 'm68k_read_l'],
    write_funcs: ['m68k_write_b', 'm68k_write_w', 'm68k_write_l'],

    // There are three different formats to encode size in opcodes.
    // The index is the pattern, the value is the size.
    size1: [1, 2, null, null],
    size2: [0, 1, 2, null],
    size3: [null, 0, 2, 1],

    // Addressing mode bit flags (same layout as the former `enum` package).
    modes: {
        DataReg: 1 << 0,
        AddrReg: 1 << 1,
        Addr: 1 << 2,
        AddrPostInc: 1 << 3,
        AddrPreDec: 1 << 4,
        AddrDisp: 1 << 5,
        AddrIndex: 1 << 6,
        AbsWord: 1 << 7,
        AbsLong: 1 << 8,
        PCDisp: 1 << 9,
        PCIndex: 1 << 10,
        Imm: 1 << 11,
    },

    mode_mask: function (...modes) {
        return modes.reduce((mask, name) => mask | this.modes[name], 0);
    },

    // Returns the mode corresponding to the MMMXXX pattern that is often part of opcodes
    mode_from_pattern: function (pattern)
    {
        switch (pattern & 0b111000)
        {
            case 0b000000: return this.modes.DataReg;
            case 0b001000: return this.modes.AddrReg;
            case 0b010000: return this.modes.Addr;
            case 0b011000: return this.modes.AddrPostInc;
            case 0b100000: return this.modes.AddrPreDec;
            case 0b101000: return this.modes.AddrDisp;
            case 0b110000: return this.modes.AddrIndex;

            case 0b111000:
                switch (pattern & 0b111)
                {
                    case 0b000: return this.modes.AbsWord;
                    case 0b001: return this.modes.AbsLong;
                    case 0b010: return this.modes.PCDisp;
                    case 0b011: return this.modes.PCIndex;
                    case 0b100: return this.modes.Imm;
                    default: throw `Invalid operand pattern ${pattern.toString(2)}`;
                }

            default: throw `Invalid operand pattern ${pattern.toString(2)}`;
        }
    },

    conditions: [
        'true',
        'false',
        'ZERO(ctx) + CARRY(ctx) == 0',
        'ZERO(ctx) | CARRY(ctx)',
        '!CARRY(ctx)',
        'CARRY(ctx)',
        '!ZERO(ctx)',
        'ZERO(ctx)',
        '!OVERFLOW(ctx)',
        'OVERFLOW(ctx)',
        '!NEGATIVE(ctx)',
        'NEGATIVE(ctx)',
        '(NEGATIVE(ctx) & OVERFLOW(ctx)) | (!NEGATIVE(ctx) & !OVERFLOW(ctx))',
        '(NEGATIVE(ctx) & !OVERFLOW(ctx)) | (!NEGATIVE(ctx) & OVERFLOW(ctx))',
        '(NEGATIVE(ctx) & OVERFLOW(ctx) & !ZERO(ctx)) | (!NEGATIVE(ctx) & !OVERFLOW(ctx) & !ZERO(ctx))',
        'ZERO(ctx) | (NEGATIVE(ctx) & !OVERFLOW(ctx)) | (!NEGATIVE(ctx) & OVERFLOW(ctx))'
    ],

    // Convert a number to its binary representation
    num_to_bin: x => x.toString(2).padStart(16, '0'),
    num_to_hex: x => x.toString(16).padStart(4, '0')
};
