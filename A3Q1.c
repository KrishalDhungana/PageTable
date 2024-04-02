#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define VIRTUALSIZE 100
#define NUMBEROFPAGES 8
#define PHYSICALSIZE 50
#define VIRTUALADDRESSES 11 // number of virtual addresses
const int PAGESIZE = VIRTUALSIZE / NUMBEROFPAGES;  //size of pages = size of page frames
const int NUMBEROFFRAMES = PHYSICALSIZE / PAGESIZE; //if number of frames < number of pages, we need to implement disk and clock algorithm
int CLOCKINDEX=0;

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

int clockAlgorithm(PageTable* pageTable) { //clock algorithm used to decide which page to evict when memory is full and virtual address needs to be mapped to physcial address
    while (true) {
        //frame number must not be -1. If = -1, this means virtual address is not mapped and therefore there is no physical address to evict
        if (pageTable->entries[CLOCKINDEX].useBit == 0 && pageTable->entries[CLOCKINDEX].frameNumber != -1) {
            int frame = pageTable->entries[CLOCKINDEX].frameNumber; //get number of frame we are evicting
            pageTable->entries[CLOCKINDEX].frameNumber = -1;
            pageTable->entries[CLOCKINDEX].useBit = 0;
            pageTable->entries[CLOCKINDEX].presentBit = 0;
            printf("We evicted page %d, which had frame %d\n", CLOCKINDEX, frame);
            return frame;
        }
        pageTable->entries[CLOCKINDEX].useBit = 0; //if useBit was at 1, change to 0 and keep running
        CLOCKINDEX = (CLOCKINDEX+1)%NUMBEROFPAGES; //increments clock index and loops back to beginning if at the end of the page table
    }
}

// sets a page table entry for a specific VPN
int setPageTableEntry(PageTable* pageTable, int VPN) {
    // Check if the VPN already has a frame mapped to it and if so, leave it alone
    if (pageTable->entries[VPN].frameNumber != -1) {
        // Entry is already mapped
        printf("Page %d has already been mapped to frame %d\n", VPN, pageTable->entries[VPN].frameNumber);
        pageTable->entries[VPN].presentBit = 1;
        pageTable->entries[VPN].useBit = 1;
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

    if (frameNumber == NUMBEROFFRAMES) { //if true, indicates no space left in physcial memory
        fprintf(stderr, "Error: No available frames to map to VPN %d\n", VPN);
        frameNumber = clockAlgorithm(pageTable);
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


//translates specific virtual address to physical address using the page table
int translation(PageTable* pageTable, int VPN, int virtualAddress) { 
    int PFN = pageTable->entries[VPN].frameNumber; //get the page frame number
    int offsetBits = bitsNeeded(PAGESIZE-1); //get the number of bits required for the offset
    int mask = 1;
    for (int i = 0; i < offsetBits-1; i++){ //create the mask 
        mask = mask << 1; //shift bit left once
        mask = mask | 1; //add on a 1 to the end by ORing
    }
    
    int offsetVal = virtualAddress & mask; //get the offset value. Which is the bit form of the virtualAddress that represent the offset
    int physicalAddress = PFN << offsetBits | offsetVal; //left shift by the number of bits needed for the offset then OR with the offset value. physicalAddress = PFN + offset
    printf("Physical Address = %d, PFN = %d & mask = %d & offset = %d \n\n", physicalAddress, PFN, mask, offsetVal);

    // int failedInput = 0;
    // for(int i = 0; i < VIRTUALADDRESSES; i++){ //get an index that is empty to store the address into
    //     if(physAddresses[i] == -1){
    //         physAddresses[i] = physicalAddress;
    //         failedInput = 1; //indicate array succeeded to find a spot to input physcial address into the array
    //         printf("address %d stored into index %d\n", physicalAddress, i);
    //         break; //exit for loop
    //     } 
    // }

    // if(failedInput == 0) printf("Failure: did not input physcial address into array"); //is it necessary to consider the case of if the array if full? is that already considered when dealing with the page table?

    return physicalAddress;
}


void generateRandom(PageTable* pageTable, int addresses[], int n) {
    int pageBits = bitsNeeded(NUMBEROFPAGES-1);
    int offsetBits = bitsNeeded(PAGESIZE-1);
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
        printf("Virtual Address = %d, VPN = %d & offset = %d\n", virtualAddress, VPN, offset);
        setPageTableEntry(pageTable, VPN);
        int physicalAddress = translation(pageTable,VPN,virtualAddress);
    }
}



int main(int argc, char *argv[])
{
    // do we assume physical memory space is larger than virtual memory space (i.e., all pages can be stored in physical memory at once?)
    PageTable pageTable;
    initializePageTable(&pageTable);
    printf("Initial page table: \n");
    printPageTable(&pageTable);
    printf("\n");
    int addresses[VIRTUALADDRESSES]; //this contains all virtual addresses
    generateRandom(&pageTable, addresses, VIRTUALADDRESSES);
    printf("Final page table: \n");
    printPageTable(&pageTable);

    // int physAddresses[VIRTUALADDRESSES]; //this contains all the physical addresses
    // for(int y=0; y<VIRTUALADDRESSES; y++){ //initilize array for physcial addresses as all being -1 to indicate no address has been stored in them
    //     physAddresses[y] = -1;
    // }

    return 0;
    //sequence?: generate virtual addresses (referencing a certain page), then reference certain virtual addresses (and add the pages to the page table; which reference the physical location of the page), and when the page table gets full, use clock algorithm to replace page table entries 

}