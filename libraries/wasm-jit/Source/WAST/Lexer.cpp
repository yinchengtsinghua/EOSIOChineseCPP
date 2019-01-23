
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "Inline/BasicTypes.h"
#include "Inline/Timing.h"
#include "Inline/Errors.h"
#include "NFA.h"
#include "Regexp.h"
#include "WAST.h"
#include "Lexer.h"
#include "IR/Operators.h"

#include <fstream>

namespace WAST
{
	const char* describeToken(TokenType tokenType)
	{
		WAVM_ASSERT_THROW(tokenType < numTokenTypes);
		static const char* tokenDescriptions[] =
		{
//此枚举标记必须位于重新定义访问运算符标记的LiteralTokenPairs定义之前。
			#define VISIT_TOKEN(name,description) description,
			ENUM_TOKENS()
			#undef VISIT_TOKEN
		};
		return tokenDescriptions[tokenType];
	}
	
	struct StaticData
	{
		NFA::Machine nfaMachine;
		StaticData();
	};

	static NFA::StateIndex createTokenSeparatorPeekState(NFA::Builder* builder,NFA::StateIndex finalState)
	{
		NFA::CharSet tokenSeparatorCharSet;
		tokenSeparatorCharSet.add(U8(' '));
		tokenSeparatorCharSet.add(U8('\t'));
		tokenSeparatorCharSet.add(U8('\r'));
		tokenSeparatorCharSet.add(U8('\n'));
		tokenSeparatorCharSet.add(U8('='));
		tokenSeparatorCharSet.add(U8('('));
		tokenSeparatorCharSet.add(U8(')'));
		tokenSeparatorCharSet.add(U8(';'));
		tokenSeparatorCharSet.add(0);
		auto separatorState = addState(builder);
		NFA::addEdge(builder,separatorState,tokenSeparatorCharSet,finalState | NFA::edgeDoesntConsumeInputFlag);
		return separatorState;
	}
	
	static void addLiteralToNFA(const char* string,NFA::Builder* builder,NFA::StateIndex initialState,NFA::StateIndex finalState)
	{
//将文本添加到NFA中，一次添加一个字符，重用可由同一字符串访问的现有状态。
		for(const char* nextChar = string;*nextChar;++nextChar)
		{
			NFA::StateIndex nextState = NFA::getNonTerminalEdge(builder,initialState,*nextChar);
			if(nextState < 0 || nextChar[1] == 0)
			{
				nextState = nextChar[1] == 0 ? finalState : addState(builder);
				NFA::addEdge(builder,initialState,NFA::CharSet(*nextChar),nextState);
			}
			initialState = nextState;
		}
	}

	StaticData::StaticData()
	{
		static const std::pair<TokenType,const char*> regexpTokenPairs[] =
		{
			{t_decimalInt,"[+\\-]?\\d+(_\\d+)*"},
			{t_decimalFloat,"[+\\-]?\\d+(_\\d+)*\\.(\\d+(_\\d+)*)*([eE][+\\-]?\\d+(_\\d+)*)?"},
			{t_decimalFloat,"[+\\-]?\\d+(_\\d+)*[eE][+\\-]?\\d+(_\\d+)*"},

			{t_hexInt,"[+\\-]?0[xX][\\da-fA-F]+(_[\\da-fA-F]+)*"},
			{t_hexFloat,"[+\\-]?0[xX][\\da-fA-F]+(_[\\da-fA-F]+)*\\.([\\da-fA-F]+(_[\\da-fA-F]+)*)*([pP][+\\-]?\\d+(_\\d+)*)?"},
			{t_hexFloat,"[+\\-]?0[xX][\\da-fA-F]+(_[\\da-fA-F]+)*[pP][+\\-]?\\d+(_\\d+)*"},

			{t_floatNaN,"[+\\-]?nan(:0[xX][\\da-fA-F]+(_[\\da-fA-F]+)*)?"},
			{t_floatInf,"[+\\-]?inf"},

			{t_string,"\"([^\"\n\\\\]*(\\\\([^0-9a-fA-Fu]|[0-9a-fA-F][0-9a-fA-F]|u\\{[0-9a-fA-F]+})))*\""},

			{t_name,"\\$[a-zA-Z0-9\'_+*/~=<>!?@#$%&|:`.\\-\\^\\\\]+"},
		};

		static const std::tuple<TokenType,const char*,bool> literalTokenTuples[] =
		{
			std::make_tuple(t_leftParenthesis,"(",true),
			std::make_tuple(t_rightParenthesis,")",true),
			std::make_tuple(t_equals,"=",true),

			#define VISIT_TOKEN(name,description) std::make_tuple(t_##name,#name,false),
			ENUM_LITERAL_TOKENS()
			#undef VISIT_TOKEN

			#undef VISIT_OPERATOR_TOKEN
			#define VISIT_OPERATOR_TOKEN(_,name,nameString,...) std::make_tuple(t_##name,nameString,false),
			ENUM_OPERATORS(VISIT_OPERATOR_TOKEN)
			#undef VISIT_OPERATOR_TOKEN
		};
	
		Timing::Timer timer;

		NFA::Builder* nfaBuilder = NFA::createBuilder();
		
		for(auto regexpTokenPair : regexpTokenPairs)
		{
			NFA::StateIndex finalState = NFA::maximumTerminalStateIndex - (NFA::StateIndex)regexpTokenPair.first;
			finalState = createTokenSeparatorPeekState(nfaBuilder,finalState);
			Regexp::addToNFA(regexpTokenPair.second,nfaBuilder,0,finalState);
		}
		
		for(auto literalTokenTuple : literalTokenTuples)
		{
			const TokenType tokenType = std::get<0>(literalTokenTuple);
			const char* literalString = std::get<1>(literalTokenTuple);
			const bool isTokenSeparator = std::get<2>(literalTokenTuple);

			NFA::StateIndex finalState = NFA::maximumTerminalStateIndex - (NFA::StateIndex)tokenType;
			if(!isTokenSeparator) { finalState = createTokenSeparatorPeekState(nfaBuilder,finalState); }

			addLiteralToNFA(literalString,nfaBuilder,0,finalState);
		}

		#ifndef _DEBUG
		if(false)
		#endif
		{
			std::ofstream debugGraphStream("nfaGraph.dot");
			debugGraphStream << NFA::dumpNFAGraphViz(nfaBuilder).c_str();
			debugGraphStream.close();
		}

		nfaMachine = NFA::Machine(nfaBuilder);

		#ifndef _DEBUG
		if(false)
		#endif
		{
			std::ofstream debugGraphStream("dfaGraph.dot");
			debugGraphStream << nfaMachine.dumpDFAGraphViz().c_str();
			debugGraphStream.close();
		}

		Timing::logTimer("built lexer tables",timer);
	}

	struct LineInfo
	{
		U32* lineStarts;
		U32 numLineStarts;
	};

	inline bool isRecoveryPointChar(char c)
	{
		switch(c)
		{
//在下一个空格或括号处恢复词法分析。
		case ' ': case '\t': case '\r': case '\n': case '\f':
		case '(': case ')':
			return true;
		default:
			return false;
		};
	}

	Token* lex(const char* string,Uptr stringLength,LineInfo*& outLineInfo)
	{
		static StaticData staticData;
		
		Timing::Timer timer;

		if(stringLength > UINT32_MAX)
		{
			Errors::fatalf("cannot lex strings with more than %u characters",UINT32_MAX);
		}
		
//在前面为令牌分配足够的内存，并为输入字符串中的每个字符分配换行符。
		Token* tokens = (Token*)malloc(sizeof(Token) * (stringLength + 1));
		U32* lineStarts = (U32*)malloc(sizeof(U32) * (stringLength + 2));

		Token* nextToken = tokens;
		U32* nextLineStart = lineStarts;
		*nextLineStart++ = 0;

		const char* nextChar = string;
		while(true)
		{
//跳过空白和注释（跟踪换行符）。
			while(true)
			{
				switch(*nextChar)
				{
//单行注释。
				case ';':
					if(nextChar[1] != ';') { goto doneSkippingWhitespace; }
					else
					{
						nextChar += 2;
						while(*nextChar)
						{
							if(*nextChar == '\n')
							{
//发出换行符的行首。
								*nextLineStart++ = U32(nextChar - string + 1);
								++nextChar;
								break;
							}
							++nextChar;
						};
					}
					break;
//分隔（可能是多行）注释。
				case '(':
					if(nextChar[1] != ';') { goto doneSkippingWhitespace; }
					else
					{
						const char* firstCommentChar = nextChar;
						nextChar += 2;
						U32 commentDepth = 1;
						while(commentDepth)
						{
							if(nextChar[0] == ';' && nextChar[1] == ')')
							{
								--commentDepth;
								nextChar += 2;
							}
							else if(nextChar[0] == '(' && nextChar[1] == ';')
							{
								++commentDepth;
								nextChar += 2;
							}
							else if(nextChar == string + stringLength)
							{
//发出未终止的注释标记。
								nextToken->type = t_unterminatedComment;
								nextToken->begin = U32(firstCommentChar - string);
								++nextToken;
								goto doneSkippingWhitespace;
							}
							else
							{
								if(*nextChar == '\n')
								{
//发出换行符的行首。
									*nextLineStart++ = U32(nextChar - string);
								}
								++nextChar;
							}
						};
					}
					break;
//Whitespace。
				case '\n':
					*nextLineStart++ = U32(nextChar - string + 1);
					++nextChar;
					break;
				case ' ': case '\t': case '\r': case '\f':
					++nextChar;
					break;
				default:
					goto doneSkippingWhitespace;
				};
			}
			doneSkippingWhitespace:

//一旦我们到达一个非空白、非注释字符，就将字符输入NFA，直到它到达一个终端状态。
			nextToken->begin = U32(nextChar - string);
			NFA::StateIndex terminalState = staticData.nfaMachine.feed(nextChar);
			if(terminalState != NFA::unmatchedCharacterTerminal)
			{
				nextToken->type = TokenType(NFA::maximumTerminalStateIndex - (NFA::StateIndex)terminalState);
				++nextToken;
			}
			else
			{
				if(nextToken->begin < stringLength)
				{
//发出无法识别的令牌。
					nextToken->type = t_unrecognized;
					++nextToken;

//前进到恢复点。
					while(!isRecoveryPointChar(*nextChar)) { ++nextChar; }
				}
				else
				{
					break;
				}
			}
		}

//发出结束令牌以标记令牌流的结束。
		nextToken->type = t_eof;
		++nextToken;
		
//为文件结尾发出一个额外的行首，这样您就可以用linestarts[行+1]来查找行尾。
		*nextLineStart++ = U32(nextChar - string) + 1;

//将行首和标记数组缩小到标记/行的最终数目。
		const Uptr numLineStarts = nextLineStart - lineStarts;
		const Uptr numTokens = nextToken - tokens;
		lineStarts = (U32*)realloc(lineStarts,sizeof(U32) * numLineStarts);
		tokens = (Token*)realloc(tokens,sizeof(Token) * numTokens);

//创建封装行开始信息的LineInfo对象。
		outLineInfo = new LineInfo {lineStarts,U32(numLineStarts)};

		Timing::logRatePerSecond("lexed WAST file",timer,stringLength/1024.0/1024.0,"MB");
		Log::printf(Log::Category::metrics,"lexer produced %u tokens (%.1fMB)\n",numTokens,numTokens*sizeof(Token)/1024.0/1024.0);

		return tokens;
	}
	
	void freeTokens(Token* tokens)
	{
		free(tokens);
	}

	void freeLineInfo(LineInfo* lineInfo)
	{
		free(lineInfo->lineStarts);
		delete lineInfo;
	}
	
	static Uptr getLineOffset(const LineInfo* lineInfo,Uptr lineIndex)
	{
		errorUnless(lineIndex < lineInfo->numLineStarts);
		return lineInfo->lineStarts[lineIndex];
	}

	TextFileLocus calcLocusFromOffset(const char* string,const LineInfo* lineInfo,Uptr charOffset)
	{
//binary search该行从charindex前面的最后一行开始。
		Uptr minLineIndex = 0;
		Uptr maxLineIndex = lineInfo->numLineStarts - 1;
		while(maxLineIndex > minLineIndex)
		{
			const Uptr medianLineIndex = (minLineIndex + maxLineIndex + 1) / 2;
			if(charOffset < lineInfo->lineStarts[medianLineIndex])
			{
				maxLineIndex = medianLineIndex - 1;
			}
			else if(charOffset > lineInfo->lineStarts[medianLineIndex])
			{
				minLineIndex = medianLineIndex;
			}
			else
			{
				minLineIndex = maxLineIndex = medianLineIndex;
			}
		};
		TextFileLocus result;
		result.newlines = (U32)minLineIndex;

//计算从行首到charindex的制表符和空格。
		for(U32 index = lineInfo->lineStarts[result.newlines];index < charOffset;++index)
		{
			if(string[index] == '\t') { ++result.tabs; }
			else { ++result.characters; }
		}

//将完整的源代码行复制到上下文的textfilerocece中。
		const Uptr lineStartOffset = getLineOffset(lineInfo,result.newlines);
		Uptr lineEndOffset = getLineOffset(lineInfo,result.newlines+1) - 1;
		result.sourceLine = std::string(string + lineStartOffset,lineEndOffset - lineStartOffset);

		return result;
	}
}