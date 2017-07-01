const u = require('./m68k_gen_utils');
const shortid = require('shortid');

// TODO directly use correct m68k_read_X/m68k_write_X func
// TODO how to ease disassembly on the C side from here? (tempalte mnemonics?)

// Replace default dictionary (had invalid characters)
shortid.characters('0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_$');

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
//   - get: generate code to get the value
//   - set: generate code to set the value
//   - str: string representation of the operand
//
// The memory fetching code will be automatically prepended to the
// instruction code if necessary.

function data_reg(n, size)
{
    return {
        get: () => `(ctx->data_registers[${n}] & ${u.size_masks[size]})`,
        set: (val) => `ctx->data_registers[${n}] = (ctx->data_registers[${n}] & ${u.size_antimasks[size]}) | ((${val}) & ${u.size_masks[size]})`,
        str: () => `D${n}`
    };
}

function addr_reg(n, size)
{
    return {
        get: () => `(ctx->address_registers[${n}] & ${u.size_masks[size]})`,
        set: (val) => `ctx->address_registers[${n}] = (ctx->address_registers[${n}] & ${u.size_antimasks[size]}) | ((${val}) & ${u.size_masks[size]})`,
        str: () => `A${n}`
    };
}

function addr(n, size)
{
    return {
        get: () => `m68k_read(ctx, ${u.size_enum[size]}, ctx->address_registers[${n}])`,
        set: (val) => `m68k_write(ctx, ${u.size_enum[size]}, ctx->address_registers[${n}], ${val})`,
        str: () => `(A${n})`
    };
}

// TODO pre/post actions
function addr_postinc(n, size)
{
    return {
        get: () => `m68k_read(ctx, ${u.size_enum[size]}, ctx->address_registers[${n}])`,
        set: (val) => `m68k_write(ctx, ${u.size_enum[size]}, ctx->address_registers[${n}], ${val})`,
        str: () => `(A${n})+`
    };
}

function addr_predec(n, size)
{
    return {
        get: () => `m68k_read(ctx, ${u.size_enum[size]}, ctx->address_registers[${n}])`,
        set: (val) => `m68k_write(ctx, ${u.size_enum[size]}, ctx->address_registers[${n}], ${val})`,
        str: () => `-(A${n})`
    };
}

function addr_disp(n, size)
{
    const id = ea_id();
    return {
        fetch: () => `uint32_t ${id} = ctx->address_registers[${n}] + (int16_t)m68k_fetch(ctx);`,
        get: () => `m68k_read(ctx, ${u.size_enum[size]}, ${id})`,
        set: (val) => `m68k_write(ctx, ${u.size_enum[size]}, ${id})`,
        str: () => `(d, A${n})`
    };
}

function addr_index(n, size)
{
    const id = ea_id();
    return {
        fetch: () => `
            uint32_t ext = m68k_fetch(ctx);
            uint32_t index = BIT(ext, 11) ? INDEX_REGISTER(ext): SIGN_EXTEND_W(INDEX_REGISTER(ext));
            uint32_t ${id} = ctx->address_registers[${n}] + (int8_t)FRAGMENT(ext, 7, 0) + (int32_t) index;`,
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
            uint32_t ext = m68k_fetch(ctx);
            uint32_t index = INDEX_LENGTH(ext) ? INDEX_REGISTER(ext) : SIGN_EXTEND_W(INDEX_REGISTER(ext));
            uint32_t ${id} = ctx->instruction_address + 2 + (int8_t)INDEX_DISPLACEMENT(ext) + (int32_t)index;`,
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
        fetch: () => `uint32_t ${id} = m68k_fetch(ctx) << 16 | m68k_fetch(ctx);`,
        get: () => `m68k_read(ctx, ${u.size_enum[size]}, ${id})`,
        set: (val) => `m68k_write(ctx, ${u.size_enum[size]}, ${id}, ${val})`,
        str: () => `(xxx).l`
    };
}

function imm(size)
{
    switch (size)
    {
        case 0:
            return {
                fetch: () => 'm68k_fetch(ctx);',
                get: () => `m68k_read_w(ctx, ctx->instruction_address) & 0xFF`,
                str: () => `#xxx`
            };
        case 1:
            return {
                fetch: () => 'm68k_fetch(ctx);',
                get: () => `m68k_read_w(ctx, ctx->instruction_address)`,
                str: () => `#xxx`
            };
        case 2:
            return {
                fetch: () => 'm68k_fetch(ctx); m68k_fetch(ctx);',
                get: () => `m68k_read_l(ctx, ctx->instruction_address)`,
                str: () => `#xxx`
            };
    }
}

// Return the operand corresponding to the MMMXXX pattern
// that is often part of opcodes.
function operand_from_pattern(pattern, size)
{
    switch (pattern & 0b111000)
    {
        case 0b000000: return data_reg(pattern & 0b111, size);
        case 0b001000: return addr_reg(pattern & 0b111, size);
        case 0b010000: return addr(pattern & 0b111, size);
        case 0b011000: return addr_postinc(pattern & 0b111, size);
        case 0b100000: return addr_predec(pattern & 0b111, size);
        case 0b101000: return addr_disp(pattern & 0b111, size);
        case 0b110000: return addr_index(pattern & 0b111, size);

        case 0b111000:
            switch (pattern & 0b111)
            {
                case 0b010: return pc_disp(size);
                case 0b011: return pc_index(size);
                case 0b000: return abs_word(size);
                case 0b001: return abs_long(size);
                case 0b100: return imm(size);
                default: throw `Invalid operand pattern ${pattern.toString(2)}`;
            }

        default: throw `Invalid operand pattern ${pattern.toString(2)}`;
    }
}

module.exports =
{
    operand_from_pattern,
    data_reg,
    addr_reg
};
