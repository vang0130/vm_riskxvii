#include "decode.h"

// DECODING FUNCTIONS
// gets the string representation of the instruction, fills in struct
void get_instruction(struct decoded* inst, uint32_t line) {
    if (inst->code == R_CODE) { // 51
        if (inst->f3 == 0 && inst->f7 == 0) {
            inst->instruction = "add";
        }
        else if (inst->f3 == 0 && inst->f7 == 32) {
            inst->instruction = "sub";
        }
        else if (inst->f3 == 4 && inst->f7 == 0) {
            inst->instruction = "xor";
        }
        else if (inst->f3 == 6 && inst->f7 == 0) {
            inst->instruction = "or";
        }
        else if (inst->f3 == 7 && inst->f7 == 0) {
            inst->instruction = "and";
        }
        else if (inst->f3 == 1 && inst->f7 == 0) {
            inst->instruction = "sll";
        }
        else if (inst->f3 == 5 && inst->f7 == 0) {
            inst->instruction = "srl";
        }
        else if (inst->f3 == 5 && inst->f7 == 32) {
                inst->instruction = "sra";
        }
        else if (inst->f3 == 2 && inst->f7 == 0) {
            inst->instruction = "slt";
        }
        else if (inst->f3 == 3 && inst->f7 == 0) {
            inst->instruction = "sltu";
        }
    }
    else if (inst->code == I_CODE) { // 19
        if (inst->f3 == 0) {
            inst->instruction = "addi";
        }
        else if (inst->f3 == 4) {
            inst->instruction = "xori";
        }
        else if (inst->f3 == 6) {
            inst->instruction = "ori";
        }
        else if (inst->f3 == 7) {
            inst->instruction = "andi";
        }
        else if (inst->f3 == 2) {
            inst->instruction = "slti";
        }
        else if (inst->f3 == 3) {
            inst->instruction = "sltiu";
        }
    }
    else if (inst->code == I2_CODE) { // 3
        if (inst->f3 == 0) {
            inst->instruction = "lb";
        }
        else if (inst->f3 == 1) {
            inst->instruction = "lh";
        }
        else if (inst->f3 == 2) {
            inst->instruction = "lw";
        }
        else if (inst->f3 == 4) {
            inst->instruction = "lbu";
        }
        else if (inst->f3 == 5) {
            inst->instruction = "lhu";
        }
    }
    else if (inst->code == I3_CODE) { // 103
        inst->instruction = "jalr";
    }
    else if (inst->code == S_CODE) {
        if (inst->f3 == 0) {
            inst->instruction = "sb";
        }
        else if (inst->f3 == 1) {
            inst->instruction = "sh";
        }
        else if (inst->f3 == 2) {
            inst->instruction = "sw";
        }
    }
    else if (inst->code == S_CODE) { // 35
        if (inst->f3 == 0) {
            inst->instruction = "sb";
        }
        else if (inst->f3 == 1) {
            inst->instruction = "sh";
        }
        else if (inst->f3 == 2) {
            inst->instruction = "sw";
        }
    }
    else if (inst->code == SB_CODE) { // 99
        if (inst->f3 == 0) {
            inst->instruction = "beq";
        }
        else if (inst->f3 == 1) {
            inst->instruction = "bne";
        }
        else if (inst->f3 == 4) {
            inst->instruction = "blt";
        }
        else if (inst->f3 == 5) {
            inst->instruction = "bge";
        }
        else if (inst->f3 == 6) {
            inst->instruction = "bltu";
        }
        else if (inst->f3 == 7) {
            inst->instruction = "bgeu";
        }
    }
    else if (inst->code == U_CODE) { // 55
        inst->instruction = "lui";
    }
    else if (inst->code == UJ_CODE) { // 111
        inst->instruction = "jal";
    }
}

// gets bits from .mi file based on index arguments
uint32_t get_bits(uint32_t line, int start, int end) {
    uint32_t mask = 0;
    for (int i = start; i <= end; i++) {
        mask = mask | (1 << i); // mask the whole length of section
    }
    return (line & mask) >> start; // pick out the section, offset by start
}

// decodes R encoding by getting bits and fills struct about instruction
void decode_R(struct decoded* curr_inst, uint32_t line) {
    curr_inst->rd = get_bits(line, 7, 11); // automatically in uint_32_t
    curr_inst->f3 = get_bits(line, 12, 14);
    curr_inst->rs1 = get_bits(line, 15, 19);
    curr_inst->rs2 = get_bits(line, 20, 24);
    curr_inst->f7 = get_bits(line, 25, 31);
    get_instruction(curr_inst, line);
}

// decodes type I
void decode_I(struct decoded* curr_inst, uint32_t line) {
    curr_inst->rd = get_bits(line, 7, 11);
    curr_inst->f3 = get_bits(line, 12, 14);
    curr_inst->rs1 = get_bits(line, 15, 19);
    curr_inst->imm = (int32_t)get_bits(line, 20, 31);
    if (curr_inst->imm & (1 << 11)) { // sign extending
        curr_inst->imm = curr_inst->imm | 0xfffff000;
    }
    get_instruction(curr_inst, line);
}

// decodes type S
void decode_S(struct decoded* curr_inst, uint32_t line) {
    curr_inst->imm = (int32_t)get_bits(line, 7, 11);
    curr_inst->f3 = get_bits(line, 12, 14);
    curr_inst->rs1 = get_bits(line, 15, 19);
    curr_inst->rs2 = get_bits(line, 20, 24);
    curr_inst->imm = curr_inst->imm | (int32_t)(get_bits(line, 25, 31) << 5);
    if (curr_inst->imm & (1 << 11)) { // sign extending
        curr_inst->imm = curr_inst->imm | 0xfffff000;
    }
    get_instruction(curr_inst, line);
}

// decodes type SB
void decode_SB(struct decoded* curr_inst, uint32_t line) {
    // combines the different sections of immediate through left shifts
    curr_inst->imm = (int32_t)get_bits(line, 8, 11);
    curr_inst->imm = curr_inst->imm | (int32_t)(get_bits(line, 25, 30) << 4);
    curr_inst->imm = curr_inst->imm | (int32_t)(get_bits(line, 7, 7) << 10);
    curr_inst->imm = curr_inst->imm | (int32_t)(get_bits(line, 31, 31) << 11);
    if (curr_inst->imm & (1 << 11)) {
        curr_inst->imm = curr_inst->imm | 0xfffff000;
    }
    curr_inst->f3 = get_bits(line, 12, 14);
    curr_inst->rs1 = get_bits(line, 15, 19);
    curr_inst->rs2 = get_bits(line, 20, 24);
    get_instruction(curr_inst, line);
}

// decodes type U
void decode_U(struct decoded* curr_inst, uint32_t line) {
    curr_inst->rd = get_bits(line, 7, 11);
    curr_inst->imm = (int32_t)get_bits(line, 12, 31);
    if (curr_inst->imm & (1 << 19)) {
        curr_inst->imm = curr_inst->imm | 0xfff00000;
    }
    get_instruction(curr_inst, line);
}

// decodes type UJ
void decode_UJ(struct decoded* curr_inst, uint32_t line) {
    curr_inst->rd = get_bits(line, 7, 11);
    curr_inst->imm = (int32_t)get_bits(line, 21, 30); // read from first index of imm first
    curr_inst->imm = curr_inst->imm | (int32_t)(get_bits(line, 20, 20) << 10);
    curr_inst->imm = curr_inst->imm | (int32_t)(get_bits(line, 12, 19) << 11);
    curr_inst->imm = curr_inst->imm | (int32_t)(get_bits(line, 31, 31) << 19);
    if (curr_inst->imm & (1 << 19)) {
        curr_inst->imm = curr_inst->imm | 0xfff00000;
    }
    get_instruction(curr_inst, line);
}

// takes an opcode and a bit line, and sends off to relevant function 
// to fill curr_inst struct
void decode(uint32_t line, struct decoded* curr_inst) {
    enum codes curr_opc = get_bits(line, 0, 6); // get opcode
    curr_inst->code = curr_opc;
    // now we know the encoding, we send off to relevant function to fill curr_inst struct
    if (curr_opc == R_CODE) {
        decode_R(curr_inst, line);
    }
    else if (curr_opc == I_CODE || curr_opc == I2_CODE || curr_opc == I3_CODE) {
        decode_I(curr_inst, line);
    }
    else if (curr_opc == S_CODE) {
        decode_S(curr_inst, line);
    }
    else if (curr_opc == SB_CODE) {
        decode_SB(curr_inst, line);
    }
    else if (curr_opc == U_CODE) {
        decode_U(curr_inst, line);
    }
    else if (curr_opc == UJ_CODE) {
        decode_UJ(curr_inst, line);
    }
}
