#include "vm_riskxvii.h"

// GLOBALS
// all registers, 32 bit, unsigned
uint32_t registers[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint32_t PC = 0; // program counter
struct node* head = NULL; // head of linked list for mallocs


// ERROR AND HALT PROGRAM=======================================================
// if command not implemented
void not_implemented(unsigned char line[5]) {
    printf("Instruction Not Implemented: 0x");
    for (int j = 0; j < 4; j++) {
        printf("%x", (unsigned char)line[j]);
    }
    printf("\n");
    reg_dump();
    free_linked_list();
    exit(1);
}

// if operation is illegal (accessing memory not allowed)
void illegal_operation(unsigned char line[5]) {
    printf("Illegal Operation: 0x");
    for (int j = 0; j < 4; j++) {
        printf("%02x", line[j]);
    }
    printf("\n");
    reg_dump();
    free_linked_list();
    exit(0); 
}

// ARITHMETIC AND LOGIC OPERATIONS==============================================
// adds two unsigned registers
void add(int rs1, int rs2, int rd) {
    registers[rd] = registers[rs1] + registers[rs2];
    PC += 4;
}

// add immediate
void addi(int rs1, int rd, int32_t imm) {
    registers[rd] = registers[rs1] + imm;
    PC += 4;
}

// subtract
void sub(int rs1, int rs2, int rd) {
    registers[rd] = registers[rs1] - registers[rs2];
    PC += 4;
}

// load upper immediate, shifts sext(imm) by 12 and loads into register (*2^12)
void lui(int rd, int32_t imm) {
    registers[rd] = imm << 12;
    PC += 4;
}

// xor function with bits
void xor(int rs1, int rs2, int rd) {
    registers[rd] = registers[rs1] ^ registers[rs2];
    PC += 4;
}

// xor function with immediate
void xori(int rs1, int rd, int32_t imm) {
    registers[rd] = registers[rs1] ^ imm;
    PC += 4;
}

// or function with bits
void or(int rs1, int rs2, int rd) {
    registers[rd] = registers[rs1] | registers[rs2];
    PC += 4;
}

// or function with immediate
void ori(int rs1, int rd, int32_t imm) {
    registers[rd] = registers[rs1] | imm;
    PC += 4;
}

// and function with bits
void and(int rs1, int rs2, int rd) {
    registers[rd] = registers[rs1] & registers[rs2];
    PC += 4;
}

// and with immediaet
void andi(int rs1, int rd, int32_t imm) {
    registers[rd] = registers[rs1] & imm;
    PC += 4;
}

// SHIFTING
// left shift by val of rs2 (<= 32)
void sll(int rs1, int rs2, int rd) {
    registers[rd] = registers[rs1] << registers[rs2];
    PC += 4;
}

// right shift (<= 32)
void srl(int rs1, int rs2, int rd) {
    registers[rd] = registers[rs1] >> registers[rs2];
    PC += 4;
}

// shift to right, but pushed bits get moved back to front
void sra(int rs1, int rs2, int rd) {
    uint32_t back = get_bits(registers[rs1], 0, rs2 - 1); // get back
    back = back << (32 - rs2); // shift to front
    uint32_t front = registers[rs1] >> rs2; // get front
    uint32_t full = front | back; // combine
    registers[rd] = full;
    PC += 4;
}



// MEMORY ACCESS OPERATIONS=====================================================
// load operations can access instruction memory, data memory, and malloc memory
// loads a single byte
void lb(int addr, int rd, unsigned char* mem) {
    int offset = 0; // if instruction mem
    // if data mem
    if (addr < START_VR_MEM && addr > END_INST_MEM) {
        offset = START_DATA_MEM;
    }
    // if malloced memory
    else if (addr >= START_MALLOC_MEM) {
        offset = START_MALLOC_MEM;
    }
    uint32_t val = mem[addr - offset];
    // sign extend the value
    if (val & (1 << 7)) { // if msb is 1
        val = val | 0xffffff00; // fill first 24 bits with 1s
    }
    registers[rd] = val;
    PC += 4;
}

// loads two bytes
void lh(int addr, int rd, unsigned char* mem) {
    int offset = 0;
    if (addr < START_VR_MEM && addr > END_INST_MEM) {
        offset = START_DATA_MEM;
    }
    else if (addr >= START_MALLOC_MEM) {
        offset = START_MALLOC_MEM;
    }
    uint32_t val = 0;
    // extract 2 bytes
    val |= (uint32_t)(mem[addr - offset]); // first byte
    // second byte, goes in front of prev byte so shift left by 8
    val |= (uint32_t) mem[addr - offset + 1] << 8;
    // sign extend
    if (val & (1 << 15)) { // if msb is 1
        val = val | 0xffff0000; // fill first 16 bits with 1s
    }
    registers[rd] = val;
    PC += 4;
}

// load 4 bytes from memory
void lw(int addr, int rd, unsigned char* mem) {
    int offset = 0;
    if (addr < START_VR_MEM && addr > END_INST_MEM) {
        offset = START_DATA_MEM;
    }
    else if (addr >= START_MALLOC_MEM) {
        offset = START_MALLOC_MEM;
    }
    uint32_t val = 0; // extracting
    // 4 bytes in total: 44444444 33333333 22222222 11111111
    val |= (uint32_t)(mem[addr - offset]); // first byte
    val |= (uint32_t) mem[addr - offset + 1] << 8; // second byte
    val |= (uint32_t) mem[addr - offset + 2] << 16; // 3rd byte, shift by 16
    val |= (uint32_t) mem[addr - offset + 3] << 24;
    registers[rd] = val;
    PC += 4;
}


// NO SIGN EXTENSION
// load one byte, no sign extending
void lbu(int addr, int rd, unsigned char* mem) {
    int offset = 0;
    // if within data memory, address will be 1024 - 2047
    if (addr < START_VR_MEM && addr > END_INST_MEM) {
        offset = START_DATA_MEM; // offset is 1024
        // (0 within the data memory array is 1024 within all memory)
    }
    else if (addr >= START_MALLOC_MEM) {
        offset = START_MALLOC_MEM;
    }

    uint8_t val = mem[addr - offset];
    registers[rd] = val; // automatically adds to back (LSB)
    PC += 4;
}

// load 2 bytes
void lhu(int addr, int rd, unsigned char* mem) {
    int offset = 0;
    if (addr < START_VR_MEM && addr > END_INST_MEM) {
        offset = START_DATA_MEM;
    }
    else if (addr >= START_MALLOC_MEM) {
        offset = START_MALLOC_MEM;
    }
    
    uint16_t val = 0; // extracting
    val |= (uint16_t)(mem[addr - offset]);
    val |= (uint16_t) mem[addr - offset + 1] << 8;
    registers[rd] = val; // automatically adds to back
    PC += 4;
}


// STORING OPERATIONS
// we can ONLY store in data memory and malloc memory
// store 1 byte in memory
void sb(int addr, int rs2, unsigned char* mem) {
    int offset = START_DATA_MEM; // if in data memory
    // otherwise, if in malloc memory
    // we have already checked that store is valid here
    if (addr >= START_MALLOC_MEM) {
        offset = START_MALLOC_MEM;
    }

    uint8_t val = registers[rs2]; // taking the LSB
    mem[addr - offset] = val;
    PC += 4;
}

// store 2 bytes in memory
void sh(int addr, int rs2, unsigned char* mem) {
    int offset = START_DATA_MEM;
    if (addr >= START_MALLOC_MEM) { // if in malloc mem
        offset = START_MALLOC_MEM;
    }

    uint16_t val = registers[rs2];
    // 0xff = 00000000 11111111
    mem[addr - offset] = val & 0xff; // mask to get LSB
    mem[addr - offset + 1] = (val >> 8) & 0xff; // shift to the right to get MSB
    PC += 4;
}

// store 4 bytes in memory
void sw(int addr, int rs2, unsigned char* mem) {
    int offset = START_DATA_MEM;
    if (addr >= START_MALLOC_MEM) {
        offset = START_MALLOC_MEM;
    }
    uint32_t val = registers[rs2];
    mem[addr - offset] = val & 0xff; // mask each time to only get LSB
    mem[addr - offset + 1] = (val >> 8) & 0xff;
    mem[addr - offset + 2] = (val >> 16) & 0xff;
    mem[addr - offset + 3] = (val >> 24) & 0xff;
    PC += 4;
}


// PROGRAM FLOW OPERATIONS======================================================
// SIGNED
// set if smaller
// signed, so cast registers
void slt(int rs1, int rs2, int rd) {
    if ((int32_t)registers[rs1] < (int32_t)registers[rs2]) {
        registers[rd] = 1;
    }
    else {
        registers[rd] = 0;
    }
    PC += 4;
}

// SIGNED
// set if smaller (immediate)
void slti(int rs1, int rd, int32_t imm) {
    if ((int32_t)registers[rs1] < imm) {
        registers[rd] = 1;
    }
    else {
        registers[rd] = 0;
    }
    PC += 4;
}

// UNSIGNED
// set if smaller unsinged, no casting
void sltu(int rs1, int rs2, int rd) {
    if (registers[rs1] < registers[rs2]) {
        registers[rd] = 1;
    }
    else {
        registers[rd] = 0;
    }
    PC += 4;
}

// UNSIGNED
// set if smaller unisnegd, with immediate
// cast immediate from signed to unsigned
void sltiu(int rs1, int rd, int32_t imm) {
    if (registers[rs1] < (uint32_t)imm) {
        registers[rd] = 1;
    }
    else {
        registers[rd] = 0;
    }
    PC += 4;
}

// branch if equal
void beq(int rs1, int rs2, int32_t imm) {
    if (registers[rs1] == registers[rs2]) {
        PC = PC + (imm << 1); // shifting imm now instead of during decoding
    }
    else {
        PC += 4;
    }
}

// branch if not equal
void bne(int rs1, int rs2, int32_t imm) {
    if (registers[rs1] != registers[rs2]) {
        PC = PC + (imm << 1);
    }
    else {
        PC += 4;
    }
}

// SIGNED
// branch if less than
void blt(int rs1, int rs2, int32_t imm) {
    if ((int32_t)registers[rs1] < (int32_t)registers[rs2]) {
        PC = PC + (imm << 1); // shift imm
    }
    else {
        PC += 4;
    }
}

// UNSIGNED
// branch if less than (unsigned)
// don't cast registers
void bltu(int rs1, int rs2, int32_t imm) {
    if (registers[rs1] < registers[rs2]) {
        PC = PC + (imm << 1);
    }
    else {
        PC += 4;
    }
}

// SIGNED
// branch if greater than (or equal to)
// cast registers
void bge(int rs1, int rs2, int32_t imm) {
    if ((int32_t)registers[rs1] >= (int32_t)registers[rs2]) {
        PC = PC + (imm << 1);
    }
    else {
        PC += 4;
    }
}

// UNSIGNED
// branch if greater than (or equal to) (unsigned)
void bgeu(int rs1, int rs2, int32_t imm) {
    if (registers[rs1] >= registers[rs2]) {
        PC = PC + (imm << 1);
    }
    else {
        PC += 4;
    }
}

// jump and link
void jal(int rd, int32_t imm) {
    registers[rd] = PC + 4;
    PC = PC + (imm << 1); // shift imm
}

// jump and link with register (no imm)
void jalr(int rs1, int rd, int32_t imm) {
    registers[rd] = PC + 4;
    PC = registers[rs1] + imm;
}

// figures out which load function to call, does all error checking to keep main clean
void call_load(int index, int size, struct decoded* curr_inst, 
               unsigned char curr_line[5], struct cblob* cblob, 
               struct mem_banks* mem_bank) {
    void (*func_arr[])(int addr, int rd, unsigned char* mem) = {lb, lh, lw, 
                                                                lbu, lhu};
    // get current address
    int addr = registers[curr_inst->rs1] + curr_inst->imm;
    //we can load from inst memory, data memory and malloc
    // in instruction memory
    if (addr >= 0 && addr <= END_INST_MEM) {
        (*func_arr[index])(addr, curr_inst->rd, cblob->im);
    }
    // data memory
    else if (addr > END_INST_MEM && addr <= END_DATA_MEM) {
        (*func_arr[index])(addr, curr_inst->rd, cblob->dm);

    }
    // virtual routine
    else if (addr > END_DATA_MEM && addr < END_VR_MEM) {
        int res = check_vrs_l(addr, curr_inst->rd);
        if (res == 0) {
            illegal_operation(curr_line);
        }
        PC += 4;
    }
    // malloced mem
    else if (addr >= START_MALLOC_MEM && addr < END_MALLOC_MEM) {
        int res = check_malloc_avail(addr, size);
        if (res == 0) {
            illegal_operation(curr_line);
        }
        else {
            (*func_arr[index])(addr, curr_inst->rd, mem_bank->banks);
        }
    }
    else {
        illegal_operation(curr_line);
    }
}

// figures out which store function to call, does all error checking to keep main clean
void call_store(int index, int size, struct decoded* curr_inst, 
                unsigned char curr_line[5], struct cblob* cblob, 
                struct mem_banks* mem_bank, uint32_t to_store) {
    void (*func_arr[])(int addr, int rs2, unsigned char* mem) = {sb, sh, sw};
    int addr = registers[curr_inst->rs1] + curr_inst->imm;

    // if data memory
    if (addr + (size - 1) < START_VR_MEM && addr > END_INST_MEM) {
        (*func_arr[index])(addr, curr_inst->rs2, cblob->dm);
    }
    // if a virtual routine
    else if (addr > END_DATA_MEM && addr < END_VR_MEM) {
        int res = check_vrs_s(curr_inst, cblob, mem_bank, 
                              to_store, size, curr_line);
        if (res == 0) { // invalid address for virtual routine
            illegal_operation(curr_line);
        }
        PC += 4;
    }
    // if writing to malloc
    else if (addr >= START_MALLOC_MEM && addr < END_MALLOC_MEM) { // writing to malloc
        // check byte can be stored at address
        int res = check_malloc_avail(addr, size);
        if (res == 0) {
            illegal_operation(curr_line);
        }
        else {
            (*func_arr[index])(addr, curr_inst->rs2, mem_bank->banks);
        }
    }
    else {
        illegal_operation(curr_line);
    }
}


int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Wrong Number of Arguments\n");
        exit(0);
    }
    // read in binary
    FILE* fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        printf("File Not Found\n");
        exit(0);
    }
    // initialise blob and cblob to 0
    struct blob blob = {0}; // uint32_t
    struct cblob cblob = {0}; // unsigned char

    size_t res = fread(blob.inst_mem, 4, 256, fp); // read inst memory into blob
    size_t res2 = fread(blob.data_mem, 4, 256, fp); // read data memory into blob
    fseek(fp, 0, SEEK_SET);
    size_t res3 = fread(cblob.im, 1, 1024, fp); // read inst memory as char
    size_t res4 = fread(cblob.dm, 1, 1024, fp); // read data memory as char
    if (res != 256 || res2 != 256 || res3 != 1024 || res4 != 1024) {
        printf("Invalid Memory Image\n");
        exit(0);
    }
    fclose(fp);

    struct mem_banks mem_bank = {0};
    // initialise "used" to 0s (no mem banks used yet)
    for (int i = 0; i < 128; i++) {
        mem_bank.used[i] = '0';
    }

    int running = 1;
    while (running == 1) {
        registers[0] = 0; // zero register

        unsigned char curr_line[5]; // get current line of instructions as char
        for (int j = 3; j >= 0; j--) {
            curr_line[j] = cblob.im[PC + 3 - j];
        }
        curr_line[4] = '\0';

        uint32_t line = blob.inst_mem[PC / 4]; // get current line as bits
        
        // decode current line and get all information inside struct
        struct decoded curr_inst = {0};
        curr_inst.instruction = NULL;
        decode(line, &curr_inst); 
        if (curr_inst.instruction == NULL) {
            not_implemented(curr_line);
        }
        // print_comm(&curr_inst);
        if (strcmp(curr_inst.instruction, "add") == 0) {
            add(curr_inst.rs1, curr_inst.rs2, curr_inst.rd);
        }
        else if (strcmp(curr_inst.instruction, "addi") == 0) {
            addi(curr_inst.rs1, curr_inst.rd, curr_inst.imm);
        }
        else if (strcmp(curr_inst.instruction, "sub") == 0) {
            sub(curr_inst.rs1, curr_inst.rs2, curr_inst.rd);
        }
        else if (strcmp(curr_inst.instruction, "lui") == 0) {
            lui(curr_inst.rd, curr_inst.imm);
        }
        else if (strcmp(curr_inst.instruction, "xor") == 0) {
            xor(curr_inst.rs1, curr_inst.rs2, curr_inst.rd);
        }
        else if (strcmp(curr_inst.instruction, "xori") == 0) {
            xori(curr_inst.rs1, curr_inst.rd, curr_inst.imm);
        }
        else if (strcmp(curr_inst.instruction, "or") == 0) {
            or(curr_inst.rs1, curr_inst.rs2, curr_inst.rd);
        }
        else if (strcmp(curr_inst.instruction, "ori") == 0) {
            ori(curr_inst.rs1, curr_inst.rd, curr_inst.imm);
        }
        else if (strcmp(curr_inst.instruction, "and") == 0) {
            and(curr_inst.rs1, curr_inst.rs2, curr_inst.rd);
        }
        else if (strcmp(curr_inst.instruction, "andi") == 0) {
            andi(curr_inst.rs1, curr_inst.rd, curr_inst.imm);
        }
        else if (strcmp(curr_inst.instruction, "sll") == 0) {
            sll(curr_inst.rs1, curr_inst.rs2, curr_inst.rd);
        }
        else if (strcmp(curr_inst.instruction, "srl") == 0) {
            srl(curr_inst.rs1, curr_inst.rs2, curr_inst.rd);  
        }
        else if (strcmp(curr_inst.instruction, "sra") == 0) {
            sra(curr_inst.rs1, curr_inst.rs2, curr_inst.rd);
        }
        else if (strcmp(curr_inst.instruction, "lb") == 0) {
            call_load(0, 1, &curr_inst, curr_line, &cblob, &mem_bank);
        }
        else if (strcmp(curr_inst.instruction, "lh") == 0) {
            call_load(1, 2, &curr_inst, curr_line, &cblob, &mem_bank);
        }
        else if (strcmp(curr_inst.instruction, "lw") == 0) {
            call_load(2, 4, &curr_inst, curr_line, &cblob, &mem_bank);
        }
        else if (strcmp(curr_inst.instruction, "lbu") == 0) {
            call_load(3, 1, &curr_inst, curr_line, &cblob, &mem_bank);
        }
        else if (strcmp(curr_inst.instruction, "lhu") == 0) {
            call_load(4, 2, &curr_inst, curr_line, &cblob, &mem_bank);
        }
        else if (strcmp(curr_inst.instruction, "sb") == 0) {
            uint8_t temp = registers[curr_inst.rs2];
            uint32_t to_store = temp;
            call_store(0, 1, &curr_inst, curr_line, 
                       &cblob, &mem_bank, to_store);
        }
        else if (strcmp(curr_inst.instruction, "sh") == 0) {
            uint16_t temp = registers[curr_inst.rs2];
            uint32_t to_store = temp;
            call_store(1, 2, &curr_inst, curr_line, 
                       &cblob, &mem_bank, to_store);
        }
        else if (strcmp(curr_inst.instruction, "sw") == 0) {
            uint32_t to_store = registers[curr_inst.rs2];
            call_store(2, 4, &curr_inst, curr_line, 
                       &cblob, &mem_bank, to_store);
        }
        else if (strcmp(curr_inst.instruction, "slt") == 0) {
            slt(curr_inst.rs1, curr_inst.rs2, curr_inst.rd);
        }
        else if (strcmp(curr_inst.instruction, "slti") == 0) {
            slti(curr_inst.rs1, curr_inst.rd, curr_inst.imm);
        }
        else if (strcmp(curr_inst.instruction, "sltu") == 0) {
            sltu(curr_inst.rs1, curr_inst.rs2, curr_inst.rd);
        }
        else if (strcmp(curr_inst.instruction, "sltiu") == 0) {
            sltiu(curr_inst.rs1, curr_inst.rd, curr_inst.imm);
        }
        else if (strcmp(curr_inst.instruction, "beq") == 0) {
            beq(curr_inst.rs1, curr_inst.rs2, curr_inst.imm);
        }
        else if (strcmp(curr_inst.instruction, "bne") == 0) {
            bne(curr_inst.rs1, curr_inst.rs2, curr_inst.imm);
        }
        else if (strcmp(curr_inst.instruction, "blt") == 0) {
            blt(curr_inst.rs1, curr_inst.rs2, curr_inst.imm);
        }
        else if (strcmp(curr_inst.instruction, "bltu") == 0) {
            bltu(curr_inst.rs1, curr_inst.rs2, curr_inst.imm);
        }
        else if (strcmp(curr_inst.instruction, "bge") == 0) {
            bge(curr_inst.rs1, curr_inst.rs2, curr_inst.imm);
        }
        else if (strcmp(curr_inst.instruction, "bgeu") == 0) {
            bgeu(curr_inst.rs1, curr_inst.rs2, curr_inst.imm);
        }
        else if (strcmp(curr_inst.instruction, "jal") == 0) {
            jal(curr_inst.rd, curr_inst.imm);
        }
        else if (strcmp(curr_inst.instruction, "jalr") == 0) {
            jalr(curr_inst.rs1, curr_inst.rd, curr_inst.imm);
        }
    }
    return 0;
}