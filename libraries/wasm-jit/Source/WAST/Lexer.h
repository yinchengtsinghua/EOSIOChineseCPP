
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "Inline/BasicTypes.h"
#include "IR/Operators.h"
#include "WAST.h"

#define VISIT_OPERATOR_TOKEN(opcode,name,nameString,...) \
	VISIT_TOKEN(name,"'" #nameString "'")

#define VISIT_LITERAL_TOKEN(name) VISIT_TOKEN(name,"'" #name "'")
#define ENUM_LITERAL_TOKENS() \
	VISIT_LITERAL_TOKEN(module) \
	VISIT_LITERAL_TOKEN(func) \
	VISIT_LITERAL_TOKEN(type) \
	VISIT_LITERAL_TOKEN(table) \
	VISIT_LITERAL_TOKEN(export) \
	VISIT_LITERAL_TOKEN(import) \
	VISIT_LITERAL_TOKEN(memory) \
	VISIT_LITERAL_TOKEN(data) \
	VISIT_LITERAL_TOKEN(elem) \
	VISIT_LITERAL_TOKEN(start) \
	VISIT_LITERAL_TOKEN(param) \
	VISIT_LITERAL_TOKEN(result) \
	VISIT_LITERAL_TOKEN(local) \
	VISIT_LITERAL_TOKEN(global) \
	VISIT_LITERAL_TOKEN(assert_return) \
	VISIT_LITERAL_TOKEN(assert_return_canonical_nan) \
	VISIT_LITERAL_TOKEN(assert_return_arithmetic_nan) \
	VISIT_LITERAL_TOKEN(assert_trap) \
	VISIT_LITERAL_TOKEN(assert_invalid) \
	VISIT_LITERAL_TOKEN(assert_unlinkable) \
	VISIT_LITERAL_TOKEN(assert_malformed) \
	VISIT_LITERAL_TOKEN(assert_exhaustion) \
	VISIT_LITERAL_TOKEN(invoke) \
	VISIT_LITERAL_TOKEN(get) \
	VISIT_LITERAL_TOKEN(align) \
	VISIT_LITERAL_TOKEN(offset) \
	VISIT_LITERAL_TOKEN(then) \
	VISIT_LITERAL_TOKEN(register) \
	VISIT_LITERAL_TOKEN(mut) \
	VISIT_LITERAL_TOKEN(i32) \
	VISIT_LITERAL_TOKEN(i64) \
	VISIT_LITERAL_TOKEN(f32) \
	VISIT_LITERAL_TOKEN(f64) \
	VISIT_LITERAL_TOKEN(anyfunc) \
	VISIT_LITERAL_TOKEN(shared) \
	VISIT_LITERAL_TOKEN(quote) \
	VISIT_LITERAL_TOKEN(binary) \
	ENUM_SIMD_LITERAL_TOKENS()

#if !ENABLE_SIMD_PROTOTYPE
#define ENUM_SIMD_LITERAL_TOKENS()
#else
#define ENUM_SIMD_LITERAL_TOKENS() \
	VISIT_LITERAL_TOKEN(v128) \
	VISIT_LITERAL_TOKEN(b8x16) \
	VISIT_LITERAL_TOKEN(b16x8) \
	VISIT_LITERAL_TOKEN(b32x4) \
	VISIT_LITERAL_TOKEN(b64x2)
#endif
	
#define ENUM_TOKENS() \
	VISIT_TOKEN(eof,"eof") \
	\
	VISIT_TOKEN(unterminatedComment,"unterminated comment") \
	VISIT_TOKEN(unrecognized,"unrecognized token") \
	\
	VISIT_TOKEN(decimalFloat,"decimal float literal") \
	VISIT_TOKEN(decimalInt,"decimal int literal") \
	VISIT_TOKEN(hexFloat,"hexadecimal float literal") \
	VISIT_TOKEN(hexInt,"hexadecimal int literal") \
	VISIT_TOKEN(floatNaN,"float NaN literal") \
	VISIT_TOKEN(floatInf,"float infinity literal") \
	VISIT_TOKEN(string,"string literal") \
	VISIT_TOKEN(name,"name literal") \
	\
	VISIT_TOKEN(leftParenthesis,"'('") \
	VISIT_TOKEN(rightParenthesis,"')'") \
	VISIT_TOKEN(equals,"'='") \
	\
	ENUM_LITERAL_TOKENS() \
	\
	ENUM_OPERATORS(VISIT_OPERATOR_TOKEN)

namespace WAST
{
	enum TokenType : U16
	{
		#define VISIT_TOKEN(name,description) t_##name,
		ENUM_TOKENS()
		#undef VISIT_TOKEN
		numTokenTypes
	};

	PACKED_STRUCT(
	struct Token
	{
		TokenType type;
		U32 begin;
	});

	struct LineInfo;

//lexes字符串并返回一个标记数组。
//还将OutlineInfo中的指针返回到解析标记的行号/列号所需的信息。
//调用方在使用完令牌和行信息后，应分别将它们传递给freetokens/freelineinfo。
	Token* lex(const char* string,Uptr stringLength,LineInfo*& outLineInfo);

	void freeTokens(Token* tokens);
	void freeLineInfo(LineInfo* lineInfo);
	
	const char* describeToken(TokenType tokenType);	

	TextFileLocus calcLocusFromOffset(const char* string,const LineInfo* lineInfo,Uptr charOffset);
}