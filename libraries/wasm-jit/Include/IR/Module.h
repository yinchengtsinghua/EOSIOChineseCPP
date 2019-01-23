
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "Inline/BasicTypes.h"
#include "IR.h"
#include "Types.h"

#include <vector>

namespace IR
{
//初始值设定项表达式：与任何其他代码一样序列化，但只能是常量或不可变全局
	struct InitializerExpression
	{
		enum class Type : U8
		{
			i32_const = 0x41,
			i64_const = 0x42,
			f32_const = 0x43,
			f64_const = 0x44,
			get_global = 0x23,
			error = 0xff
		};
		Type type;
		union
		{
			I32 i32;
			I64 i64;
			F32 f32;
			F64 f64;
			Uptr globalIndex;
		};
		InitializerExpression(): type(Type::error) {}
		InitializerExpression(I32 inI32): type(Type::i32_const), i32(inI32) {}
		InitializerExpression(I64 inI64): type(Type::i64_const), i64(inI64) {}
		InitializerExpression(F32 inF32): type(Type::f32_const), f32(inF32) {}
		InitializerExpression(F64 inF64): type(Type::f64_const), f64(inF64) {}
		InitializerExpression(Type inType,Uptr inGlobalIndex): type(inType), globalIndex(inGlobalIndex) { WAVM_ASSERT_THROW(inType == Type::get_global); }
	};

//函数定义
	struct FunctionDef
	{
		IndexedFunctionType type;
		std::vector<ValueType> nonParameterLocalTypes;
		std::vector<U8> code;
		std::vector<std::vector<U32>> branchTables;
	};

//表定义
	struct TableDef
	{
		TableType type;
	};
	
//内存定义
	struct MemoryDef
	{
		MemoryType type;
	};

//全局定义
	struct GlobalDef
	{
		GlobalType type;
		InitializerExpression initializer;
	};

//描述导入模块或特定类型的对象
	template<typename Type>
	struct Import
	{
		Type type;
		std::string moduleName;
		std::string exportName;
	};

	typedef Import<Uptr> FunctionImport;
	typedef Import<TableType> TableImport;
	typedef Import<MemoryType> MemoryImport;
	typedef Import<GlobalType> GlobalImport;

//描述从模块导出。指数的解释取决于种类
	struct Export
	{
		std::string name;
		ObjectKind kind;
		Uptr index;
	};
	
//数据段：实例化模块时复制到运行时：：内存中的字节的文字序列。
	struct DataSegment
	{
		Uptr memoryIndex;
		InitializerExpression baseOffset;
		std::vector<U8> data;
	};

//表段：在实例化模块时复制到runtime:：table中的函数索引的文本序列。
	struct TableSegment
	{
		Uptr tableIndex;
		InitializerExpression baseOffset;
		std::vector<Uptr> indices;
	};

//作为字节数组的用户定义的模块部分
	struct UserSection
	{
		std::string name;
		std::vector<U8> data;
	};

//用于导入和定义特定类型的索引空间。
	template<typename Definition,typename Type>
	struct IndexSpace
	{
		std::vector<Import<Type>> imports;
		std::vector<Definition> defs;

		Uptr size() const { return imports.size() + defs.size(); }
		Type getType(Uptr index) const
		{
			if(index < imports.size()) { return imports[index].type; }
			else { return defs[index - imports.size()].type; }
		}
	};

//WebAssembly模块定义
	struct Module
	{
		std::vector<const FunctionType*> types;

		IndexSpace<FunctionDef,IndexedFunctionType> functions;
		IndexSpace<TableDef,TableType> tables;
		IndexSpace<MemoryDef,MemoryType> memories;
		IndexSpace<GlobalDef,GlobalType> globals;

		std::vector<Export> exports;
		std::vector<DataSegment> dataSegments;
		std::vector<TableSegment> tableSegments;
		std::vector<UserSection> userSections;

		Uptr startFunctionIndex;

		Module() : startFunctionIndex(UINTPTR_MAX) {}
	};
	
//在模块中查找命名用户部分。
	inline bool findUserSection(const Module& module,const char* userSectionName,Uptr& outUserSectionIndex)
	{
		for(Uptr sectionIndex = 0;sectionIndex < module.userSections.size();++sectionIndex)
		{
			if(module.userSections[sectionIndex].name == userSectionName)
			{
				outUserSectionIndex = sectionIndex;
				return true;
			}
		}
		return false;
	}

//将模块中的声明映射到要在反汇编中使用的名称。
	struct DisassemblyNames
	{
		struct Function
		{
			std::string name;
			std::vector<std::string> locals;
		};

		std::vector<std::string> types;
		std::vector<Function> functions;
		std::vector<std::string> tables;
		std::vector<std::string> memories;
		std::vector<std::string> globals;
	};
	
//在模块中查找名称部分。如果它存在，则将其反序列化为outname。
//如果它不存在，请用合理的默认值填充名称。
	IR_API void getDisassemblyNames(const Module& module,DisassemblyNames& outNames);

//序列化DisassemblyNames结构并将其作为名称部分添加到模块中。
	IR_API void setDisassemblyNames(Module& module,const DisassemblyNames& names);
}
