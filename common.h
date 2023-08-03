#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

// macros
#define INST_MEM_SIZE 1024
#define DATA_MEM_SIZE 1024
#define NUM_MEM_BANKS 128
#define MEM_BANK_SIZE 64
#define END_INST_MEM 1023
#define START_DATA_MEM 1024
#define END_DATA_MEM 2047
#define START_VR_MEM 2048
#define END_VR_MEM 2304
#define START_MALLOC_MEM 46848
#define END_MALLOC_MEM 55040

// globals
extern uint32_t registers[32];
extern uint32_t PC;
extern struct node* head;

// struct declarations
typedef struct mem_banks mem_banks;
typedef struct blob blob;
typedef struct cblob cblob;
typedef enum codes codes;
typedef struct decoded decoded;

// enum
enum codes{ // opcodes
    R_CODE = 51, I_CODE = 19, I2_CODE = 3, I3_CODE = 103, 
    S_CODE = 35, SB_CODE = 99, U_CODE = 55, UJ_CODE = 111
};

// structs
struct decoded {  // each instruction, decoded
    char* instruction;
    uint32_t rd;
    uint32_t rs1;
    uint32_t rs2;
    int32_t imm;
    enum codes code;
    uint32_t f3;
    uint32_t f7;
};

struct mem_banks {
    unsigned char banks[NUM_MEM_BANKS*MEM_BANK_SIZE];
    char used[NUM_MEM_BANKS]; // 0 = free, 1 = used
};

struct node {
    int address;
    int size; 
    int num_banks;
    struct node* next;
};

struct blob { // contains uint32_t representation of inst mem and data mem
    uint32_t inst_mem[INST_MEM_SIZE/sizeof(uint32_t)];
    uint32_t data_mem[DATA_MEM_SIZE/sizeof(uint32_t)];
};

struct cblob { // char representation of file
    unsigned char im[INST_MEM_SIZE];
    unsigned char dm[INST_MEM_SIZE];
};