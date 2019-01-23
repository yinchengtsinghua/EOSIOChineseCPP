
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "Inline/BasicTypes.h"
#include "NFA.h"

namespace Regexp
{
//解析字符串中的正则表达式，并将其识别器添加到给定的NFA中
//识别器将从InitialState开始，并在正则表达式完全匹配后以finalState结束。
	void addToNFA(const char* regexpString,NFA::Builder* nfaBuilder,NFA::StateIndex initialState,NFA::StateIndex finalState);
};
