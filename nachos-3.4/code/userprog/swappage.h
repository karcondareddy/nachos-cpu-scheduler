#ifndef SWAPPAGE_H
#define SWAPPAGE_H

class SwapPage{
public:
	SwapPage();
	~SwapPage();
	int PageIn(int physicalPageNo);
	int PageOut(int physicalPageNo);
private:
	char *page;
};

#endif