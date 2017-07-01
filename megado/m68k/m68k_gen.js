const u = require('./m68k_gen_utils');
const instr = require('./m68k_gen_instructions');

// Metadata to generate the M68000's instruction set.
// http://goldencrystal.free.fr/M68kOpcodes-v2.3.pdf
//
// Patterns may contain the following tokens:
//   - 0/1: cleared/set bit
//   - RRR: register number
//   - S/S2/S3: size
//   - MMMXXX: addressing mode

const patterns =
[
    // [generator, format, source addressing modes, destination addressing modes]
    //[instr.gen_add, '1101RRR0S2MMMXXX'], // TODO add legal addressing modes
    [instr.gen_bset, '0000RRR111MMMXXX']
];

// Check if the given opcode matches the pattern.
function match(opcode, pattern)
{
    const opcode_bin = u.num_to_bin(opcode);
    const format_regexp = /([01]+|RRR|MMMXXX|S2|S3|S)+?/g;

    // Go through each token
    let match;
    while (match = format_regexp.exec(pattern[1]))
    {
        const opcode_section = opcode_bin.substr(match.index, match[0].length);
        //if (opcode === 0x1c0) console.log(opcode.toString(16), opcode_bin, match[0], opcode_section);
        // Check that the opcode section corresponds to the token type
        if (!check_opcode_token(match[0], opcode_section))
            return false;
    }

    return true;
}

function check_opcode_token(format, token)
{
    switch (format)
    {
        case 'RRR': return true; // Any 3-bit value is valid as a register number
        case 'S': return true; // Any value is valid as a 1-bit size
        case 'S2': return ['00', '01', '10'].some(x => x === token);
        case 'S3': return ['01', '11', '10'].some(x => x === token);

        case 'MMMXXX':

            // For data/address registers, all formats are valid under 111000
            const value = parseInt(token, 2);
            if (value < 0b111000)
                return true;

            // Other addressing modes have specific patterns
            return ['111010',
                    '111011',
                    '111000',
                    '111001',
                    '111100'].some(x => x === token);

        default:
            // The last case should be fixed bits
            return /^[01]+$/.test(token) && format === token;
    }
};

///////////////////////////////////////////////////////////

// Generate the instruction set

const instructions = new Array(0x10000);
instructions.fill(null);

for (let opcode = 0; opcode < 0x10000; ++opcode)
{
    for (let pattern of patterns)
    {
        if (match(opcode, pattern))
        {
            instructions[opcode] = pattern[0](opcode);
            break;
        }
    }
}

// Concatenate the instructions into a single switch

let code = instructions.map((instr, opcode) =>
{
    if (instr === null)
        return '';

    // Switch case, and mnemonics as a comment
    c = `case 0x${u.num_to_hex(opcode)}: /* ${instr.mnemonics} */ {\n`;

    // Add optional fetching code
    if (instr.src.fetch) c += instr.src.fetch() + '\n';
    if (instr.dst.fetch) c += instr.dst.fetch() + '\n';

    c += instr.code;
    c += '\n}\n';
    return c;
});

let output = `switch (opcode) {
    ${code.join('')}
}`;
// TODO clean up wild indentation

const fs = require('fs');
fs.writeFile('instructions_switch.gen', output, handle_file_error);

// Build a table listing generated instructions

const generated_table = instructions.map((instr, opcode) => !!instr);

output = `bool generated_instructions[] = {${generated_table.join(', ')}};\n`;

fs.writeFile('instructions_generated.gen', output, handle_file_error);

function handle_file_error(err)
{
    if (err)
        console.error(err);
}