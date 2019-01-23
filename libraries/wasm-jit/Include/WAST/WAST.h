
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#ifndef WAST_API
	#define WAST_API DLL_IMPORT
#endif

#include "Inline/BasicTypes.h"
#include "Runtime/Runtime.h"
#include "WASM/WASM.h"

namespace WAST
{
//文本文件中的位置。
	struct TextFileLocus
	{
		std::string sourceLine;
		U32 newlines;
		U32 tabs;
		U32 characters;

		TextFileLocus(): newlines(0), tabs(0), characters(0) {}

		U32 lineNumber() const { return newlines + 1; }
		U32 column(U32 spacesPerTab = 4) const { return tabs * spacesPerTab + characters + 1; }

		std::string describe(U32 spacesPerTab = 4) const
		{
			return std::to_string(lineNumber()) + ":" + std::to_string(column(spacesPerTab));
		}
	};

//WAST分析错误。
	struct Error
	{
		TextFileLocus locus;
		std::string message;
	};

//从字符串分析模块。如果成功，则返回true，并将模块写入outmodule。
//如果失败，则返回false并将错误列表追加到outErrors。
	WAST_API bool parseModule(const char* string,Uptr stringLength,IR::Module& outModule,std::vector<Error>& outErrors);

//以WAST格式打印模块。
	WAST_API std::string print(const IR::Module& module);
}