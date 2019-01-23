
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
#include "Lexer.h"
#include "IR/Types.h"
#include "IR/Module.h"

#include <functional>
#include <map>
#include <unordered_map>

namespace WAST
{
	struct FatalParseException {};
	struct RecoverParseException {};

//与wast:：error类似，但输入字符串中只有偏移量，而不是完整的textfile轨迹。
	struct UnresolvedError
	{
		Uptr charOffset;
		std::string message;
		UnresolvedError(Uptr inCharOffset,std::string&& inMessage)
		: charOffset(inCharOffset), message(inMessage) {}
	};

//通过各种解析器进行线程化的状态。
	struct ParseState
	{
		const char* string;
		const LineInfo* lineInfo;
		std::vector<UnresolvedError>& errors;
		const Token* nextToken;
		
		ParseState(const char* inString,const LineInfo* inLineInfo,std::vector<UnresolvedError>& inErrors,const Token* inNextToken)
		: string(inString)
		, lineInfo(inLineInfo)
		, errors(inErrors)
		, nextToken(inNextToken)
		{}
	};

//封装从WAST文件解析的名称（$whatever）。
//引用从中解析名称的输入字符串中的字符，
//因此，它可以在固定大小的结构中处理任意长度的名称，但只能
//只要输入字符串没有释放。
//包括名称字符的哈希。
	struct Name
	{
		Name(): begin(nullptr), numChars(0), hash(0) {}
		Name(const char* inBegin,U32 inNumChars)
		: begin(inBegin)
		, numChars(inNumChars)
		, hash(calcHash(inBegin,inNumChars))
		{}

		operator bool() const { return begin != nullptr; }
		std::string getString() const { return begin ? std::string(begin + 1,numChars - 1) : std::string(); }
		Uptr getCharOffset(const char* string) const { return begin - string; }

		friend bool operator==(const Name& a,const Name& b)
		{
			return a.hash == b.hash && a.numChars == b.numChars && memcmp(a.begin,b.begin,a.numChars) == 0;
		}
		friend bool operator!=(const Name& a,const Name& b)
		{
			return !(a == b);
		}
	
		struct Hasher
		{
			Uptr operator()(const Name& name) const
			{
				return name.hash;
			}
		};

	private:

		const char* begin;
		U32 numChars;
		U32 hash;

		static U32 calcHash(const char* begin,U32 numChars);
	};
	
//使用哈希表从名称到索引的映射。
	typedef std::unordered_map<Name,U32,Name::Hasher> NameToIndexMap;
	
//表示尚未解析的引用，解析为名称或索引。
	struct Reference
	{
		enum class Type { invalid, name, index };
		Type type;
		union
		{
			Name name;
			U32 index;
		};
		const Token* token;
		Reference(const Name& inName): type(Type::name), name(inName) {}
		Reference(U32 inIndex): type(Type::index), index(inIndex) {}
		Reference(): type(Type::invalid), token(nullptr) {}
		operator bool() const { return type != Type::invalid; }
	};

//表示函数类型，可以是未解析的名称/索引，也可以是显式类型，或者两者都表示。
	struct UnresolvedFunctionType
	{
		Reference reference;
		const IR::FunctionType* explicitType;
	};

//与分析模块关联的状态。
	struct ModuleParseState : ParseState
	{
		IR::Module& module;

		std::map<const IR::FunctionType*,U32> functionTypeToIndexMap;
		NameToIndexMap typeNameToIndexMap;

		NameToIndexMap functionNameToIndexMap;
		NameToIndexMap tableNameToIndexMap;
		NameToIndexMap memoryNameToIndexMap;
		NameToIndexMap globalNameToIndexMap;

		IR::DisassemblyNames disassemblyNames;

//在分析所有类型之后调用的thunk。
		std::vector<std::function<void(ModuleParseState&)>> postTypeCallbacks;

//在分析所有声明后调用的thunk。
		std::vector<std::function<void(ModuleParseState&)>> postDeclarationCallbacks;

		ModuleParseState(const char* inString,const LineInfo* inLineInfo,std::vector<UnresolvedError>& inErrors,const Token* inNextToken,IR::Module& inModule)
		: ParseState(inString,inLineInfo,inErrors,inNextToken)
		, module(inModule)
		{}
	};
	
//错误处理。
	void parseErrorf(ParseState& state,Uptr charOffset,const char* messageFormat,...);
	void parseErrorf(ParseState& state,const char* nextChar,const char* messageFormat,...);
	void parseErrorf(ParseState& state,const Token* nextToken,const char* messageFormat,...);

	void require(ParseState& state,TokenType type);

//类型分析和统一
	bool tryParseValueType(ParseState& state,IR::ValueType& outValueType);
	bool tryParseResultType(ParseState& state,IR::ResultType& outResultType);
	IR::ValueType parseValueType(ParseState& state);

	const IR::FunctionType* parseFunctionType(ModuleParseState& state,NameToIndexMap& outLocalNameToIndexMap,std::vector<std::string>& outLocalDisassemblyNames);
	UnresolvedFunctionType parseFunctionTypeRefAndOrDecl(ModuleParseState& state,NameToIndexMap& outLocalNameToIndexMap,std::vector<std::string>& outLocalDisassemblyNames);
	IR::IndexedFunctionType resolveFunctionType(ModuleParseState& state,const UnresolvedFunctionType& unresolvedType);
	IR::IndexedFunctionType getUniqueFunctionTypeIndex(ModuleParseState& state,const IR::FunctionType* functionType);

//文字分析。
	bool tryParseHexit(const char*& nextChar,U8& outValue);
	
	bool tryParseI32(ParseState& state,U32& outI32);
	bool tryParseI64(ParseState& state,U64& outI64);

	U8 parseI8(ParseState& state);
	U32 parseI32(ParseState& state);
	U64 parseI64(ParseState& state);
	F32 parseF32(ParseState& state);
	F64 parseF64(ParseState& state);

	bool tryParseString(ParseState& state,std::string& outString);

	std::string parseUTF8String(ParseState& state);

//名称解析和解析。
	bool tryParseName(ParseState& state,Name& outName);
	bool tryParseNameOrIndexRef(ParseState& state,Reference& outRef);
	U32 parseAndResolveNameOrIndexRef(ParseState& state,const NameToIndexMap& nameToIndexMap,Uptr maxIndex,const char* context);

	void bindName(ParseState& state,NameToIndexMap& nameToIndexMap,const Name& name,Uptr index);
	U32 resolveRef(ParseState& state,const NameToIndexMap& nameToIndexMap,Uptr maxIndex,const Reference& ref);

//查找结束当前s表达式的括号。
	void findClosingParenthesis(ParseState& state,const Token* openingParenthesisToken);

//为内部分析器解析周围的括号，并在右括号处处理恢复。
	template<typename ParseInner>
	static void parseParenthesized(ParseState& state,ParseInner parseInner)
	{
		const Token* openingParenthesisToken = state.nextToken;
		require(state,t_leftParenthesis);
		try
		{
			parseInner();
			require(state,t_rightParenthesis);
		}
		catch(RecoverParseException)
		{
			findClosingParenthesis(state,openingParenthesisToken);
		}
	}

//尝试分析“（”tagtype parseinner“）”，处理右括号处的恢复。
//如果使用了任何令牌，则返回true。
	template<typename ParseInner>
	static bool tryParseParenthesizedTagged(ParseState& state,TokenType tagType,ParseInner parseInner)
	{
		const Token* openingParenthesisToken = state.nextToken;
		if(state.nextToken[0].type != t_leftParenthesis || state.nextToken[1].type != tagType)
		{
			return false;
		}
		try
		{
			state.nextToken += 2;
			parseInner();
			require(state,t_rightParenthesis);
		}
		catch(RecoverParseException)
		{
			findClosingParenthesis(state,openingParenthesisToken);
		}
		return true;
	}

//函数分析。
	IR::FunctionDef parseFunctionDef(ModuleParseState& state,const Token* funcToken);

//模块解析。
	void parseModuleBody(ModuleParseState& state);
};