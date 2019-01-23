
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "Inline/BasicTypes.h"
#include "WAST.h"
#include "Runtime/TaggedValue.h"

#include <vector>
#include <memory>

namespace WAST
{	
	struct Command
	{
		enum Type
		{
			_register,
			action,
			assert_return,
			assert_return_canonical_nan,
			assert_return_arithmetic_nan,
			assert_trap,
			assert_invalid,
			assert_malformed,
			assert_unlinkable
		};
		const Type type;
		const TextFileLocus locus;

		Command(Type inType,TextFileLocus&& inLocus): type(inType), locus(inLocus) {}
	};
	
//从字符串分析测试脚本。如果成功，则返回true，并将测试命令写入outtestcommands。
	WAST_API void parseTestCommands(
		const char* string,
		Uptr stringLength,
		std::vector<std::unique_ptr<Command>>& outTestCommands,
		std::vector<Error>& outErrors);
	
//行动

	enum class ActionType
	{
		_module,
		invoke,
		get,
	};

	struct Action
	{
		const ActionType type;
		const TextFileLocus locus;

		Action(ActionType inType,TextFileLocus&& inLocus): type(inType), locus(inLocus) {}
	};

	struct ModuleAction : Action
	{
		std::string internalModuleName;
		std::unique_ptr<IR::Module> module;
		ModuleAction(TextFileLocus&& inLocus,std::string&& inInternalModuleName,IR::Module* inModule)
		: Action(ActionType::_module,std::move(inLocus)), internalModuleName(inInternalModuleName), module(inModule) {}
	};

	struct InvokeAction : Action
	{
		std::string internalModuleName;
		std::string exportName;
		std::vector<Runtime::Value> arguments;
		InvokeAction(TextFileLocus&& inLocus,std::string&& inInternalModuleName,std::string&& inExportName,std::vector<Runtime::Value>&& inArguments)
		: Action(ActionType::invoke,std::move(inLocus)), internalModuleName(inInternalModuleName), exportName(inExportName), arguments(inArguments)
		{}
	};

	struct GetAction : Action
	{
		std::string internalModuleName;
		std::string exportName;
		GetAction(TextFileLocus&& inLocus,std::string&& inInternalModuleName,std::string&& inExportName)
		: Action(ActionType::get,std::move(inLocus)), internalModuleName(inInternalModuleName), exportName(inExportName)
		{}
	};

//命令

	struct RegisterCommand : Command
	{
		std::string moduleName;
		std::string internalModuleName;
		RegisterCommand(TextFileLocus&& inLocus,std::string&& inModuleName,std::string&& inInternalModuleName)
		: Command(Command::_register,std::move(inLocus)), moduleName(inModuleName), internalModuleName(inInternalModuleName)
		{}
	};

	struct ActionCommand : Command
	{
		std::unique_ptr<Action> action;
		ActionCommand(TextFileLocus&& inLocus,Action* inAction)
		: Command(Command::action,std::move(inLocus)), action(inAction) {}
	};

	struct AssertReturnCommand : Command
	{
		std::unique_ptr<Action> action;
		Runtime::Result expectedReturn;
		AssertReturnCommand(TextFileLocus&& inLocus,Action* inAction,Runtime::Result inExpectedReturn)
		: Command(Command::assert_return,std::move(inLocus)), action(inAction), expectedReturn(inExpectedReturn) {}
	};

	struct AssertReturnNaNCommand : Command
	{
		std::unique_ptr<Action> action;
		AssertReturnNaNCommand(Command::Type inType,TextFileLocus&& inLocus,Action* inAction)
		: Command(inType,std::move(inLocus)), action(inAction) {}
	};

	struct AssertTrapCommand : Command
	{
		std::unique_ptr<Action> action;
		Runtime::Exception::Cause expectedCause;
		AssertTrapCommand(TextFileLocus&& inLocus,Action* inAction,Runtime::Exception::Cause inExpectedCause)
		: Command(Command::assert_trap,std::move(inLocus)), action(inAction), expectedCause(inExpectedCause) {}
	};

	struct AssertInvalidOrMalformedCommand : Command
	{
		bool wasInvalidOrMalformed;
		AssertInvalidOrMalformedCommand(Command::Type inType,TextFileLocus&& inLocus,bool inWasInvalidOrMalformed)
		: Command(inType,std::move(inLocus)), wasInvalidOrMalformed(inWasInvalidOrMalformed) {}
	};
	
	struct AssertUnlinkableCommand : Command
	{
		std::unique_ptr<ModuleAction> moduleAction;
		AssertUnlinkableCommand(TextFileLocus&& inLocus,ModuleAction* inModuleAction)
		: Command(Command::assert_unlinkable,std::move(inLocus)), moduleAction(inModuleAction) {}
	};
}