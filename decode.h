#include "common.h"

void get_instruction(struct decoded* inst, uint32_t line);
uint32_t get_bits(uint32_t line, int start, int end);
void decode_R(struct decoded* curr_inst, uint32_t line);
void decode_I(struct decoded* curr_inst, uint32_t line);
void decode_S(struct decoded* curr_inst, uint32_t line);
void decode_SB(struct decoded* curr_inst, uint32_t line);
void decode_U(struct decoded* curr_inst, uint32_t line);
void decode_UJ(struct decoded* curr_inst, uint32_t line);
void decode(uint32_t line, struct decoded* curr_inst);