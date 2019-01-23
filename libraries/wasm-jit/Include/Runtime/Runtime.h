
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "Inline/BasicTypes.h"
#include "TaggedValue.h"
#include "IR/Types.h"

#ifndef RUNTIME_API
	#define RUNTIME_API DLL_IMPORT
#endif

//声明ir:：module以避免包含定义。
namespace IR { struct Module; }

namespace Runtime
{
//初始化运行时。每个进程只能调用一次。
	RUNTIME_API void init();

//有关运行时异常的信息。
	struct Exception
	{
		enum class Cause : U8
		{
			unknown,
			accessViolation,
			stackOverflow,
			integerDivideByZeroOrIntegerOverflow,
			invalidFloatOperation,
			invokeSignatureMismatch,
			reachedUnreachable,
			indirectCallSignatureMismatch,
			undefinedTableElement,
			calledAbort,
			calledUnimplementedIntrinsic,
			outOfMemory,
			invalidSegmentOffset,
			misalignedAtomicMemoryAccess
		};

		Cause cause;
		std::vector<std::string> callStack;		
	};
	
//返回描述给定异常原因的字符串。
	inline const char* describeExceptionCause(Exception::Cause cause)
	{
		switch(cause)
		{
		case Exception::Cause::accessViolation: return "access violation";
		case Exception::Cause::stackOverflow: return "stack overflow";
		case Exception::Cause::integerDivideByZeroOrIntegerOverflow: return "integer divide by zero or signed integer overflow";
		case Exception::Cause::invalidFloatOperation: return "invalid floating point operation";
		case Exception::Cause::invokeSignatureMismatch: return "invoke signature mismatch";
		case Exception::Cause::reachedUnreachable: return "reached unreachable code";
		case Exception::Cause::indirectCallSignatureMismatch: return "call_indirect to function with wrong signature";
		case Exception::Cause::undefinedTableElement: return "undefined function table element";
		case Exception::Cause::calledAbort: return "called abort";
		case Exception::Cause::calledUnimplementedIntrinsic: return "called unimplemented intrinsic";
		case Exception::Cause::outOfMemory: return "out of memory";
		case Exception::Cause::invalidSegmentOffset: return "invalid segment offset";
		case Exception::Cause::misalignedAtomicMemoryAccess: return "misaligned atomic memory access";
		default: return "unknown";
		}
	}

//导致运行时异常。
	[[noreturn]] RUNTIME_API void causeException(Exception::Cause cause);

//这些是对象的子类，但仅在运行时内定义，因此其他模块必须
//将这些前向声明用作不透明指针。
	struct FunctionInstance;
	struct TableInstance;
	struct MemoryInstance;
	struct GlobalInstance;
	struct ModuleInstance;

//任何类型的运行时对象。
	struct ObjectInstance
	{
		const IR::ObjectKind kind;

		ObjectInstance(IR::ObjectKind inKind): kind(inKind) {}
		virtual ~ObjectInstance() {}
	};
	
//测试对象是否属于给定类型。
	RUNTIME_API bool isA(ObjectInstance* object,const IR::ObjectType& type);

//从对象到子类的强制转换，反之亦然。
	inline FunctionInstance* asFunction(ObjectInstance* object)		{ WAVM_ASSERT_THROW(object && object->kind == IR::ObjectKind::function); return (FunctionInstance*)object; }
	inline TableInstance* asTable(ObjectInstance* object)			{ WAVM_ASSERT_THROW(object && object->kind == IR::ObjectKind::table); return (TableInstance*)object; }
	inline MemoryInstance* asMemory(ObjectInstance* object)		{ WAVM_ASSERT_THROW(object && object->kind == IR::ObjectKind::memory); return (MemoryInstance*)object; }
	inline GlobalInstance* asGlobal(ObjectInstance* object)		{ WAVM_ASSERT_THROW(object && object->kind == IR::ObjectKind::global); return (GlobalInstance*)object; }
	inline ModuleInstance* asModule(ObjectInstance* object)		{ WAVM_ASSERT_THROW(object && object->kind == IR::ObjectKind::module); return (ModuleInstance*)object; }

	template<typename Instance> Instance* as(ObjectInstance* object);
	template<> inline FunctionInstance* as<FunctionInstance>(ObjectInstance* object) { return asFunction(object); }
	template<> inline TableInstance* as<TableInstance>(ObjectInstance* object) { return asTable(object); }
	template<> inline MemoryInstance* as<MemoryInstance>(ObjectInstance* object) { return asMemory(object); }
	template<> inline GlobalInstance* as<GlobalInstance>(ObjectInstance* object) { return asGlobal(object); }
	template<> inline ModuleInstance* as<ModuleInstance>(ObjectInstance* object) { return asModule(object); }

	inline ObjectInstance* asObject(FunctionInstance* function) { return (ObjectInstance*)function; }
	inline ObjectInstance* asObject(TableInstance* table) { return (ObjectInstance*)table; }
	inline ObjectInstance* asObject(MemoryInstance* memory) { return (ObjectInstance*)memory; }
	inline ObjectInstance* asObject(GlobalInstance* global) { return (ObjectInstance*)global; }
	inline ObjectInstance* asObject(ModuleInstance* module) { return (ObjectInstance*)module; }

//将对象强制转换为子类，检查对象的类型是否正确，如果不正确，则返回空值。
	inline FunctionInstance* asFunctionNullable(ObjectInstance* object)	{ return object && object->kind == IR::ObjectKind::function ? (FunctionInstance*)object : nullptr; }
	inline TableInstance* asTableNullable(ObjectInstance* object)		{ return object && object->kind == IR::ObjectKind::table ? (TableInstance*)object : nullptr; }
	inline MemoryInstance* asMemoryNullable(ObjectInstance* object)	{ return object && object->kind == IR::ObjectKind::memory ? (MemoryInstance*)object : nullptr; }
	inline GlobalInstance* asGlobalNullable(ObjectInstance* object)		{ return object && object->kind == IR::ObjectKind::global ? (GlobalInstance*)object : nullptr; }
	inline ModuleInstance* asModuleNullable(ObjectInstance* object)	{ return object && object->kind == IR::ObjectKind::module ? (ModuleInstance*)object : nullptr; }
	
//使用提供的对象数组作为根集释放未引用的对象。
	RUNTIME_API void freeUnreferencedObjects(std::vector<ObjectInstance*>&& rootObjectReferences);

//
//功能
//

//使用给定参数调用函数实例，并返回结果。
//如果发生陷阱，则引发运行时：：异常。
	RUNTIME_API Result invokeFunction(FunctionInstance* function,const std::vector<Value>& parameters);

//返回FunctionInstance的类型。
	RUNTIME_API const IR::FunctionType* getFunctionType(FunctionInstance* function);

//
//桌子
//

//创建表。如果内存分配失败，则可能返回空值。
	RUNTIME_API TableInstance* createTable(IR::TableType type);

//从表中读取元素。假设索引在边界内。
	RUNTIME_API ObjectInstance* getTableElement(TableInstance* table,Uptr index);

//将元素写入表。假定索引在边界内，并返回指向元素上一个值的指针。
	RUNTIME_API ObjectInstance* setTableElement(TableInstance* table,Uptr index,ObjectInstance* newValue);

//获取表的当前大小或最大大小。
	RUNTIME_API Uptr getTableNumElements(TableInstance* table);
	RUNTIME_API Uptr getTableMaxElements(TableInstance* table);

//按numElements增大或缩小表的大小。返回表的前一个大小。
	RUNTIME_API Iptr growTable(TableInstance* table,Uptr numElements);
	RUNTIME_API Iptr shrinkTable(TableInstance* table,Uptr numElements);

//
//回忆
//

//创建内存。如果内存分配失败，则可能返回空值。
	RUNTIME_API MemoryInstance* createMemory(IR::MemoryType type);

//获取内存数据的基址。
	RUNTIME_API U8* getMemoryBaseAddress(MemoryInstance* memory);

//获取页面中内存的当前或最大大小。
	RUNTIME_API Uptr getMemoryNumPages(MemoryInstance* memory);
	RUNTIME_API Uptr getMemoryMaxPages(MemoryInstance* memory);

//将内存大小增加或缩小numpages。返回以前的内存大小。
	RUNTIME_API Iptr growMemory(MemoryInstance* memory,Uptr numPages);
	RUNTIME_API Iptr shrinkMemory(MemoryInstance* memory,Uptr numPages);

//验证偏移范围是否完全在内存的虚拟地址范围内。
	RUNTIME_API U8* getValidatedMemoryOffsetRange(MemoryInstance* memory,Uptr offset,Uptr numBytes);
	
//在给定的偏移量处验证对单个内存元素的访问，并返回对它的引用。
	template<typename Value> Value& memoryRef(MemoryInstance* memory,U32 offset)
	{ return *(Value*)getValidatedMemoryOffsetRange(memory,offset,sizeof(Value)); }

//在给定的偏移量处验证对多个内存元素的访问，并返回指向它的指针。
	template<typename Value> Value* memoryArrayPtr(MemoryInstance* memory,U32 offset,U32 numElements)
	{ return (Value*)getValidatedMemoryOffsetRange(memory,offset,numElements * sizeof(Value)); }

//
//全局的
//

//创建具有指定类型和初始值的GlobalInstance。
	RUNTIME_API GlobalInstance* createGlobal(IR::GlobalType type,Value initialValue);

//读取全局的当前值。
	RUNTIME_API Value getGlobalValue(GlobalInstance* global);

//将新值写入全局，并返回上一个值。
	RUNTIME_API Value setGlobalValue(GlobalInstance* global,Value newValue);

//
//模块
//

	struct ImportBindings
	{
		std::vector<FunctionInstance*> functions;
		std::vector<TableInstance*> tables;
		std::vector<MemoryInstance*> memories;
		std::vector<GlobalInstance*> globals;
	};

//实例化模块，将其导入绑定到指定的对象。可能引发InstantiationException。
	RUNTIME_API ModuleInstance* instantiateModule(const IR::Module& module,ImportBindings&& imports);

//获取moduleInstance的默认表/内存。
	RUNTIME_API MemoryInstance* getDefaultMemory(ModuleInstance* moduleInstance);
	RUNTIME_API uint64_t getDefaultMemorySize(ModuleInstance* moduleInstance);
	RUNTIME_API TableInstance* getDefaultTable(ModuleInstance* moduleInstance);

	RUNTIME_API void runInstanceStartFunc(ModuleInstance* moduleInstance);
	RUNTIME_API void resetGlobalInstances(ModuleInstance* moduleInstance);
	RUNTIME_API void resetMemory(MemoryInstance* memory, IR::MemoryType& newMemoryType);

//获取通过moduleInstance按名称导出的对象。
	RUNTIME_API ObjectInstance* getInstanceExport(ModuleInstance* moduleInstance,const std::string& name);
}
