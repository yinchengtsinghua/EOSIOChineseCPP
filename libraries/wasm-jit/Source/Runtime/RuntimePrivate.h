
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "Inline/BasicTypes.h"
#include "Platform/Platform.h"
#include "Runtime.h"

#include <functional>
#include <map>
#include <atomic>

#define HAS_64BIT_ADDRESS_SPACE (sizeof(Uptr) == 8 && !PRETEND_32BIT_ADDRESS_SPACE)

namespace LLVMJIT
{
	using namespace Runtime;
	
	struct JITModuleBase
	{
		virtual ~JITModuleBase() {}
	};

	void init();
	void instantiateModule(const IR::Module& module,Runtime::ModuleInstance* moduleInstance);
	bool describeInstructionPointer(Uptr ip,std::string& outDescription);
	
	typedef void (*InvokeFunctionPointer)(void*,U64*);

//为特定函数类型生成invoke thunk。
	InvokeFunctionPointer getInvokeThunk(const IR::FunctionType* functionType);
}

namespace Runtime
{
	using namespace IR;
	
//处理垃圾收集的所有运行时对象的专用根。
	struct GCObject : ObjectInstance
	{
		GCObject(ObjectKind inKind);
		~GCObject() override;
	};

//函数的实例：在实例化模块中定义的函数，或内部函数。
	struct FunctionInstance : GCObject
	{
		ModuleInstance* moduleInstance;
		const FunctionType* type;
		void* nativeFunction;
		std::string debugName;

		FunctionInstance(ModuleInstance* inModuleInstance,const FunctionType* inType,void* inNativeFunction = nullptr,const char* inDebugName = "<unidentified FunctionInstance>")
		: GCObject(ObjectKind::function), moduleInstance(inModuleInstance), type(inType), nativeFunction(inNativeFunction), debugName(inDebugName) {}
	};

//WebAssembly表的实例。
	struct TableInstance : GCObject
	{
		struct FunctionElement
		{
			const FunctionType* type;
			void* value;
		};

		TableType type;

		FunctionElement* baseAddress;
		Uptr endOffset;

		U8* reservedBaseAddress;
		Uptr reservedNumPlatformPages;

//与基地址处的函数元素相对应的对象。
		std::vector<ObjectInstance*> elements;

		TableInstance(const TableType& inType): GCObject(ObjectKind::table), type(inType), baseAddress(nullptr), endOffset(0), reservedBaseAddress(nullptr), reservedNumPlatformPages(0) {}
		~TableInstance() override;
	};

//Web程序集内存的实例。
	struct MemoryInstance : GCObject
	{
		MemoryType type;

		U8* baseAddress;
		std::atomic<Uptr> numPages;
		Uptr endOffset;

		U8* reservedBaseAddress;
		Uptr reservedNumPlatformPages;

		MemoryInstance(const MemoryType& inType): GCObject(ObjectKind::memory), type(inType), baseAddress(nullptr), numPages(0), endOffset(0), reservedBaseAddress(nullptr), reservedNumPlatformPages(0) {}
		~MemoryInstance() override;

      static MemoryInstance* theMemoryInstance;
	};

//Web程序集全局的实例。
	struct GlobalInstance : GCObject
	{
		GlobalType type;
		UntaggedValue value;
		UntaggedValue initialValue;

		GlobalInstance(GlobalType inType,UntaggedValue inValue): GCObject(ObjectKind::global), type(inType), value(inValue), initialValue(value) {}
	};

//WebAssembly模块的实例。
	struct ModuleInstance : GCObject
	{
		std::map<std::string,ObjectInstance*> exportMap;

		std::vector<FunctionInstance*> functionDefs;

		std::vector<FunctionInstance*> functions;
		std::vector<TableInstance*> tables;
		std::vector<MemoryInstance*> memories;
		std::vector<GlobalInstance*> globals;

		MemoryInstance* defaultMemory;
		TableInstance* defaultTable;

		LLVMJIT::JITModuleBase* jitModule;

		Uptr startFunctionIndex = UINTPTR_MAX;

		ModuleInstance(
			std::vector<FunctionInstance*>&& inFunctionImports,
			std::vector<TableInstance*>&& inTableImports,
			std::vector<MemoryInstance*>&& inMemoryImports,
			std::vector<GlobalInstance*>&& inGlobalImports
			)
		: GCObject(ObjectKind::module)
		, functions(inFunctionImports)
		, tables(inTableImports)
		, memories(inMemoryImports)
		, globals(inGlobalImports)
		, defaultMemory(nullptr)
		, defaultTable(nullptr)
		, jitModule(nullptr)
		{}

		~ModuleInstance() override;
	};

//初始化wavm内部函数使用的全局状态。
	void initWAVMIntrinsics();

//检查地址是属于表还是内存。
	bool isAddressOwnedByTable(U8* address);
	bool isAddressOwnedByMemory(U8* address);
	
//使用填充的AlignBytes分配虚拟页，并返回对齐的基址。
//未对齐的分配地址和大小将写入OutUnAlignedBaseAddress和OutUnAlignedNumPlatformPages。
	U8* allocateVirtualPagesAligned(Uptr numBytes,Uptr alignmentBytes,U8*& outUnalignedBaseAddress,Uptr& outUnalignedNumPlatformPages);

//将WASM代码中发生的硬件陷阱转换为运行时异常或致命错误。
	[[noreturn]] void handleHardwareTrap(Platform::HardwareTrapType trapType,Platform::CallStack&& trapCallStack,Uptr trapOperand);

//将WASM线程的GC根添加到提供的数组中。
	void getThreadGCRoots(std::vector<ObjectInstance*>& outGCRoots);
}
