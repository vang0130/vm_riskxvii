#include "common.h"

extern void illegal_operation(unsigned char line[5]);
void free_linked_list();
int check_malloc_avail(int addr, int bytes);
void reg_dump();
void memory_dump(unsigned char* mem, uint32_t rs2);
int attempt_mem_dump(uint32_t addr, struct cblob* cblob, 
                     struct mem_banks* mem_bank, int size);
void request_mem(int size, struct decoded* inst, struct mem_banks* mem_bank);
int free_mem(int addr, struct decoded* inst, struct mem_banks* mem_bank);
int check_vrs_s(struct decoded* inst, struct cblob* cblob, 
                struct mem_banks* mem_bank, uint32_t val, 
                int size, unsigned char line[5]);
int check_vrs_l(int addr, int rd);
