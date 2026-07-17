#ifndef ALLOCATE_H
#define ALLOCATE_H

#include <h3.h>

#define MAX_NAME_SIZE   16

typedef enum {blockOwnerUsed, blockOwnerLocked, blockOwnerEmpty} BlockOwnerStatus;
typedef enum {blockUsed, blockEmpty} BlockStatus;
typedef enum 
{
  normalPlacement,  //Place from lower memory up 
  upperPlacement,   //Place as high up as possible
  randomPlacement,  //Place at a random location with enough space
  randomBelowSixteenMegPlacement  //Place randomly under 16 meg (for Cmd FIFO)
} PlacementStrategy;

typedef struct 
{
  BlockOwnerStatus status;
  char name[MAX_NAME_SIZE];
  FxU32 address;  //Starting address of memory
  FxU32 size;     //Size of memory owned by this owner  
  FxU32 nBlocks;  //Number of blocks owned by this owner
} BlockOwner;

typedef struct block
{  
  BlockStatus status;
  BlockOwner *owner;
} Block;

typedef struct
{
  FxU32 memorySize;   //The size of the memory space
  FxU32 nBlocks;      //Total number of blocks in pool
  FxU32 blockSize;    //The size of each block
  FxU32 nBlockOwners; //The number of memory chunks that can be allocated  


  FxU32 nFreeBlocks;       //Number of unallocated blocks
  FxU32 nFreeBlockOwners;  //Number of unused block owners

  FxBool allowJiggle;      //Should the memory be place randomly in the allocated space?

  BlockOwner *blockOwners;
  Block *blocks;

  FxU32 highestAddress;   //The highest memory location used
} MemoryManager;


//Global variable that keeps track of current context
extern MemoryManager *currentMemoryManager;


FxU32 allocate(FxU32 size, char *name, PlacementStrategy placement);
FxBool allocateWorker(FxU32 *address, FxU32 size, char *name, PlacementStrategy placement, FxBool dieOnFailure);
FxBool getEmptyRange(FxU32 *start, FxU32 *stop, FxBool startOver);
FxU32 getFreeMem(void);
FxU32 getHighestAddress(void);
FxU32 getMaxExpansion(FxU32 address);  //Tells how far a certain memory segment can be extended
char* getName(FxU32 address);
FxBool lockByName(char *name);        //Doesn't allow segment to be unallocated
FxBool lockByOwner(BlockOwner *owner);
void memoryMap(void);
MemoryManager* newMemoryManager(FxU32 memorySize, FxU32 blockSize, FxU32 nBlockOwners);
FxBool unallocate(FxU32 address);
FxBool unallocateAll(void);
FxBool unallocateByName(char *name);
FxBool unallocateByOwner(BlockOwner *owner);
FxBool unlockByName(char *name);        //Unlocks a locked segment which allow for deallocation
FxBool unlockByOwner(BlockOwner *owner);


#endif
