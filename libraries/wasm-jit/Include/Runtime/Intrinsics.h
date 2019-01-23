
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "Inline/BasicTypes.h"
#include "IR/IR.h"
#include "Runtime.h"

namespace Intrinsics
{
//内在函数。
	struct Function
	{
		Runtime::FunctionInstance* function;

		RUNTIME_API Function(const char* inName,const IR::FunctionType* type,void* nativeFunction);
		RUNTIME_API ~Function();

	private:
		const char* name;
	};
	
//内部全局的基类。
	struct Global
	{
		Runtime::GlobalInstance* global;

		RUNTIME_API Global(const char* inName,IR::GlobalType inType);
		RUNTIME_API ~Global();
		
		RUNTIME_API void reset();

	protected:
		void* value;
	private:
		const char* name;
		IR::GlobalType globalType;
	};
	
//用于内部全局的部分专用模板：
//通过隐式强制提供对值的访问，并为可变全局提供赋值运算符。
	template<IR::ValueType type,bool isMutable>
	struct GenericGlobal : Global
	{
		typedef typename IR::ValueTypeInfo<type>::Value Value;

		GenericGlobal(const char* inName,Value inValue)
		: Global(inName,IR::GlobalType(type,isMutable)) { *(Value*)value = inValue; }

		operator Value() const { return *(Value*)value; }
		void operator=(Value newValue) { *(Value*)value = newValue; }
	};
	template<IR::ValueType type>
	struct GenericGlobal<type,false> : Global
	{
		typedef typename IR::ValueTypeInfo<type>::Value Value;

		GenericGlobal(const char* inName,Value inValue)
		: Global(inName,IR::GlobalType(type,false)) { *(Value*)value = inValue; }

		operator Value() const { return *(Value*)value; }
		
		void reset(Value inValue)
		{
			Global::reset();
			*(Value*)value = inValue;
		}
	};

//内存和表
	struct Memory
	{
		RUNTIME_API Memory(const char* inName,const IR::MemoryType& inType);
		RUNTIME_API ~Memory();

		operator Runtime::MemoryInstance*() const { return memory; }

	private:
		const char* name;
		Runtime::MemoryInstance* const memory;
	};

	struct Table
	{
		RUNTIME_API Table(const char* inName,const IR::TableType& inType);
		RUNTIME_API ~Table();
		
		operator Runtime::TableInstance*() const { return table; }

	private:
		const char* name;
		Runtime::TableInstance* const table;
	};

//按名称和类型查找内部对象。
	RUNTIME_API Runtime::ObjectInstance* find(const std::string& name,const IR::ObjectType& type);

//返回所有内部运行时对象的数组；用作垃圾收集的根。
	RUNTIME_API std::vector<Runtime::ObjectInstance*> getAllIntrinsicObjects();
}

namespace NativeTypes
{
	typedef I32 i32;
	typedef I64 i64;
	typedef F32 f32;
	typedef F64 f64;
	typedef void none;
	#if ENABLE_SIMD_PROTOTYPE
	typedef IR::V128 v128;
	typedef IR::V128 b8x16;
	typedef IR::V128 b16x8;
	typedef IR::V128 b32x4;
	typedef IR::V128 b64x2;
	#endif
};

//用于定义各种算术运算的内部函数的宏。
#define DEFINE_INTRINSIC_FUNCTION0(module,cName,name,returnType) \
	NativeTypes::returnType cName##returnType(); \
	static Intrinsics::Function cName##returnType##Function(#module "." #name,IR::FunctionType::get(IR::ResultType::returnType),(void*)&cName##returnType); \
	NativeTypes::returnType cName##returnType()

#define DEFINE_INTRINSIC_FUNCTION1(module,cName,name,returnType,arg0Type,arg0Name) \
	NativeTypes::returnType cName##returnType##arg0Type(NativeTypes::arg0Type); \
	static Intrinsics::Function cName##returnType##arg0Type##Function(#module "." #name,IR::FunctionType::get(IR::ResultType::returnType,{IR::ValueType::arg0Type}),(void*)&cName##returnType##arg0Type); \
	NativeTypes::returnType cName##returnType##arg0Type(NativeTypes::arg0Type arg0Name)

#define DEFINE_INTRINSIC_FUNCTION2(module,cName,name,returnType,arg0Type,arg0Name,arg1Type,arg1Name) \
	NativeTypes::returnType cName##returnType##arg0Type##arg1Type(NativeTypes::arg0Type,NativeTypes::arg1Type); \
	static Intrinsics::Function cName##returnType##arg0Type##arg1Type##Function(#module "." #name,IR::FunctionType::get(IR::ResultType::returnType,{IR::ValueType::arg0Type,IR::ValueType::arg1Type}),(void*)&cName##returnType##arg0Type##arg1Type); \
	NativeTypes::returnType cName##returnType##arg0Type##arg1Type(NativeTypes::arg0Type arg0Name,NativeTypes::arg1Type arg1Name)

#define DEFINE_INTRINSIC_FUNCTION3(module,cName,name,returnType,arg0Type,arg0Name,arg1Type,arg1Name,arg2Type,arg2Name) \
	NativeTypes::returnType cName##returnType##arg0Type##arg1Type##arg2Type(NativeTypes::arg0Type,NativeTypes::arg1Type,NativeTypes::arg2Type); \
	static Intrinsics::Function cName##returnType##arg0Type##arg1Type##arg2Type##Function(#module "." #name,IR::FunctionType::get(IR::ResultType::returnType,{IR::ValueType::arg0Type,IR::ValueType::arg1Type,IR::ValueType::arg2Type}),(void*)&cName##returnType##arg0Type##arg1Type##arg2Type); \
	NativeTypes::returnType cName##returnType##arg0Type##arg1Type##arg2Type(NativeTypes::arg0Type arg0Name,NativeTypes::arg1Type arg1Name,NativeTypes::arg2Type arg2Name)

#define DEFINE_INTRINSIC_FUNCTION4(module,cName,name,returnType,arg0Type,arg0Name,arg1Type,arg1Name,arg2Type,arg2Name,arg3Type,arg3Name) \
	NativeTypes::returnType cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type(NativeTypes::arg0Type,NativeTypes::arg1Type,NativeTypes::arg2Type,NativeTypes::arg3Type); \
	static Intrinsics::Function cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type##Function(#module "." #name,IR::FunctionType::get(IR::ResultType::returnType,{IR::ValueType::arg0Type,IR::ValueType::arg1Type,IR::ValueType::arg2Type,IR::ValueType::arg3Type}),(void*)&cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type); \
	NativeTypes::returnType cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type(NativeTypes::arg0Type arg0Name,NativeTypes::arg1Type arg1Name,NativeTypes::arg2Type arg2Name,NativeTypes::arg3Type arg3Name)

#define DEFINE_INTRINSIC_FUNCTION5(module,cName,name,returnType,arg0Type,arg0Name,arg1Type,arg1Name,arg2Type,arg2Name,arg3Type,arg3Name,arg4Type,arg4Name) \
	NativeTypes::returnType cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type##arg4Type(NativeTypes::arg0Type,NativeTypes::arg1Type,NativeTypes::arg2Type,NativeTypes::arg3Type,NativeTypes::arg4Type); \
	static Intrinsics::Function cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type##arg4Type##Function(#module "." #name,IR::FunctionType::get(IR::ResultType::returnType,{IR::ValueType::arg0Type,IR::ValueType::arg1Type,IR::ValueType::arg2Type,IR::ValueType::arg3Type,IR::ValueType::arg4Type}),(void*)&cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type##arg4Type); \
	NativeTypes::returnType cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type##arg4Type(NativeTypes::arg0Type arg0Name,NativeTypes::arg1Type arg1Name,NativeTypes::arg2Type arg2Name,NativeTypes::arg3Type arg3Name,NativeTypes::arg4Type arg4Name)

#define DEFINE_INTRINSIC_FUNCTION6(module,cName,name,returnType,arg0Type,arg0Name,arg1Type,arg1Name,arg2Type,arg2Name,arg3Type,arg3Name,arg4Type,arg4Name,arg5Type,arg5Name) \
	NativeTypes::returnType cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type##arg4Type##arg5Type(NativeTypes::arg0Type,NativeTypes::arg1Type,NativeTypes::arg2Type,NativeTypes::arg3Type,NativeTypes::arg4Type,NativeTypes::arg5Type); \
	static Intrinsics::Function cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type##arg4Type##arg5Type##Function(#module "." #name,IR::FunctionType::get(IR::ResultType::returnType,{IR::ValueType::arg0Type,IR::ValueType::arg1Type,IR::ValueType::arg2Type,IR::ValueType::arg3Type,IR::ValueType::arg4Type,IR::ValueType::arg5Type}),(void*)&cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type##arg4Type##arg5Type); \
	NativeTypes::returnType cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type##arg4Type##arg5Type(NativeTypes::arg0Type arg0Name,NativeTypes::arg1Type arg1Name,NativeTypes::arg2Type arg2Name,NativeTypes::arg3Type arg3Name,NativeTypes::arg4Type arg4Name,NativeTypes::arg5Type arg5Name )

#define DEFINE_INTRINSIC_FUNCTION7(module,cName,name,returnType,arg0Type,arg0Name,arg1Type,arg1Name,arg2Type,arg2Name,arg3Type,arg3Name,arg4Type,arg4Name,arg5Type,arg5Name,arg6Type,arg6Name) \
	NativeTypes::returnType cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type##arg4Type##arg5Type##arg6Type(NativeTypes::arg0Type,NativeTypes::arg1Type,NativeTypes::arg2Type,NativeTypes::arg3Type,NativeTypes::arg4Type,NativeTypes::arg5Type,NativeTypes::arg6Type); \
	static Intrinsics::Function cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type##arg4Type##arg5Type##arg6Type##Function(#module "." #name,IR::FunctionType::get(IR::ResultType::returnType,{IR::ValueType::arg0Type,IR::ValueType::arg1Type,IR::ValueType::arg2Type,IR::ValueType::arg3Type,IR::ValueType::arg4Type,IR::ValueType::arg5Type,IR::ValueType::arg6Type}),(void*)&cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type##arg4Type##arg5Type##arg6Type); \
	NativeTypes::returnType cName##returnType##arg0Type##arg1Type##arg2Type##arg3Type##arg4Type##arg5Type##arg6Type(NativeTypes::arg0Type arg0Name,NativeTypes::arg1Type arg1Name,NativeTypes::arg2Type arg2Name,NativeTypes::arg3Type arg3Name,NativeTypes::arg4Type arg4Name,NativeTypes::arg5Type arg5Name,NativeTypes::arg6Type arg6Name )

//用于定义内部全局、内存和表的宏。
#define DEFINE_INTRINSIC_GLOBAL(module,cName,name,valueType,isMutable,initializer) \
	static Intrinsics::GenericGlobal<IR::ValueType::valueType,isMutable> \
		cName(#module "." #name,initializer);

#define DEFINE_INTRINSIC_MEMORY(module,cName,name,type) static Intrinsics::Memory cName(#module "." #name,type);
#define DEFINE_INTRINSIC_TABLE(module,cName,name,type) static Intrinsics::Table cName(#module "." #name,type);
