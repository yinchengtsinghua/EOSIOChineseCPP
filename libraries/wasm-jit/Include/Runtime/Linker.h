
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "Inline/BasicTypes.h"
#include "Runtime.h"

#include <functional>

namespace Runtime
{
//抽象解析器：将模块+导出名称对映射到运行时：：对象。
	struct Resolver
	{
		virtual bool resolve(const std::string& moduleName,const std::string& exportName,IR::ObjectType type,ObjectInstance*& outObject) = 0;
	};

//内部分解器。
	struct IntrinsicResolver : Resolver
	{
		static RUNTIME_API IntrinsicResolver singleton;
		RUNTIME_API bool resolve(const std::string& moduleName,const std::string& exportName,IR::ObjectType type,ObjectInstance*& outObject) override;
	};

//忽略modulename并在单个模块中查找exportname的解析程序。
	struct ModuleExportResolver : Resolver
	{
		ModuleExportResolver(const IR::Module& inModule,ModuleInstance* inModuleInstance): module(inModule), moduleInstance(inModuleInstance) {}

		bool resolve(const std::string& moduleName,const std::string& exportName,IR::ObjectType type,ObjectInstance*& outObject) override;
	private:
		const IR::Module& module;
		ModuleInstance* moduleInstance;
	};

//第一次使用内部冲突解决程序时，它会延迟地创建内部冲突解决程序，然后将所有查询转发给内部冲突解决程序。
	struct LazyResolver : Resolver
	{
		LazyResolver(std::function<Resolver*()>& inInnerResolverThunk)
		: innerResolverThunk(std::move(inInnerResolverThunk)), innerResolver(nullptr) {}

		bool resolve(const std::string& moduleName,const std::string& exportName,IR::ObjectType type,Runtime::ObjectInstance*& outObject) override
		{
			if(!innerResolver) { innerResolver = innerResolverThunk(); }
			return innerResolver->resolve(moduleName,exportName,type,outObject);		
		}

	private:

		std::function<Resolver*()> innerResolverThunk;
		Resolver* innerResolver;
	};

//总是返回失败的冲突解决程序。
	struct NullResolver : Resolver
	{
		bool resolve(const std::string& moduleName,const std::string& exportName,IR::ObjectType type,Runtime::ObjectInstance*& outObject) override
		{
			return false; 
		}
	};

//使用给定的冲突解决程序链接模块，将数组映射导入索引返回到对象。
//如果冲突解决程序未能解析任何导入，将引发LinkException。
	struct LinkResult
	{
		struct MissingImport
		{
			std::string moduleName;
			std::string exportName;
			IR::ObjectType type;
		};

		std::vector<MissingImport> missingImports;
		ImportBindings resolvedImports;
		bool success;
	};

	RUNTIME_API LinkResult linkModule(const IR::Module& module,Resolver& resolver);
}