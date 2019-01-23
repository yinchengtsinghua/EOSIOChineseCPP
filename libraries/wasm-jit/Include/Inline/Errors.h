
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include <stdarg.h>
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace Errors
{
//致命错误处理。
	[[noreturn]] inline void fatalf(const char* messageFormat,...)
	{
		va_list varArgs;
		va_start(varArgs,messageFormat);
		std::vfprintf(stderr,messageFormat,varArgs);
		std::fflush(stderr);
		va_end(varArgs);
		std::abort();
	}
	[[noreturn]] inline void fatal(const char* message) { fatalf("%s\n",message); }
	[[noreturn]] inline void unreachable() { fatalf("reached unreachable code\n"); }
	[[noreturn]] inline void unimplemented(const char* context) { fatalf("unimplemented: %s\n",context); }
}

//与断言类似，但在任何生成配置中都不会删除。
#define errorUnless(condition) if(!(condition)) { Errors::fatalf("errorUnless(%s) failed\n",#condition); }

#define WAVM_ASSERT_THROW(cond) ({ if( !(cond) ) throw std::runtime_error{"wavm assert: " #cond}; })

#define WAVM_ASSERT_TERMINATE(cond) ({ if( !(cond) ) { fprintf(stderr, "wavm assert in destructor: %s", #cond); std::terminate(); } })
