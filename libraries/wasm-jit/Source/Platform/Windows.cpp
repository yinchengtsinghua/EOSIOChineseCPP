
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#if _WIN32

#include "Inline/BasicTypes.h"
#include "Inline/Errors.h"
#include "Platform.h"

#include <inttypes.h>
#include <algorithm>
#include <Windows.h>
#include <DbgHelp.h>

#undef min

namespace Platform
{
	static Uptr internalGetPreferredVirtualPageSizeLog2()
	{
		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);
		Uptr preferredVirtualPageSize = systemInfo.dwPageSize;
//验证我们假设的虚拟页面大小是2的幂。
		errorUnless(!(preferredVirtualPageSize & (preferredVirtualPageSize - 1)));
		return floorLogTwo(preferredVirtualPageSize);
	}
	Uptr getPageSizeLog2()
	{
		static Uptr preferredVirtualPageSizeLog2 = internalGetPreferredVirtualPageSizeLog2();
		return preferredVirtualPageSizeLog2;
	}

	U32 memoryAccessAsWin32Flag(MemoryAccess access)
	{
		switch(access)
		{
		default:
		case MemoryAccess::None: return PAGE_NOACCESS;
		case MemoryAccess::ReadOnly: return PAGE_READONLY;
		case MemoryAccess::ReadWrite: return PAGE_READWRITE;
		case MemoryAccess::Execute: return PAGE_EXECUTE_READ;
		case MemoryAccess::ReadWriteExecute: return PAGE_EXECUTE_READWRITE;
		}
	}

	static bool isPageAligned(U8* address)
	{
		const Uptr addressBits = reinterpret_cast<Uptr>(address);
		return (addressBits & ((1ull << getPageSizeLog2()) - 1)) == 0;
	}

	U8* allocateVirtualPages(Uptr numPages)
	{
		Uptr numBytes = numPages << getPageSizeLog2();
		auto result = VirtualAlloc(nullptr,numBytes,MEM_RESERVE,PAGE_NOACCESS);
		if(result == NULL)
		{
			return nullptr;
		}
		return (U8*)result;
	}

	bool commitVirtualPages(U8* baseVirtualAddress,Uptr numPages,MemoryAccess access)
	{
		errorUnless(isPageAligned(baseVirtualAddress));
		return baseVirtualAddress == VirtualAlloc(baseVirtualAddress,numPages << getPageSizeLog2(),MEM_COMMIT,memoryAccessAsWin32Flag(access));
	}

	bool setVirtualPageAccess(U8* baseVirtualAddress,Uptr numPages,MemoryAccess access)
	{
		errorUnless(isPageAligned(baseVirtualAddress));
		DWORD oldProtection = 0;
		return VirtualProtect(baseVirtualAddress,numPages << getPageSizeLog2(),memoryAccessAsWin32Flag(access),&oldProtection) != 0;
	}
	
	void decommitVirtualPages(U8* baseVirtualAddress,Uptr numPages)
	{
		errorUnless(isPageAligned(baseVirtualAddress));
		auto result = VirtualFree(baseVirtualAddress,numPages << getPageSizeLog2(),MEM_DECOMMIT);
		if(baseVirtualAddress && !result) { Errors::fatal("VirtualFree(MEM_DECOMMIT) failed"); }
	}

	void freeVirtualPages(U8* baseVirtualAddress,Uptr numPages)
	{
		errorUnless(isPageAligned(baseVirtualAddress));
  /*o result=virtualfree（basevirtualAddress，0/*numpages<<getpagesizelog2（）*/，mem_release）；
  如果（basevirtualAddress&！结果）错误：：致命（“virtualfree（mem_release）failed”）；
 }

 //到dbghelp dll的接口
 结构数据库帮助
 {
  typedef bool（winapi*symfromaddr）（handle，u64，u64*，symbol_info*）；
  symfromaddr、symfromaddr；
  数据库帮助（）
  {
   hmodule dbghelpmodule=：：loadLibraryA（“dbghelp.dll”）；
   如果（dbghelpmodule）
   {
    symFromAddr=（symFromAddr）：getProcAddress（dbghelpModule，“symFromAddr”）；

    //初始化调试符号查找。
    typedef bool（winapi*syminitialize）（句柄、pctstr、bool）；
    symInitialize symInitialize=（symInitialize）：getProcAddress（dbghelpModule，“symInitialize”）；
    if（符号初始化）
    {
     symInitialize（getcurrentprocess（），nullptr，真）；
    }
   }
  }
 }；
 dbghelp*dbghelp=nullptr；

 bool describe指令指针（uptr ip，std:：string&outdescription）
 {
  //初始化dbghelp。
  如果（！）dbghelp）dbghelp=新dbghelp（）；

  //分配一个符号信息结构以接收有关此指令指针符号的信息。
  const uptr maxsymbolnamechars=256；
  const uptr符号分配大小=sizeof（symbol_info）+sizeof（tchar）*（maxsymbolnamechars-1）；
  symbol揤info*symbol info=（symbol揤info*）alloca（symbolAllocationSize）；
  零内存（符号信息，符号分配大小）；
  symbol info->sizeofstrut=sizeof（symbol_info）；
  symbolinfo->maxnamelen=maxsymbolnamechars；

  //调用dbghelp:：symfromaddr以尝试查找包含此地址的任何调试符号。
  如果（！）dbghelp->symFromAddr（getcurrentprocess（），ip，nullptr，symbolinfo））返回false；
  其他的
  {
   outdescription=symbolinfo->name；
   回归真实；
  }
 }
	
 如果已定义（_win32）&&defined（_amd64_u）
  空寄存器ehunwindinfo（uptr imageloadaddress、uptr pdataaddress、uptr pdatanumbytes）
  {
   const u32 numFunctions=（u32）（pDataNumbytes/sizeof（runtime_function））；

   //注册我们手动修复的函数表副本。
   如果（！）rtladdFunctionTable（reinterpret_cast<runtime_function*>（pdataAddress），numFunctions，imageLoadAddress））
   {
    错误：：致命（“rtladdFunctionTable失败”）；
   }
  }
  void注销sehunwindinfo（uptr pdata地址）
  {
   auto functiontable=reinterpret_cast<runtime_function*>（pdata地址）；
   rtldeleteFunctionTable（功能表）；
   删除[]功能表；
  }
 第二节
	
 调用堆栈展开（常量上下文和不可变上下文）
 {
  //创建上下文的可变副本。
  上下文上下文；
  memcpy（&context，&immutablecontext，sizeof（context））；

  //展开堆栈，直到没有有效的指令指针，这表示我们已经到达了基值。
  调用堆栈调用堆栈；
  γIFIFF和WI64
  while（context.rip）
  {
   callstack.stackframes.push_back（context.rip）；

   //查找此函数的SEH释放信息。
   U64影像库；
   auto runtimefunction=rtllookupfunctionentry（context.rip，&imagebase，nullptr）；
   如果（！）运行时函数）
   {
    //不接触rsp的叶函数可能没有展开信息。
    context.rip=*（u64*）context.rsp；
    context.rsp+=8；
   }
   其他的
   {
    //使用SEH信息展开到下一个堆栈帧。
    void*处理程序数据；
    U64建立框架；
    风（
     联合国国旗勋章，
     图像库，
     上下文，RIP，
     运行时函数，
     语境，
     和手工数据，
     建立框架（&A）
     努尔普特
     ；
   }
  }
  第二节

  返回调用堆栈；
 }

 调用堆栈CaptureCallStack（uptr nummotedFramesFromTop）
 {
  //捕获当前处理器状态。
  上下文上下文；
  rtlcaptureContext（&context）；

  //展开堆栈。
  自动结果=取消绑定（上下文）；

  //远程请求的省略帧数，对于此函数为+1。
  const uptr nummotedframes=std:：min（result.stackframes.size（），nummotedframesFromTop+1）；
  result.stackframes.erase（result.stackframes.begin（），result.stackframes.begin（）+nummotedframes）；

  返回结果；
 }

 thread_local bool isreentranteexception=false；
 长回调sehfilterfunction（exception_pointers*exception pointers、hardwaretraptype&outtype、uptr&outtrapoperand、callstack&outcallstack）
 {
  if（isreentranteexception）错误：：致命（“可重入异常”）；
  其他的
  {
   //决定如何处理此异常代码。
   开关（exceptionpointers->exceptionrecord->exceptioncode）
   {
   案例例外\访问\违规：
    outtype=hardwaretraptype:：accessviolation；
    outTrapOperand=exceptionpointers->exceptionrecord->exceptionInformation[1]；
    断裂；
   case exception_stack_overflow:outtype=hardwaretraptype:：stack overflow；break；
   case status_integer_除以_zero:outtype=hardwaretraptype:：intDividedByZeroorOverflow；break；
   case status_integer_overflow:outtype=hardwaretraptype:：intDivideByZeroorOverflow；break；
   默认：返回异常继续搜索；
   }
   IsReentranteException=真；

   //从异常的上下文中展开堆栈帧。
   

   返回异常\执行\处理程序；
  }
 }
	
 thread_local bool isthreadinialized=false；
 void inithread（））
 {
  如果（！）已初始化）
  {
   IsThreadInitialized=true；

   //确保在堆栈溢出的情况下堆栈上有足够的空间来准备堆栈跟踪。
   ulong stackoverflowreservebytes=32768；
   设置线程跟踪保证（&stackoverflowreservebytes）；
  }
 }

 hardwaretraptype catchhardwaretraps（
  调用堆栈和OuttrapCallStack，
  
  const std:：function<void（）>&thunk
  ）
 {
  InthTHead（）；

  hardwaretraptype result=hardwaretraptype:：none；
  试用
  {
   Tunk（）；
  }
  _uuExcept（sehFilterFunction（getExceptionInformation（），result，outTrapOperand，outTrapCallStack））。
  {
   isreentrantexception=false；
			
   //堆栈溢出后，堆栈将保持损坏状态。让CRT来修理它。
   if（result==hardwaretraptype:：stackOverflow）u resetstKoflw（）；
  }
  返回结果；
 }
	
 U64获取单调锁（）
 {
  大整数性能计数器；
  大整数性能计数器频率；
  queryperformanceCounter（&performanceCounter）；
  QueryPerformanceFrequency（&PerformanceCounterFrequency）；

  const u64 wavmfrequency=1000000；

  返回PerformanceCounterFrequency.Quadpart>WavmFrequency
   ？性能计数器.quadpart/（性能计数器频率.quadpart/wavmfrequency）
   ：PerformanceCounter.Quadpart*（wavmFrequency/PerformanceCounterFrequency.Quadpart）；
 }

 结构互斥
 {
  关键部分关键部分；
 }；

 mutex*创建mutex（）
 {
  自动互斥=new mutex（）；
  initializeCriticalSection（&mutex->criticalSection）；
  返回互斥；
 }

 空的DestroyMutex（互斥*互斥）
 {
  删除关键部分（&mutex->criticalsection）；
  删除互斥；
 }

 void lockmutex（互斥*互斥）
 {
  输入criticalsection（&mutex->criticalsection）；
 }

 void unlockmutex（互斥*互斥）
 {
  离开CriticalSection（&mutex->CriticalSection）；
 }

 事件*CreateEvent（）
 {
  return reinterpret_cast<event*>（createEvent（nullptr，false，false，nullptr））；
 }

 void destroyevent（事件*事件）
 {
  closehandle（reinterpret_cast<handle>（event））；
 }

 bool waitforevent（事件*事件，U64直到时间）
 {
  u64 currentTime=get单调icLock（）；
  const u64 startprocessTime=当前时间；
  虽然（真）
  {
   const u64 timeoutmicroseconds=currentTime>untiltime？0:（untiltime-当前时间）；
   const u64 timeoutmilliseconds64=timeoutmicroseconds/1000；
   常量U32超时毫秒32=
    超时毫秒64>uint32_最大值
    ？（uint32_最大值-1）
    ：u32（超时毫秒64）；
		
   const u32 waitresult=waitForSingleObject（reinterpret_cast<handle>（event），timeoutMilliseconds32）；
   如果（结果）！= WaigiTimeOUT
   {
    错误除非（waitresult==wait_object_0）；
    回归真实；
   }
   其他的
   {
    currentTime=get单调icLock（）；
    如果（当前时间>=直到时间）
    {
     返回错误；
    }
   }
  }；
 }

 无效信号事件（事件*事件）
 {
  错误除非（setevent（reinterpret_cast<handle>（event））；
 }
}

第二节
