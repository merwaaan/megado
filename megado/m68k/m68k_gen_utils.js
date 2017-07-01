module.exports =
{
    bit: (value, bit) => (value >> bit) & 1,
    bits: (value, bit_from, bit_to) => (value >> bit_to) & ~(0xFFFFFFFF << (bit_from - bit_to + 1)), // from > to

    // For sizes: byte is 0, word is 1, long is 2
    size_masks: ['0xFF', '0xFFFF', '0xFFFFFFFF'],
    size_antimasks: ['0xFFFFFF00', '0xFFFF0000', '0'],
    size_types: ['uint8_t', 'uint16_t', 'uint32_t'],
    size_enum: ['Byte', 'Word', 'Long'],
    size_values: ['8', '16', '32'],

    // There are three different formats to encode size in opcodes.
    // The index is the pattern, the value is the size.
    size1: [1, 2, null, null],
    size2: [0, 1, 2, null],
    size3: [null, 0, 2, 1],

    // Convert a number to its binary representation
    num_to_bin: x => x.toString(2).padStart(16, '0'),
    num_to_hex: x => x.toString(16).padStart(4, '0')
};
