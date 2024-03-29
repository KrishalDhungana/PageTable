#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define VIRTUALSIZE 100
#define NUMBEROFPAGES 12
#define PHYSICALSIZE 200
const int PAGESIZE = VIRTUALSIZE / NUMBEROFPAGES;  //size of pages = size of page frames
const int NUMBEROFFRAMES = PHYSICALSIZE / PAGESIZE; //if number of frames < number of pages, we need to implement disk and clock algorithm

typedef struct {
    int frameNumber; // corresponds to frame (in physical memory) containing page
    //NOTE: number of page frames do not have to equal the number of pages 
    int presentBit; // 0 or 1
    int useBit; // 0 or 1
} PageTableEntry;

typedef struct {
    PageTableEntry entries[NUMBEROFPAGES]; 
    //page table has NUMBEROFPAGES entries, each of size 256/10
    //in a virtual address, we need to specify the page number and offset.
    //page number can vary from 0 (0000) to 11 (1011) (since we have 12 pages)
    //offset can vary from 0 (000) to 7 (111) (??round down??)
    //So, a virtual address should have 4 bits for page number and 3 bits for offset 
} PageTable;

int setPageTableEntry(PageTable* pageTable, PageTableEntry* entry, int frameNumber, int presentBit, int useBit) {
    if (frameNumber < -1 || frameNumber >= NUMBEROFFRAMES) {
        fprintf(stderr, "Error: Frame number %d is out of bounds (0 - %d).\n", frameNumber, NUMBEROFFRAMES - 1);
        return -1; 
    }
    if (presentBit != 0 && presentBit != 1) {
        fprintf(stderr, "Error: Present bit %d is not valid. Must be 0 or 1.\n", presentBit);
        return -1;
    }
    if (useBit != 0 && useBit != 1) {
        fprintf(stderr, "Error: Use bit %d is not valid. Must be 0 or 1.\n", useBit);
        return -1;
    }
    entry->frameNumber = frameNumber;
    entry->presentBit = presentBit;
    entry->useBit = useBit;
    return 0; // return 0 if all insertions are successful
}

void initializePageTable(PageTable* pageTable) {
    for (int i = 0; i < NUMBEROFPAGES; ++i) {
             // set all values to 0 at the start since no pages exist in physical memory yet
             setPageTableEntry(pageTable, &pageTable->entries[i], -1,0,0);
    }
}

void printPageTable(PageTable* pageTable) {
    printf("Page Table Entries:\n");
    for (int i = 0; i < NUMBEROFPAGES; ++i) {
        printf("Page %d: Frame Number: %d, Present Bit: %d, Use Bit: %d\n", 
               i, pageTable->entries[i].frameNumber, 
               pageTable->entries[i].presentBit, 
               pageTable->entries[i].useBit);
    }
}

//aux function for generateRandom
int bitwiseAdd(int x, int y) {
    while (y != 0) {
        int carry = x & y;
        x = x ^ y; 
        y = carry << 1;
    }
    return x;
}
//aux function for generateRandom
int bitsNeeded(int value) {
    return (int)ceil(log2(value));
}
//aux function for generateRandom
bool isAddressUnique(int addresses[], int n, int address) {
    for (int i = 0; i < n; i++) {
        if (addresses[i] == address) {
            return false; // Address already exists
        }
    }
    return true; // Unique address
}

//add to page table 


void generateRandom(int addresses[], int n) {
    int pageBits = bitsNeeded(NUMBEROFPAGES);
    int offsetBits = bitsNeeded(PAGESIZE);
    int maxUniqueAddresses = NUMBEROFPAGES * PAGESIZE; // Maximum possible unique addresses
    if (n > maxUniqueAddresses) {
        printf("Requested number of unique addresses exceeds the available address space.\n");
        return; // Early exit to avoid infinite loop
    }
    for (int i = 0; i < n; i++) {
        int VPN, offset, virtualAddress;  //must initialize here or else do-while won't work
        do {
            VPN = rand() % NUMBEROFPAGES;
            offset = rand() % PAGESIZE;
            int shiftAmount = offsetBits; // Shift by the number of bits needed for the offset
            virtualAddress = (VPN << shiftAmount) | offset;
        } while (!isAddressUnique(addresses, i, virtualAddress));
        addresses[i] = virtualAddress;
        printf("VPN = %d & offset = %d, virtual address: %d\n", VPN, offset, virtualAddress);
    }
}



int main(int argc, char *argv[])
{
    // do we assume physical memory space is larger than virtual memory space (i.e., all pages can be stored in physical memory at once?)
    PageTable pageTable;
    initializePageTable(&pageTable);
    printPageTable(&pageTable);
    int addresses[20];
    generateRandom(addresses, 20);
    for (int i=0; i<20; i++) {
        printf("%d ",addresses[i]);
    }
    return 0;
    //sequence?: generate virtual addresses (referencing a certain page), then reference certain virtual addresses (and add the pages to the page table; which reference the physical location of the page), and when the page table gets full, use clock algorithm to replace page table entries 

}