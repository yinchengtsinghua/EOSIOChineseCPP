
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "Inline/BasicTypes.h"
#include "Inline/UTF8.h"
#include "WAST.h"
#include "Lexer.h"
#include "IR/Module.h"
#include "Parse.h"

#include <cstdarg>
#include <cstdio>

#define XXH_FORCE_NATIVE_FORMAT 1
#define XXH_PRIVATE_API
#include "../ThirdParty/xxhash/xxhash.h"

using namespace IR;

namespace WAST
{
	void findClosingParenthesis(ParseState& state,const Token* openingParenthesisToken)
	{
//跳过标记，直到找到关闭当前括号嵌套深度的'）'。
		Uptr depth = 1;
		while(depth > 0)
		{
			switch(state.nextToken->type)
			{
			default:
				++state.nextToken;
				break;
			case t_leftParenthesis:
				++state.nextToken;
				++depth;
				break;
			case t_rightParenthesis:
				++state.nextToken;
				--depth;
				break;
			case t_eof:
				parseErrorf(state,openingParenthesisToken,"reached end of input while trying to find closing parenthesis");
				throw FatalParseException();
			}
		}
	}

	void parseErrorf(ParseState& state,Uptr charOffset,const char* messageFormat,va_list messageArguments)
	{
//格式化消息。
		char messageBuffer[1024];
		int numPrintedChars = std::vsnprintf(messageBuffer,sizeof(messageBuffer),messageFormat,messageArguments);
		if(numPrintedChars >= 1023 || numPrintedChars < 0) { Errors::unreachable(); }
		messageBuffer[numPrintedChars] = 0;

//将错误添加到状态的错误列表中。
		state.errors.emplace_back(charOffset,messageBuffer);
	}
	void parseErrorf(ParseState& state,Uptr charOffset,const char* messageFormat,...)
	{
		va_list messageArguments;
		va_start(messageArguments,messageFormat);
		parseErrorf(state,charOffset,messageFormat,messageArguments);
		va_end(messageArguments);
	}
	void parseErrorf(ParseState& state,const char* nextChar,const char* messageFormat,...)
	{
		va_list messageArguments;
		va_start(messageArguments,messageFormat);
		parseErrorf(state,nextChar - state.string,messageFormat,messageArguments);
		va_end(messageArguments);
	}
	void parseErrorf(ParseState& state,const Token* nextToken,const char* messageFormat,...)
	{
		va_list messageArguments;
		va_start(messageArguments,messageFormat);
		parseErrorf(state,nextToken->begin,messageFormat,messageArguments);
		va_end(messageArguments);
	}

	void require(ParseState& state,TokenType type)
	{
		if(state.nextToken->type != type)
		{
			parseErrorf(state,state.nextToken,"expected %s",describeToken(type));
			throw RecoverParseException();
		}
		++state.nextToken;
	}
	
	bool tryParseValueType(ParseState& state,ValueType& outValueType)
	{
		switch(state.nextToken->type)
		{
		case t_i32: ++state.nextToken; outValueType = ValueType::i32; return true;
		case t_i64: ++state.nextToken; outValueType = ValueType::i64; return true;
		case t_f32: ++state.nextToken; outValueType = ValueType::f32; return true;
		case t_f64: ++state.nextToken; outValueType = ValueType::f64; return true;
		#if ENABLE_SIMD_PROTOTYPE
		case t_v128: ++state.nextToken; outValueType = ValueType::v128; return true;
		#endif
		default:
			outValueType = ValueType::any;
			return false;
		};
	}
	
	bool tryParseResultType(ParseState& state,ResultType& outResultType)
	{
		return tryParseValueType(state,*(ValueType*)&outResultType);
	}

	ValueType parseValueType(ParseState& state)
	{
		ValueType result;
		if(!tryParseValueType(state,result))
		{
			parseErrorf(state,state.nextToken,"expected value type");
			throw RecoverParseException();
		}
		return result;
	}

	const FunctionType* parseFunctionType(ModuleParseState& state,NameToIndexMap& outLocalNameToIndexMap,std::vector<std::string>& outLocalDisassemblyNames)
	{
		std::vector<ValueType> parameters;
		ResultType ret = ResultType::none;

//分析函数参数。
		while(tryParseParenthesizedTagged(state,t_param,[&]
		{
			Name parameterName;
			if(tryParseName(state,parameterName))
			{
//（参数<名称><类型>）
				bindName(state,outLocalNameToIndexMap,parameterName,parameters.size());
				parameters.push_back(parseValueType(state));
				outLocalDisassemblyNames.push_back(parameterName.getString());
			}
			else
			{
//（PARAM <类型> *）
				ValueType parameterType;
				while(tryParseValueType(state,parameterType))
				{
					parameters.push_back(parameterType);
					outLocalDisassemblyNames.push_back(std::string());
				};
			}
		}));

//分析<=1结果类型：（result<value type>>）*
		while(state.nextToken[0].type == t_leftParenthesis
		&& state.nextToken[1].type == t_result)
		{
			parseParenthesized(state,[&]
			{
				require(state,t_result);

				ResultType resultElementType;
				const Token* elementToken = state.nextToken;
				while(tryParseResultType(state,resultElementType))
				{
					if(ret != ResultType::none) { parseErrorf(state,elementToken,"function type cannot have more than 1 result element"); }
					ret = resultElementType;
				};
			});
		};

		return FunctionType::get(ret,parameters);
	}
	
	UnresolvedFunctionType parseFunctionTypeRefAndOrDecl(ModuleParseState& state,NameToIndexMap& outLocalNameToIndexMap,std::vector<std::string>& outLocalDisassemblyNames)
	{
//分析可选的函数类型引用。
		Reference functionTypeRef;
		if(state.nextToken[0].type == t_leftParenthesis
		&& state.nextToken[1].type == t_type)
		{
			parseParenthesized(state,[&]
			{
				require(state,t_type);
				if(!tryParseNameOrIndexRef(state,functionTypeRef))
				{
					parseErrorf(state,state.nextToken,"expected type name or index");
					throw RecoverParseException();
				}
			});
		}

//解析显式函数参数和结果类型。
		const FunctionType* explicitFunctionType = parseFunctionType(state,outLocalNameToIndexMap,outLocalDisassemblyNames);

		UnresolvedFunctionType result;
		result.reference = functionTypeRef;
		result.explicitType = explicitFunctionType;
		return result;
	}

	IndexedFunctionType resolveFunctionType(ModuleParseState& state,const UnresolvedFunctionType& unresolvedType)
	{
		if(!unresolvedType.reference)
		{
			return getUniqueFunctionTypeIndex(state,unresolvedType.explicitType);
		}
		else
		{
//解析引用的类型。
			const U32 referencedFunctionTypeIndex = resolveRef(state,state.typeNameToIndexMap,state.module.types.size(),unresolvedType.reference);

//验证如果函数定义同时具有类型引用和显式参数/结果类型声明，则它们匹配。
			const bool hasExplicitParametersOrResultType = unresolvedType.explicitType != FunctionType::get();
			if(hasExplicitParametersOrResultType)
			{
				if(referencedFunctionTypeIndex != UINT32_MAX
				&& state.module.types[referencedFunctionTypeIndex] != unresolvedType.explicitType)
				{
					parseErrorf(state,unresolvedType.reference.token,"referenced function type (%s) does not match declared parameters and results (%s)",
						asString(state.module.types[referencedFunctionTypeIndex]).c_str(),
						asString(unresolvedType.explicitType).c_str()
						);
				}
			}

			return {referencedFunctionTypeIndex};
		}
	}

	IndexedFunctionType getUniqueFunctionTypeIndex(ModuleParseState& state,const FunctionType* functionType)
	{
//如果此类型还不在模块的类型表中，请添加它。
		auto functionTypeToIndexMapIt = state.functionTypeToIndexMap.find(functionType);
		if(functionTypeToIndexMapIt != state.functionTypeToIndexMap.end())
		{
			return IndexedFunctionType {functionTypeToIndexMapIt->second};
		}
		else
		{
			const Uptr functionTypeIndex = state.module.types.size();
			state.module.types.push_back(functionType);
			state.disassemblyNames.types.push_back(std::string());
			errorUnless(functionTypeIndex < UINT32_MAX);
			state.functionTypeToIndexMap.emplace(functionType,(U32)functionTypeIndex);
			return IndexedFunctionType {(U32)functionTypeIndex};
		}
	}

	U32 Name::calcHash(const char* begin,U32 numChars)
	{
//使用XXHASH32散列名称。xxash64理论上对于64位机器上的长字符串来说更快，
//但我发现对于典型的名称长度来说，它没有更快的速度。
		return XXH32(begin,numChars,0);
	}

	bool tryParseName(ParseState& state,Name& outName)
	{
		if(state.nextToken->type != t_name) { return false; }

//查找第一个非名称字符。
		const char* firstChar = state.string + state.nextToken->begin;;
		const char* nextChar = firstChar;
		WAVM_ASSERT_THROW(*nextChar == '$');
		++nextChar;
		while(true)
		{
			const char c = *nextChar;
			if((c >= 'a' && c <= 'z')
			|| (c >= 'A' && c <= 'Z')
			|| (c >= '0' && c <= '9')
			|| c=='_' || c=='\'' || c=='+' || c=='-' || c=='*' || c=='/' || c=='\\' || c=='^' || c=='~' || c=='='
			|| c=='<' || c=='>' || c=='!' || c=='?' || c=='@' || c=='#' || c=='$' || c=='%' || c=='&' || c=='|'
			|| c==':' || c=='`' || c=='.')
			{
				++nextChar;
			}
			else
			{
				break;
			}
		};

		WAVM_ASSERT_THROW(U32(nextChar - state.string) > state.nextToken->begin + 1);
		++state.nextToken;
		WAVM_ASSERT_THROW(U32(nextChar - state.string) <= state.nextToken->begin);
		WAVM_ASSERT_THROW(U32(nextChar - firstChar) <= UINT32_MAX);
		outName = Name(firstChar,U32(nextChar - firstChar));
		return true;
	}

	bool tryParseNameOrIndexRef(ParseState& state,Reference& outRef)
	{
		outRef.token = state.nextToken;
		if(tryParseName(state,outRef.name)) { outRef.type = Reference::Type::name; return true; }
		else if(tryParseI32(state,outRef.index)) { outRef.type = Reference::Type::index; return true; }
		return false;
	}

	U32 parseAndResolveNameOrIndexRef(ParseState& state,const NameToIndexMap& nameToIndexMap,Uptr maxIndex,const char* context)
	{
		Reference ref;

if(strcmp(context, "type") == 0     //严格限制此块调用\u间接
		&& state.nextToken[0].type == t_leftParenthesis
		&& state.nextToken[1].type == t_type)
		{
			parseParenthesized(state,[&]
			{
				require(state,t_type);
				if(!tryParseNameOrIndexRef(state,ref))
				{
					parseErrorf(state,state.nextToken,"expected type name or index");
					throw RecoverParseException();
				}
			});
		}

		if(ref.type == Reference::Type::invalid && !tryParseNameOrIndexRef(state,ref))
		{
			parseErrorf(state,state.nextToken,"expected %s name or index",context);
			throw RecoverParseException();
		}
		return resolveRef(state,nameToIndexMap,maxIndex,ref);
	}

	void bindName(ParseState& state,NameToIndexMap& nameToIndexMap,const Name& name,Uptr index)
	{
		errorUnless(index <= UINT32_MAX);

		if(name)
		{
			auto mapIt = nameToIndexMap.find(name);
			if(mapIt != nameToIndexMap.end())
			{
				const TextFileLocus previousDefinitionLocus = calcLocusFromOffset(state.string,	state.lineInfo,mapIt->first.getCharOffset(state.string));
				parseErrorf(state,name.getCharOffset(state.string),"redefinition of name defined at %s",previousDefinitionLocus.describe().c_str());
			}
			nameToIndexMap.emplace(name,U32(index));
		}
	}

	U32 resolveRef(ParseState& state,const NameToIndexMap& nameToIndexMap,Uptr maxIndex,const Reference& ref)
	{
		switch(ref.type)
		{
		case Reference::Type::index:
		{
			if(ref.index >= maxIndex)
			{
				parseErrorf(state,ref.token,"invalid index");
				return UINT32_MAX;
			}
			return ref.index;
		}
		case Reference::Type::name:
		{
			auto nameToIndexMapIt = nameToIndexMap.find(ref.name);
			if(nameToIndexMapIt == nameToIndexMap.end())
			{
				parseErrorf(state,ref.token,"unknown name");
				return UINT32_MAX;
			}
			else
			{
				return nameToIndexMapIt->second;
			}
		}
		default: Errors::unreachable();
		};
	}
	
	bool tryParseHexit(const char*& nextChar,U8& outValue)
	{
		if(*nextChar >= '0' && *nextChar <= '9') { outValue = *nextChar - '0'; }
		else if(*nextChar >= 'a' && *nextChar <= 'f') { outValue = *nextChar - 'a' + 10; }
		else if(*nextChar >= 'A' && *nextChar <= 'F') { outValue = *nextChar - 'A' + 10; }
		else
		{
			outValue = 0;
			return false;
		}
		++nextChar;
		return true;
	}
	
	static void parseCharEscapeCode(const char*& nextChar,ParseState& state,std::string& outString)
	{
		U8 firstNibble;
		if(tryParseHexit(nextChar,firstNibble))
		{
//从两个十六进制中解析一个8位文本。
			U8 secondNibble;
			if(!tryParseHexit(nextChar,secondNibble)) { parseErrorf(state,nextChar,"expected hexit"); }
			outString += char(firstNibble * 16 + secondNibble);
		}
		else
		{
			switch(*nextChar)
			{
			case 't':	outString += '\t'; ++nextChar; break;
			case 'n':	outString += '\n'; ++nextChar; break;
			case 'r':	outString += '\r'; ++nextChar; break;
			case '\"':	outString += '\"'; ++nextChar; break;
			case '\'':	outString += '\''; ++nextChar; break;
			case '\\':	outString += '\\'; ++nextChar; break;
			case 'u':
			{
//\u…-十六进制数的Unicode码位
				if(nextChar[1] != '{') { parseErrorf(state,nextChar,"expected '{'"); }
				nextChar += 2;
				
//分析十六进制数。
				const char* firstHexit = nextChar;
				U32 codepoint = 0;
				U8 hexit = 0;
				while(tryParseHexit(nextChar,hexit))
				{
					if(codepoint > (UINT32_MAX - hexit) / 16)
					{
						codepoint = UINT32_MAX;
						while(tryParseHexit(nextChar,hexit)) {};
						break;
					}
					WAVM_ASSERT_THROW(codepoint * 16 + hexit >= codepoint);
					codepoint = codepoint * 16 + hexit;
				}

//检查它是否表示有效的Unicode码位。
				if((codepoint >= 0xD800 && codepoint <= 0xDFFF)
				|| codepoint >= 0x110000)
				{
					parseErrorf(state,firstHexit,"invalid Unicode codepoint");
					codepoint = 0x1F642;
				}

//将代码点编码为UTF-8。
				UTF8::encodeCodepoint(codepoint,outString);

				if(*nextChar != '}') { parseErrorf(state,nextChar,"expected '}'"); }
				++nextChar;
				break;
			}
			default:
				outString += '\\';
				++nextChar;
				parseErrorf(state,nextChar,"invalid escape code");
				break;
			}
		}
	}

	bool tryParseString(ParseState& state,std::string& outString)
	{
		if(state.nextToken->type != t_string)
		{
			return false;
		}

//分析字符串文字；lexer已拒绝未终止的字符串，
//所以这只需要复制字符并计算转义码。
		const char* nextChar = state.string + state.nextToken->begin;
		WAVM_ASSERT_THROW(*nextChar == '\"');
		++nextChar;
		while(true)
		{
			switch(*nextChar)
			{
			case '\\':
			{
				++nextChar;
				parseCharEscapeCode(nextChar,state,outString);
				break;
			}
			case '\"':
				++state.nextToken;
				WAVM_ASSERT_THROW(state.string + state.nextToken->begin > nextChar);
				return true;
			default:
				outString += *nextChar++;
				break;
			};
		};
	}

	std::string parseUTF8String(ParseState& state)
	{
		const Token* stringToken = state.nextToken;
		std::string result;
		if(!tryParseString(state,result))
		{
			parseErrorf(state,stringToken,"expected string literal");
			throw RecoverParseException();
		}

//检查字符串是否为有效的UTF-8编码。
		const U8* endChar = (const U8*)result.data() + result.size();
		const U8* nextChar = UTF8::validateString((const U8*)result.data(),endChar);
		if(nextChar != endChar)
		{
			const Uptr charOffset = stringToken->begin + (nextChar - (const U8*)result.data()) + 1;
			parseErrorf(state,charOffset,"invalid UTF-8 encoding");
		}

		return result;
	}
}