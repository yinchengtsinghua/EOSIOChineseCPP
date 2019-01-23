
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "Inline/BasicTypes.h"
#include "Logging/Logging.h"
#include "Intrinsics.h"
#include "RuntimePrivate.h"

#include <thread>
#include <vector>
#include <atomic>
#include <cmath>
#include <algorithm>

#if ENABLE_THREADING_PROTOTYPE
//跟踪正在运行的WebAssembly生成的线程使用的入口和错误函数。
//用于查找垃圾收集根。
struct Thread
{
	Runtime::FunctionInstance* entryFunction;
	Runtime::FunctionInstance* errorFunction;
};

//保存一个线程列表（以事件的形式唤醒线程），该列表
//正在等待特定地址。
struct WaitList
{
	Platform::Mutex* mutex;
	std::vector<Platform::Event*> wakeEvents;
	std::atomic<Uptr> numReferences;

	WaitList(): mutex(Platform::createMutex()), numReferences(1) {}
	~WaitList() { destroyMutex(mutex); }
};

//线程在等待列表中等待时在线程中重用的事件。
THREAD_LOCAL Platform::Event* threadWakeEvent = nullptr;

//从地址到等待该地址的线程列表的映射。
static Platform::Mutex* addressToWaitListMapMutex = Platform::createMutex();
static std::map<Uptr,WaitList*> addressToWaitListMap;

//由WebAssembly代码创建的运行线程的全局列表。
static Platform::Mutex* threadsMutex = Platform::createMutex();
static std::vector<Thread*> threads;

//打开给定地址的等待列表。
//增加等待列表的引用计数，并返回指向该列表的指针。
//请注意，它不会锁定等待列表互斥体。
//调用openwaitlist后应调用closewaitlist以避免泄漏。
static WaitList* openWaitList(Uptr address)
{
	Platform::Lock mapLock(addressToWaitListMapMutex);
	auto mapIt = addressToWaitListMap.find(address);
	if(mapIt != addressToWaitListMap.end())
	{
		++mapIt->second->numReferences;
		return mapIt->second;
	}
	else
	{
		WaitList* waitList = new WaitList();
		addressToWaitListMap.emplace(address,waitList);
		return waitList;
	}
}

//关闭等待列表，将其删除，如果是最后一个引用，则将其从全局映射中删除。
static void closeWaitList(Uptr address,WaitList* waitList)
{
	if(--waitList->numReferences == 0)
	{
		Platform::Lock mapLock(addressToWaitListMapMutex);
		if(!waitList->numReferences)
		{
			WAVM_ASSERT_THROW(!waitList->wakeEvents.size());
			delete waitList;
			addressToWaitListMap.erase(address);
		}
	}
}

//按顺序从内存加载值。
//调用方必须确保指针自然对齐。
template<typename Value>
static Value atomicLoad(const Value* valuePointer)
{
	static_assert(sizeof(std::atomic<Value>) == sizeof(Value),"relying on non-standard behavior");
	std::atomic<Value>* valuePointerAtomic = (std::atomic<Value>*)valuePointer;
	return valuePointerAtomic->load();
}

//按顺序将值存储到内存。
//调用方必须确保指针自然对齐。
template<typename Value>
static void atomicStore(Value* valuePointer,Value newValue)
{
	static_assert(sizeof(std::atomic<Value>) == sizeof(Value),"relying on non-standard behavior");
	std::atomic<Value>* valuePointerAtomic = (std::atomic<Value>*)valuePointer;
	valuePointerAtomic->store(newValue);
}

//解码相对于starttime的浮点超时值。
U64 getEndTimeFromTimeout(U64 startTime,F64 timeout)
{
	const F64 timeoutMicroseconds = timeout * 1000.0;
	U64 endTime = UINT64_MAX;
	if(!std::isnan(timeoutMicroseconds) && std::isfinite(timeoutMicroseconds))
	{
		if(timeoutMicroseconds <= 0.0)
		{
			endTime = startTime;
		}
		else if(timeoutMicroseconds <= F64(UINT64_MAX - 1))
		{
			endTime = startTime + U64(timeoutMicroseconds);
			errorUnless(endTime >= startTime);
		}
	}
	return endTime;
}

template<typename Value>
static U32 waitOnAddress(Value* valuePointer,Value expectedValue,F64 timeout)
{
	const U64 endTime = getEndTimeFromTimeout(Platform::getMonotonicClock(),timeout);

//打开此地址的等待列表。
	const Uptr address = reinterpret_cast<Uptr>(valuePointer);
	WaitList* waitList = openWaitList(address);

//锁定等待列表，并检查*valuepointer是否仍然是调用者期望的值。
	lockMutex(waitList->mutex);
	if(atomicLoad(valuePointer) != expectedValue)
	{
//如果*valuepointer不是预期值，请解锁等待列表并返回。
		unlockMutex(waitList->mutex);
		closeWaitList(address,waitList);
		return 1;
	}
	else
	{
//如果线程尚未创建唤醒事件，请执行此操作。
		if(!threadWakeEvent) { threadWakeEvent = Platform::createEvent(); }

//将唤醒事件添加到等待列表，然后解锁等待列表。
		waitList->wakeEvents.push_back(threadWakeEvent);
		unlockMutex(waitList->mutex);
	}

//等待线程的唤醒事件发出信号。
	bool timedOut = false;
	if(!Platform::waitForEvent(threadWakeEvent,endTime))
	{
//如果等待超时，请锁定等待列表，并检查线程的唤醒事件是否仍在等待列表中。
		Platform::Lock waitListLock(waitList->mutex);
		auto wakeEventIt = std::find(waitList->wakeEvents.begin(),waitList->wakeEvents.end(),threadWakeEvent);
		if(wakeEventIt != waitList->wakeEvents.end())
		{
//如果事件仍在等待列表中，请将其删除，然后返回“超时”结果。
			waitList->wakeEvents.erase(wakeEventIt);
			timedOut = true;
		}
		else
		{
//在等待超时和锁定等待列表之间，其他线程试图唤醒此线程。
//现在将向事件发出信号，因此使用立即过期的等待来重置它。
			errorUnless(Platform::waitForEvent(threadWakeEvent,Platform::getMonotonicClock()));
		}
	}

	closeWaitList(address,waitList);
	return timedOut ? 2 : 0;
}

static U32 wakeAddress(Uptr address,U32 numToWake)
{
	if(numToWake == 0) { return 0; }

//打开此地址的等待列表。
	WaitList* waitList = openWaitList(address);
	{
		Platform::Lock waitListLock(waitList->mutex);

//确定要唤醒的线程数。
//numtowake==uint32_max表示唤醒所有等待线程。
		Uptr actualNumToWake = numToWake;
		if(numToWake == UINT32_MAX || numToWake > waitList->wakeEvents.size())
		{
			actualNumToWake = waitList->wakeEvents.size();
		}

//向对应于最旧等待线程的事件发出信号。
		for(Uptr wakeIndex = 0;wakeIndex < actualNumToWake;++wakeIndex)
		{
			signalEvent(waitList->wakeEvents[wakeIndex]);
		}

//从等待列表中删除事件。
		waitList->wakeEvents.erase(waitList->wakeEvents.begin(),waitList->wakeEvents.begin() + actualNumToWake);
	}
	closeWaitList(address,waitList);

	return numToWake;
}

namespace Runtime
{
	DEFINE_INTRINSIC_FUNCTION1(wavmIntrinsics,misalignedAtomicTrap,misalignedAtomicTrap,none,i32,address)
	{
		causeException(Exception::Cause::misalignedAtomicMemoryAccess);
	}

	DEFINE_INTRINSIC_FUNCTION1(wavmIntrinsics,isLockFree,isLockFree,i32,i32,numBytes)
	{
//假设我们运行的是x86。
		switch(numBytes)
		{
		case 1: case 2: case 4: case 8: return true;
		default: return false;
		};
	}

	DEFINE_INTRINSIC_FUNCTION3(wavmIntrinsics,wake,wake,i32,i32,addressOffset,i32,numToWake,i64,memoryInstanceBits)
	{
		MemoryInstance* memoryInstance = reinterpret_cast<MemoryInstance*>(memoryInstanceBits);

//验证地址是否在内存边界内，并且4字节对齐。
		if(U32(addressOffset) > memoryInstance->endOffset) { causeException(Exception::Cause::accessViolation); }
		if(addressOffset & 3) { causeException(Exception::Cause::misalignedAtomicMemoryAccess); }

		const Uptr address = reinterpret_cast<Uptr>(getMemoryBaseAddress(memoryInstance)) + addressOffset;
		return wakeAddress(address,numToWake);
	}

	DEFINE_INTRINSIC_FUNCTION4(wavmIntrinsics,wait,wait,i32,i32,addressOffset,i32,expectedValue,f64,timeout,i64,memoryInstanceBits)
	{
		MemoryInstance* memoryInstance = reinterpret_cast<MemoryInstance*>(memoryInstanceBits);

//验证地址是否在内存边界内并且自然对齐。
		if(U32(addressOffset) > memoryInstance->endOffset) { causeException(Exception::Cause::accessViolation); }
		if(addressOffset & 3) { causeException(Exception::Cause::misalignedAtomicMemoryAccess); }

		I32* valuePointer = reinterpret_cast<I32*>(getMemoryBaseAddress(memoryInstance) + addressOffset);
		return waitOnAddress(valuePointer,expectedValue,timeout);
	}
	DEFINE_INTRINSIC_FUNCTION4(wavmIntrinsics,wait,wait,i32,i32,addressOffset,i64,expectedValue,f64,timeout,i64,memoryInstanceBits)
	{
		MemoryInstance* memoryInstance = reinterpret_cast<MemoryInstance*>(memoryInstanceBits);

//验证地址是否在内存边界内并且自然对齐。
		if(U32(addressOffset) > memoryInstance->endOffset) { causeException(Exception::Cause::accessViolation); }
		if(addressOffset & 7) { causeException(Exception::Cause::misalignedAtomicMemoryAccess); }

		I64* valuePointer = reinterpret_cast<I64*>(getMemoryBaseAddress(memoryInstance) + addressOffset);
		return waitOnAddress(valuePointer,expectedValue,timeout);
	}

	FunctionInstance* getFunctionFromTable(TableInstance* table,const FunctionType* expectedType,U32 elementIndex)
	{
//验证索引是否有效。
		if(elementIndex * sizeof(TableInstance::FunctionElement) >= table->endOffset)
		{
			causeException(Runtime::Exception::Cause::undefinedTableElement);
		}
//验证索引函数的类型是否与预期类型匹配。
		const FunctionType* actualSignature = table->baseAddress[elementIndex].type;
		if(actualSignature != expectedType)
		{
			causeException(Runtime::Exception::Cause::indirectCallSignatureMismatch);
		}
		return asFunction(table->elements[elementIndex]);
	}

	void callAndTurnHardwareTrapsIntoRuntimeExceptions(void (*function)(I32),I32 argument)
	{
		Platform::CallStack trapCallStack;
		Uptr trapOperand;
		const Platform::HardwareTrapType trapType = Platform::catchHardwareTraps(
			trapCallStack,trapOperand,
			[function,argument]{(*function)(argument);}
			);
		if(trapType != Platform::HardwareTrapType::none)
		{
			handleHardwareTrap(trapType,std::move(trapCallStack),trapOperand);
		}
	}
	
	static void threadFunc(Thread* thread,I32 argument)
	{
		try
		{
//调用线程入口函数。
			callAndTurnHardwareTrapsIntoRuntimeExceptions((void(*)(I32))thread->entryFunction->nativeFunction,argument);
		}
		catch(Runtime::Exception exception)
		{
//由线程错误函数处理运行时异常的日志。
			Log::printf(Log::Category::error,"Runtime exception in thread: %s\n",describeExceptionCause(exception.cause));
			for(auto calledFunction : exception.callStack) { Log::printf(Log::Category::error,"  %s\n",calledFunction.c_str()); }
			Log::printf(Log::Category::error,"Passing exception on to thread error handler\n");

			try
			{
//调用线程错误函数。
				callAndTurnHardwareTrapsIntoRuntimeExceptions((void(*)(I32))thread->errorFunction->nativeFunction,argument);
			}
			catch(Runtime::Exception secondException)
			{
//记录线程错误函数导致运行时异常，并以致命错误退出。
				Log::printf(Log::Category::error,"Runtime exception in thread error handler: %s\n",describeExceptionCause(secondException.cause));
				for(auto calledFunction : secondException.callStack) { Log::printf(Log::Category::error,"  %s\n",calledFunction.c_str()); }
				Errors::fatalf("double fault");
			}
		}

//在退出线程之前销毁线程唤醒事件。
		if(threadWakeEvent)
		{
			Platform::destroyEvent(threadWakeEvent);
		}
		
//从全局列表中删除线程。
		{
			Platform::Lock threadsLock(threadsMutex);
			auto threadIt = std::find(threads.begin(),threads.end(),thread);
			threads.erase(threadIt);
		}

//删除线程对象。
		delete thread;
	}
	
	DEFINE_INTRINSIC_FUNCTION4(wavmIntrinsics,launchThread,launchThread,none,
		i32,entryFunctionIndex,
		i32,argument,
		i32,errorFunctionIndex,
		i64,functionTablePointer)
	{
		TableInstance* defaultTable = reinterpret_cast<TableInstance*>(functionTablePointer);
		const FunctionType* functionType = FunctionType::get(ResultType::none,{ValueType::i32});

//创建一个线程对象，该对象将其入口函数和错误函数作为根公开给垃圾收集器。
		Thread* thread = new Thread();
		thread->entryFunction = getFunctionFromTable(defaultTable,functionType,entryFunctionIndex);
		thread->errorFunction = getFunctionFromTable(defaultTable,functionType,errorFunctionIndex);
		{
			Platform::Lock threadsLock(threadsMutex);
			threads.push_back(thread);
		}

//使用std:：thread生成线程。
		std::thread stdThread([thread,argument]()
		{
			threadFunc(thread,argument);
		});

//从基础线程分离std:：thread。
		stdThread.detach();
	}

	void getThreadGCRoots(std::vector<ObjectInstance*>& outGCRoots)
	{
		Platform::Lock threadsLock(threadsMutex);
		for(auto thread : threads)
		{
			outGCRoots.push_back(thread->entryFunction);
			outGCRoots.push_back(thread->errorFunction);
		}
	}
}
#else
namespace Runtime
{
	void getThreadGCRoots(std::vector<ObjectInstance*>& outGCRoots) {}
}
#endif