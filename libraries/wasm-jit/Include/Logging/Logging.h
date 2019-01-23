
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#ifndef LOGGING_API
	#define LOGGING_API DLL_IMPORT
#endif

#include "Inline/BasicTypes.h"
#include "Platform/Platform.h"

//调试日志记录。
namespace Log
{
//允许按类别筛选日志记录。
	enum class Category
	{
		error,
		debug,
		metrics,
		num
	};
	LOGGING_API void setCategoryEnabled(Category category,bool enable);
	LOGGING_API bool isCategoryEnabled(Category category);

//打印一些已分类、格式化的字符串，并刷新输出。不包括换行符。
	LOGGING_API void printf(Category category,const char* format,...);
};