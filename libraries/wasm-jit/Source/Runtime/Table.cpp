
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
#include <eosio/chain/wasm_eosio_constraints.hpp>

namespace Runtime
{
//表的全局列表；用于查询其中一个表是否保留了地址。
	std::vector<TableInstance*> tables;

	static Uptr getNumPlatformPages(Uptr numBytes)
	{
		return (numBytes + (Uptr(1)<<Platform::getPageSizeLog2()) - 1) >> Platform::getPageSizeLog2();
	}

	TableInstance* createTable(TableType type)
	{
		TableInstance* table = new TableInstance(type);

		const Uptr tableMaxBytes = sizeof(TableInstance::FunctionElement)*eosio::chain::wasm_constraints::maximum_table_elements;
		
		const Uptr alignmentBytes = 1U << Platform::getPageSizeLog2();
		table->baseAddress = (TableInstance::FunctionElement*)allocateVirtualPagesAligned(tableMaxBytes,alignmentBytes,table->reservedBaseAddress,table->reservedNumPlatformPages);
		table->endOffset = tableMaxBytes;
		if(!table->baseAddress) { delete table; return nullptr; }
		
//将表增大到类型的最小大小。
		WAVM_ASSERT_THROW(type.size.min <= UINTPTR_MAX);
		if(growTable(table,Uptr(type.size.min)) == -1) { delete table; return nullptr; }
		
//将表添加到全局数组。
		tables.push_back(table);
		return table;
	}
	
	TableInstance::~TableInstance()
	{
//关闭所有页面。
		if(elements.size() > 0) { Platform::decommitVirtualPages((U8*)baseAddress,getNumPlatformPages(elements.size() * sizeof(TableInstance::FunctionElement))); }

//释放虚拟地址空间。
		if(reservedNumPlatformPages > 0) { Platform::freeVirtualPages((U8*)reservedBaseAddress,reservedNumPlatformPages); }
		reservedBaseAddress = nullptr;
		reservedNumPlatformPages = 0;
		baseAddress = nullptr;
		
//从全局数组中移除表。
		for(Uptr tableIndex = 0;tableIndex < tables.size();++tableIndex)
		{
			if(tables[tableIndex] == this) { tables.erase(tables.begin() + tableIndex); break; }
		}
	}

	bool isAddressOwnedByTable(U8* address)
	{
//遍历所有表并检查地址是否在每个表的保留地址空间内。
		for(auto table : tables)
		{
			U8* startAddress = (U8*)table->reservedBaseAddress;
			U8* endAddress = ((U8*)table->reservedBaseAddress) + (table->reservedNumPlatformPages << Platform::getPageSizeLog2());
			if(address >= startAddress && address < endAddress) { return true; }
		}
		return false;
	}

	ObjectInstance* setTableElement(TableInstance* table,Uptr index,ObjectInstance* newValue)
	{
//将新的表元素写入表的元素数组及其间接函数调用数据。
		WAVM_ASSERT_THROW(index < table->elements.size());
		FunctionInstance* functionInstance = asFunction(newValue);
		WAVM_ASSERT_THROW(functionInstance->nativeFunction);
		table->baseAddress[index].type = functionInstance->type;
		table->baseAddress[index].value = functionInstance->nativeFunction;
		auto oldValue = table->elements[index];
		table->elements[index] = newValue;
		return oldValue;
	}

	Uptr getTableNumElements(TableInstance* table)
	{
		return table->elements.size();
	}

	Iptr growTable(TableInstance* table,Uptr numNewElements)
	{
		const Uptr previousNumElements = table->elements.size();
		if(numNewElements > 0)
		{
//如果要增长的元素数会导致表的大小超过其最大值，则返回-1。
			if(numNewElements > table->type.size.max || table->elements.size() > table->type.size.max - numNewElements) { return -1; }
			
//尝试提交新元素的页面，如果提交失败，则返回-1。
			const Uptr previousNumPlatformPages = getNumPlatformPages(table->elements.size() * sizeof(TableInstance::FunctionElement));
			const Uptr newNumPlatformPages = getNumPlatformPages((table->elements.size()+numNewElements) * sizeof(TableInstance::FunctionElement));
			if(newNumPlatformPages != previousNumPlatformPages
			&& !Platform::commitVirtualPages(
				(U8*)table->baseAddress + (previousNumPlatformPages << Platform::getPageSizeLog2()),
				newNumPlatformPages - previousNumPlatformPages
				))
			{
				return -1;
			}

//同时增大表的元素数组。
			table->elements.insert(table->elements.end(),numNewElements,nullptr);
		}
		return previousNumElements;
	}

	Iptr shrinkTable(TableInstance* table,Uptr numElementsToShrink)
	{
		const Uptr previousNumElements = table->elements.size();
		if(numElementsToShrink > 0)
		{
//如果要收缩的元素数量会导致表的大小降到其最小值以下，则返回-1。
			if(numElementsToShrink > table->elements.size()
			|| table->elements.size() - numElementsToShrink < table->type.size.min) { return -1; }

//缩小表的元素数组。
			table->elements.resize(table->elements.size() - numElementsToShrink);
			
//将从表的间接函数调用数据末尾收缩的页解除委托。
			const Uptr previousNumPlatformPages = getNumPlatformPages(previousNumElements * sizeof(TableInstance::FunctionElement));
			const Uptr newNumPlatformPages = getNumPlatformPages(table->elements.size() * sizeof(TableInstance::FunctionElement));
			if(newNumPlatformPages != previousNumPlatformPages)
			{
				Platform::decommitVirtualPages(
					(U8*)table->baseAddress + (newNumPlatformPages << Platform::getPageSizeLog2()),
					(previousNumPlatformPages - newNumPlatformPages) << Platform::getPageSizeLog2()
					);
			}
		}
		return previousNumElements;
	}
}
