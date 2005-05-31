#include "Core.h"


#if MEM_DEBUG
// force MEM_STATS==1 in MEM_DEBUG mode (both uses dual-linked lists of memory blocks)
#undef MEM_STATS
#define MEM_STATS 1
#endif

//!! should gather chains stats for dumpallocs too

#if MEM_STATS
// Stats (make class-static ?)
static int	allocsNum, allocsSize;
static int	chainBlocks, chainsSize;
#endif

#if MEM_DEBUG
unsigned GCurrentMemAllocator;
#endif


/*-----------------------------------------------------------------------------
	Basic memory allocation
-----------------------------------------------------------------------------*/


// 16 bytes long struct (when non-debug version compiled)
struct FMemHeader
{
	byte	magic;					//?? use it
	byte	offset;					// for alignment
#if MEM_STATS
	size_t	size;					// stats
	FMemHeader *prev, *next;		// used for debug
#endif
#if MEM_DEBUG
	address_t owner;
#endif

#if MEM_STATS
	/*------ all blocks are in double-linked list -------*/
	static FMemHeader *first;

	inline void Link (void)
	{
		if (first) first->prev = this;
		next = first;
		prev = NULL;
		first = this;
	}
	inline void Unlink (void)
	{
		if (first == this) first = next;
		if (prev) prev->next = next;
		if (next) next->prev = prev;
	}
#endif
};

#if MEM_STATS
FMemHeader *FMemHeader::first;
#endif


static void OutOfMemory (void)
{
	appError ("Out of memory");
}


void *appMalloc (size_t size, int alignment)
{
	FMemHeader *hdr;
	void	*data;
	size_t	alloc;
	int		pad;

	guardSlow(appMalloc);
	// allocate block
	alloc = size + alignment - 1 + sizeof(FMemHeader);
	hdr = (FMemHeader *) malloc (alloc);
	if (!hdr)
		OutOfMemory ();
	memset (hdr, 0, alloc);
	data = hdr + 1;
	// align pointers
	pad = SizeToAlign (data, alignment);
	hdr = OffsetPointer (hdr, pad);
	data = OffsetPointer (data, pad);
	// fill header
	hdr->offset = pad;
#if MEM_STATS
	hdr->size = size;
	hdr->Link ();
	// update stats
	allocsSize += size;
	allocsNum++;
#endif
#if MEM_DEBUG
	if (GCurrentMemAllocator)
	{
		hdr->owner = GCurrentMemAllocator;
		GCurrentMemAllocator = NULL;
	}
	else
		hdr->owner = GET_RETADDR(size);
#endif

	return data;
	unguardSlow;
}


void appFree (void *ptr)
{
	FMemHeader *hdr;
	void	*block;

	guardSlow(appFree);
//	if (!ptr) return;		// ignore NULL pointers
	// get pointers
	hdr = (FMemHeader *)ptr - 1;
	block = OffsetPointer (hdr, -hdr->offset);
#if MEM_STATS
	// update stats
	allocsSize -= hdr->size;
	allocsNum--;
	// free block
	hdr->Unlink ();
#endif
	free (block);
	unguardSlow;
}


/*-----------------------------------------------------------------------------
	Memory chains
-----------------------------------------------------------------------------*/

#if MEM_DEBUG

CMemoryChain *CMemoryChain::first;

void CMemoryChain::Link (void)
{
	if (first) first->dPrev = this;
	dNext = first;
	dPrev = NULL;
	first = this;
}
void CMemoryChain::Unlink (void)
{
	if (first == this) first = dNext;
	if (dPrev) dPrev->dNext = dNext;
	if (dNext) dNext->dPrev = dPrev;
}

#endif

void *CMemoryChain::operator new (size_t size, int dataSize)
{
	int		alloc;
	CMemoryChain *chain;

	guardSlow(CMemoryChain::new);
	alloc = Align (size + dataSize, MEM_CHUNK_SIZE);
	chain = (CMemoryChain *) malloc (alloc);
	if (!chain)
		OutOfMemory ();
	chain->size = alloc;
	chain->next = NULL;
	chain->data = (byte*) OffsetPointer (chain, size);
	chain->end = (byte*) OffsetPointer (chain, alloc);

	memset (chain->data, 0, chain->end - chain->data);

#if MEM_DEBUG
	chain->owner = GET_RETADDR(size);
	chain->Link ();
#endif

#if MEM_STATS
	// update stats
	chainsSize += alloc;
	chainBlocks++;
#endif

	return chain;
	unguardSlow;
}


void CMemoryChain::operator delete (void *ptr)
{
	CMemoryChain *curr, *next;

	guardSlow(CMemoryChain::delete);
	for (curr = (CMemoryChain *)ptr; curr; curr = next)
	{
#if MEM_STATS
		// update stats
		chainsSize -= curr->size;
		chainBlocks--;
#endif
		// free memory block
		next = curr->next;
#if MEM_DEBUG
		curr->Unlink ();
#endif
		free (curr);
	}
	unguardSlow;
}


void *CMemoryChain::Alloc (size_t size, int alignment)
{
	CMemoryChain *b;
	byte	*start;

	guardSlow(CMemoryChain::Alloc);
	if (!size) return NULL;

	// sequence of blocks (with using "next" field): 1(==this)->5(last)->4->3->2->NULL
	b = (next) ? next : this;				// block for allocation
	start = Align (b->data, alignment);		// start of new allocation
	// check block free space
	if (start + size > b->end)
	{
		//?? may be, search in other blocks ...
		// allocate in the new block
		b = new (size + alignment - 1) CMemoryChain;
		// insert new block immediately after 1st block (==this)
		b->next = next;
		next = b;
		start = Align (b->data, alignment);
	}
	// update pointer to a free space
	b->data = start + size;
#if MEM_DEBUG
	b->owner = owner;						// make all subchains to be owned by a single place
#endif

	return start;
	unguardSlow;
}


#if MEM_STATS
static void Cmd_Meminfo (void)
{
	appPrintf ("Memory usage information:\n"
		"  %8d bytes allocated in %d blocks\n"
		"  %8d bytes allocated in %d chains\n"
		"  ------------------------\n"
		"  %8d bytes total\n",
		allocsSize, allocsNum, chainsSize, chainBlocks, allocsSize + chainsSize);
}
#endif


/*-----------------------------------------------------------------------------
	Debug stuff
-----------------------------------------------------------------------------*/

#if MEM_DEBUG

struct FAllocatorInfo
{
	address_t address;
	unsigned total;
	bool	isChain;					//?? correctly initialized, but not displayed/used
	//?? byte pkgIndex;					// number of package
	unsigned short count;
};

static int CompareAllocators (const FAllocatorInfo *a1, const FAllocatorInfo *a2)
{
	return a1->address - a2->address;
}


//?? used "void*" to avoid FAllocatorInfo declaration in MemoryMgr.h (may be, can do this better ?)
//?? BuildAllocatorsTable declared in MemoryMgr.h as friend of CMemoryChain
static int BuildAllocatorsTable (void *info1, int maxCount)
{
	int		numAllocators, i;
	FAllocatorInfo *p;

	FAllocatorInfo* info = (FAllocatorInfo*)info1;
	// enumerate all memory blocks
	numAllocators = 0;
	GUARD_BEGIN {
		for (FMemHeader *hdr = FMemHeader::first; hdr; hdr = hdr->next)
		{
			// find same owner
			for (i = 0, p = info; i < numAllocators; i++, p++)
				if (p->address == hdr->owner) break;

			if (i >= numAllocators)
			{
				if (numAllocators >= maxCount) continue;	// list is full
				// init new cell
				p->count = 1;
				p->isChain = false;
				p->total = hdr->size;
				p->address = hdr->owner;
				numAllocators++;
			}
			else
			{
				p->count++;
				p->total += hdr->size;
			}
		}
	} GUARD_CATCH {
		appWPrintf ("BuildAllocatorsTable: error in memory blocks\n");
	}
	// enumerate memory chains
	GUARD_BEGIN {
		for (CMemoryChain *ch = CMemoryChain::first; ch; ch = ch->dNext)
		{
			// find same owner
			for (i = 0, p = info; i < numAllocators; i++, p++)
				if (p->address == ch->owner) break;

			if (i >= numAllocators)
			{
				if (numAllocators >= maxCount) continue;	// list ls full
				// init new block
				p->count = 1;
				p->isChain = true;
				p->total = ch->size;
				p->address = ch->owner;
				numAllocators++;
			}
			else
			{
				p->count++;
				p->total += ch->size;
			}
		}
	} GUARD_CATCH {
		appWPrintf ("BuildAllocatorsTable: error in memory chains\n");
	}

	if (numAllocators == maxCount)
		appWPrintf ("maxCount hit\n");

	// sort allocations info by address
	qsort (info, numAllocators, sizeof(FAllocatorInfo), (int (*) (const void*, const void*))CompareAllocators);

	return numAllocators;
}


#define MAX_ALLOCATORS		256

static FAllocatorInfo *MarkedAllocs;
static int numMarkedAllocs;


static void Cmd_DumpAllocs (void)
{
	FAllocatorInfo info[MAX_ALLOCATORS], *p;

	int numAllocators = BuildAllocatorsTable (ARRAY_ARG(info));

	//?? filter symbol names (and/or package name) ?!
	//?? can split by packages and dump per-package info too (requires appSymbolName() returns package info)
	//?? (in this case, should display "/?" help for command)
	int i;
	int totalSize = 0;
	int totalCount = 0;
	for (i = 0, p = info; i < numAllocators; i++, p++)
	{
		char	symbol[256];
		const char *color = p->isChain ? S_GREEN : "";
		if (!appSymbolName (p->address, ARRAY_ARG(symbol)))
			appPrintf ("%8d/%-4d  %s%08X\n", p->total, p->count, color, p->address);
		else
			appPrintf ("%8d/%-4d  %s%s\n", p->total, p->count, color, symbol);
		totalSize += p->total;
		totalCount += p->count;
	}
	appPrintf ("Displayed %d points, %d/%d bytes\n", numAllocators, totalSize, totalCount);
}


static void Cmd_MarkAllocs (bool usage)
{
	FAllocatorInfo info[MAX_ALLOCATORS];

	if (usage)
	{
		appPrintf ("Remembers current memory allocations for later \"mem_check\"\n");
		return;
	}

	numMarkedAllocs = BuildAllocatorsTable (ARRAY_ARG(info));
	if (MarkedAllocs) appFree (MarkedAllocs);
	// NOTE: VC6 produces non-beautiful code here ! (computes block size twice, + asm-level may be much more optimized)
	MarkedAllocs = new FAllocatorInfo[numMarkedAllocs];
	memcpy (MarkedAllocs, info, sizeof(FAllocatorInfo) * numMarkedAllocs);

	appPrintf ("Marked %d memory allocations\n", numMarkedAllocs);
}


static void Cmd_CheckAllocs (bool usage)
{
	FAllocatorInfo info[MAX_ALLOCATORS];
	int		numAllocators;
	// array iterators: "n" - new (current), "m" - marked
	FAllocatorInfo *n, *m;
	int		in, im;

	if (usage)
	{
		appPrintf ("Compares current memory allocations with marked by \"mem_mark\"\n");
		return;
	}

	if (!MarkedAllocs)
	{
		appWPrintf ("no \"mem_mark\" preformed\n");
		return;
	}

	numAllocators = BuildAllocatorsTable (ARRAY_ARG(info));
	//?? filter symbol names (and/or package name) ?!
	//?? can filter different types of info (i.e. ignore "not modfied" lines)

	// prepare for loop
	n = info; m = MarkedAllocs;
	in = im = 0;

	while (in < numAllocators || im < numMarkedAllocs)
	{
		FAllocatorInfo *p1, *p2;
		char	symbol[256];
		address_t addr;

		p1 = (in == numAllocators) ? NULL : n;
		p2 = (im == numMarkedAllocs) ? NULL : m;

		if (p1 && p2)
		{
			// both lists have non-processed elements
			if (p1->address < p2->address)
				p2 = NULL;						// new memory allocation
			else if (p1->address > p2->address)
				p1 = NULL;						// allocation was released
			// else -- allocation present in both lists
		}
		// HERE: p1 and p2 either the same or one of them is NULL
		if (p1) addr = p1->address;
		else if (p2) addr = p2->address;
		else break;								// should not happen

		//?? highlight different variants of allocation info (released, new, old={growed/shrinked/unchanged})
		//?? + allocs from already unloaded packages (highlight whole package, package title only ??)
		if (p1 && !p2)
		{
			// new allocation
			appPrintf ("     ---        %8d/%-4d ", p1->total, p1->count);
		}
		else if (!p1)
		{
			// released allocation
			appPrintf ("  %8d/%-4d      ---      ", p2->total, p2->count);
		}
		else
		{	// p1 && p2 -- both-list-allocation
			if (p1->count == p2->count && p1->total == p2->total)
				appPrintf ("     "S_BLUE"["S_WHITE"  %9d/%-4d  "S_BLUE"]"S_WHITE"     ", p1->total, p1->count);
			else
			{
				int totalDelta = p1->total - p2->total;
				int countDelta = p1->count - p2->count;
#if 1
				static const char *colors[] = { S_GREEN, S_WHITE, S_RED };
				const char *totalColor = colors[Sign (totalDelta) + 1];
				const char *countColor = colors[Sign (countDelta) + 1];
				appPrintf ("%21s/%-15s ",
					va("%d%s%+d"S_WHITE, p2->total, totalColor, totalDelta),
					va("%d%s%+d"S_WHITE, p2->count, countColor, countDelta));
#else	// no colors
				appPrintf ("%17s/%-11s ",
					va("%d%+d", p2->total, totalDelta),
					va("%d%+d", p2->count, countDelta));
#endif
			}
		}

		if (!appSymbolName (addr, ARRAY_ARG(symbol)))
			appSprintf (ARRAY_ARG(symbol), "%08X", addr);
		bool isChain;
		if (p1) isChain = p1->isChain;
		else if (p2) isChain = p2->isChain;
		else continue; // should not happen
		appPrintf ("%s%s\n", isChain ? S_GREEN : S_WHITE, symbol);

		// advance list pointers
		if (p1)
		{
			in++; n++;
		}
		if (p2)
		{
			im++; m++;
		}
	}
//??	appPrintf ("Displayed %d points\n", numAllocators);
	//?? display stats for "allocated (bytes) in (blocks), released (bytes) ..."
}


#endif // MEM_DEBUG


/*-----------------------------------------------------------------------------
	Initialization
-----------------------------------------------------------------------------*/


void appInitMemory (void)
{
#if MEM_STATS
	RegisterCommand ("meminfo", Cmd_Meminfo);
#endif
#if MEM_DEBUG
	RegisterCommand ("mem_mark", Cmd_MarkAllocs);
	RegisterCommand ("mem_dump", Cmd_DumpAllocs);
	RegisterCommand ("mem_check", Cmd_CheckAllocs);
#endif
}
