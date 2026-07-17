#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allocate.h"
#include "udiag.h"

//This is a hookeee implementation of malloc like functions
//It's really slow and bloated, but the functions are
//called very frequently.


//Global variable that keeps track of current context
MemoryManager *currentMemoryManager=NULL;

FxU32 allocate(FxU32 size, char *name, PlacementStrategy placement)
{
  FxU32 address;

  if(allocateWorker(&address, size, name, placement, FXTRUE) == FXFALSE)
    {
      GDBG_ERROR("allocate", "This should never occur %s(%d)\n", __FILE__, __LINE__);
      DIAG_FAIL();
    }
  
  return(address);
}

FxBool allocateWorker(FxU32 *address, FxU32 size, char *name, PlacementStrategy placement, FxBool dieOnFailure)
{
  FxU32 i, start, stop;
  FxU32 nBlocks;   //free blocks needed to fulfill request
  FxU32 startBlock;    //block to start searching on
  FxI32 direction;     //direction to search for blocks
  FxU32 wrapBlock;     //Block to wrap search on
  FxU32 nSearchBlocks; //Number of blocks to search 
  FxU32 randomNumber;
  BlockOwner *blockOwner;

  assert(currentMemoryManager!=NULL);

  nBlocks = (size + currentMemoryManager->blockSize - 1)/currentMemoryManager->blockSize;

  //Check to see if there's an owner available
  if(currentMemoryManager->nFreeBlockOwners < 1)
    {
      if(dieOnFailure)
	{
	  memoryMap();
	  GDBG_ERROR("allocate", "Ran out of memory owners! %s(%d)\n", __FILE__, __LINE__);
	  DIAG_FAIL();
	}
      else
	return(FXFALSE);
    }

  //Necessary, but not sufficient condition for memory needed
  if(currentMemoryManager->nFreeBlocks < nBlocks)
    {
      if(dieOnFailure)
	{
	  memoryMap();
	  GDBG_ERROR("allocate", "Not enough memory to allocate \"%s\"(%d bytes)! %s(%d)\n", name, 
		     size, __FILE__, __LINE__);
	  DIAG_FAIL();
	}
      else
	return(FXFALSE);
    }

  //find the blockOwner;
  for(i=0; i<currentMemoryManager->nBlockOwners; i++)
    if(currentMemoryManager->blockOwners[i].status == blockOwnerEmpty)
      {
	blockOwner = &currentMemoryManager->blockOwners[i];
	break;
      }
    

  switch(placement)
    {
    case normalPlacement:
      startBlock = 0;
      direction = +1;
      wrapBlock = currentMemoryManager->nBlocks;
      nSearchBlocks = currentMemoryManager->nBlocks;
      break;

    case upperPlacement:
      startBlock = currentMemoryManager->nBlocks - 1;
      direction = -1;
      wrapBlock = currentMemoryManager->nBlocks;
      nSearchBlocks = currentMemoryManager->nBlocks;      
      break;      

    case randomPlacement:

      //Help the random number generator out a little bit by
      //calling it a bunch of times. Otherwise, the output
      //randomness doesn't appear very random
      while(iRandom(15) != 0);
      randomNumber = iRandom(0xFFFF);
      randomNumber |= iRandom(0xFFFF)<<16;

      startBlock = randomNumber % currentMemoryManager->nBlocks;	
      direction = +1;
      wrapBlock = currentMemoryManager->nBlocks;
      nSearchBlocks = currentMemoryManager->nBlocks;
      break;

    case randomBelowSixteenMegPlacement:

      //Help the random number generator out a little bit by
      //calling it a bunch of times. Otherwise, the output
      //randomness doesn't appear very random
      while(iRandom(15) != 0);
      randomNumber = iRandom(0xFFFF);
      randomNumber |= iRandom(0xFFFF)<<16;

      startBlock = randomNumber % currentMemoryManager->nBlocks;	
      direction = +1;
      wrapBlock = 16*1024*1024 / currentMemoryManager->blockSize;
      nSearchBlocks = currentMemoryManager->nBlocks;

      if(nSearchBlocks > wrapBlock)
	nSearchBlocks = wrapBlock;
      break;
    }

  //Try to find the blocks
  for(i=0, start = startBlock; i < nSearchBlocks; start += direction, i++)
    {
      //You can't trust mod of negatives to work the same on all processors
      if(start < 0)
	start += wrapBlock;

      start = start % wrapBlock; 

      if(currentMemoryManager->blocks[start].status == blockEmpty)
	{
	  //Check to see if there are enough contiguous blocks here
	  FxU32 nNeededBlocks = nBlocks;
	  
	  stop=start;
	  while(nNeededBlocks > 0)
	    {
	      //Make sure we're still in memory range
	      if(stop >= currentMemoryManager->nBlocks)
		break;

	      //Check that block isn't used
	      if(currentMemoryManager->blocks[stop].status == blockUsed)
		break;	      	      

	      nNeededBlocks--;
	      stop++;
	    }

	  //See if we found a winner
	  if(nNeededBlocks == 0)
	    {
	      //Allocate the owner
	      currentMemoryManager->nFreeBlockOwners --;
	      strncpy(blockOwner->name, name, MAX_NAME_SIZE-1);
	      blockOwner->status = blockOwnerUsed; 
	      blockOwner->address = start * currentMemoryManager->blockSize;
	      blockOwner->size = size;
	      blockOwner->nBlocks = nBlocks;
	      
	      //Jiggle if appropriate
	      if(currentMemoryManager->allowJiggle)
		{
		  FxU32 slop;  //Amount of wasted memory
		  
		  slop = iRandom((nBlocks * currentMemoryManager->blockSize) - size);
		  
		  //Alignment on 64 byte boundaries
		  slop &= 0xFFFFFFC0;
		  
		  blockOwner->address += slop;
		}	      

	      //Allocate the blocks
	      currentMemoryManager->nFreeBlocks -= nBlocks;
	      for(i = start; i < stop; i++)
		{
		  currentMemoryManager->blocks[i].status = blockUsed;
		  currentMemoryManager->blocks[i].owner = blockOwner;		      
		}
	      
	      //Check if this is the highest memory used so far
	      if(blockOwner->address + size - 1> currentMemoryManager->highestAddress)
		currentMemoryManager->highestAddress = blockOwner->address + size - 1;
	      
	      //God damn bastard hack that had to be added to support the dumb ass
	      //old way of allocating framebuffer space. Otherwise, a lot of
	      //diags would have to be modified.
	      diago.maxTrashMem = currentMemoryManager->highestAddress;

	      //Return the pointer
	      *address = blockOwner->address;

	      return(FXTRUE);
	    }
	  else  //if(nNeededBlocks == 0)     implies this chunk isn't big enough
 	    {
	      if(direction==1)
		{ //If this block failed, then all the following blocks up to the used block will too.
		  i+= stop-start;
		  start += stop-start-1;
		}
	    }
	}      
    }
  
  //Not a big enough chunk of memory available
  if(dieOnFailure)
    {
      memoryMap();
      GDBG_ERROR("allocate", "Not enough memory for allocate \"%s\"(%d bytes)! %s(%d)\n", name,
		 size, __FILE__, __LINE__);
      DIAG_FAIL();  
    }

  return(FXFALSE);
}

//This returns a range of non-allocated memory
FxBool getEmptyRange(FxU32 *start, FxU32 *stop, FxBool startOver)
{
  static FxU32 lastBlockIndex=0;
  FxU32 blockIndex;

  if(startOver)
    lastBlockIndex = 0;

  for(blockIndex = lastBlockIndex; blockIndex < currentMemoryManager->nBlocks; blockIndex++)
    {
      if(currentMemoryManager->blocks[blockIndex].status == blockEmpty)
	{
	  *start = blockIndex * currentMemoryManager->blockSize;

	  for(; blockIndex < currentMemoryManager->nBlocks; blockIndex++)
	    if(currentMemoryManager->blocks[blockIndex].status == blockUsed)
	      break;

	  lastBlockIndex = blockIndex;
	  
	  *stop = (blockIndex-1) * currentMemoryManager->blockSize;
	  return(FXTRUE);
	}
    }

  return(FXFALSE);
}

FxU32 getFreeMem(void)
{
  return(currentMemoryManager->nFreeBlocks * currentMemoryManager->blockSize);
}

//Tells how far a certain memory segment can be extended
FxU32 getMaxExpansion(FxU32 address)
{
  FxU32 blockIndex;
  BlockOwner *blockOwner;

  if(address >= currentMemoryManager->memorySize)
    {
      GDBG_ERROR("getMaxExpansion", "Illegal address of 0x%x! %s(%d)\n",
		 address, __FILE__, __LINE__);
      DIAG_FAIL();
    }
  
  blockIndex = address / currentMemoryManager->blockSize;
  blockOwner = currentMemoryManager->blocks[blockIndex].owner;

  while(blockIndex < currentMemoryManager->nBlocks &&
	(currentMemoryManager->blocks[blockIndex].status == blockEmpty ||
	 currentMemoryManager->blocks[blockIndex].owner == blockOwner))
    blockIndex++;
  
  return(blockIndex*currentMemoryManager->blockSize - 1);
}

//This returns the highest memory location used
FxU32 getHighestAddress(void)
{
  return(currentMemoryManager->highestAddress);
}

//This returns the name of an allocated segment
char* getName(FxU32 address)
{
  FxU32 i;
  BlockOwner *blockOwner;

  assert(currentMemoryManager!=NULL);
  
  for(i=0; i<currentMemoryManager->nBlockOwners; i++)
    {
      blockOwner = &currentMemoryManager->blockOwners[i];
      if(blockOwner->status = blockOwnerUsed)
	{
	  if(address >= blockOwner->address &&
	     address < blockOwner->address + blockOwner->size)
	      return(blockOwner->name);
	}
    }

  return("UNALLOCATED!");
}


//Doesn't allow segment to be unallocated
FxBool lockByName(char *name)
{
  FxU32 i;

  for(i=0; i<currentMemoryManager->nBlockOwners; i++)
    if(!strncmp(currentMemoryManager->blockOwners[i].name, name, strlen(name)))
      {
	GDBG_INFO(1, "locking segment \"%s\"\n",
		  currentMemoryManager->blockOwners[i].name);
	
	lockByOwner(&currentMemoryManager->blockOwners[i]);
      }

  return(FXTRUE);
}

FxBool lockByOwner(BlockOwner *owner)
{
  if(owner->status == blockOwnerEmpty)
    return(FXFALSE);
  
  owner->status = blockOwnerLocked;

  return(FXTRUE);
}


void memoryMap(void)
{
  FxU32 i;
  BlockOwner *blockOwner;

  GDBG_INFO(1, "\n");
  GDBG_INFO(1, "\n");
  GDBG_INFO(1, "***************************** Memory Map ******************************\n");
  GDBG_INFO(1, "memorySize: %d  blockSize: %d  nBlocks: %d  nBlockOwners: %d\n", 
	    currentMemoryManager->memorySize, currentMemoryManager->blockSize,
	    currentMemoryManager->nBlocks, currentMemoryManager->nBlockOwners);
  GDBG_INFO(1, "nFreeBlocks: %d  nFreeBlockOwners: %d\n", currentMemoryManager->nFreeBlocks,
	    currentMemoryManager->nFreeBlockOwners);
  GDBG_INFO(1, "highestAddress: 0x%x\n", currentMemoryManager->highestAddress);
  GDBG_INFO(1, "***********************************************************************\n");

  for(i=0; i<currentMemoryManager->nBlockOwners; i++)
    {
      blockOwner = &currentMemoryManager->blockOwners[i];
      
      if(blockOwner->status == blockOwnerUsed)
	{
	  GDBG_INFO(1, "%-16s: address=0x%08x  size=%-8d  nBlocks=%-4d\n", blockOwner->name, 
		 blockOwner->address, blockOwner->size, blockOwner->nBlocks);
	}
      else if(blockOwner->status == blockOwnerLocked)
	{
	  GDBG_INFO(1, "%-16s: address=0x%08x  size=%-8d  nBlocks=%-4d lock\n", blockOwner->name, 
		 blockOwner->address, blockOwner->size, blockOwner->nBlocks);
	}
    }

  GDBG_INFO(1, "***********************************************************************\n");
  GDBG_INFO(1, "\n");  
}



MemoryManager* newMemoryManager(FxU32 memorySize, FxU32 blockSize, FxU32 nBlockOwners)
{
  FxU32 nBlocks;
  MemoryManager *memoryManager;
  Block *blocks;
  BlockOwner *blockOwners;
  FxU32 i;

  assert(memorySize > 0);
  assert(blockSize > 0);
  assert(nBlockOwners > 0);
  assert((memorySize % blockSize) == 0);

  //Calculate how many blocks there will be
  nBlocks = memorySize / blockSize;

  //Allocate and initialize the blocks
  blocks = (Block *)malloc(nBlocks*sizeof(Block));
  for(i=0; i<nBlocks; i++)
    {
      blocks[i].status=blockEmpty;
      blocks[i].owner=NULL;
    }

  //Allocate and initialize the blockOwners
  blockOwners = (BlockOwner *)malloc(nBlockOwners*sizeof(BlockOwner));
  for(i=0; i<nBlockOwners; i++)
    {
      blockOwners[i].status = blockOwnerEmpty;
      strncpy(blockOwners[i].name, "EMPTY", MAX_NAME_SIZE);
      blockOwners[i].size=0;      
      blockOwners[i].nBlocks=0;
    }

  //Initialize the memory manager
  memoryManager=(MemoryManager *)malloc(sizeof(MemoryManager));
  memoryManager->memorySize = memorySize;
  memoryManager->nBlocks = nBlocks;
  memoryManager->blockSize = blockSize;
  memoryManager->nBlockOwners = nBlockOwners;
  memoryManager->blocks = blocks;
  memoryManager->blockOwners = blockOwners;
  memoryManager->nFreeBlocks = nBlocks;
  memoryManager->nFreeBlockOwners = nBlockOwners;
  memoryManager->allowJiggle = FXTRUE;
  memoryManager->highestAddress = 0;

  currentMemoryManager=memoryManager;

  return(memoryManager);
}


FxBool unallocate(FxU32 address)
{
  FxU32 i;

  for(i=0; i<currentMemoryManager->nBlockOwners; i++)
    if(currentMemoryManager->blockOwners[i].status == blockOwnerUsed)
      if(currentMemoryManager->blockOwners[i].address == address)
	unallocateByOwner(&currentMemoryManager->blockOwners[i]); 
  
  return(FXTRUE);
}

FxBool unallocateAll(void)
{
  FxU32 i;
  
  for(i=0; i<currentMemoryManager->nBlockOwners; i++)
    if(currentMemoryManager->blockOwners[i].status == blockOwnerUsed)
      unallocateByOwner(&currentMemoryManager->blockOwners[i]); 

  return(FXTRUE);
}

//This unallocates memory based on the name
//A name is matched if the first characters of the
//owners name match the name argument
FxBool unallocateByName(char *name)
{
  FxU32 i;

  for(i=0; i<currentMemoryManager->nBlockOwners; i++)
    if(!strncmp(currentMemoryManager->blockOwners[i].name, name, strlen(name)))
      unallocateByOwner(&currentMemoryManager->blockOwners[i]);
  
  return(FXTRUE);
}

FxBool unallocateByOwner(BlockOwner *owner)
{
  FxU32 i;

  if(owner->status == blockOwnerLocked)
    return(FXFALSE);
  
  GDBG_INFO(2, "unallocating segment \"%s\"\n", owner->name);

  for(i=0; i<currentMemoryManager->nBlocks; i++)
    {
      if(currentMemoryManager->blocks[i].owner == owner)
	{
	  currentMemoryManager->blocks[i].owner  = NULL;
	  currentMemoryManager->blocks[i].status = blockEmpty;
	  currentMemoryManager->nFreeBlocks++;	  
	}
    }
  
  owner->status = blockOwnerEmpty;
  currentMemoryManager->nFreeBlockOwners++;

  //Check to see if we need to rescan for the highest used address
  if(owner->address + owner->size - 1 >= currentMemoryManager->highestAddress)
    {
      if(owner->address + owner->size - 1 != currentMemoryManager->highestAddress)
	{
	  DIAG_FAIL();
	}
      
      for(i=0; i<currentMemoryManager->nBlockOwners; i++)	
	{
	  BlockOwner *ownerPtr;
	  
	  ownerPtr = &currentMemoryManager->blockOwners[i];
	  if(ownerPtr->status != blockOwnerEmpty)
	    {
	      if(ownerPtr->address + ownerPtr->size - 1 > currentMemoryManager->highestAddress)	    
		currentMemoryManager->highestAddress = ownerPtr->address + ownerPtr->size - 1;
	    }
	}
      
      if(i=0)
	currentMemoryManager->highestAddress=0;
    }

  //God damn bastard hack that had to be added to support the dumb ass
  //old way of allocating framebuffer space. Otherwise, a lot of
  //diags would have to be modified.
  diago.maxTrashMem = currentMemoryManager->highestAddress;

  return(FXTRUE);
}

//Doesn't allow segment to be unallocated
FxBool unlockByName(char *name)
{
  FxU32 i;

  for(i=0; i<currentMemoryManager->nBlockOwners; i++)
    if(!strncmp(currentMemoryManager->blockOwners[i].name, name, strlen(name)))
      {
	GDBG_INFO(1, "unlocking segment \"%s\"\n",
		  currentMemoryManager->blockOwners[i].name);
	
	unlockByOwner(&currentMemoryManager->blockOwners[i]);
      }

  return(FXTRUE);
}

FxBool unlockByOwner(BlockOwner *owner)
{
  if(owner->status == blockOwnerEmpty)
    return(FXFALSE);
  
  owner->status = blockOwnerUsed;

  return(FXTRUE);
}
