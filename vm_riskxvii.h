#include "common.h"

extern void reg_dump(void);
extern void free_linked_list(void);
extern uint32_t get_bits(uint32_t line, int start, int end);
extern int check_vrs_l(int addr, int rd);
extern int check_vrs_s(struct decoded* inst, struct cblob* cblob, 
                       struct mem_banks* mem_bank, uint32_t val, 
                       int size, unsigned char line[5]);
extern int check_malloc_avail(int addr, int bytes);
extern void decode(uint32_t line, struct decoded* curr_inst);
void not_implemented(unsigned char line[5]);
void illegal_operation(unsigned char line[5]);
void add(int rs1, int rs2, int rd);
void addi(int rs1, int rd, int32_t imm);
void sub(int rs1, int rs2, int rd);
void lui(int rd, int32_t imm);
void xor(int rs1, int rs2, int rd);
void xori(int rs1, int rd, int32_t imm);
void or(int rs1, int rs2, int rd);
void ori(int rs1, int rd, int32_t imm);
void and(int rs1, int rs2, int rd);
void andi(int rs1, int rd, int32_t imm);
void sll(int rs1, int rs2, int rd);
void srl(int rs1, int rs2, int rd);
void sra(int rs1, int rs2, int rd);
void lb(int addr, int rd, unsigned char* mem);
void lh(int addr, int rd, unsigned char* mem);
void lw(int addr, int rd, unsigned char* mem);
void lbu(int addr, int rd, unsigned char* mem);
void lhu(int addr, int rd, unsigned char* mem);
void sb(int addr, int rs2, unsigned char* mem);
void sh(int addr, int rs2, unsigned char* mem);
void sw(int addr, int rs2, unsigned char* mem);
void slt(int rs1, int rs2, int rd);
void slti(int rs1, int rd, int32_t imm);
void sltu(int rs1, int rs2, int rd);
void sltiu(int rs1, int rd, int32_t imm);
void beq(int rs1, int rs2, int32_t imm);
void bne(int rs1, int rs2, int32_t imm);
void blt(int rs1, int rs2, int32_t imm);
void bltu(int rs1, int rs2, int32_t imm);
void bge(int rs1, int rs2, int32_t imm);
void bgeu(int rs1, int rs2, int32_t imm);
void jal(int rd, int32_t imm);
void jalr(int rs1, int rd, int32_t imm);
void call_load(int index, int size, struct decoded* curr_inst, 
               unsigned char curr_line[5], struct cblob* cblob, 
               struct mem_banks* mem_bank);
void call_store(int index, int size, struct decoded* curr_inst, 
                unsigned char curr_line[5], struct cblob* cblob, 
                struct mem_banks* mem_bank, uint32_t to_store);
