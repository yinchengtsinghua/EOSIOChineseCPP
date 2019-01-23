
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "Inline/BasicTypes.h"
#include "Inline/Errors.h"
#include "Platform/Platform.h"

#include <string.h>
#include <assert.h>

//封装一组介于0到MaxInExplusOne（不包括MaxInExplusOne）之间的整数。
//它对范围内的每个整数使用1位的存储，许多操作查看所有位，因此它最适合小范围。
//但是，这避免了堆分配，对于小整数集（例如U8）来说，这是非常快的。
template<typename Index,Uptr maxIndexPlusOne>
struct DenseStaticIntSet
{
	DenseStaticIntSet()
	{
		memset(elements,0,sizeof(elements));
	}
	DenseStaticIntSet(Index index)
	{
		memset(elements,0,sizeof(elements));
		add(index);
	}

//查询

	inline bool contains(Index index) const
	{
		WAVM_ASSERT_THROW((Uptr)index < maxIndexPlusOne);
		return (elements[index / indicesPerElement] & (Element(1) << (index % indicesPerElement))) != 0;
	}
	bool isEmpty() const
	{
		Element combinedElements = 0;
		for(Uptr elementIndex = 0;elementIndex < numElements;++elementIndex)
		{
			combinedElements |= elements[elementIndex];
		}
		return combinedElements == 0;
	}
	inline Index getSmallestMember() const
	{
//找到第一个有任何位集的元素。
		for(Uptr elementIndex = 0;elementIndex < numElements;++elementIndex)
		{
			if(elements[elementIndex])
			{
//使用counttrailingzeroes查找元素中最低集合位的索引。
				const Index result = (Index)(elementIndex * indicesPerElement + Platform::countTrailingZeroes(elements[elementIndex]));
				WAVM_ASSERT_THROW(contains(result));
				return result;
			}
		}
		return maxIndexPlusOne;
	}

//添加/删除索引

	inline void add(Index index)
	{
		WAVM_ASSERT_THROW((Uptr)index < maxIndexPlusOne);
		elements[index / indicesPerElement] |= Element(1) << (index % indicesPerElement);
	}
	inline void addRange(Index rangeMin,Index rangeMax)
	{
		WAVM_ASSERT_THROW(rangeMin <= rangeMax);
		WAVM_ASSERT_THROW((Uptr)rangeMax < maxIndexPlusOne);
		for(Index index = rangeMin;index <= rangeMax;++index)
		{
			add(index);
		}
	}
	inline bool remove(Index index)
	{
		const Element elementMask = Element(1) << (index % indicesPerElement);
		const bool hadIndex = (elements[index / indicesPerElement] & elementMask) != 0;
		elements[index / indicesPerElement] &= ~elementMask;
		return hadIndex;
	}

//逻辑运算符

	friend DenseStaticIntSet operator~(const DenseStaticIntSet& set)
	{
		DenseStaticIntSet result;
		for(Uptr elementIndex = 0;elementIndex < numElements;++elementIndex)
		{
			result.elements[elementIndex] = ~set.elements[elementIndex];
		}
		return result;
	}
	friend DenseStaticIntSet operator|(const DenseStaticIntSet& left,const DenseStaticIntSet& right)
	{
		DenseStaticIntSet result;
		for(Uptr elementIndex = 0;elementIndex < numElements;++elementIndex)
		{
			result.elements[elementIndex] = left.elements[elementIndex] | right.elements[elementIndex];
		}
		return result;
	}
	friend DenseStaticIntSet operator&(const DenseStaticIntSet& left,const DenseStaticIntSet& right)
	{
		DenseStaticIntSet result;
		for(Uptr elementIndex = 0;elementIndex < numElements;++elementIndex)
		{
			result.elements[elementIndex] = left.elements[elementIndex] & right.elements[elementIndex];
		}
		return result;
	}
	friend DenseStaticIntSet operator^(const DenseStaticIntSet& left,const DenseStaticIntSet& right)
	{
		DenseStaticIntSet result;
		for(Uptr elementIndex = 0;elementIndex < numElements;++elementIndex)
		{
			result.elements[elementIndex] = left.elements[elementIndex] ^ right.elements[elementIndex];
		}
		return result;
	}

//比较

	friend bool operator==(const DenseStaticIntSet& left,const DenseStaticIntSet& right)
	{
		return memcmp(left.elements,right.elements,sizeof(DenseStaticIntSet::elements)) == 0;
	}
	friend bool operator!=(const DenseStaticIntSet& left,const DenseStaticIntSet& right)
	{
		return memcmp(left.elements,right.elements,sizeof(DenseStaticIntSet::elements)) != 0;
	}
	friend bool operator<(const DenseStaticIntSet& left,const DenseStaticIntSet& right)
	{
		return memcmp(left.elements,right.elements,sizeof(DenseStaticIntSet::elements)) < 0;
	}

private:
	typedef Uptr Element;
	enum { indicesPerElement = sizeof(Element) * 8 };
	enum { numElements = (maxIndexPlusOne + indicesPerElement - 1) / indicesPerElement };
	Element elements[numElements];
};
