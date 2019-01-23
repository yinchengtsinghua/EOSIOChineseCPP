
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
#include "Linker.h"
#include "Intrinsics.h"
#include "IR/Module.h"

#include <string.h>

namespace Runtime
{
	RUNTIME_API IntrinsicResolver IntrinsicResolver::singleton;

	bool IntrinsicResolver::resolve(const std::string& moduleName,const std::string& exportName,ObjectType type,ObjectInstance*& outObject)
	{
//确保wavmintrinsics模块不能直接导入。
		if(moduleName == "wavmIntrinsics") { return false; }

		outObject = Intrinsics::find(moduleName + "." + exportName,type);
		return outObject != nullptr;
	}

	const FunctionType* resolveImportType(const IR::Module& module,IndexedFunctionType type)
	{
		return module.types[type.index];
	}
	TableType resolveImportType(const IR::Module& module,TableType type) { return type; }
	MemoryType resolveImportType(const IR::Module& module,MemoryType type) { return type; }
	GlobalType resolveImportType(const IR::Module& module,GlobalType type) { return type; }

	template<typename Instance,typename Type>
	void linkImport(const IR::Module& module,const Import<Type>& import,Resolver& resolver,LinkResult& linkResult,std::vector<Instance*>& resolvedImports)
	{
//向冲突解决程序请求此导入的值。
		ObjectInstance* importValue;
		if(resolver.resolve(import.moduleName,import.exportName,resolveImportType(module,import.type),importValue))
		{
//健全性检查解析程序是否返回正确类型的对象。
			WAVM_ASSERT_THROW(isA(importValue,resolveImportType(module,import.type)));
			resolvedImports.push_back(as<Instance>(importValue));
		}
		else { linkResult.missingImports.push_back({import.moduleName,import.exportName,resolveImportType(module,import.type)}); }
	}

	LinkResult linkModule(const IR::Module& module,Resolver& resolver)
	{
		LinkResult linkResult;
		for(const auto& import : module.functions.imports)
		{
			linkImport(module,import,resolver,linkResult,linkResult.resolvedImports.functions);
		}
		for(const auto& import : module.tables.imports)
		{
			linkImport(module,import,resolver,linkResult,linkResult.resolvedImports.tables);
		}
		for(const auto& import : module.memories.imports)
		{
			linkImport(module,import,resolver,linkResult,linkResult.resolvedImports.memories);
		}
		for(const auto& import : module.globals.imports)
		{
			linkImport(module,import,resolver,linkResult,linkResult.resolvedImports.globals);
		}

		linkResult.success = linkResult.missingImports.size() == 0;
		return linkResult;
	}
}