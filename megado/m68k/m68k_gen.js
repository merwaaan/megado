const u = require('./m68k_gen_utils');
const instr = require('./m68k_gen_instructions');

// Group modes that often go together
const modes_data_reg = u.mode_mask('DataReg');
const modes_addr_reg = u.mode_mask('AddrReg');
const modes_addr = u.mode_mask('Addr | AddrPostInc | AddrPreDec');
const modes_addr_offset = u.mode_mask('AddrDisp | AddrIndex');
const modes_abs = u.mode_mask('AbsWord | AbsLong');
const modes_pc = u.mode_mask('PCDisp | PCIndex');
const modes_imm = u.mode_mask('Imm');
const modes_all = modes_data_reg + modes_addr_reg + modes_addr + modes_addr_offset + modes_abs + modes_pc + modes_imm;

// Metadata to generate the M68000's instruction set.
// http://goldencrystal.free.fr/M68kOpcodes-v2.3.pdf
// https://emu-docs.org/CPU%2068k/68kstat.txt
//
// Patterns may contain the following tokens:
//   - ?: any bit
//   - 0/1: cleared/set bit
//   - RRR: register number
//   - S/S2/S3: size
//   - MMMXXX/XXXMMM: addressing mode

const patterns =
[
    // [generator, format, legal addressing modes for MMMXXX...]
    //[instr.gen_add, '1101RRR0S2MMMXXX'],
    //[instr.gen_bset, '0000RRR111MMMXXX'],
    [instr.gen_move, '0001XXXMMMMMMXXX', modes_data_reg + modes_addr + modes_addr_offset + modes_abs, modes_all - modes_addr_reg], // Byte size: address register is not allowed as source
    [instr.gen_move, '001?XXXMMMMMMXXX', modes_data_reg + modes_addr + modes_addr_offset + modes_abs, modes_all], // Word, long
    [instr.gen_tst, '01001010S2MMMXXX', modes_all - modes_addr_reg - modes_imm]
];

// Check if the given opcode matches the pattern.
function match(opcode, pattern)
{
    const opcode_bin = u.num_to_bin(opcode);
    const format_regexp = /([01]+|RRR|MMMXXX|XXXMMM|S2|S3|S)+?/g;

    let ea = 0; // Count effective addresses to refer to the appropriate mask

    // Go through each token
    let match;
    while (match = format_regexp.exec(pattern[1]))
    {
        let opcode_section = opcode_bin.substr(match.index, match[0].length);
        //if (opcode === 0x1c0) console.log(opcode.toString(16), opcode_bin, match[0], opcode_section);

        // Check that the opcode section corresponds to the token type
        if (!check_opcode_token(match[0], opcode_section))
            return false;

        // Check that the bit pattern describes a legal addressing mode
        if (match[0] === 'MMMXXX' || match[0] === 'XXXMMM')
        {
            // Swap the halves if necessary
            opcode_section = match[0] === 'MMMXXX' ? opcode_section : opcode_section.substr(3, 3) + opcode_section.substr(0, 3);

            const mode = u.mode_from_pattern(parseInt(opcode_section, 2));

            if ((pattern[2 + ea] & mode) === 0)
                return false;

            ++ea;
        }
    }

    return true;
}

function check_opcode_token(format, token)
{
    // Make sure that the token only contains zeros and ones
    // and that is has the same size as the format
    if (!/^[01]+$/.test(token) || format.length !== token.length)
        return false;

    switch (format)
    {
        case '?': return true;
        case 'S': return true; // Any value is valid as a 1-bit size
        case 'S2': return ['00', '01', '10'].some(x => x === token);
        case 'S3': return ['01', '11', '10'].some(x => x === token);
        case 'RRR': return true; // Any 3-bit value is valid as a register number

        case 'XXXMMM':
            // Swap the halves and let the next case handle it
            token = token.substr(3, 3) + token.substr(0, 3);

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
            return /^[01]+$/.test(format) && format === token;
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

    // Add optional pre-step
    if (instr.src && instr.src.pre) c += instr.src.pre();
    if (instr.dst && instr.dst.pre) c += instr.dst.pre();

    // Add optional fetching code
    if (instr.src && instr.src.fetch) c += `${instr.src.fetch()} // src fetch\n`;
    if (instr.dst && instr.dst.fetch) c += `${instr.dst.fetch()} // dst fetch\n`;

    c += instr.code;

    // Add optional post-step
    if (instr.src && instr.src.post) c += instr.src.post();
    if (instr.dst && instr.dst.post) c += instr.dst.post();

    c += 'return 0;\n}\n'; // TODO cycles
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