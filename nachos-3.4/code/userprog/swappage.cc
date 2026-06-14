#include "swappage.h"
#include "copyright.h"
#include "system.h"

SwapPage::SwapPage()
{
	page = new char[PageSize];
}

SwapPage::~SwapPage()
{
	delete page;
}

int
SwapPage::PageIn(int physicalPageNo)
{
	for(int i = 0; i < PageSize; ++i)
		machine->mainMemory[ physicalPageNo * PageSize + i ] = page[i];
	//printf("page in\n");
	return 1;
}

int
SwapPage::PageOut(int physicalPageNo)
{
	for(int i = 0; i < PageSize; ++i)
		page[i] = machine->mainMemory[ physicalPageNo * PageSize + i ];
	//printf("page out\n");
	return 1;
}