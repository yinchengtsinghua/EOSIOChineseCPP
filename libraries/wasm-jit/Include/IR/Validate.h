
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "IR.h"
#include "IR/Operators.h"

#include <string>

namespace IR
{
	struct Module;
	struct FunctionDef;

	struct ValidationException
	{
		std::string message;
		ValidationException(std::string&& inMessage): message(inMessage) {}
	};

	struct CodeValidationStreamImpl;

	struct CodeValidationStream
	{
		IR_API CodeValidationStream(const Module& module,const FunctionDef& function);
		IR_API ~CodeValidationStream();
		
		IR_API void finish();

		#define VISIT_OPCODE(_,name,nameString,Imm,...) IR_API void name(Imm imm = {});
		ENUM_OPERATORS(VISIT_OPCODE)
		#undef VISIT_OPCODE

	private:
		CodeValidationStreamImpl* impl;
	};

	template<typename InnerStream>
	struct CodeValidationProxyStream
	{
		CodeValidationProxyStream(const Module& module,const FunctionDef& function,InnerStream& inInnerStream)
		: codeValidationStream(module,function)
		, innerStream(inInnerStream)
		{}
		
		void finishValidation() { codeValidationStream.finish(); }

		#define VISIT_OPCODE(_,name,nameString,Imm,...) \
			void name(Imm imm = {}) \
			{ \
				codeValidationStream.name(imm); \
				innerStream.name(imm); \
			}
		ENUM_OPERATORS(VISIT_OPCODE)
		#undef VISIT_OPCODE

	private:

		CodeValidationStream codeValidationStream;
		InnerStream& innerStream;
	};

	IR_API void validateDefinitions(const IR::Module& module);
}
