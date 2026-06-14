#include "memorymanager.h"
#include "processtable.h"

extern ProcessTable *processTable;
extern int *pageOuts;

MemoryManager::MemoryManager(int numPages)
{
	bitMap = new BitMap(numPages);
	lock = new Lock("lock of memory manager");
	processMap = new int[numPages];
	entries = new TranslationEntry*[numPages];
}

MemoryManager::~MemoryManager()
{
	delete bitMap;
	delete lock;
	delete processMap;
	delete entries;
}

int 
MemoryManager::Alloc(int processNo, TranslationEntry *entry)
{
	lock->Acquire();
	int ret = bitMap->Find();
	if(ret >= 0)
	{
		processMap[ret] = processNo;
		entries[ret] = entry;
	}
	else
	{
		printf("kono jhamela hoiche\n");
	}
	lock->Release();
	return ret;
}

int
MemoryManager::AllocByForce(int processNo, TranslationEntry *entry)
{
	lock->Acquire();
	int ret;//victim page
	//kon physical page ke evict korbo

	//random
	ret = Random() % NumPhysPages;
	//ret = (Random() % (NumPhysPages - 1)) + 1;
	
	//lru
	/*ret = 1;
	for(int i = 1; i < NumPhysPages; ++i)
	{
		if(entries[i]->lastAccessed < entries[ret]->lastAccessed)
		{
			ret = i;
		}
	}*/

	//printf("#%d of physical page will be evicted\n", ret);

	if(entries[ret] != NULL)
	{

		Thread* t = (Thread*) processTable->Get( processMap[ret] );
		int isPageOut = t->space->saveIntoSwapSpace( entries[ret]->virtualPage );

		if(isPageOut)
			pageOuts[processNo]++;

		processMap[ret] = processNo;
		entries[ret] = entry;
	}
	else
	{
		printf("entry is not valid\n");
	}
	lock->Release();
	return ret;
}

int
MemoryManager::AllocPage()
{
	lock->Acquire();
	int ret = bitMap->Find();
	lock->Release();
	return ret;
}

void
MemoryManager::FreePage(int physPageNum)
{
	lock->Acquire();
	bitMap->Clear(physPageNum);
	lock->Release();
}

bool
MemoryManager::PageIsAllocated(int physPageNum)
{
	lock->Acquire();
	bool ret = bitMap->Test(physPageNum);
	lock->Release();
	return ret;
}

bool
MemoryManager::IsAnyPageFree()
{
	lock->Acquire();
	bool ret;
	if(bitMap->NumClear() == 0)
		ret = false;
	else
		ret = true;
	lock->Release();
	return ret;
}

int
MemoryManager::NumFreePages()
{
	lock->Acquire();
	int ret = bitMap->NumClear();
	lock->Release();
	return ret;
}