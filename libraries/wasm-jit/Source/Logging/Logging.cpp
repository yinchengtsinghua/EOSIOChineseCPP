
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "Logging.h"
#include "Platform/Platform.h"

#include <cstdio>
#include <cstdarg>
#include <cstdlib>

namespace Log
{
	static Platform::Mutex* categoryEnabledMutex = Platform::createMutex();
	static bool categoryEnabled[(Uptr)Category::num] =
	{
true, //错误
#ifdef _DEBUG //调试
			true,
		#else
			false,
		#endif
WAVM_METRICS_OUTPUT != 0 //韵律学
	};
	void setCategoryEnabled(Category category,bool enable)
	{
		Platform::Lock lock(categoryEnabledMutex);
		WAVM_ASSERT_THROW(category < Category::num);
		categoryEnabled[(Uptr)category] = enable;
	}
	bool isCategoryEnabled(Category category)
	{
		Platform::Lock lock(categoryEnabledMutex);
		WAVM_ASSERT_THROW(category < Category::num);
		return categoryEnabled[(Uptr)category];
	}
	void printf(Category category,const char* format,...)
	{
		Platform::Lock lock(categoryEnabledMutex);
		if(categoryEnabled[(Uptr)category])
		{
			va_list varArgs;
			va_start(varArgs,format);
			vfprintf(stdout,format,varArgs);
			fflush(stdout);
			va_end(varArgs);
		}
	}
}
