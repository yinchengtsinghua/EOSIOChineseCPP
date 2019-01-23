
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "Inline/BasicTypes.h"
#include "Inline/Serialization.h"
#include "Platform/Platform.h"
#include "WAST/WAST.h"
#include "WAST/TestScript.h"
#include "WASM/WASM.h"
#include "Runtime/Runtime.h"
#include "Runtime/Linker.h"
#include "Runtime/Intrinsics.h"

#include "CLI.h"

#include <map>
#include <vector>
#include <cstdio>
#include <cstdarg>

using namespace WAST;
using namespace IR;
using namespace Runtime;

struct TestScriptState
{
	bool hasInstantiatedModule;
	ModuleInstance* lastModuleInstance;
	
	std::map<std::string,ModuleInstance*> moduleInternalNameToInstanceMap;
	std::map<std::string,ModuleInstance*> moduleNameToInstanceMap;
	
	std::vector<WAST::Error> errors;
	
	TestScriptState() : hasInstantiatedModule(false), lastModuleInstance(nullptr) {}
};

struct TestScriptResolver : Resolver
{
	TestScriptResolver(const TestScriptState& inState): state(inState) {}
	bool resolve(const std::string& moduleName,const std::string& exportName,ObjectType type,ObjectInstance*& outObject) override
	{
//首先尝试解决一个固有问题。
		if(IntrinsicResolver::singleton.resolve(moduleName,exportName,type,outObject)) { return true; }

//然后查找命名模块。
		auto mapIt = state.moduleNameToInstanceMap.find(moduleName);
		if(mapIt != state.moduleNameToInstanceMap.end())
		{
			outObject = getInstanceExport(mapIt->second,exportName);
			return outObject != nullptr && isA(outObject,type);
		}

		return false;
	}
private:
	const TestScriptState& state;
};

void testErrorf(TestScriptState& state,const TextFileLocus& locus,const char* messageFormat,...)
{
	va_list messageArguments;
	va_start(messageArguments,messageFormat);
	char messageBuffer[1024];
	int numPrintedChars = std::vsnprintf(messageBuffer,sizeof(messageBuffer),messageFormat,messageArguments);
	if(numPrintedChars >= 1023 || numPrintedChars < 0) { Errors::unreachable(); }
	messageBuffer[numPrintedChars] = 0;
	va_end(messageArguments);
		
	state.errors.push_back({locus,messageBuffer});
}

void collectGarbage(TestScriptState& state)
{
	std::vector<ObjectInstance*> rootObjects;
	rootObjects.push_back(asObject(state.lastModuleInstance));
	for(auto& mapIt : state.moduleInternalNameToInstanceMap) { rootObjects.push_back(asObject(mapIt.second)); }
	for(auto& mapIt : state.moduleNameToInstanceMap) { rootObjects.push_back(asObject(mapIt.second)); }
	freeUnreferencedObjects(std::move(rootObjects));
}

ModuleInstance* getModuleContextByInternalName(TestScriptState& state,const TextFileLocus& locus,const char* context,const std::string& internalName)
{
//查找此调用使用的模块。
	if(!state.hasInstantiatedModule) { testErrorf(state,locus,"no module to use in %s",context); return nullptr; }
	ModuleInstance* moduleInstance = state.lastModuleInstance;
	if(internalName.size())
	{
		auto mapIt = state.moduleInternalNameToInstanceMap.find(internalName);
		if(mapIt == state.moduleInternalNameToInstanceMap.end())
		{
			testErrorf(state,locus,"unknown %s module name: %s",context,internalName.c_str());
			return nullptr;
		}
		moduleInstance = mapIt->second;
	}
	return moduleInstance;
}

bool processAction(TestScriptState& state,Action* action,Result& outResult)
{
	outResult = Result();

	switch(action->type)
	{
	case ActionType::_module:
	{
		auto moduleAction = (ModuleAction*)action;

//清除上一个模块。
		state.lastModuleInstance = nullptr;
		collectGarbage(state);

//链接并实例化模块。
		TestScriptResolver resolver(state);
		LinkResult linkResult = linkModule(*moduleAction->module,resolver);
		if(linkResult.success)
		{
			state.hasInstantiatedModule = true;
			state.lastModuleInstance = instantiateModule(*moduleAction->module,std::move(linkResult.resolvedImports));
		}
		else
		{
//为无法链接的每个导入创建一个错误。
			for(auto& missingImport : linkResult.missingImports)
			{
				testErrorf(
					state,
					moduleAction->locus,
					"missing import module=\"%s\" export=\"%s\" type=\"%s\"",
					missingImport.moduleName.c_str(),
					missingImport.exportName.c_str(),
					asString(missingImport.type).c_str()
					);
			}
		}

//在模块的内部名称下注册模块。
		if(moduleAction->internalModuleName.size())
		{
			state.moduleInternalNameToInstanceMap[moduleAction->internalModuleName] = state.lastModuleInstance;
		}

		return true;
	}
	case ActionType::invoke:
	{
		auto invokeAction = (InvokeAction*)action;

//查找此调用使用的模块。
		ModuleInstance* moduleInstance = getModuleContextByInternalName(state,invokeAction->locus,"invoke",invokeAction->internalModuleName);

//此时的空模块实例表示一个模块未能链接或实例化，因此不会产生进一步的错误。
		if(!moduleInstance) { return false; }

//在模块实例中查找命名的导出。
		auto functionInstance = asFunctionNullable(getInstanceExport(moduleInstance,invokeAction->exportName));
		if(!functionInstance) { testErrorf(state,invokeAction->locus,"couldn't find exported function with name: %s",invokeAction->exportName.c_str()); return false; }

//执行调用
		outResult = invokeFunction(functionInstance,invokeAction->arguments);

		return true;
	}
	case ActionType::get:
	{
		auto getAction = (GetAction*)action;

//查找此GET使用的模块。
		ModuleInstance* moduleInstance = getModuleContextByInternalName(state,getAction->locus,"get",getAction->internalModuleName);

//此时的一个空模块实例表示一个模块未能链接或实例化，因此返回时不会出现进一步的错误。
		if(!moduleInstance) { return false; }

//在模块实例中查找命名的导出。
		auto globalInstance = asGlobalNullable(getInstanceExport(moduleInstance,getAction->exportName));
		if(!globalInstance) { testErrorf(state,getAction->locus,"couldn't find exported global with name: %s",getAction->exportName.c_str()); return false; }

//获取指定全局的值。
		outResult = getGlobalValue(globalInstance);
			
		return true;
	}
	default:
		Errors::unreachable();
	}
}

//测试一个浮点是否是一个“规范的”NaN，这意味着它只是一个NaN，它的有效位集的最高位。
template<typename Float> bool isCanonicalOrArithmeticNaN(Float value,bool requireCanonical)
{
	Floats::FloatComponents<Float> components;
	components.value = value;
	return components.bits.exponent == Floats::FloatComponents<Float>::maxExponentBits
	&& (!requireCanonical || components.bits.significand == Floats::FloatComponents<Float>::canonicalSignificand);
}

void processCommand(TestScriptState& state,const Command* command)
{
	try
	{
		switch(command->type)
		{
		case Command::_register:
		{
			auto registerCommand = (RegisterCommand*)command;

//按内部名称查找模块，并将结果绑定到公共名称。
			ModuleInstance* moduleInstance = getModuleContextByInternalName(state,registerCommand->locus,"register",registerCommand->internalModuleName);
			state.moduleNameToInstanceMap[registerCommand->moduleName] = moduleInstance;
			break;
		}
		case Command::action:
		{
			Result result;
			processAction(state,((ActionCommand*)command)->action.get(),result);
			break;
		}
		case Command::assert_return:
		{
			auto assertCommand = (AssertReturnCommand*)command;
//执行该操作，并将结果按位与预期结果进行比较。
			Result actionResult;
			if(processAction(state,assertCommand->action.get(),actionResult)
			&& !areBitsEqual(actionResult,assertCommand->expectedReturn))
			{
				testErrorf(state,assertCommand->locus,"expected %s but got %s",
					asString(assertCommand->expectedReturn).c_str(),
					asString(actionResult).c_str());
			}
			break;
		}
		case Command::assert_return_canonical_nan: case Command::assert_return_arithmetic_nan:
		{
			auto assertCommand = (AssertReturnNaNCommand*)command;
//执行操作并检查结果是否为预期类型的NaN。
			Result actionResult;
			if(processAction(state,assertCommand->action.get(),actionResult))
			{
				const bool requireCanonicalNaN = assertCommand->type == Command::assert_return_canonical_nan;
				const bool isError =
						actionResult.type == ResultType::f32 ? !isCanonicalOrArithmeticNaN(actionResult.f32,requireCanonicalNaN)
					:	actionResult.type == ResultType::f64 ? !isCanonicalOrArithmeticNaN(actionResult.f64,requireCanonicalNaN)
					:	true;
				if(isError)
				{
					testErrorf(state,assertCommand->locus,
						requireCanonicalNaN ? "expected canonical float NaN but got %s" : "expected float NaN but got %s",
						asString(actionResult).c_str());
				}
			}
			break;
		}
		case Command::assert_trap:
		{
			auto assertCommand = (AssertTrapCommand*)command;
			try
			{
				Result actionResult;
				if(processAction(state,assertCommand->action.get(),actionResult))
				{
					testErrorf(state,assertCommand->locus,"expected trap but got %s",asString(actionResult).c_str());
				}
			}
			catch(Runtime::Exception exception)
			{
				if(exception.cause != assertCommand->expectedCause)
				{
					testErrorf(state,assertCommand->action->locus,"expected %s trap but got %s trap",
						describeExceptionCause(assertCommand->expectedCause),
						describeExceptionCause(exception.cause));
				}
			}
			break;
		}
		case Command::assert_invalid: case Command::assert_malformed:
		{
			auto assertCommand = (AssertInvalidOrMalformedCommand*)command;
			if(!assertCommand->wasInvalidOrMalformed)
			{
				testErrorf(state,assertCommand->locus,"module was %s",
					assertCommand->type == Command::assert_invalid ? "valid" : "well formed");
			}
			break;
		}
		case Command::assert_unlinkable:
		{
			auto assertCommand = (AssertUnlinkableCommand*)command;
			Result result;
			try
			{
				TestScriptResolver resolver(state);
				LinkResult linkResult = linkModule(*assertCommand->moduleAction->module,resolver);
				if(linkResult.success)
				{
					instantiateModule(*assertCommand->moduleAction->module,std::move(linkResult.resolvedImports));
					testErrorf(state,assertCommand->locus,"module was linkable");
				}
			}
			catch(Runtime::Exception)
			{
//如果实例化引发异常，则断言不可链接成功。
			}
			break;
		}
		};
	}
	catch(Runtime::Exception exception)
	{
		testErrorf(state,command->locus,"unexpected trap: %s",describeExceptionCause(exception.cause));
	}
}

DEFINE_INTRINSIC_FUNCTION0(spectest,spectest_print,print,none) {}
DEFINE_INTRINSIC_FUNCTION1(spectest,spectest_print,print,none,i32,a) { std::cout << a << " : i32" << std::endl; }
DEFINE_INTRINSIC_FUNCTION1(spectest,spectest_print,print,none,i64,a) { std::cout << a << " : i64" << std::endl; }
DEFINE_INTRINSIC_FUNCTION1(spectest,spectest_print,print,none,f32,a) { std::cout << a << " : f32" << std::endl; }
DEFINE_INTRINSIC_FUNCTION1(spectest,spectest_print,print,none,f64,a) { std::cout << a << " : f64" << std::endl; }
DEFINE_INTRINSIC_FUNCTION2(spectest,spectest_print,print,none,f64,a,f64,b) { std::cout << a << " : f64" << std::endl << b << " : f64" << std::endl; }
DEFINE_INTRINSIC_FUNCTION2(spectest,spectest_print,print,none,i32,a,f32,b) { std::cout << a << " : i32" << std::endl << b << " : f32" << std::endl; }
DEFINE_INTRINSIC_FUNCTION2(spectest,spectest_print,print,none,i64,a,f64,b) { std::cout << a << " : i64" << std::endl << b << " : f64" << std::endl; }

DEFINE_INTRINSIC_GLOBAL(spectest,spectest_globalI32,global,i32,false,666)
DEFINE_INTRINSIC_GLOBAL(spectest,spectest_globalI64,global,i64,false,0)
DEFINE_INTRINSIC_GLOBAL(spectest,spectest_globalF32,global,f32,false,0.0f)
DEFINE_INTRINSIC_GLOBAL(spectest,spectest_globalF64,global,f64,false,0.0)

DEFINE_INTRINSIC_TABLE(spectest,spectest_table,table,TableType(TableElementType::anyfunc,false,SizeConstraints {10,20}))
DEFINE_INTRINSIC_MEMORY(spectest,spectest_memory,memory,MemoryType(false,SizeConstraints {1,2}))

int commandMain(int argc,char** argv)
{
	if(argc != 2)
	{
		std::cerr <<  "Usage: Test in.wast" << std::endl;
		return EXIT_FAILURE;
	}
	const char* filename = argv[1];
	
//始终为测试启用调试日志记录。
	Log::setCategoryEnabled(Log::Category::debug,true);

	Runtime::init();
	
//将文件读入字符串。
	const std::string testScriptString = loadFile(filename);
	if(!testScriptString.size()) { return EXIT_FAILURE; }

//处理测试脚本。
	TestScriptState testScriptState;
	std::vector<std::unique_ptr<Command>> testCommands;
	
//分析测试脚本。
	WAST::parseTestCommands(testScriptString.c_str(),testScriptString.size(),testCommands,testScriptState.errors);
	if(!testScriptState.errors.size())
	{
//处理测试脚本命令。
		for(auto& command : testCommands)
		{
			processCommand(testScriptState,command.get());
		}
	}
	
	if(testScriptState.errors.size())
	{
//打印任何错误；
		for(auto& error : testScriptState.errors)
		{
			std::cerr << filename << ":" << error.locus.describe() << ": " << error.message.c_str() << std::endl;
			std::cerr << error.locus.sourceLine << std::endl;
			std::cerr << std::setw(error.locus.column(8)) << "^" << std::endl;
		}

		std::cerr << filename << ": testing failed!" << std::endl;
		return EXIT_FAILURE;
	}
	else
	{
		std::cout << filename << ": all tests passed." << std::endl;
		return EXIT_SUCCESS;
	}
}
