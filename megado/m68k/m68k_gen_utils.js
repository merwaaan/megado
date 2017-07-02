const Enum = require('enum');

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

    // There are three different formats to encode size in opcodes.
    // The index is the pattern, the value is the size.
    size1: [1, 2, null, null],
    size2: [0, 1, 2, null],
    size3: [null, 0, 2, 1],

    // Existing addressing modes
    modes: new Enum([
        'DataReg',
        'AddrReg',
        'Addr',
        'AddrPostInc',
        'AddrPreDec',
        'AddrDisp',
        'AddrIndex',
        'AbsWord',
        'AbsLong',
        'PCDisp',
        'PCIndex',
        'Imm'
    ]),

    // Shortcut to get a mask value from addressing mode names (eg 'DataReg | AddrReg' -> 3)
    mode_mask: function (mode) { return this.modes.get(mode).value; },

    // Returns the mode corresponding to the MMMXXX pattern that is often part of opcodes
    mode_from_pattern: function (pattern)
    {
        switch (pattern & 0b111000)
        {
            case 0b000000: return this.mode_mask('DataReg');
            case 0b001000: return this.mode_mask('AddrReg');
            case 0b010000: return this.mode_mask('Addr');
            case 0b011000: return this.mode_mask('AddrPostInc');
            case 0b100000: return this.mode_mask('AddrPreDec');
            case 0b101000: return this.mode_mask('AddrDisp');
            case 0b110000: return this.mode_mask('AddrIndex');

            case 0b111000:
                switch (pattern & 0b111)
                {
                    case 0b000: return this.mode_mask('AbsWord');
                    case 0b001: return this.mode_mask('AbsLong');
                    case 0b010: return this.mode_mask('PCDisp');
                    case 0b011: return this.mode_mask('PCIndex');
                    case 0b100: return this.mode_mask('Imm');
                    default: throw `Invalid operand pattern ${pattern.toString(2)}`;
                }

            default: throw `Invalid operand pattern ${pattern.toString(2)}`;
        }
    },

    // Convert a number to its binary representation
    num_to_bin: x => x.toString(2).padStart(16, '0'),
    num_to_hex: x => x.toString(16).padStart(4, '0')
};
