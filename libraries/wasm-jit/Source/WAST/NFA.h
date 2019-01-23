
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "Inline/BasicTypes.h"
#include "Inline/DenseStaticIntSet.h"

#include <string>

namespace NFA
{
//一组字符。
	typedef DenseStaticIntSet<U8,256> CharSet;

//DFA状态的索引。负索引表示“接受”或终端状态。
	typedef I16 StateIndex;

	enum
	{
edgeDoesntConsumeInputFlag = (StateIndex)0x4000, //在不消耗任何输入的终端DFA状态转换上设置的标志。
unmatchedCharacterTerminal = (StateIndex)0x8000, //表示DFA不识别输入的隐式终端状态。
maximumTerminalStateIndex = (StateIndex)0xbfff, //应为未设置EdgedoesNTConsumerInputFlag的最大负数。
	};

//创建一个抽象对象，该对象保存正在构造的BFA的状态。
	struct Builder;
	Builder* createBuilder();

//向正在构造的DFA添加状态和边的函数。
	StateIndex addState(Builder* builder);
	void addEdge(Builder* builder,StateIndex initialState,const CharSet& predicate,StateIndex nextState);
	void addEpsilonEdge(Builder* builder,StateIndex initialState,StateIndex nextState);
	StateIndex getNonTerminalEdge(Builder* builder,StateIndex initialState,char c);

//将NFA的状态和边缘转储到graphviz.dot格式。
	std::string dumpNFAGraphViz(const Builder* builder);

//封装已转换为可有效执行的DFA的NFA。
	struct Machine
	{
		Machine(): stateAndOffsetToNextStateMap(nullptr), numClasses(0), numStates(0) {}
		~Machine();
		
		Machine(Machine&& inMachine) { moveFrom(std::move(inMachine)); }
		void operator=(Machine&& inMachine) { moveFrom(std::move(inMachine)); }

//从抽象生成器对象（已销毁）构造DFA。
		Machine(Builder* inBuilder);
		
//将字符输入DFA，直到达到终端状态。
//到达终端状态后，返回状态，并返回nextchar指针
//更新为指向DFA未使用的第一个字符。
		inline StateIndex feed(const char*& nextChar) const
		{
			Iptr state = 0;
			do
			{
				state = stateAndOffsetToNextStateMap[state + charToOffsetMap[(U8)nextChar[0]]];
				if(state < 0) { nextChar += 1; break; }
				state = stateAndOffsetToNextStateMap[state + charToOffsetMap[(U8)nextChar[1]]];
				if(state < 0) { nextChar += 2; break; }
				state = stateAndOffsetToNextStateMap[state + charToOffsetMap[(U8)nextChar[2]]];
				if(state < 0) { nextChar += 3; break; }
				state = stateAndOffsetToNextStateMap[state + charToOffsetMap[(U8)nextChar[3]]];
				nextChar += 4;
			}
			while(state >= 0);
			if(state & edgeDoesntConsumeInputFlag)
			{
				--nextChar;
				state &= ~edgeDoesntConsumeInputFlag;
			}
			return (StateIndex)state;
		}

//将DFA的状态和边缘转储为graphviz.dot格式。
		std::string dumpDFAGraphViz() const;

	private:

		typedef I16 InternalStateIndex;
		enum { internalMaxStates = INT16_MAX };

		U32 charToOffsetMap[256];
		InternalStateIndex* stateAndOffsetToNextStateMap;
		Uptr numClasses;
		Uptr numStates;

		void moveFrom(Machine&& inMachine);
	};
}