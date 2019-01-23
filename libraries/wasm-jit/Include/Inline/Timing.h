
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include <chrono>

#include "Logging/Logging.h"

namespace Timing
{
//封装计时器，该计时器在构造时启动，在读取时停止。
	struct Timer
	{
		Timer(): startTime(std::chrono::high_resolution_clock::now()), isStopped(false) {}
		void stop() { endTime = std::chrono::high_resolution_clock::now(); }
		U64 getMicroseconds()
		{
			if(!isStopped) { stop(); }
			return std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
		}
		F64 getMilliseconds() { return getMicroseconds() / 1000.0; }
		F64 getSeconds() { return getMicroseconds() / 1000000.0; }
	private:
		std::chrono::high_resolution_clock::time_point startTime;
		std::chrono::high_resolution_clock::time_point endTime;
		bool isStopped;
	};
	
//打印计时器的帮助程序。
	inline void logTimer(const char* context,Timer& timer) { Log::printf(Log::Category::metrics,"%s in %.2fms\n",context,timer.getMilliseconds()); }
	inline void logRatePerSecond(const char* context,Timer& timer,F64 numerator,const char* numeratorUnit)
	{
		Log::printf(Log::Category::metrics,"%s in %.2fms (%f %s/s)\n",
			context,
			timer.getMilliseconds(),
			numerator / timer.getSeconds(),
			numeratorUnit
			);
	}
}