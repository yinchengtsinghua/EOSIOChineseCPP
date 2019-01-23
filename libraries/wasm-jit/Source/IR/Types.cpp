
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "Types.h"

#include <map>

namespace IR
{
	struct FunctionTypeMap
	{
		struct Key
		{
			ResultType ret;
			std::vector<ValueType> parameters;

			friend bool operator==(const Key& left,const Key& right) { return left.ret == right.ret && left.parameters == right.parameters; }
			friend bool operator!=(const Key& left,const Key& right) { return left.ret != right.ret || left.parameters != right.parameters; }
			friend bool operator<(const Key& left,const Key& right) { return left.ret < right.ret || (left.ret == right.ret && left.parameters < right.parameters); }
		};
		static std::map<Key,FunctionType*>& get()
		{
			static std::map<Key,FunctionType*> map;
			return map;
		}
	};

	template<typename Key,typename Value,typename CreateValueThunk>
	Value findExistingOrCreateNew(std::map<Key,Value>& map,Key&& key,CreateValueThunk createValueThunk)
	{
		auto mapIt = map.find(key);
		if(mapIt != map.end()) { return mapIt->second; }
		else
		{
			Value value = createValueThunk();
			map.insert({std::move(key),value});
			return value;
		}
	}

	const FunctionType* FunctionType::get(ResultType ret,const std::initializer_list<ValueType>& parameters)
	{ return findExistingOrCreateNew(FunctionTypeMap::get(),FunctionTypeMap::Key {ret,parameters},[=]{return new FunctionType(ret,parameters);}); }
	const FunctionType* FunctionType::get(ResultType ret,const std::vector<ValueType>& parameters)
	{ return findExistingOrCreateNew(FunctionTypeMap::get(),FunctionTypeMap::Key {ret,parameters},[=]{return new FunctionType(ret,parameters);}); }
	const FunctionType* FunctionType::get(ResultType ret)
	{ return findExistingOrCreateNew(FunctionTypeMap::get(),FunctionTypeMap::Key {ret,{}},[=]{return new FunctionType(ret,{});}); }
}