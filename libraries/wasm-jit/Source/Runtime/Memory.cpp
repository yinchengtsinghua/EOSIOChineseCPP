
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "Inline/BasicTypes.h"
#include "Runtime.h"
#include "Platform/Platform.h"
#include "RuntimePrivate.h"

namespace Runtime
{
//内存的全局列表；用于查询其中一个是否保留了地址。
	std::vector<MemoryInstance*> memories;

	static Uptr getPlatformPagesPerWebAssemblyPageLog2()
	{
		errorUnless(Platform::getPageSizeLog2() <= IR::numBytesPerPageLog2);
		return IR::numBytesPerPageLog2 - Platform::getPageSizeLog2();
	}

	U8* allocateVirtualPagesAligned(Uptr numBytes,Uptr alignmentBytes,U8*& outUnalignedBaseAddress,Uptr& outUnalignedNumPlatformPages)
	{
		const Uptr numAllocatedVirtualPages = numBytes >> Platform::getPageSizeLog2();
		const Uptr alignmentPages = alignmentBytes >> Platform::getPageSizeLog2();
		outUnalignedNumPlatformPages = numAllocatedVirtualPages + alignmentPages;
		outUnalignedBaseAddress = Platform::allocateVirtualPages(outUnalignedNumPlatformPages);
		if(!outUnalignedBaseAddress) { outUnalignedNumPlatformPages = 0; return nullptr; }
		else { return (U8*)((Uptr)(outUnalignedBaseAddress + alignmentBytes - 1) & ~(alignmentBytes - 1)); }
	}

	MemoryInstance* createMemory(MemoryType type)
	{
		MemoryInstance* memory = new MemoryInstance(type);

//在64位运行时，为内存分配8GB的地址空间。
//这允许删除对内存访问的边界检查，因为32位索引+32位偏移量总是在保留的地址空间内。
//在32位运行时，分配256MB。
		const Uptr memoryMaxBytes = HAS_64BIT_ADDRESS_SPACE ? Uptr(8ull*1024*1024*1024) : 0x10000000;
		
//在64位运行时，将实例内存基与4GB边界对齐，因此较低的32位将全部为零。也许它将允许更好的代码生成？
//请注意，这将保留一个完整的额外4GB，但只使用（4GB-1页）进行对齐，因此在结尾处始终会有一个保护页
//防止跨越地址空间末端的未对齐的加载/存储。
		const Uptr alignmentBytes = HAS_64BIT_ADDRESS_SPACE ? Uptr(4ull*1024*1024*1024) : ((Uptr)1 << Platform::getPageSizeLog2());
		memory->baseAddress = allocateVirtualPagesAligned(memoryMaxBytes,alignmentBytes,memory->reservedBaseAddress,memory->reservedNumPlatformPages);
		memory->endOffset = memoryMaxBytes;
		if(!memory->baseAddress) { delete memory; return nullptr; }

//将内存增长到类型的最小大小。
		WAVM_ASSERT_THROW(type.size.min <= UINTPTR_MAX);
		if(growMemory(memory,Uptr(type.size.min)) == -1) { delete memory; return nullptr; }

//将内存添加到全局数组。
		memories.push_back(memory);
		return memory;
	}

	MemoryInstance::~MemoryInstance()
	{
//停用所有默认内存页。
		if(numPages > 0) { Platform::decommitVirtualPages(baseAddress,numPages << getPlatformPagesPerWebAssemblyPageLog2()); }

//释放虚拟地址空间。
		if(reservedNumPlatformPages > 0) { Platform::freeVirtualPages(reservedBaseAddress,reservedNumPlatformPages); }
		reservedBaseAddress = baseAddress = nullptr;
		reservedNumPlatformPages = 0;

//从全局数组中删除内存。
		for(Uptr memoryIndex = 0;memoryIndex < memories.size();++memoryIndex)
		{
			if(memories[memoryIndex] == this) { memories.erase(memories.begin() + memoryIndex); break; }
		}

		theMemoryInstance = nullptr;
	}
	
	bool isAddressOwnedByMemory(U8* address)
	{
//遍历所有内存，检查地址是否在每个内存的保留地址空间内。
		for(auto memory : memories)
		{
			U8* startAddress = memory->reservedBaseAddress;
			U8* endAddress = memory->reservedBaseAddress + (memory->reservedNumPlatformPages << Platform::getPageSizeLog2());
			if(address >= startAddress && address < endAddress) { return true; }
		}
		return false;
	}

	Uptr getMemoryNumPages(MemoryInstance* memory) { return memory->numPages; }
	Uptr getMemoryMaxPages(MemoryInstance* memory)
	{
		WAVM_ASSERT_THROW(memory->type.size.max <= UINTPTR_MAX);
		return Uptr(memory->type.size.max);
	}

	void resetMemory(MemoryInstance* memory, MemoryType& newMemoryType) {
		memory->type.size.min = 1;
		if(shrinkMemory(memory, memory->numPages - 1) == -1)
			causeException(Exception::Cause::outOfMemory);
		memset(memory->baseAddress, 0, 1<<IR::numBytesPerPageLog2);
		memory->type = newMemoryType;
		if(growMemory(memory, memory->type.size.min - 1) == -1)
			causeException(Exception::Cause::outOfMemory);
   }

	Iptr growMemory(MemoryInstance* memory,Uptr numNewPages)
	{
		const Uptr previousNumPages = memory->numPages;
		if(numNewPages > 0)
		{
//如果要增加的页数会导致内存大小超过最大值，则返回-1。
			if(numNewPages > memory->type.size.max || memory->numPages > memory->type.size.max - numNewPages) { return -1; }

//尝试提交新页面，如果提交失败，则返回-1。
			if(!Platform::commitVirtualPages(
				memory->baseAddress + (memory->numPages << IR::numBytesPerPageLog2),
				numNewPages << getPlatformPagesPerWebAssemblyPageLog2()
				))
			{
				return -1;
			}
			memset(memory->baseAddress + (memory->numPages << IR::numBytesPerPageLog2), 0, numNewPages << IR::numBytesPerPageLog2);
			memory->numPages += numNewPages;
		}
		return previousNumPages;
	}

	Iptr shrinkMemory(MemoryInstance* memory,Uptr numPagesToShrink)
	{
		const Uptr previousNumPages = memory->numPages;
		if(numPagesToShrink > 0)
		{
//如果要收缩的页数会导致内存大小降到最小值以下，则返回-1。
			if(numPagesToShrink > memory->numPages
			|| memory->numPages - numPagesToShrink < memory->type.size.min)
			{ return -1; }
			memory->numPages -= numPagesToShrink;

//将从内存末尾缩小的页面退役。
			Platform::decommitVirtualPages(
				memory->baseAddress + (memory->numPages << IR::numBytesPerPageLog2),
				numPagesToShrink << getPlatformPagesPerWebAssemblyPageLog2()
				);
		}
		return previousNumPages;
	}

	U8* getMemoryBaseAddress(MemoryInstance* memory)
	{
		return memory->baseAddress;
	}
	
	U8* getValidatedMemoryOffsetRange(MemoryInstance* memory,Uptr offset,Uptr numBytes)
	{
//验证范围[offset..offset+numbytes]是否包含在内存的保留页中。
		U8* address = memory->baseAddress + offset;
		if(	!memory
		||	address < memory->reservedBaseAddress
		||	address + numBytes < address
		||	address + numBytes >= memory->reservedBaseAddress + (memory->reservedNumPlatformPages << Platform::getPageSizeLog2()))
		{
			causeException(Exception::Cause::accessViolation);
		}
		return address;
	}

}
