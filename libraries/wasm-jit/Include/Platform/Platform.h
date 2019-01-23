
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include <vector>
#include <functional>

#include "Inline/BasicTypes.h"
#include "Inline/Errors.h"

#ifdef _WIN32
	#define THREAD_LOCAL thread_local
	#define DLL_EXPORT __declspec(dllexport)
	#define DLL_IMPORT __declspec(dllimport)
	#define FORCEINLINE __forceinline
	#define SUPPRESS_UNUSED(variable) (void)(variable);
	#include <intrin.h>
	#define PACKED_STRUCT(definition) __pragma(pack(push, 1)) definition; __pragma(pack(pop))
#else
//使用Y-Type代替C++ 11 TreReLoad，因为苹果的CLAN不支持TyRead本地。
	#define THREAD_LOCAL __thread
	#define DLL_EXPORT
	#define DLL_IMPORT
	#define FORCEINLINE inline __attribute__((always_inline))
	#define SUPPRESS_UNUSED(variable) (void)(variable);
	#define PACKED_STRUCT(definition) definition __attribute__((packed));
#endif

#ifndef PLATFORM_API
	#define PLATFORM_API DLL_IMPORT
#endif

namespace Platform
{
//countleadingzeroes/counttrailingzeroes返回前导/尾随零的数目，或者如果未设置任何位，则返回输入的位宽度。
	#ifdef _WIN32
//如果输入为0，则BitscanReverse/BitscanForward返回0。
		inline U32 countLeadingZeroes(U32 value) { unsigned long result; return _BitScanReverse(&result,value) ? (31 - result) : 32; }
		inline U32 countTrailingZeroes(U32 value) { unsigned long result; return _BitScanForward(&result,value) ? result : 32; }
		
		#ifdef _WIN64
			inline U64 countLeadingZeroes(U64 value) { unsigned long result; return _BitScanReverse64(&result,value) ? (63 - result) : 64; }
			inline U64 countTrailingZeroes(U64 value) { unsigned long result; return _BitScanForward64(&result,value) ? result : 64; }
		#else
			inline U64 countLeadingZeroes(U64 value) { throw; }
			inline U64 countTrailingZeroes(U64 value) { throw; }
		#endif
	#else
//_uu builtin_clz/u builtin_ctz如果输入为0，则未定义。
		inline U64 countLeadingZeroes(U64 value) { return value == 0 ? 64 : __builtin_clzll(value); }
		inline U32 countLeadingZeroes(U32 value) { return value == 0 ? 32 : __builtin_clz(value); }
		inline U64 countTrailingZeroes(U64 value) { return value == 0 ? 64 : __builtin_ctzll(value); }
		inline U32 countTrailingZeroes(U32 value) { return value == 0 ? 32 : __builtin_ctz(value); }
	#endif
	inline U64 floorLogTwo(U64 value) { return value <= 1 ? 0 : 63 - countLeadingZeroes(value); }
	inline U32 floorLogTwo(U32 value) { return value <= 1 ? 0 : 31 - countLeadingZeroes(value); }
	inline U64 ceilLogTwo(U64 value) { return floorLogTwo(value * 2 - 1); }
	inline U32 ceilLogTwo(U32 value) { return floorLogTwo(value * 2 - 1); }

//
//记忆
//

//描述允许的内存访问。
	enum class MemoryAccess
	{
		None,
		ReadOnly,
		ReadWrite,
		Execute,
		ReadWriteExecute
	};

//返回最小虚拟页大小的底2对数。
	PLATFORM_API Uptr getPageSizeLog2();

//分配虚拟地址而不向其提交物理页。
//返回已分配地址的基本虚拟地址，如果虚拟地址空间已用完，则返回nullptr。
	PLATFORM_API U8* allocateVirtualPages(Uptr numPages);

//将物理内存提交到指定的虚拟页。
//basevirtualAddress必须是首选页面大小的倍数。
//如果成功，则返回true；如果物理内存耗尽，则返回false。
	PLATFORM_API bool commitVirtualPages(U8* baseVirtualAddress,Uptr numPages,MemoryAccess access = MemoryAccess::ReadWrite);

//更改对指定虚拟页的允许访问权限。
//basevirtualAddress必须是首选页面大小的倍数。
//如果成功，则返回true；如果无法设置访问级别，则返回false。
	PLATFORM_API bool setVirtualPageAccess(U8* baseVirtualAddress,Uptr numPages,MemoryAccess access);

//解除提交给指定虚拟页的物理内存。
//basevirtualAddress必须是首选页面大小的倍数。
	PLATFORM_API void decommitVirtualPages(U8* baseVirtualAddress,Uptr numPages);

//释放虚拟地址。提交到地址的任何物理内存都必须已被解除授权。
//basevirtualAddress必须是首选页面大小的倍数。
	PLATFORM_API void freeVirtualPages(U8* baseVirtualAddress,Uptr numPages);

//
//调用堆栈和异常
//

//描述调用堆栈。
	struct CallStack
	{
		struct Frame
		{
			Uptr ip;
		};
		std::vector<Frame> stackFrames;
	};

//捕获调用方的执行上下文。
	PLATFORM_API CallStack captureCallStack(Uptr numOmittedFramesFromTop = 0);
	
//描述指令指针。
	PLATFORM_API bool describeInstructionPointer(Uptr ip,std::string& outDescription);

	#ifdef _WIN64
//注册/注销Windows SEH用于展开堆栈帧的数据。
		PLATFORM_API void registerSEHUnwindInfo(Uptr imageLoadAddress,Uptr pdataAddress,Uptr pdataNumBytes);
		PLATFORM_API void deregisterSEHUnwindInfo(Uptr pdataAddress);
	#endif

//调用thunk，如果它导致某些特定的硬件陷阱，则返回true。
//如果捕获了陷阱，则会设置outCause、outContext和outOperand参数来描述陷阱。
	enum HardwareTrapType
	{
		none,
		accessViolation,
		stackOverflow,
		intDivideByZeroOrOverflow
	};
	PLATFORM_API HardwareTrapType catchHardwareTraps(
		CallStack& outTrapCallStack,
		Uptr& outTrapOperand,
		const std::function<void()>& thunk
		);
	PLATFORM_API void immediately_exit();

//
//螺纹加工
//

//返回可用作等待超时的绝对时间的时钟的当前值。
//分辨率是微秒，原点是任意的。
	PLATFORM_API U64 getMonotonicClock();

//独立于平台的互斥体。
	struct Mutex;
	PLATFORM_API Mutex* createMutex();
	PLATFORM_API void destroyMutex(Mutex* mutex);
	PLATFORM_API void lockMutex(Mutex* mutex);
	PLATFORM_API void unlockMutex(Mutex* mutex);

//RAII风格的静音锁。
	struct Lock
	{
		Lock(Mutex* inMutex) : mutex(inMutex) { lockMutex(mutex); }
		~Lock() { unlockMutex(mutex); }

		void release()
		{
			if(mutex)
			{
				unlockMutex(mutex);
			}
			mutex = nullptr;
		}

		void detach()
		{
			WAVM_ASSERT_THROW(mutex);
			mutex = nullptr;
		}

	private:
		Mutex* mutex;
	};

//平台独立事件。
	struct Event;
	PLATFORM_API Event* createEvent();
	PLATFORM_API void destroyEvent(Event* event);
	PLATFORM_API bool waitForEvent(Event* event,U64 untilClock);
	PLATFORM_API void signalEvent(Event* event);
}
