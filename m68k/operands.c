#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "instruction.h"
#include "operands.h"
#include "m68k.h"

char* operand_tostring(Operand* operand)
{
    static char buffer[1024];

    switch (operand->type)
    {
    case DataRegister:
        sprintf(buffer, "D%d", operand->n);
        break;
    case AddressRegister:
        sprintf(buffer, "A%d", operand->n);
        break;
    case Address:
        sprintf(buffer, "(A%d)", operand->n);
        break;
    default:
        sprintf(buffer, "UNSUPPORTED");
    }

    return buffer;
}

uint8_t operand_size(uint8_t pattern)
{
    switch (pattern)
    {
    case 0:
        return 8;
    case 1:
        return 16;
    case 2:
        return 32;
    default:
        return 0;
    }
}

Operand* operand_make(uint16_t pattern, Instruction* instr)
{
    switch (pattern & 0x38)
    {
    case 0:
        return operand_make_data_register(pattern & 7, instr);
        /*case 0x8:
            return operand_make_address_register(pattern & 7);
        case 0x10:
            return operand_make_address(pattern & 7);*/
    default:
        return NULL;
    }
}

/*
 *
 */

uint16_t data_register_get(Operand* this)
{
    return this->instruction->context->data_registers[this->n];
}

void data_register_set(Operand* this, uint16_t value)
{
    this->instruction->context->data_registers[this->n] = value;
}

Operand* operand_make_data_register(int n, Instruction* instr)
{
    Operand* op = calloc(1, sizeof(Operand));
    op->type = DataRegister;
    op->get = data_register_get;
    op->set = data_register_set;
    op->n = n;
    op->instruction = instr;
    return op;
}

/*
 *
 */

 /*uint16_t address_register_get(Operand this)
 {
     return _m68k.address_registers[this.n];
 }

 void address_register_set(Operand this, uint16_t value)
 {
     _m68k.address_registers[this.n] = value;
 }

 Operand operand_make_address_register(int n)
 {
     return (Operand) {
         .type = AddressRegister,
         .get = address_register_get,
         .set = address_register_set,
         .n = n
     };
 }*/

 /*
  *
  */

  /*uint16_t address_get(Operand this)
  {
      return _memory[_m68k.address_registers[this.n]];
  }

  void address_set(Operand this, uint16_t value)
  {
      _memory[_m68k.address_registers[this.n]] = value;
  }

  Operand operand_make_address(int n)
  {
      return (Operand) {
          .type = Address,
          .get = address_get,
          .set = address_set,
          .n = n
      };
  }
  */
