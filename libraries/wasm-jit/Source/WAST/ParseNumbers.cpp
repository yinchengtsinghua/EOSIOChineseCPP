
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "Inline/BasicTypes.h"
#include "Inline/Floats.h"
#include "WAST.h"
#include "Lexer.h"
#include "Parse.h"

#include <climits>
#include <cerrno>

//包括大卫盖伊的DTOA代码。
//定义strtod和dtoa以避免与C标准库版本冲突
//包含stdlib.h的dtoa.c很复杂，所以我们还需要包含stdlib.h
//这里是为了确保它不包含在strtod和dtoa定义中。
#include <stdlib.h>

#define IEEE_8087
#define NO_INFNAN_CHECK
#define strtod parseNonSpecialF64
#define dtoa printNonSpecialF64

#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 4244 4083 4706 4701 4703 4018)
#elif defined(__GNUC__) && !defined(__clang__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
	#define Long int
#else
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#define Long int
#endif

#include "../ThirdParty/dtoa.c"

#ifdef _MSC_VER
	#pragma warning(pop)
#else
	#pragma GCC diagnostic pop
	#undef Long
#endif

#undef IEEE_8087
#undef NO_STRTOD_BIGCOMP
#undef NO_INFNAN_CHECK
#undef strtod
#undef dtoa

using namespace WAST;

//分析可选的+或-符号，如果分析了-符号，则返回true。
//如果解析了+或-符号，则nextchar将被高级处理。
static bool parseSign(const char*& nextChar)
{
	if(*nextChar == '-') { ++nextChar; return true; }
	else if(*nextChar == '+') { ++nextChar; }
	return false;
}

//从十六进制分析无符号整数，从“0x”开始，并将nextchar移过已分析的十六进制。
//调用已被lexer接受为十六进制整数的输入。
static U64 parseHexUnsignedInt(const char*& nextChar,ParseState& state,U64 maxValue)
{
	const char* firstHexit = nextChar;
	WAVM_ASSERT_THROW(nextChar[0] == '0' && (nextChar[1] == 'x' || nextChar[1] == 'X'));
	nextChar += 2;
	
	U64 result = 0;
	U8 hexit = 0;
	while(true)
	{
		if(*nextChar == '_') { ++nextChar; continue; }
		if(!tryParseHexit(nextChar,hexit)) { break; }
		if(result > (maxValue - hexit) / 16)
		{
			parseErrorf(state,firstHexit,"integer literal is too large");
			result = maxValue;
			while(tryParseHexit(nextChar,hexit)) {};
			break;
		}
		WAVM_ASSERT_THROW(result * 16 + hexit >= result);
		result = result * 16 + hexit;
	}
	return result;
}

//从数字中解析无符号整数，将nextchar移过解析的数字。
//假设只对lexer已经接受为十进制整数的输入调用它。
static U64 parseDecimalUnsignedInt(const char*& nextChar,ParseState& state,U64 maxValue,const char* context)
{
	U64 result = 0;
	const char* firstDigit = nextChar;
	while(true)
	{
		if(*nextChar == '_') { ++nextChar; continue; }
		if(*nextChar < '0' || *nextChar > '9') { break; }

		const U8 digit = *nextChar - '0';
		++nextChar;

		if(result > U64(maxValue - digit) / 10)
		{
			parseErrorf(state,firstDigit,"%s is too large",context);
			result = maxValue;
			while((*nextChar >= '0' && *nextChar <= '9') || *nextChar == '_') { ++nextChar; };
			break;
		}
		WAVM_ASSERT_THROW(result * 10 + digit >= result);
		result = result * 10 + digit;
	};
	return result;
}

//解析一个浮点NaN，将nextchar移过解析的字符。
//假设它将只对lexer已经接受的作为文本NaN的输入进行调用。
template<typename Float>
Float parseNaN(const char*& nextChar,ParseState& state)
{
	typedef typename Floats::FloatComponents<Float> FloatComponents;
	FloatComponents resultComponents;
	resultComponents.bits.sign = parseSign(nextChar) ? 1 : 0;
	resultComponents.bits.exponent = FloatComponents::maxExponentBits;

	WAVM_ASSERT_THROW(nextChar[0] == 'n'
	&& nextChar[1] == 'a'
	&& nextChar[2] == 'n');
	nextChar += 3;
	
	if(*nextChar == ':')
	{
		++nextChar;

		const U64 significandBits = parseHexUnsignedInt(nextChar,state,FloatComponents::maxSignificand);
		resultComponents.bits.significand = typename FloatComponents::Bits(significandBits);
	}
	else
	{
//如果未指定NaN的意义，只需设置顶部位。
		resultComponents.bits.significand = typename FloatComponents::Bits(1) << (FloatComponents::numSignificandBits-1);
	}
	
	return resultComponents.value;
}


//解析浮点无穷大。不前进nextchar。
//假设它只对lexer已经接受的输入调用，作为一个字面无穷大。
template<typename Float>
Float parseInfinity(const char* nextChar)
{
//浮点无穷大用零有效位的最大指数表示。
	typedef typename Floats::FloatComponents<Float> FloatComponents;
	FloatComponents resultComponents;
	resultComponents.bits.sign = parseSign(nextChar) ? 1 : 0;
	resultComponents.bits.exponent = FloatComponents::maxExponentBits;
	resultComponents.bits.significand = 0;
	return resultComponents.value;
}

//解析十进制浮点文字，将nextchar移过已解析的字符。
//假设只对lexer已经接受为十进制浮点文字的输入调用它。
template<typename Float>
Float parseFloat(const char*& nextChar,ParseState& state)
{
//扫描令牌的字符以查找下划线，并在不带下划线的情况下复制strtod。
	const char* firstChar = nextChar;
	std::string noUnderscoreString;
	bool hasUnderscores = false;
	while(true)
	{
//确定下一个字符是否仍然是数字的一部分。
		bool isNumericChar = false;
		switch(*nextChar)
		{
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case '+': case '-': case 'x': case 'X': case '.': case 'p': case 'P': case '_':
			isNumericChar = true;
			break;
		};
		if(!isNumericChar) { break; }

		if(*nextChar == '_' && !hasUnderscores)
		{
//如果这是遇到的第一个下划线，请将数字前面的字符复制到std：：string。
			noUnderscoreString = std::string(firstChar,nextChar);
			hasUnderscores = true;
		}
		else if(*nextChar != '_' && hasUnderscores)
		{
//如果以前遇到下划线，请将非下划线字符复制到该字符串。
			noUnderscoreString += *nextChar;
		}

		++nextChar;
	};

//将无下划线字符串传递给parsenonspecialf64，而不是原始输入字符串。
	if(hasUnderscores) { firstChar = noUnderscoreString.c_str(); }

//使用david gay的strtod解析浮点数。
	char* endChar = nullptr;
	F64 f64 = parseNonSpecialF64(firstChar,&endChar);
	if(endChar == firstChar)
	{
		Errors::fatalf("strtod failed to parse number accepted by lexer");
	}
	if(Float(f64) < std::numeric_limits<Float>::lowest() || Float(f64) > std::numeric_limits<Float>::max())
	{
		parseErrorf(state,firstChar,"float literal is too large");
	}

	return (Float)f64;
}

//尝试将数字文本标记解析为整数，前进状态.nexttoken。
//如果与标记匹配，则返回true。
template<typename UnsignedInt>
bool tryParseInt(ParseState& state,UnsignedInt& outUnsignedInt,I64 minSignedValue,U64 maxUnsignedValue)
{
	bool isNegative = false;
	U64 u64 = 0;

	const char* nextChar = state.string + state.nextToken->begin;
	switch(state.nextToken->type)
	{
	case t_decimalInt:
		isNegative = parseSign(nextChar);
		u64 = parseDecimalUnsignedInt(nextChar,state,isNegative ? U64(-minSignedValue) : maxUnsignedValue,"int literal");
		break;
	case t_hexInt:
		isNegative = parseSign(nextChar);
		u64 = parseHexUnsignedInt(nextChar,state,isNegative ? U64(-minSignedValue) : maxUnsignedValue);
		break;
	default:
		return false;
	};

	outUnsignedInt = isNegative ? UnsignedInt(-I64(u64)) : UnsignedInt(u64);
		
	++state.nextToken;
	WAVM_ASSERT_THROW(nextChar <= state.string + state.nextToken->begin);

	return true;
}

//尝试将数字文本标记解析为float，advancing state.nexttoken。
//如果与标记匹配，则返回true。
template<typename Float>
bool tryParseFloat(ParseState& state,Float& outFloat)
{
	const char* nextChar = state.string + state.nextToken->begin;
	switch(state.nextToken->type)
	{
	case t_decimalInt:
	case t_decimalFloat: outFloat = parseFloat<Float>(nextChar,state); break;
	case t_hexInt:
	case t_hexFloat: outFloat = parseFloat<Float>(nextChar,state); break;
	case t_floatNaN: outFloat = parseNaN<Float>(nextChar,state); break;
	case t_floatInf: outFloat = parseInfinity<Float>(nextChar); break;
	default:
		parseErrorf(state,state.nextToken,"expected float literal");
		return false;
	};

	++state.nextToken;
	WAVM_ASSERT_THROW(nextChar <= state.string + state.nextToken->begin);

	return true;
}

namespace WAST
{
	bool tryParseI32(ParseState& state,U32& outI32)
	{
		return tryParseInt<U32>(state,outI32,INT32_MIN,UINT32_MAX);
	}

	bool tryParseI64(ParseState& state,U64& outI64)
	{
		return tryParseInt<U64>(state,outI64,INT64_MIN,UINT64_MAX);
	}
	
	U8 parseI8(ParseState& state)
	{
		U32 result;
		if(!tryParseInt<U32>(state,result,INT8_MIN,UINT8_MAX))
		{
			parseErrorf(state,state.nextToken,"expected i8 literal");
			throw RecoverParseException();
		}
		return U8(result);
	}

	U32 parseI32(ParseState& state)
	{
		U32 result;
		if(!tryParseI32(state,result))
		{
			parseErrorf(state,state.nextToken,"expected i32 literal");
			throw RecoverParseException();
		}
		return result;
	}

	U64 parseI64(ParseState& state)
	{
		U64 result;
		if(!tryParseI64(state,result))
		{
			parseErrorf(state,state.nextToken,"expected i64 literal");
			throw RecoverParseException();
		}
		return result;
	}

	F32 parseF32(ParseState& state)
	{
		F32 result;
		if(!tryParseFloat(state,result))
		{
			parseErrorf(state,state.nextToken,"expected f32 literal");
			throw RecoverParseException();
		}
		return result;
	}

	F64 parseF64(ParseState& state)
	{
		F64 result;
		if(!tryParseFloat(state,result))
		{
			parseErrorf(state,state.nextToken,"expected f64 literal");
			throw RecoverParseException();
		}
		return result;
	}
}