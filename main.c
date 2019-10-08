#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 100
#define MAX_SIZE 65000
#define BLOCKHEAD (2 * sizeof(unsigned short))
void *memory;

typedef struct Block {
    unsigned short prevBlockFreeSize;
    unsigned short size;
    int pPrevFree;
    int pNextFree;
} BLOCK;
/*
typedef struct BigHeader {
    unsigned int p8;
    int p12;
    int p16;
    int p32;
    int pMore;
} BIG_HEADER;
*/
typedef struct Header {
    int p8;
    int p12;
    int pMore;
} HEADER;

char *offset(void *ptr, int offset);
int is_valid(void *valid_ptr);
char *get_memory();
HEADER *get_header();



void memory_init(void *ram, unsigned int size) {
    memory = ram;
    *((unsigned int *)memory) =  size;
    struct Header *header = (HEADER *)(get_memory() + sizeof(int));
    header->p8 = 0;
    header->p12 = 0;
    header->pMore = sizeof(HEADER) + sizeof(int);
    struct Block *block = (BLOCK *) (get_memory() + header->pMore);
    if (size < MAX_SIZE) {
        block->size = size - (sizeof(HEADER) + BLOCKHEAD + sizeof(int));
    } else
        block->size = MAX_SIZE;
    block->prevBlockFreeSize = 0;
    block->pPrevFree = 0;
    block->pNextFree = 0;
}

char *get_memory() {
    return (char *) memory;
}

char *offset(void *ptr, int offset) {
    if (is_valid((char *)((char *)ptr + offset)))
        return (char *)((char *)ptr + offset);
    else
        return NULL;
}

int is_valid(void *valid_ptr) {
    if (((char *)valid_ptr > get_memory() + sizeof(HEADER) + sizeof(int) - 1) &&
        ((char *)valid_ptr < (get_memory() + *((unsigned int *)memory)))) {
        return 1;
    }
    else return 0;
}
//helper function to find the index of a block
int index_of(struct Block *block) {
    int index = (int)((char *)block - (char *)memory);
    return index;
}

//inserts a block into the correct list according to its size
void free_list_insert(struct Block *currentBlock, unsigned short freedSpace) {
    struct Header *header = get_header();
    struct Block *freeListHead;
    int newIndex = index_of(currentBlock);
    if (freedSpace > 3 * sizeof(int)) {
        freeListHead = (BLOCK *)offset(memory, header->pMore);
        if(freeListHead != NULL) {
            freeListHead->pPrevFree = newIndex;
            currentBlock->pNextFree = header->pMore;
        }
        header->pMore = newIndex;
    } else if (freedSpace > 2 * sizeof(int)) {
        freeListHead = (BLOCK *)offset(memory, header->p12);
        if(freeListHead != NULL) {
            freeListHead->pPrevFree = newIndex;
            currentBlock->pNextFree = header->p12;
        }
        header->p12 = newIndex;
    } else {
        freeListHead = (BLOCK *)offset(memory, header->p8);
        if(freeListHead != NULL) {
            freeListHead->pPrevFree = newIndex;
            currentBlock->pNextFree = header->p8;
        }
        header->p8 = newIndex;
    }
}

struct Block *get_next_block(struct Block *block) {
    struct Block *neighbor = (BLOCK *)offset(block, block->size + (int)BLOCKHEAD);
    if(is_valid(neighbor)) {
        return neighbor;
    } else
        return NULL;
}

int is_free(struct Block *block) {
    int index = index_of(block);
    int index2 = block->size;
    if(get_next_block(block)->prevBlockFreeSize) {
        return 1;
    } else
        return 0;
}

void pull(struct Block *block) {
    if(get_header()->pMore == index_of(block)) {
        get_header()->pMore = block->pNextFree;
    }
    if(get_header()->p12 == index_of(block)) {
        get_header()->p12 = block->pNextFree;
    }
    if(get_header()->p8 == index_of(block)) {
        get_header()->p8 = block->pNextFree;
    }
    struct Block *next = (BLOCK *)offset(memory, block->pNextFree);
    if (next != NULL) {
        next->pPrevFree = block->pPrevFree;
    }
    struct Block *prev = (BLOCK *)offset(memory, block->pPrevFree);
    if (prev != NULL) {
        prev->pNextFree = block->pNextFree;
    }
}
//merges all free blocks around the freed block (helper for the memory_free() function)
void merge(struct Block *currentBlock) {
    unsigned short newSize = currentBlock->size;
    if(is_free(get_next_block(currentBlock))) {
        newSize += get_next_block(currentBlock)->size + BLOCKHEAD;
        pull(get_next_block(currentBlock));
    }
    if(currentBlock->prevBlockFreeSize) {
        int off = currentBlock->prevBlockFreeSize + (int)BLOCKHEAD;
        newSize += off;
        off *= -1;
        currentBlock = (BLOCK *)offset(currentBlock, off);
        pull(currentBlock);
        currentBlock->size = newSize;
    }
    get_next_block(currentBlock)->prevBlockFreeSize = newSize;
    free_list_insert(currentBlock, newSize);
}

int memory_free(void *valid_ptr) {
    if (is_valid(valid_ptr) && valid_ptr != NULL) {
        int off = BLOCKHEAD;
        off *= -1;
        struct Block *currentBlock = (BLOCK *)offset(valid_ptr, off);
        merge(currentBlock);
        return 1;
    } else {
        return 0;
    }
}

void split(struct Block *block, int size) {
    int newSize = block->size - size;
    block->prevBlockFreeSize = newSize - BLOCKHEAD;
    struct Block *newBlock = (BLOCK *)offset(block, size + (int)BLOCKHEAD);
    newBlock->prevBlockFreeSize = 0;
    newBlock->size = newSize;
    if (get_next_block(block) != NULL) {
        get_next_block(block)->prevBlockFreeSize = newSize;
    }
    free_list_insert(newBlock, newSize);
};

HEADER *get_header() {
    char *ptr = memory + sizeof(int);
    return (HEADER *)ptr;
}

void alloc_block(struct Block *block, unsigned int size) {
    pull(block);
    unsigned short newSize = block->size - size;
    if (newSize > (BLOCKHEAD + (2 * sizeof(int)))) {
        split(block, (int)size);
        block->size = size;
    } else {
        get_next_block(block)->prevBlockFreeSize = 0;
        block->size = size;
    }

    char *array = offset(block, BLOCKHEAD);
    for (int i = 0; i < (int)size; ++i) {
        array[i] = '1';
    }
}

void *memory_alloc(unsigned int size) {
    void *pointer = NULL;
    BLOCK *block;
    HEADER *header = get_header();

    //Determine the size of the allocated space and where to look for it.
    if(size > 12) {
        block = (BLOCK *)offset(memory, header->pMore);
        if (block == NULL) {
            return NULL;
        }
    } else if (size > 8) {
        block = (BLOCK *)offset(memory, header->p12);
        if (block == NULL) {
            block = (BLOCK *)offset(memory, header->pMore);
            if (block == NULL) {
                return NULL;
            }
        }
    } else {
        block = (BLOCK *)offset(memory, header->p8);
        if (block == NULL) {
            block = (BLOCK *) offset(memory, header->p12);
            if (block == NULL) {
                block = (BLOCK *) offset(memory, header->pMore);
                if (block == NULL) {
                    return NULL;
                }
            }
        }
    }

    do {
        if (block->size > size) {
            alloc_block(block, size);
            pointer = offset(block, BLOCKHEAD);
            break;
        }
        block = (BLOCK *)offset(memory, block->pNextFree);
    } while (block->pNextFree);
    return pointer;
}

int memory_check(void *ptr) {
    int val = BLOCKHEAD;
    val *= -1;
    return !is_free((BLOCK *) offset(ptr, val));
}

int main() {
    char ram[SIZE];
    memory_init(ram, SIZE);
    char* mem = (char*)memory_alloc(10);
    printf("%d\n", memory_check(mem));
    if (mem)
        memset(mem, 0, 10);
    if (mem)
        memory_free(mem);
    getchar();
    return 0;
}