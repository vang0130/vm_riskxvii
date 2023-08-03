#include "virtual_routines.h"

// VIRTUAL ROUTINES=============================================================
// frees linked list for mallocs right before exit(0)
void free_linked_list() {
    // free linked list
    struct node* curr = head;
    struct node* prev = NULL;
    while (curr != NULL) {
        prev = curr;
        curr = curr->next;
        free(prev);
    }
}

// check if we can write to malloc memory
// memory has to have been allocated
int check_malloc_avail(int addr, int bytes) {
    if (head == NULL) { // no allocated memory
        return 0;
    }
    
    struct node* curr = head;
    while (curr != NULL) {
        // if address is within range of allocated memory
        if (addr >= curr->address && 
           (addr + bytes) <= (curr->address + curr->size)) {
            return 1; // available
        }
        curr = curr->next;
    }
    return 0; // mem not allocated, cannot write
}

// VIRTUAL ROUTINE FUNCTIONS====================================================
void reg_dump() { 
    printf("PC = 0x%08x;\n", PC);
    for (int i = 0; i < 32; i++) {
        printf("R[%d] = 0x%08x;\n", i, registers[i]);
    }
}

void memory_dump(unsigned char* mem, uint32_t rs2) {
    // extract 4 bytes
    uint32_t val = 0; // extracting
    // 4 bytes in total: 44444444 33333333 22222222 11111111
    val |= (uint32_t) mem[rs2]; // first byte
    val |= (uint32_t) mem[rs2 + 1] << 8; // second byte
    val |= (uint32_t) mem[rs2 + 2] << 16; // 3rd byte, shift by 16
    val |= (uint32_t) mem[rs2 + 3] << 24;
    printf("%08x", val);
}

// check if we can memory dump
int attempt_mem_dump(uint32_t addr, struct cblob* cblob, 
                     struct mem_banks* mem_bank, int size) {
    // in instruction memory
    if (addr >= 0 && addr + 3 <= END_INST_MEM) {
        memory_dump(cblob->im, addr);
    }
    // in data memory
    else if (addr >= START_DATA_MEM && addr + 3 < START_VR_MEM) {
        memory_dump(cblob->dm, addr - START_DATA_MEM);
    }
    // in malloced memory
    else if (addr >= START_MALLOC_MEM && addr + 3 < END_MALLOC_MEM) {
        // check address allocated
        int res = check_malloc_avail(addr, size);
        if (res == 0) {
            return 0;
        }
        memory_dump(mem_bank->banks, addr - START_MALLOC_MEM);
    }
    else {
        return 0; // illegal operation
    }
    return 1; // successful
}

// requesting malloced memory
void request_mem(int size, struct decoded* inst, struct mem_banks* mem_bank) {
    int num_banks = (size + (64 - 1)) / 64;
    // search for first chunk of free memory
    int found = 0;
    int i = 0;
    for (i = 0; i < 128; i++) {
        if (mem_bank->used[i] == '0') {
            found = 1;
            for (int j = 1; j < num_banks; j++) {
                if ((i + j) >= 128) {
                    found = 0;
                    break;
                }
                if (mem_bank->used[i + j] == '1') {
                    found = 0;
                    break;
                }
            }
            if (found == 1) {
                break;
            }
        }
    }
    if (found == 0) {
        registers[28] = 0; // failed, set to 0
        return;
    }

    // set these blocks as used
    for (int j = 0; j < num_banks; j++) {
        mem_bank->used[i + j] = '1';
    }
    // set the value of the pointer to the first bank
    registers[28] = START_MALLOC_MEM + i * 64;
    // create a new node for linked list
    struct node* curr_alloc = (struct node*)calloc(1, sizeof(struct node));
    // initialise values
    curr_alloc->address = START_MALLOC_MEM + i * 64;
    curr_alloc->size = size;
    curr_alloc->num_banks = num_banks;
    curr_alloc->next = head; // we always add to FRONT of list
    head = curr_alloc; // new head of list
}

// freeing malloced memory function
int free_mem(int addr, struct decoded* inst, struct mem_banks* mem_bank) {
    if (head == NULL) {
        return 0;
    }
    struct node* temp = head;
    struct node* prev = NULL;
    while (temp->address != (int32_t)addr) {
        if (temp->next == NULL) {
            return 0; // given address not here
        }
        else {
            prev = temp;
            temp = temp->next;
        }
    }
    // set used[i] to 0
    for (int i = 0; i < temp->num_banks; i++) {
        mem_bank->used[(temp->address - START_MALLOC_MEM) / 64 + i] = '0';
    }
    // remove from linked list
    if (temp == head) { // if at front of list
        head = head->next; // set head to next node
    }
    else { // if not at front
        prev->next = temp->next; // the previous node points to node after!!
    }
    free(temp); // free unallocated node
    return 1;
}

// VIRTUAL ROUTIME CHECKInG FUNCTIONS===========================================
// virtual routines for store operations
int check_vrs_s(struct decoded* inst, struct cblob* cblob, 
                struct mem_banks* mem_bank, uint32_t val, 
                int size, unsigned char line[5]) {
    // val = value being stored
    int addr = registers[inst->rs1] + inst->imm;
    if (addr == START_VR_MEM) { // 0x800
        printf("%c", val); // print character
    }
    else if (addr == 2052) {
        printf("%d", val); // print signed int
    }
    else if (addr == 2056) {
        printf("%x", val); // print unsigned int as hex
    }
    else if (addr == 2060) {
        printf("CPU Halt Requested\n"); // end program
        free_linked_list(); // free mallocs before exit
        exit(0);
    }
    else if (addr == 2080) { // 0x820
        printf("0x%02x", PC); // pc dump, print PC as hex
    }
    else if (addr == 2084) { // 0x824
        reg_dump(); // print all registers as hex
    }
    else if (addr == 2088) { // 0x828
        // check memory dump can be made at address
        // sends off to relevant memory (instruction, data, malloc)
        // checks address is valid
        // if address in malloc memory, MUST be allocated
        int res = attempt_mem_dump(val, cblob, mem_bank, size);
        if (res == 0) {
            illegal_operation(line);
        }
    }
    else if (addr == 2096) { // try to get malloced memory
        request_mem(val, inst, mem_bank);
        // print_list();
    }
    else if (addr == 2100) { // try to free malloced memory
        int res = free_mem(val, inst, mem_bank);
        if (res == 0) {
            illegal_operation(line);
        }
        // print_list();
 
    }
    else { // failed, address not valid routine
        return 0;
    }
    return 1; // successful
}

// virtual routine for load operations (only 2)
int check_vrs_l(int addr, int rd) {
    if (addr == 2066) { // scan char from stdin
        char c;
        int ret = scanf("%c", &c);
        if (ret != 1) {
            return 0;
        }
        registers[rd] = c;
    }
    else if (addr == 2070) { //0x816
        int32_t i; // scan int from stdin
        int ret = scanf("%d", &i);
        if (ret != 1) {
            return 0;
        }
        registers[rd] = i;
    }
    else {
        return 0;
    }
    return 1;
}
