[0x0000] = { 1, "NOP ", None, None },
[0x0001] = { 3, "LD BC, $%04x", None, UnsignedWord },
[0x0002] = { 1, "LD (BC), A", None, None },
[0x0003] = { 1, "INC BC", None, None },
[0x0004] = { 1, "INC B", None, None },
[0x0005] = { 1, "DEC B", None, None },
[0x0006] = { 2, "LD B, $%02x", None, Unsigned },
[0x0007] = { 1, "RLCA ", None, None },
[0x0008] = { 1, "EX AF, AF'", None, None },
[0x0009] = { 1, "ADD HL, BC", None, None },
[0x000a] = { 1, "LD A, (BC)", None, None },
[0x000b] = { 1, "DEC BC", None, None },
[0x000c] = { 1, "INC C", None, None },
[0x000d] = { 1, "DEC C", None, None },
[0x000e] = { 2, "LD C, $%02x", None, Unsigned },
[0x000f] = { 1, "RRCA ", None, None },
[0x0010] = { 2, "DJNZ PC%+d [$%04x]", Relative, None },
[0x0011] = { 3, "LD DE, $%04x", None, UnsignedWord },
[0x0012] = { 1, "LD (DE), A", None, None },
[0x0013] = { 1, "INC DE", None, None },
[0x0014] = { 1, "INC D", None, None },
[0x0015] = { 1, "DEC D", None, None },
[0x0016] = { 2, "LD D, $%02x", None, Unsigned },
[0x0017] = { 1, "RLA ", None, None },
[0x0018] = { 2, "JR PC%+d [$%04x]", Relative, None },
[0x0019] = { 1, "ADD HL, DE", None, None },
[0x001a] = { 1, "LD A, (DE)", None, None },
[0x001b] = { 1, "DEC DE", None, None },
[0x001c] = { 1, "INC E", None, None },
[0x001d] = { 1, "DEC E", None, None },
[0x001e] = { 2, "LD E, $%02x", None, Unsigned },
[0x001f] = { 1, "RRA ", None, None },
[0x0020] = { 2, "JR NZ, PC%+d [$%04x]", None, Relative },
[0x0021] = { 3, "LD HL, $%04x", None, UnsignedWord },
[0x0022] = { 3, "LD ($%04x), HL", UnsignedWord, None },
[0x0023] = { 1, "INC HL", None, None },
[0x0024] = { 1, "INC H", None, None },
[0x0025] = { 1, "DEC H", None, None },
[0x0026] = { 2, "LD H, $%02x", None, Unsigned },
[0x0027] = { 1, "DAA ", None, None },
[0x0028] = { 2, "JR Z, PC%+d [$%04x]", None, Relative },
[0x0029] = { 1, "ADD HL, HL", None, None },
[0x002a] = { 3, "LD HL, ($%04x)", None, UnsignedWord },
[0x002b] = { 1, "DEC HL", None, None },
[0x002c] = { 1, "INC L", None, None },
[0x002d] = { 1, "DEC L", None, None },
[0x002e] = { 2, "LD L, $%02x", None, Unsigned },
[0x002f] = { 1, "CPL ", None, None },
[0x0030] = { 2, "JR NC, PC%+d [$%04x]", None, Relative },
[0x0031] = { 3, "LD SP, $%04x", None, UnsignedWord },
[0x0032] = { 3, "LD ($%04x), A", UnsignedWord, None },
[0x0033] = { 1, "INC SP", None, None },
[0x0034] = { 1, "INC (HL)", None, None },
[0x0035] = { 1, "DEC (HL)", None, None },
[0x0036] = { 2, "LD (HL), $%02x", None, Unsigned },
[0x0037] = { 1, "SCF ", None, None },
[0x0038] = { 2, "JR Cy, PC%+d [$%04x]", None, Relative },
[0x0039] = { 1, "ADD HL, SP", None, None },
[0x003a] = { 3, "LD A, ($%04x)", None, UnsignedWord },
[0x003b] = { 1, "DEC SP", None, None },
[0x003c] = { 1, "INC A", None, None },
[0x003d] = { 1, "DEC A", None, None },
[0x003e] = { 2, "LD A, $%02x", None, Unsigned },
[0x003f] = { 1, "CCF ", None, None },
[0x0040] = { 1, "LD B, B", None, None },
[0x0041] = { 1, "LD B, C", None, None },
[0x0042] = { 1, "LD B, D", None, None },
[0x0043] = { 1, "LD B, E", None, None },
[0x0044] = { 1, "LD B, H", None, None },
[0x0045] = { 1, "LD B, L", None, None },
[0x0046] = { 1, "LD B, (HL)", None, None },
[0x0047] = { 1, "LD B, A", None, None },
[0x0048] = { 1, "LD C, B", None, None },
[0x0049] = { 1, "LD C, C", None, None },
[0x004a] = { 1, "LD C, D", None, None },
[0x004b] = { 1, "LD C, E", None, None },
[0x004c] = { 1, "LD C, H", None, None },
[0x004d] = { 1, "LD C, L", None, None },
[0x004e] = { 1, "LD C, (HL)", None, None },
[0x004f] = { 1, "LD C, A", None, None },
[0x0050] = { 1, "LD D, B", None, None },
[0x0051] = { 1, "LD D, C", None, None },
[0x0052] = { 1, "LD D, D", None, None },
[0x0053] = { 1, "LD D, E", None, None },
[0x0054] = { 1, "LD D, H", None, None },
[0x0055] = { 1, "LD D, L", None, None },
[0x0056] = { 1, "LD D, (HL)", None, None },
[0x0057] = { 1, "LD D, A", None, None },
[0x0058] = { 1, "LD E, B", None, None },
[0x0059] = { 1, "LD E, C", None, None },
[0x005a] = { 1, "LD E, D", None, None },
[0x005b] = { 1, "LD E, E", None, None },
[0x005c] = { 1, "LD E, H", None, None },
[0x005d] = { 1, "LD E, L", None, None },
[0x005e] = { 1, "LD E, (HL)", None, None },
[0x005f] = { 1, "LD E, A", None, None },
[0x0060] = { 1, "LD H, B", None, None },
[0x0061] = { 1, "LD H, C", None, None },
[0x0062] = { 1, "LD H, D", None, None },
[0x0063] = { 1, "LD H, E", None, None },
[0x0064] = { 1, "LD H, H", None, None },
[0x0065] = { 1, "LD H, L", None, None },
[0x0066] = { 1, "LD H, (HL)", None, None },
[0x0067] = { 1, "LD H, A", None, None },
[0x0068] = { 1, "LD L, B", None, None },
[0x0069] = { 1, "LD L, C", None, None },
[0x006a] = { 1, "LD L, D", None, None },
[0x006b] = { 1, "LD L, E", None, None },
[0x006c] = { 1, "LD L, H", None, None },
[0x006d] = { 1, "LD L, L", None, None },
[0x006e] = { 1, "LD L, (HL)", None, None },
[0x006f] = { 1, "LD L, A", None, None },
[0x0070] = { 1, "LD (HL), B", None, None },
[0x0071] = { 1, "LD (HL), C", None, None },
[0x0072] = { 1, "LD (HL), D", None, None },
[0x0073] = { 1, "LD (HL), E", None, None },
[0x0074] = { 1, "LD (HL), H", None, None },
[0x0075] = { 1, "LD (HL), L", None, None },
[0x0076] = { 1, "HALT ", None, None },
[0x0077] = { 1, "LD (HL), A", None, None },
[0x0078] = { 1, "LD A, B", None, None },
[0x0079] = { 1, "LD A, C", None, None },
[0x007a] = { 1, "LD A, D", None, None },
[0x007b] = { 1, "LD A, E", None, None },
[0x007c] = { 1, "LD A, H", None, None },
[0x007d] = { 1, "LD A, L", None, None },
[0x007e] = { 1, "LD A, (HL)", None, None },
[0x007f] = { 1, "LD A, A", None, None },
[0x0080] = { 1, "ADD A, B", None, None },
[0x0081] = { 1, "ADD A, C", None, None },
[0x0082] = { 1, "ADD A, D", None, None },
[0x0083] = { 1, "ADD A, E", None, None },
[0x0084] = { 1, "ADD A, H", None, None },
[0x0085] = { 1, "ADD A, L", None, None },
[0x0086] = { 1, "ADD A, (HL)", None, None },
[0x0087] = { 1, "ADD A, A", None, None },
[0x0088] = { 1, "ADC A, B", None, None },
[0x0089] = { 1, "ADC A, C", None, None },
[0x008a] = { 1, "ADC A, D", None, None },
[0x008b] = { 1, "ADC A, E", None, None },
[0x008c] = { 1, "ADC A, H", None, None },
[0x008d] = { 1, "ADC A, L", None, None },
[0x008e] = { 1, "ADC A, (HL)", None, None },
[0x008f] = { 1, "ADC A, A", None, None },
[0x0090] = { 1, "SUB B", None, None },
[0x0091] = { 1, "SUB C", None, None },
[0x0092] = { 1, "SUB D", None, None },
[0x0093] = { 1, "SUB E", None, None },
[0x0094] = { 1, "SUB H", None, None },
[0x0095] = { 1, "SUB L", None, None },
[0x0096] = { 1, "SUB (HL)", None, None },
[0x0097] = { 1, "SUB A", None, None },
[0x0098] = { 1, "SBC A, B", None, None },
[0x0099] = { 1, "SBC A, C", None, None },
[0x009a] = { 1, "SBC A, D", None, None },
[0x009b] = { 1, "SBC A, E", None, None },
[0x009c] = { 1, "SBC A, H", None, None },
[0x009d] = { 1, "SBC A, L", None, None },
[0x009e] = { 1, "SBC A, (HL)", None, None },
[0x009f] = { 1, "SBC A, A", None, None },
[0x00a0] = { 1, "AND B", None, None },
[0x00a1] = { 1, "AND C", None, None },
[0x00a2] = { 1, "AND D", None, None },
[0x00a3] = { 1, "AND E", None, None },
[0x00a4] = { 1, "AND H", None, None },
[0x00a5] = { 1, "AND L", None, None },
[0x00a6] = { 1, "AND (HL)", None, None },
[0x00a7] = { 1, "AND A", None, None },
[0x00a8] = { 1, "XOR B", None, None },
[0x00a9] = { 1, "XOR C", None, None },
[0x00aa] = { 1, "XOR D", None, None },
[0x00ab] = { 1, "XOR E", None, None },
[0x00ac] = { 1, "XOR H", None, None },
[0x00ad] = { 1, "XOR L", None, None },
[0x00ae] = { 1, "XOR (HL)", None, None },
[0x00af] = { 1, "XOR A", None, None },
[0x00b0] = { 1, "OR B", None, None },
[0x00b1] = { 1, "OR C", None, None },
[0x00b2] = { 1, "OR D", None, None },
[0x00b3] = { 1, "OR E", None, None },
[0x00b4] = { 1, "OR H", None, None },
[0x00b5] = { 1, "OR L", None, None },
[0x00b6] = { 1, "OR (HL)", None, None },
[0x00b7] = { 1, "OR A", None, None },
[0x00b8] = { 1, "CP B", None, None },
[0x00b9] = { 1, "CP C", None, None },
[0x00ba] = { 1, "CP D", None, None },
[0x00bb] = { 1, "CP E", None, None },
[0x00bc] = { 1, "CP H", None, None },
[0x00bd] = { 1, "CP L", None, None },
[0x00be] = { 1, "CP (HL)", None, None },
[0x00bf] = { 1, "CP A", None, None },
[0x00c0] = { 1, "RET NZ", None, None },
[0x00c1] = { 1, "POP BC", None, None },
[0x00c2] = { 3, "JP NZ, $%04x", None, UnsignedWord },
[0x00c3] = { 3, "JP $%04x", UnsignedWord, None },
[0x00c4] = { 3, "CALL NZ, $%04x", None, UnsignedWord },
[0x00c5] = { 1, "PUSH BC", None, None },
[0x00c6] = { 2, "ADD A, $%02x", None, Unsigned },
[0x00c7] = { 1, "RST $00", Unsigned, None },
[0x00c8] = { 1, "RET Z", None, None },
[0x00c9] = { 1, "RET ", None, None },
[0x00ca] = { 3, "JP Z, $%04x", None, UnsignedWord },
[0xcb00] = { 1, "RLC B", None, None },
[0xcb01] = { 1, "RLC C", None, None },
[0xcb02] = { 1, "RLC D", None, None },
[0xcb03] = { 1, "RLC E", None, None },
[0xcb04] = { 1, "RLC H", None, None },
[0xcb05] = { 1, "RLC L", None, None },
[0xcb06] = { 1, "RLC (HL)", None, None },
[0xcb07] = { 1, "RLC A", None, None },
[0xcb08] = { 1, "RRC B", None, None },
[0xcb09] = { 1, "RRC C", None, None },
[0xcb0a] = { 1, "RRC D", None, None },
[0xcb0b] = { 1, "RRC E", None, None },
[0xcb0c] = { 1, "RRC H", None, None },
[0xcb0d] = { 1, "RRC L", None, None },
[0xcb0e] = { 1, "RRC (HL)", None, None },
[0xcb0f] = { 1, "RRC A", None, None },
[0xcb10] = { 1, "RL B", None, None },
[0xcb11] = { 1, "RL C", None, None },
[0xcb12] = { 1, "RL D", None, None },
[0xcb13] = { 1, "RL E", None, None },
[0xcb14] = { 1, "RL H", None, None },
[0xcb15] = { 1, "RL L", None, None },
[0xcb16] = { 1, "RL (HL)", None, None },
[0xcb17] = { 1, "RL A", None, None },
[0xcb18] = { 1, "RR B", None, None },
[0xcb19] = { 1, "RR C", None, None },
[0xcb1a] = { 1, "RR D", None, None },
[0xcb1b] = { 1, "RR E", None, None },
[0xcb1c] = { 1, "RR H", None, None },
[0xcb1d] = { 1, "RR L", None, None },
[0xcb1e] = { 1, "RR (HL)", None, None },
[0xcb1f] = { 1, "RR A", None, None },
[0xcb20] = { 1, "SLA B", None, None },
[0xcb21] = { 1, "SLA C", None, None },
[0xcb22] = { 1, "SLA D", None, None },
[0xcb23] = { 1, "SLA E", None, None },
[0xcb24] = { 1, "SLA H", None, None },
[0xcb25] = { 1, "SLA L", None, None },
[0xcb26] = { 1, "SLA (HL)", None, None },
[0xcb27] = { 1, "SLA A", None, None },
[0xcb28] = { 1, "SRA B", None, None },
[0xcb29] = { 1, "SRA C", None, None },
[0xcb2a] = { 1, "SRA D", None, None },
[0xcb2b] = { 1, "SRA E", None, None },
[0xcb2c] = { 1, "SRA H", None, None },
[0xcb2d] = { 1, "SRA L", None, None },
[0xcb2e] = { 1, "SRA (HL)", None, None },
[0xcb2f] = { 1, "SRA A", None, None },
[0xcb38] = { 1, "SRL B", None, None },
[0xcb39] = { 1, "SRL C", None, None },
[0xcb3a] = { 1, "SRL D", None, None },
[0xcb3b] = { 1, "SRL E", None, None },
[0xcb3c] = { 1, "SRL H", None, None },
[0xcb3d] = { 1, "SRL L", None, None },
[0xcb3e] = { 1, "SRL (HL)", None, None },
[0xcb3f] = { 1, "SRL A", None, None },
[0xcb40] = { 1, "BIT 0, B", None, None },
[0xcb41] = { 1, "BIT 0, C", None, None },
[0xcb42] = { 1, "BIT 0, D", None, None },
[0xcb43] = { 1, "BIT 0, E", None, None },
[0xcb44] = { 1, "BIT 0, H", None, None },
[0xcb45] = { 1, "BIT 0, L", None, None },
[0xcb46] = { 1, "BIT 0, (HL)", None, None },
[0xcb47] = { 1, "BIT 0, A", None, None },
[0xcb48] = { 1, "BIT 1, B", None, None },
[0xcb49] = { 1, "BIT 1, C", None, None },
[0xcb4a] = { 1, "BIT 1, D", None, None },
[0xcb4b] = { 1, "BIT 1, E", None, None },
[0xcb4c] = { 1, "BIT 1, H", None, None },
[0xcb4d] = { 1, "BIT 1, L", None, None },
[0xcb4e] = { 1, "BIT 1, (HL)", None, None },
[0xcb4f] = { 1, "BIT 1, A", None, None },
[0xcb50] = { 1, "BIT 2, B", None, None },
[0xcb51] = { 1, "BIT 2, C", None, None },
[0xcb52] = { 1, "BIT 2, D", None, None },
[0xcb53] = { 1, "BIT 2, E", None, None },
[0xcb54] = { 1, "BIT 2, H", None, None },
[0xcb55] = { 1, "BIT 2, L", None, None },
[0xcb56] = { 1, "BIT 2, (HL)", None, None },
[0xcb57] = { 1, "BIT 2, A", None, None },
[0xcb58] = { 1, "BIT 3, B", None, None },
[0xcb59] = { 1, "BIT 3, C", None, None },
[0xcb5a] = { 1, "BIT 3, D", None, None },
[0xcb5b] = { 1, "BIT 3, E", None, None },
[0xcb5c] = { 1, "BIT 3, H", None, None },
[0xcb5d] = { 1, "BIT 3, L", None, None },
[0xcb5e] = { 1, "BIT 3, (HL)", None, None },
[0xcb5f] = { 1, "BIT 3, A", None, None },
[0xcb60] = { 1, "BIT 4, B", None, None },
[0xcb61] = { 1, "BIT 4, C", None, None },
[0xcb62] = { 1, "BIT 4, D", None, None },
[0xcb63] = { 1, "BIT 4, E", None, None },
[0xcb64] = { 1, "BIT 4, H", None, None },
[0xcb65] = { 1, "BIT 4, L", None, None },
[0xcb66] = { 1, "BIT 4, (HL)", None, None },
[0xcb67] = { 1, "BIT 4, A", None, None },
[0xcb68] = { 1, "BIT 5, B", None, None },
[0xcb69] = { 1, "BIT 5, C", None, None },
[0xcb6a] = { 1, "BIT 5, D", None, None },
[0xcb6b] = { 1, "BIT 5, E", None, None },
[0xcb6c] = { 1, "BIT 5, H", None, None },
[0xcb6d] = { 1, "BIT 5, L", None, None },
[0xcb6e] = { 1, "BIT 5, (HL)", None, None },
[0xcb6f] = { 1, "BIT 5, A", None, None },
[0xcb70] = { 1, "BIT 6, B", None, None },
[0xcb71] = { 1, "BIT 6, C", None, None },
[0xcb72] = { 1, "BIT 6, D", None, None },
[0xcb73] = { 1, "BIT 6, E", None, None },
[0xcb74] = { 1, "BIT 6, H", None, None },
[0xcb75] = { 1, "BIT 6, L", None, None },
[0xcb76] = { 1, "BIT 6, (HL)", None, None },
[0xcb77] = { 1, "BIT 6, A", None, None },
[0xcb78] = { 1, "BIT 7, B", None, None },
[0xcb79] = { 1, "BIT 7, C", None, None },
[0xcb7a] = { 1, "BIT 7, D", None, None },
[0xcb7b] = { 1, "BIT 7, E", None, None },
[0xcb7c] = { 1, "BIT 7, H", None, None },
[0xcb7d] = { 1, "BIT 7, L", None, None },
[0xcb7e] = { 1, "BIT 7, (HL)", None, None },
[0xcb7f] = { 1, "BIT 7, A", None, None },
[0xcb80] = { 1, "RES 0, B", None, None },
[0xcb81] = { 1, "RES 0, C", None, None },
[0xcb82] = { 1, "RES 0, D", None, None },
[0xcb83] = { 1, "RES 0, E", None, None },
[0xcb84] = { 1, "RES 0, H", None, None },
[0xcb85] = { 1, "RES 0, L", None, None },
[0xcb86] = { 1, "RES 0, (HL)", None, None },
[0xcb87] = { 1, "RES 0, A", None, None },
[0xcb88] = { 1, "RES 1, B", None, None },
[0xcb89] = { 1, "RES 1, C", None, None },
[0xcb8a] = { 1, "RES 1, D", None, None },
[0xcb8b] = { 1, "RES 1, E", None, None },
[0xcb8c] = { 1, "RES 1, H", None, None },
[0xcb8d] = { 1, "RES 1, L", None, None },
[0xcb8e] = { 1, "RES 1, (HL)", None, None },
[0xcb8f] = { 1, "RES 1, A", None, None },
[0xcb90] = { 1, "RES 2, B", None, None },
[0xcb91] = { 1, "RES 2, C", None, None },
[0xcb92] = { 1, "RES 2, D", None, None },
[0xcb93] = { 1, "RES 2, E", None, None },
[0xcb94] = { 1, "RES 2, H", None, None },
[0xcb95] = { 1, "RES 2, L", None, None },
[0xcb96] = { 1, "RES 2, (HL)", None, None },
[0xcb97] = { 1, "RES 2, A", None, None },
[0xcb98] = { 1, "RES 3, B", None, None },
[0xcb99] = { 1, "RES 3, C", None, None },
[0xcb9a] = { 1, "RES 3, D", None, None },
[0xcb9b] = { 1, "RES 3, E", None, None },
[0xcb9c] = { 1, "RES 3, H", None, None },
[0xcb9d] = { 1, "RES 3, L", None, None },
[0xcb9e] = { 1, "RES 3, (HL)", None, None },
[0xcb9f] = { 1, "RES 3, A", None, None },
[0xcba0] = { 1, "RES 4, B", None, None },
[0xcba1] = { 1, "RES 4, C", None, None },
[0xcba2] = { 1, "RES 4, D", None, None },
[0xcba3] = { 1, "RES 4, E", None, None },
[0xcba4] = { 1, "RES 4, H", None, None },
[0xcba5] = { 1, "RES 4, L", None, None },
[0xcba6] = { 1, "RES 4, (HL)", None, None },
[0xcba7] = { 1, "RES 4, A", None, None },
[0xcba8] = { 1, "RES 5, B", None, None },
[0xcba9] = { 1, "RES 5, C", None, None },
[0xcbaa] = { 1, "RES 5, D", None, None },
[0xcbab] = { 1, "RES 5, E", None, None },
[0xcbac] = { 1, "RES 5, H", None, None },
[0xcbad] = { 1, "RES 5, L", None, None },
[0xcbae] = { 1, "RES 5, (HL)", None, None },
[0xcbaf] = { 1, "RES 5, A", None, None },
[0xcbb0] = { 1, "RES 6, B", None, None },
[0xcbb1] = { 1, "RES 6, C", None, None },
[0xcbb2] = { 1, "RES 6, D", None, None },
[0xcbb3] = { 1, "RES 6, E", None, None },
[0xcbb4] = { 1, "RES 6, H", None, None },
[0xcbb5] = { 1, "RES 6, L", None, None },
[0xcbb6] = { 1, "RES 6, (HL)", None, None },
[0xcbb7] = { 1, "RES 6, A", None, None },
[0xcbb8] = { 1, "RES 7, B", None, None },
[0xcbb9] = { 1, "RES 7, C", None, None },
[0xcbba] = { 1, "RES 7, D", None, None },
[0xcbbb] = { 1, "RES 7, E", None, None },
[0xcbbc] = { 1, "RES 7, H", None, None },
[0xcbbd] = { 1, "RES 7, L", None, None },
[0xcbbe] = { 1, "RES 7, (HL)", None, None },
[0xcbbf] = { 1, "RES 7, A", None, None },
[0xcbc0] = { 1, "SET 0, B", None, None },
[0xcbc1] = { 1, "SET 0, C", None, None },
[0xcbc2] = { 1, "SET 0, D", None, None },
[0xcbc3] = { 1, "SET 0, E", None, None },
[0xcbc4] = { 1, "SET 0, H", None, None },
[0xcbc5] = { 1, "SET 0, L", None, None },
[0xcbc6] = { 1, "SET 0, (HL)", None, None },
[0xcbc7] = { 1, "SET 0, A", None, None },
[0xcbc8] = { 1, "SET 1, B", None, None },
[0xcbc9] = { 1, "SET 1, C", None, None },
[0xcbca] = { 1, "SET 1, D", None, None },
[0xcbcc] = { 1, "SET 1, H", None, None },
[0xcbcd] = { 1, "SET 1, L", None, None },
[0xcbce] = { 1, "SET 1, (HL)", None, None },
[0xcbcf] = { 1, "SET 1, A", None, None },
[0xcbd0] = { 1, "SET 2, B", None, None },
[0xcbd1] = { 1, "SET 2, C", None, None },
[0xcbd2] = { 1, "SET 2, D", None, None },
[0xcbd3] = { 1, "SET 2, E", None, None },
[0xcbd4] = { 1, "SET 2, H", None, None },
[0xcbd5] = { 1, "SET 2, L", None, None },
[0xcbd6] = { 1, "SET 2, (HL)", None, None },
[0xcbd7] = { 1, "SET 2, A", None, None },
[0xcbd8] = { 1, "SET 3, B", None, None },
[0xcbd9] = { 1, "SET 3, C", None, None },
[0xcbda] = { 1, "SET 3, D", None, None },
[0xcbdb] = { 1, "SET 3, E", None, None },
[0xcbdc] = { 1, "SET 3, H", None, None },
[0xcbdd] = { 1, "SET 3, L", None, None },
[0xcbde] = { 1, "SET 3, (HL)", None, None },
[0xcbdf] = { 1, "SET 3, A", None, None },
[0xcbe0] = { 1, "SET 4, B", None, None },
[0xcbe1] = { 1, "SET 4, C", None, None },
[0xcbe2] = { 1, "SET 4, D", None, None },
[0xcbe3] = { 1, "SET 4, E", None, None },
[0xcbe4] = { 1, "SET 4, H", None, None },
[0xcbe5] = { 1, "SET 4, L", None, None },
[0xcbe6] = { 1, "SET 4, (HL)", None, None },
[0xcbe7] = { 1, "SET 4, A", None, None },
[0xcbe8] = { 1, "SET 5, B", None, None },
[0xcbe9] = { 1, "SET 5, C", None, None },
[0xcbea] = { 1, "SET 5, D", None, None },
[0xcbeb] = { 1, "SET 5, E", None, None },
[0xcbec] = { 1, "SET 5, H", None, None },
[0xcbed] = { 1, "SET 5, L", None, None },
[0xcbee] = { 1, "SET 5, (HL)", None, None },
[0xcbef] = { 1, "SET 5, A", None, None },
[0xcbf0] = { 1, "SET 6, B", None, None },
[0xcbf1] = { 1, "SET 6, C", None, None },
[0xcbf2] = { 1, "SET 6, D", None, None },
[0xcbf3] = { 1, "SET 6, E", None, None },
[0xcbf4] = { 1, "SET 6, H", None, None },
[0xcbf5] = { 1, "SET 6, L", None, None },
[0xcbf6] = { 1, "SET 6, (HL)", None, None },
[0xcbf7] = { 1, "SET 6, A", None, None },
[0xcbf8] = { 1, "SET 7, B", None, None },
[0xcbf9] = { 1, "SET 7, C", None, None },
[0xcbfa] = { 1, "SET 7, D", None, None },
[0xcbfb] = { 1, "SET 7, E", None, None },
[0xcbfc] = { 1, "SET 7, H", None, None },
[0xcbfd] = { 1, "SET 7, L", None, None },
[0xcbfe] = { 1, "SET 7, (HL)", None, None },
[0xcbff] = { 1, "SET 7, A", None, None },
[0x00cc] = { 3, "CALL Z, $%04x", None, UnsignedWord },
[0x00cd] = { 3, "CALL $%04x", UnsignedWord, None },
[0x00ce] = { 2, "ADC A, $%02x", None, Unsigned },
[0x00cf] = { 1, "RST $08", Unsigned, None },
[0x00d0] = { 1, "RET NC", None, None },
[0x00d1] = { 1, "POP DE", None, None },
[0x00d2] = { 3, "JP NC, $%04x", None, UnsignedWord },
[0x00d3] = { 2, "OUT ($%02x), A", Unsigned, None },
[0x00d4] = { 3, "CALL NC, $%04x", None, UnsignedWord },
[0x00d5] = { 1, "PUSH DE", None, None },
[0x00d6] = { 2, "SUB $%02x", Unsigned, None },
[0x00d7] = { 1, "RST $10", Unsigned, None },
[0x00d8] = { 1, "RET Cy", None, None },
[0x00d9] = { 1, "EXX ", None, None },
[0x00da] = { 3, "JP Cy, $%04x", None, UnsignedWord },
[0x00db] = { 2, "IN A, ($%02x)", None, Unsigned },
[0x00dc] = { 3, "CALL Cy, $%04x", None, UnsignedWord },
[0xdd09] = { 1, "ADD IX, BC", None, None },
[0xdd19] = { 1, "ADD IX, DE", None, None },
[0xdd21] = { 3, "LD IX, $%04x", None, UnsignedWord },
[0xdd22] = { 3, "LD ($%04x), IX", UnsignedWord, None },
[0xdd23] = { 1, "INC IX", None, None },
[0xdd29] = { 1, "ADD IX, IX", None, None },
[0xdd2a] = { 3, "LD IX, ($%04x)", None, UnsignedWord },
[0xdd2b] = { 1, "DEC IX", None, None },
[0xdd34] = { 2, "INC (IX + %d)", Signed, None },
[0xdd35] = { 2, "DEC (IX + %d)", Signed, None },
[0xdd36] = { 3, "LD (IX + %d), $%02x", Signed, Unsigned },
[0xdd39] = { 1, "ADD IX, SP", None, None },
[0xdd46] = { 2, "LD B, (IX + %d)", None, Signed },
[0xdd4e] = { 2, "LD C, (IX + %d)", None, Signed },
[0xdd56] = { 2, "LD D, (IX + %d)", None, Signed },
[0xdd5e] = { 2, "LD E, (IX + %d)", None, Signed },
[0xdd66] = { 2, "LD H, (IX + %d)", None, Signed },
[0xdd6e] = { 2, "LD L, (IX + %d)", None, Signed },
[0xdd70] = { 2, "LD (IX + %d), B", Signed, None },
[0xdd71] = { 2, "LD (IX + %d), C", Signed, None },
[0xdd72] = { 2, "LD (IX + %d), D", Signed, None },
[0xdd73] = { 2, "LD (IX + %d), E", Signed, None },
[0xdd74] = { 2, "LD (IX + %d), H", Signed, None },
[0xdd75] = { 2, "LD (IX + %d), L", Signed, None },
[0xdd77] = { 2, "LD (IX + %d), A", Signed, None },
[0xdd7e] = { 2, "LD A, (IX + %d)", None, Signed },
[0xdd86] = { 2, "ADD A, (IX + %d)", None, Signed },
[0xdd8e] = { 2, "ADC A, (IX + %d)", None, Signed },
[0xdd96] = { 2, "SUB (IX + %d)", Signed, None },
[0xdd9e] = { 2, "SBC A, (IX + %d)", None, Signed },
[0xdda6] = { 2, "AND (IX + %d)", Signed, None },
[0xddae] = { 2, "XOR (IX + %d)", Signed, None },
[0xddb6] = { 2, "OR (IX + %d)", Signed, None },
[0xddbe] = { 2, "CP (IX + %d)", Signed, None },
[0xdde1] = { 1, "POP IX", None, None },
[0xdde3] = { 1, "EX (SP), IX", None, None },
[0xdde5] = { 1, "PUSH IX", None, None },
[0xdde9] = { 1, "JP (IX)", None, None },
[0xddf9] = { 1, "LD SP, IX", None, None },
[0x00de] = { 2, "SBC A, $%02x", None, Unsigned },
[0x00df] = { 1, "RST $18", Unsigned, None },
[0x00e0] = { 1, "RET PO", None, None },
[0x00e1] = { 1, "POP HL", None, None },
[0x00e2] = { 3, "JP PO, $%04x", None, UnsignedWord },
[0x00e3] = { 1, "EX (SP), HL", None, None },
[0x00e4] = { 3, "CALL PO, $%04x", None, UnsignedWord },
[0x00e5] = { 1, "PUSH HL", None, None },
[0x00e6] = { 2, "AND $%02x", Unsigned, None },
[0x00e7] = { 1, "RST $20", Unsigned, None },
[0x00e8] = { 1, "RET PE", None, None },
[0x00e9] = { 1, "JP (HL)", None, None },
[0x00ea] = { 3, "JP PE, ($%04x)", None, UnsignedWord },
[0x00eb] = { 1, "EX DE, HL", None, None },
[0x00ec] = { 3, "CALL PE, $%04x", None, UnsignedWord },
[0xed40] = { 1, "IN B, (C)", None, None },
[0xed41] = { 1, "OUT (C), B", None, None },
[0xed42] = { 1, "SBC HL, BC", None, None },
[0xed43] = { 3, "LD ($%04x), BC", UnsignedWord, None },
[0xed44] = { 1, "NEG ", None, None },
[0xed45] = { 1, "RETN ", None, None },
[0xed46] = { 1, "IM 0", None, None },
[0xed47] = { 1, "LD I, A", None, None },
[0xed48] = { 1, "IN C, (C)", None, None },
[0xed49] = { 1, "OUT (C), C", None, None },
[0xed4a] = { 1, "ADC HL, BC", None, None },
[0xed4b] = { 3, "LD BC, ($%04x)", None, UnsignedWord },
[0xed4d] = { 1, "RETI ", None, None },
[0xed4f] = { 1, "LD R, A", None, None },
[0xed50] = { 1, "IN D, (C)", None, None },
[0xed51] = { 1, "OUT (C), D", None, None },
[0xed52] = { 1, "SBC HL, DE", None, None },
[0xed53] = { 3, "LD ($%04x), DE", UnsignedWord, None },
[0xed56] = { 1, "IM 1", None, None },
[0xed57] = { 1, "LD A, I", None, None },
[0xed58] = { 1, "IN E, (C)", None, None },
[0xed59] = { 1, "OUT (C), E", None, None },
[0xed5a] = { 1, "ADC HL, DE", None, None },
[0xed5b] = { 3, "LD DE, ($%04x)", None, UnsignedWord },
[0xed5e] = { 1, "IM 2", None, None },
[0xed5f] = { 1, "LD A, R", None, None },
[0xed60] = { 1, "IN H, (C)", None, None },
[0xed61] = { 1, "OUT (C), H", None, None },
[0xed62] = { 1, "SBC HL, HL", None, None },
[0xed63] = { 3, "LD ($%04x), HL", UnsignedWord, None },
[0xed67] = { 1, "RRD ", None, None },
[0xed68] = { 1, "IN L, (C)", None, None },
[0xed69] = { 1, "OUT (C), L", None, None },
[0xed6a] = { 1, "ADC HL, HL", None, None },
[0xed6b] = { 3, "LD HL, ($%04x)", None, UnsignedWord },
[0xed6f] = { 1, "RLD ", None, None },
[0xed72] = { 1, "SBC HL, SP", None, None },
[0xed73] = { 3, "LD ($%04x), SP", UnsignedWord, None },
[0xed78] = { 1, "IN A, (C)", None, None },
[0xed79] = { 1, "OUT (C), A", None, None },
[0xed7a] = { 1, "ADC HL, SP", None, None },
[0xed7b] = { 3, "LD SP, ($%04x)", None, UnsignedWord },
[0xeda0] = { 1, "LDI ", None, None },
[0xeda1] = { 1, "CPI ", None, None },
[0xeda2] = { 1, "INI ", None, None },
[0xeda3] = { 1, "OUTI ", None, None },
[0xeda8] = { 1, "LDD ", None, None },
[0xeda9] = { 1, "CPD ", None, None },
[0xedaa] = { 1, "IND ", None, None },
[0xedab] = { 1, "OUTD ", None, None },
[0xedb0] = { 1, "LDIR ", None, None },
[0xedb1] = { 1, "CPIR ", None, None },
[0xedb2] = { 1, "INIR ", None, None },
[0xedb3] = { 1, "OTIR ", None, None },
[0xedb8] = { 1, "LDDR ", None, None },
[0xedb9] = { 1, "CPDR ", None, None },
[0xedba] = { 1, "INDR ", None, None },
[0xedbb] = { 1, "OTDR ", None, None },
[0x00ee] = { 2, "XOR $%02x", Unsigned, None },
[0x00ef] = { 1, "RST $28", Unsigned, None },
[0x00f0] = { 1, "RET P", None, None },
[0x00f1] = { 1, "POP AF", None, None },
[0x00f2] = { 3, "JP P, $%04x", None, UnsignedWord },
[0x00f3] = { 1, "DI ", None, None },
[0x00f4] = { 3, "CALL P, $%04x", None, UnsignedWord },
[0x00f5] = { 1, "PUSH AF", None, None },
[0x00f6] = { 2, "OR $%02x", Unsigned, None },
[0x00f7] = { 1, "RST $30", Unsigned, None },
[0x00f8] = { 1, "RET M", None, None },
[0x00f9] = { 1, "LD SP, HL", None, None },
[0x00fa] = { 3, "JP M, $%04x", None, UnsignedWord },
[0x00fb] = { 1, "EI ", None, None },
[0x00fc] = { 3, "CALL M, $%04x", None, UnsignedWord },
[0xfd09] = { 1, "ADD IY, BC", None, None },
[0xfd19] = { 1, "ADD IY, DE", None, None },
[0xfd21] = { 3, "LD IY, $%04x", None, UnsignedWord },
[0xfd22] = { 3, "LD ($%04x), IY", UnsignedWord, None },
[0xfd23] = { 1, "INC IY", None, None },
[0xfd29] = { 1, "ADD IY, IY", None, None },
[0xfd2a] = { 3, "LD IY, ($%04x)", None, UnsignedWord },
[0xfd2b] = { 1, "DEC IY", None, None },
[0xfd34] = { 2, "INC (IY + %d)", Signed, None },
[0xfd35] = { 2, "DEC (IY + %d)", Signed, None },
[0xfd36] = { 3, "LD (IY + %d), $%02x", Signed, Unsigned },
[0xfd39] = { 1, "ADD IY, SP", None, None },
[0xfd46] = { 2, "LD B, (IY + %d)", None, Signed },
[0xfd4e] = { 2, "LD C, (IY + %d)", None, Signed },
[0xfd56] = { 2, "LD D, (IY + %d)", None, Signed },
[0xfd5e] = { 2, "LD E, (IY + %d)", None, Signed },
[0xfd66] = { 2, "LD H, (IY + %d)", None, Signed },
[0xfd6e] = { 2, "LD L, (IY + %d)", None, Signed },
[0xfd70] = { 2, "LD (IY + %d), B", Signed, None },
[0xfd71] = { 2, "LD (IY + %d), C", Signed, None },
[0xfd72] = { 2, "LD (IY + %d), D", Signed, None },
[0xfd73] = { 2, "LD (IY + %d), E", Signed, None },
[0xfd74] = { 2, "LD (IY + %d), H", Signed, None },
[0xfd75] = { 2, "LD (IY + %d), L", Signed, None },
[0xfd77] = { 2, "LD (IY + %d), A", Signed, None },
[0xfd7e] = { 2, "LD A, (IY + %d)", None, Signed },
[0xfd86] = { 2, "ADD A, (IY + %d)", None, Signed },
[0xfd8e] = { 2, "ADC A, (IY + %d)", None, Signed },
[0xfd96] = { 2, "SUB (IY + %d)", Signed, None },
[0xfd9e] = { 2, "SBC A, (IY + %d)", None, Signed },
[0xfda6] = { 2, "AND (IY + %d)", Signed, None },
[0xfdae] = { 2, "XOR (IY + %d)", Signed, None },
[0xfdb6] = { 2, "OR (IY + %d)", Signed, None },
[0xfdbe] = { 2, "CP (IY + %d)", Signed, None },
[0xfde1] = { 1, "POP IY", None, None },
[0xfde3] = { 1, "EX (SP), IY", None, None },
[0xfde5] = { 1, "PUSH IY", None, None },
[0xfde9] = { 1, "JP (IY)", None, None },
[0xfdf9] = { 1, "LD SP, IY", None, None },
[0x00fe] = { 2, "CP $%02x", Unsigned, None },
[0x00ff] = { 1, "RST $38", Unsigned, None },
