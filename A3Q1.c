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

// sets a page table entry for a specific VPN
int setPageTableEntry(PageTable* pageTable, int VPN) {
    // Check if the VPN already has a frame mapped to it and if so, leave it alone
    if (pageTable->entries[VPN].frameNumber != -1) {
        // Entry is already mapped
        printf("Page %d has already been mapped to frame %d\n", VPN, pageTable->entries[VPN].frameNumber);
        return 0; // Success, nothing to do
    }

    // find the lowest unused frame number
    int frameNumber;
    for (frameNumber = 0; frameNumber < NUMBEROFFRAMES; ++frameNumber) {
        int isUsed = 0;
        for (int i = 0; i < NUMBEROFPAGES; ++i) {
            if (pageTable->entries[i].frameNumber == frameNumber) {
                isUsed = 1;
                break;
            }
        }
        if (!isUsed) {
            break; // found unused frame number
        }
    }
    if (frameNumber == NUMBEROFFRAMES) {
        fprintf(stderr, "Error: No available frames to map to VPN %d\n", VPN);
        //call clock algorithm to evict an existing page and take its page frame number: frame number = clockalgorithm()
        return -1;  //remove this once clock algorithm is in place.
    }
    pageTable->entries[VPN].frameNumber = frameNumber;
    pageTable->entries[VPN].presentBit = 1;
    pageTable->entries[VPN].useBit = 1;
    printf("Page %d has been successfully mapped to frame %d\n", VPN, frameNumber);
    return 0; // Success
}

void initializePageTable(PageTable* pageTable) {
    for (int i = 0; i < NUMBEROFPAGES; ++i) {
        pageTable->entries[i].frameNumber = -1;
        pageTable->entries[i].presentBit = 0;
        pageTable->entries[i].useBit = 0;
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


void generateRandom(PageTable* pageTable, int addresses[], int n) {
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
        setPageTableEntry(pageTable, VPN);
    }
}



int main(int argc, char *argv[])
{
    // do we assume physical memory space is larger than virtual memory space (i.e., all pages can be stored in physical memory at once?)
    PageTable pageTable;
    initializePageTable(&pageTable);
    printPageTable(&pageTable);
    int addresses[20];
    generateRandom(&pageTable, addresses, 20);
    for (int i=0; i<20; i++) {
        printf("%d ",addresses[i]);
    }
    printPageTable(&pageTable);
    return 0;
    //sequence?: generate virtual addresses (referencing a certain page), then reference certain virtual addresses (and add the pages to the page table; which reference the physical location of the page), and when the page table gets full, use clock algorithm to replace page table entries 

}