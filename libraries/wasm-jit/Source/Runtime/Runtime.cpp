
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "Inline/BasicTypes.h"
#include "Platform/Platform.h"
#include "Logging/Logging.h"
#include "Runtime.h"
#include "RuntimePrivate.h"

#include <iostream>

namespace Runtime
{
	void init()
	{
		LLVMJIT::init();
		initWAVMIntrinsics();
	}
	
//返回字符串的向量，每个元素描述调用堆栈的帧。
//如果帧是一个JIT函数，请使用关于该函数的JIT信息
//描述它，否则返回到平台特定的符号分辨率
//是可用的。
	std::vector<std::string> describeCallStack(const Platform::CallStack& callStack)
	{
		std::vector<std::string> frameDescriptions;
		for(auto frame : callStack.stackFrames)
		{
			std::string frameDescription;
			if(	LLVMJIT::describeInstructionPointer(frame.ip,frameDescription)
			||	Platform::describeInstructionPointer(frame.ip,frameDescription))
			{
				frameDescriptions.push_back(frameDescription);
			}
			else { frameDescriptions.push_back("<unknown function>"); }
		}
		return frameDescriptions;
	}

	[[noreturn]] void causeException(Exception::Cause cause)
	{
		auto callStack = Platform::captureCallStack();
		throw Exception {cause,describeCallStack(callStack)};
	}

	bool isA(ObjectInstance* object,const ObjectType& type)
	{
		if(type.kind != object->kind) { return false; }

		switch(type.kind)
		{
		case ObjectKind::function: return asFunctionType(type) == asFunction(object)->type;
		case ObjectKind::global: return asGlobalType(type) == asGlobal(object)->type;
		case ObjectKind::table: return isSubset(asTableType(type),asTable(object)->type);
		case ObjectKind::memory: return isSubset(asMemoryType(type),asMemory(object)->type);
		default: Errors::unreachable();
		}
	}

	[[noreturn]] void handleHardwareTrap(Platform::HardwareTrapType trapType,Platform::CallStack&& trapCallStack,Uptr trapOperand)
	{
      std::cerr << "handle hardware trap\n";
		std::vector<std::string> callStackDescription = describeCallStack(trapCallStack);

		switch(trapType)
		{
		case Platform::HardwareTrapType::accessViolation:
		{
//如果访问冲突发生在表的保留页中，则将其视为未定义的表元素运行时错误。
			if(isAddressOwnedByTable(reinterpret_cast<U8*>(trapOperand))) { throw Exception { Exception::Cause::undefinedTableElement, callStackDescription }; }
//如果访问冲突发生在内存的保留页中，则将其视为访问冲突运行时错误。
			else if(isAddressOwnedByMemory(reinterpret_cast<U8*>(trapOperand))) { throw Exception { Exception::Cause::accessViolation, callStackDescription }; }
			else
			{
//如果访问冲突发生在表或内存之外，则将其视为bug（可能是安全漏洞）
//而不是WebAssembly代码中的运行时错误。
				Log::printf(Log::Category::error,"Access violation outside of table or memory reserved addresses. Call stack:\n");
				for(auto calledFunction : callStackDescription) { Log::printf(Log::Category::error,"  %s\n",calledFunction.c_str()); }
				Errors::fatalf("unsandboxed access violation");
			}
		}
		case Platform::HardwareTrapType::stackOverflow: throw Exception { Exception::Cause::stackOverflow, callStackDescription };
		case Platform::HardwareTrapType::intDivideByZeroOrOverflow: throw Exception { Exception::Cause::integerDivideByZeroOrIntegerOverflow, callStackDescription };
		default: Errors::unreachable();
		};
	}


	Result invokeFunction(FunctionInstance* function,const std::vector<Value>& parameters)
	{
		const FunctionType* functionType = function->type;
		
//检查参数类型是否与函数匹配，并将它们复制到存储为64位值的内存块中。
		if(parameters.size() != functionType->parameters.size())
		{ 
       throw Exception {Exception::Cause::invokeSignatureMismatch}; 
    }

		U64* thunkMemory = (U64*)alloca((functionType->parameters.size() + getArity(functionType->ret)) * sizeof(U64));
		for(Uptr parameterIndex = 0;parameterIndex < functionType->parameters.size();++parameterIndex)
		{
			if(functionType->parameters[parameterIndex] != parameters[parameterIndex].type)
			{
				throw Exception {Exception::Cause::invokeSignatureMismatch};
			}

			thunkMemory[parameterIndex] = parameters[parameterIndex].i64;
		}
		
//获取此函数类型的invoke thunk。
		LLVMJIT::InvokeFunctionPointer invokeFunctionPointer = LLVMJIT::getInvokeThunk(functionType);

//捕获特定于平台的运行时异常并将其转换为运行时：：值。
		Result result;
		Platform::HardwareTrapType trapType;
		Platform::CallStack trapCallStack;
		Uptr trapOperand;
		trapType = Platform::catchHardwareTraps(trapCallStack,trapOperand,
			[&]
			{
//调用调用thunk。
				(*invokeFunctionPointer)(function->nativeFunction,thunkMemory);

//从thunk内存块中读取返回值。
				if(functionType->ret != ResultType::none)
				{
					result.type = functionType->ret;
					result.i64 = thunkMemory[functionType->parameters.size()];
				}
			});

//如果没有硬件陷阱，只返回结果。
		if(trapType == Platform::HardwareTrapType::none) { return result; }
		else { handleHardwareTrap(trapType,std::move(trapCallStack),trapOperand); }
	}

	const FunctionType* getFunctionType(FunctionInstance* function)
	{
		return function->type;
	}

	GlobalInstance* createGlobal(GlobalType type,Value initialValue)
	{
		return new GlobalInstance(type,initialValue);
	}

	Value getGlobalValue(GlobalInstance* global)
	{
		return Value(global->type.valueType,global->value);
	}

	Value setGlobalValue(GlobalInstance* global,Value newValue)
	{
		WAVM_ASSERT_THROW(newValue.type == global->type.valueType);
		WAVM_ASSERT_THROW(global->type.isMutable);
		const Value previousValue = Value(global->type.valueType,global->value);
		global->value = newValue;
		return previousValue;
	}
}
