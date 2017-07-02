const u = require('./m68k_gen_utils');

const shortid = require('shortid');
shortid.characters('0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_$'); // Replace default dictionary (had invalid characters)

// TODO directly use correct m68k_read_X/m68k_write_X func
// TODO how to ease disassembly on the C side from here? (tempalte mnemonics?)

// Generate a unique variable name for effective addresses.
//
// Some operands require memory fetches that are automatically
// prepended to the instruction code. The variable holding the
// value of the effective address is given a unique name that 
// is also used by the get/set functions.

function ea_id()
{
    return `ea_${shortid.generate()}`;
};

// The following functions generate code for each operand type.
//
// Return an object with:
//   - fetch: generate code to fetch the effective address (optional)
//   - pre: generate code to run before the body of the instruction (optional)
//   - post: generate code to run after the body of the instruction (optional)
//   - get: generate code to get the value
//   - set: generate code to set the value
//   - str: string representation of the operand
//
// The memory fetching code will be automatically prepended to the
// instruction code if necessary.

function data_reg(size, n)
{
    return {
        get: () => `(ctx->data_registers[${n}] & ${u.size_masks[size]})`,
        set: (val) => `ctx->data_registers[${n}] = (ctx->data_registers[${n}] & ${u.size_antimasks[size]}) | ((${val}) & ${u.size_masks[size]})`,
        str: () => `D${n}`
    };
}

function addr_reg(size, n)
{
    return {
        get: () => `(ctx->address_registers[${n}] & ${u.size_masks[size]})`,
        set: (val) => `ctx->address_registers[${n}] = (ctx->address_registers[${n}] & ${u.size_antimasks[size]}) | ((${val}) & ${u.size_masks[size]})`,
        str: () => `A${n}`
    };
}

function addr(size, n)
{
    return {
        get: () => `m68k_read(ctx, ${u.size_enum[size]}, ctx->address_registers[${n}])`,
        set: (val) => `m68k_write(ctx, ${u.size_enum[size]}, ctx->address_registers[${n}], ${val})`,
        str: () => `(A${n})`
    };
}

function addr_postinc(size, n)
{
    return {
        post: () => `ctx->address_registers[${n}] += ${u.size_bytes[size]};`,
        get: () => `m68k_read(ctx, ${u.size_enum[size]}, ctx->address_registers[${n}])`,
        set: (val) => `m68k_write(ctx, ${u.size_enum[size]}, ctx->address_registers[${n}], ${val})`,
        str: () => `(A${n})+`
    };
}

function addr_predec(size, n)
{
    return {
        pre: () => `ctx->address_registers[${n}] -= ${u.size_bytes[size]};`,
        get: () => `m68k_read(ctx, ${u.size_enum[size]}, ctx->address_registers[${n}])`,
        set: (val) => `m68k_write(ctx, ${u.size_enum[size]}, ctx->address_registers[${n}], ${val})`,
        str: () => `-(A${n})`
    };
}

function addr_disp(size, n)
{
    const id = ea_id();
    return {
        fetch: () => `uint32_t ${id} = ctx->address_registers[${n}] + (int16_t)m68k_fetch(ctx);`,
        get: () => `m68k_read(ctx, ${u.size_enum[size]}, ${id})`,
        set: (val) => `m68k_write(ctx, ${u.size_enum[size]}, ${id}, ${val})`,
        str: () => `(d, A${n})`
    };
}

function addr_index(size, n)
{
    const id = ea_id();
    return {
        fetch: () => `
            uint32_t ext_${id} = m68k_fetch(ctx);
            uint32_t index_${id} = BIT(ext_${id}, 11) ? INDEX_REGISTER(ext_${id}): SIGN_EXTEND_W(INDEX_REGISTER(ext_${id}));
            uint32_t ${id} = ctx->address_registers[${n}]+(int8_t) FRAGMENT(ext_${id}, 7, 0) +(int32_t) index_${id}; `,
        get: () => `m68k_read(ctx, ${u.size_enum[size]}, ${id})`,
        set: (val) => `m68k_write(ctx, ${u.size_enum[size]}, ${id}, ${val})`,
        str: () => `(d, A${n}, X)`
    };
}

function pc_disp(size)
{
    const id = ea_id();
    return {
        fetch: () => `uint32_t ${id} = ctx->instruction_address + 2 + (int16_t)m68k_fetch(ctx);`,
        get: () => `m68k_read(ctx, ${u.size_enum[size]}, ${id})`,
        set: (val) => `m68k_write(ctx, ${u.size_enum[size]}, ${id})`,
        str: () => `(d, PC)`
    };
}

function pc_index(size)
{
    const id = ea_id();
    return {
        fetch: () => `
            uint32_t ext_${id} = m68k_fetch(ctx);
            uint32_t index_${id} = INDEX_LENGTH(ext_${id}) ? INDEX_REGISTER(ext_${id}): SIGN_EXTEND_W(INDEX_REGISTER(ext_${id}));
            uint32_t ${id} = ctx->instruction_address +2 +(int8_t) INDEX_DISPLACEMENT(ext_${id}) +(int32_t) index_${id}; `,
        get: () => `m68k_read(ctx, ${u.size_enum[size]}, ${id})`,
        set: (val) => `m68k_write(ctx, ${u.size_enum[size]}, ${id})`,
        str: () => `(d, PC, X)`
    };
}

function abs_word(size)
{
    const id = ea_id();
    return {
        fetch: () => `uint16_t ${id} = SIGN_EXTEND_W(m68k_fetch(ctx));`,
        get: () => `m68k_read(ctx, ${u.size_enum[size]}, ${id})`,
        set: (val) => `m68k_write(ctx, ${u.size_enum[size]}, ${id}, ${val})`,
        str: () => `(xxx).w`
    };
}

function abs_long(size)
{
    const id = ea_id();
    return {
        fetch: () => `uint32_t ${id} = (m68k_fetch(ctx) << 16) | m68k_fetch(ctx);`,
        get: () => `m68k_read(ctx, ${u.size_enum[size]}, ${id})`,
        set: (val) => `m68k_write(ctx, ${u.size_enum[size]}, ${id}, ${val})`,
        str: () => `(xxx).l`
    };
}

function imm(size)
{
    const id = ea_id();
    switch (size)
    {
        case 0:
            return {
                fetch: () => `uint16_t ${id} = m68k_fetch(ctx);`,
                get: () => `${id} & 0xFF`,
                str: () => `#xxx`
            };
        case 1:
            return {
                fetch: () => `uint16_t ${id} = m68k_fetch(ctx);`,
                get: () => id,
                str: () => `#xxx`
            };
        case 2:
            return {
                fetch: () => `uint32_t ${id} = (m68k_fetch(ctx) << 16) | m68k_fetch(ctx);`,
                get: () => id,
                str: () => `#xxx`
            };
    }
}

// Associates addressing modes with generators.
const operand_generators =
{
    [u.mode_mask('DataReg')]: data_reg,
    [u.mode_mask('AddrReg')]: addr_reg,
    [u.mode_mask('Addr')]: addr,
    [u.mode_mask('AddrPostInc')]: addr_postinc,
    [u.mode_mask('AddrPreDec')]: addr_predec,
    [u.mode_mask('AddrDisp')]: addr_disp,
    [u.mode_mask('AddrIndex')]: addr_index,
    [u.mode_mask('PCDisp')]: pc_disp,
    [u.mode_mask('PCIndex')]: pc_index,
    [u.mode_mask('AbsWord')]: abs_word,
    [u.mode_mask('AbsLong')]: abs_long,
    [u.mode_mask('Imm')]: imm
};

// Returns the operand that corresponds to the given MMMXXX pattern.
function operand_from_pattern(pattern, size)
{
    return operand_generators[u.mode_from_pattern(pattern)](size, pattern & 0b111);
}

module.exports =
{
    operand_from_pattern,
    data_reg,
    addr_reg
};
