
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
/*
 *@文件
 *@eos/license中定义的版权
 **/


#include <eosiolib/memory.hpp>


void verify_mem(const void* const ptr, const uint32_t val, const uint32_t size)
{
   const char* char_ptr = (const char*)ptr;
   for (uint32_t i = 0; i < size; ++i)
   {
      eosio_assert(static_cast<uint32_t>(static_cast<unsigned char>(char_ptr[i])) == val, "buf slot doesn't match");
   }
}

/*
无效打印（const void*const ptr，const uint32_t size）
{
   const char*char_ptr=（const char*）ptr；
   eosio：：打印（“\n”）；
   对于（uint32_t i=0；i<size；+i）
   {
      const char*delim=（i%8==7）？“，“：”；
      eosio:：print（“”，static_cast<uint32_t>（static_cast<unsigned char>（char_ptr[i]）），delim）；
   }
   eosio:：print（“\n”）；
}
**/


/*
*malloc和realloc总是基于总分配在8字节边界上分配，因此
*如果请求的大小+2字节头不能被8整除，则分配的空间
*将大于请求的大小
**/

void test_memory::test_memory_allocs()
{
    char* ptr1 = (char*)malloc(0);
    eosio_assert(ptr1 == nullptr, "should not have allocated a 0 char buf");

//20个字符-20+4（标题），可被8整除
    ptr1 = (char*)malloc(20);
    eosio_assert(ptr1 != nullptr, "should have allocated a 20 char buf");
    verify_mem(ptr1, 0, 20);
//现有内存布局->24

//分配了36个字符-30+4加上一个额外的6，可以被8整除
    char* ptr1_realloc = (char*)realloc(ptr1, 30);
    eosio_assert(ptr1_realloc != nullptr, "should have returned a 30 char buf");
    eosio_assert(ptr1_realloc == ptr1, "should have enlarged the 20 char buf");
//现有内存布局->40_

//分配20个字符
    char* ptr2 = (char*)malloc(20);
    eosio_assert(ptr2 != nullptr, "should have allocated another 20 char buf");
eosio_assert(ptr1 + 36 < ptr2, "20 char buf should have been created after ptr1"); //特定于实现的测试（可以移除进行重构）
    verify_mem(ptr1, 0, 36);
eosio_assert(ptr1[36] != 0, "should not have empty bytes following since block allocated"); //特定于实现的测试（可以移除进行重构）
//现有内存布局->40 24

//缩小缓冲区
    ptr1[14] = 0x7e;
//分配了20个字符（静止）
    ptr1_realloc = (char*)realloc(ptr1, 15);
    eosio_assert(ptr1_realloc != nullptr, "should have returned a 15 char buf");
    eosio_assert(ptr1_realloc == ptr1, "should have shrunk the reallocated 30 char buf");
verify_mem(ptr1, 0, 14); //特定于实现的测试（可以移除进行重构）
    eosio_assert(ptr1[14] == 0x7e, "remaining 15 chars of buf should be untouched");
//现有内存布局->24（收缩）16（释放）24

//缓冲区大小相同（验证角大小写）
//分配了20个字符（静止）
    ptr1_realloc = (char*)realloc(ptr1, 15);
    eosio_assert(ptr1_realloc != nullptr, "should have returned a reallocated 15 char buf");
    eosio_assert(ptr1_realloc == ptr1, "should have reallocated 15 char buf as the same buf");
    eosio_assert(ptr1[14] == 0x7e, "remaining 15 chars of buf should be untouched for unchanged buf");

//与最大分配缓冲区的大小相同--特定于实现的测试（可以移除进行重构）
    ptr1_realloc = (char*)realloc(ptr1, 30);
    eosio_assert(ptr1_realloc != nullptr, "should have returned a 30 char buf");
eosio_assert(ptr1_realloc == ptr1, "should have increased the buf back to orig max"); //特定于实现的测试（可以移除进行重构）
    eosio_assert(ptr1[14] == 0x7e, "remaining 15 chars of buf should be untouched for expanded buf");

//增加超出（指示）分配空间的缓冲区
//分配了36个字符（静止）
    ptr1_realloc = (char*)realloc(ptr1, 36);
    eosio_assert(ptr1_realloc != nullptr, "should have returned a 36 char buf");
eosio_assert(ptr1_realloc == ptr1, "should have increased char buf to actual size"); //特定于实现的测试（可以移除进行重构）

//增加超出分配空间的缓冲区
    ptr1[35] = 0x7f;
//分配了44个字符-37+4加上一个额外的7可以被8整除
    ptr1_realloc = (char*)realloc(ptr1, 37);
    eosio_assert(ptr1_realloc != nullptr, "should have returned a 37 char buf");
    eosio_assert(ptr1_realloc != ptr1, "should have had to create new 37 char buf from 36 char buf");
eosio_assert(ptr2 < ptr1_realloc, "should have been created after ptr2"); //特定于实现的测试（可以移除进行重构）
    eosio_assert(ptr1_realloc[14] == 0x7e, "orig 36 char buf's content should be copied");
    eosio_assert(ptr1_realloc[35] == 0x7f, "orig 36 char buf's content should be copied");

//使用nullptr重新分配
    char* nullptr_realloc = (char*)realloc(nullptr, 50);
    eosio_assert(nullptr_realloc != nullptr, "should have returned a 50 char buf and ignored nullptr");
eosio_assert(ptr1_realloc < nullptr_realloc, "should have created after ptr1_realloc"); //特定于实现的测试（可以移除进行重构）

//使用无效的ptr重新分配
    char* invalid_ptr_realloc = (char*)realloc(nullptr_realloc + 4, 10);
    eosio_assert(invalid_ptr_realloc != nullptr, "should have returned a 10 char buf and ignored invalid ptr");
eosio_assert(nullptr_realloc < invalid_ptr_realloc, "should have created invalid_ptr_realloc after nullptr_realloc"); //特定于实现的测试（可以移除进行重构）
}

//此测试验证malloc可以分配1564k页，并将其视为一个大堆空间（如果平均时间内未调用sbrk）
void test_memory::test_memory_hunk()
{
//尝试分配最大的缓冲区，即15个连续的64K页（ptr头有4个字符的空间）
   char* ptr1 = (char*)malloc(15 * 64 * 1024 - 4);
   eosio_assert(ptr1 != nullptr, "should have allocated a ~983K char buf");
}

void test_memory::test_memory_hunks()
{
//保留784字节的初始缓冲区供以后分配（四舍五入到最接近的8字节边界，
//16个字节大于1564K页堆中剩余的字节）
   char* ptr1 = (char*)malloc(7404);
   eosio_assert(ptr1 != nullptr, "should have allocated a 7404 char buf");

   char* last_ptr = nullptr;
//96*（10*1024-15）=>15~64K页，剩余768字节缓冲区分配
   for (int i = 0; i < 96; ++i)
   {
      char* ptr2 =  (char*)malloc(10 * 1024 - 15);
      eosio_assert(ptr2 != nullptr, "should have allocated a ~10K char buf");
      if (last_ptr != nullptr)
      {
//-15发至-8发
eosio_assert(last_ptr + 10 * 1024 - 8 == ptr2, "should allocate the very next ptr"); //特定于实现的测试（可以移除进行重构）
      }

      last_ptr = ptr2;
   }

//试着分配一个比剩余缓冲区略大的缓冲区765+4轮到776轮
   char* ptr3 =  (char*)malloc(765);
   eosio_assert(ptr3 != nullptr, "should have allocated a 772 char buf");
eosio_assert(ptr1 + 7408 == ptr3, "should allocate the very next ptr after ptr1 in initial heap"); //特定于实现的测试（可以移除进行重构）

//只使用8个字符
   char* ptr4 = (char*)malloc(764);
   eosio_assert(ptr4 != nullptr, "should have allocated a 764 char buf");
eosio_assert(last_ptr + 10 * 1024 - 8 == ptr4, "should allocate the very next ptr after last_ptr at end of contiguous heap"); //特定于实现的测试（可以移除进行重构）

//用完剩下的8个字符
   char* ptr5 = (char*)malloc(4);
   eosio_assert(ptr5 != nullptr, "should have allocated a 4 char buf");
eosio_assert(ptr3 + 776 == ptr5, "should allocate the very next ptr after ptr3 in initial heap"); //特定于实现的测试（可以移除进行重构）

//没有剩余可分配的内容
   char* ptr6 = (char*)malloc(4);
   eosio_assert(ptr6 == nullptr, "should not have allocated a char buf");
}

void test_memory::test_memory_hunks_disjoint()
{
//保留8字节的初始缓冲区供以后分配
   char* ptr1 = (char*)malloc(8 * 1024 - 12);
   eosio_assert(ptr1 != nullptr, "should have allocated a 8184 char buf");

//只能为malloc多出14（64K）堆，因为对sbrk的调用将占用部分
   char* loop_ptr1[14];
//14*（64*1024-28）=>14~64K页，每页有24个字节可分配
   for (int i = 0; i < 14; ++i)
   {
//为每个请求分配一个新堆，因为sbrk调用不允许连续堆增长
      loop_ptr1[i] = (char*)malloc(64 * 1024 - 28);
      eosio_assert(loop_ptr1[i] != nullptr, "should have allocated a 64K char buf");

      eosio_assert(reinterpret_cast<int32_t>(sbrk(4)) != -1, "should be able to allocate 8 bytes");
   }

//由于SBRK调用分配了14*8个字节，第15个额外堆的大小减小了。
//将留下8个字节供以后分配（验证我们是否返回列表中）
   char* ptr2 = (char*)malloc(65412);
   eosio_assert(ptr2 != nullptr, "should have allocated a 65412 char buf");

   char* loop_ptr2[14];
   for (int i = 0; i < 14; ++i)
   {
//12个字符的缓冲区，为另一个传递保留8个字节
      loop_ptr2[i] = (char*)malloc(12);
      eosio_assert(loop_ptr2[i] != nullptr, "should have allocated a 12 char buf");
      eosio_assert(loop_ptr1[i] + 64 * 1024 - 24 == loop_ptr2[i], "loop_ptr2[i] should be very next pointer after loop_ptr1[i]");
   }

//这表明，搜索空闲指针从最后一个循环开始，以查找空闲内存，而不是从一开始
   char* ptr3 = (char*)malloc(4);
   eosio_assert(ptr3 != nullptr, "should have allocated a 4 char buf");
eosio_assert(loop_ptr2[13] + 16 == ptr3, "should allocate the very next ptr after loop_ptr2[13]"); //特定于实现的测试（可以移除进行重构）

   char* ptr4 = (char*)malloc(4);
   eosio_assert(ptr4 != nullptr, "should have allocated a 4 char buf");
eosio_assert(ptr2 + 65416 == ptr4, "should allocate the very next ptr after ptr2 in last heap"); //特定于实现的测试（可以移除进行重构）

   char* ptr5 = (char*)malloc(4);
   eosio_assert(ptr5 != nullptr, "should have allocated a 4 char buf");
eosio_assert(ptr1 + 8184 == ptr5, "should allocate the very next ptr after ptr1 in last heap"); //特定于实现的测试（可以移除进行重构）

//将耗尽剩余内存（第14堆已用完）
   char* loop_ptr3[13];
   for (int i = 0; i < 13; ++i)
   {
//使用4个字符的缓冲区
      loop_ptr3[i] = (char*)malloc(4);
      eosio_assert(loop_ptr3[i] != nullptr, "should have allocated a 4 char buf");
      eosio_assert(loop_ptr2[i] + 16 == loop_ptr3[i], "loop_ptr3[i] should be very next pointer after loop_ptr2[i]");
   }

   char* ptr6 = (char*)malloc(4);
   eosio_assert(ptr6 == nullptr, "should not have allocated a char buf");

   free(loop_ptr1[3]);
   free(loop_ptr2[3]);
   free(loop_ptr3[3]);

   char* slot3_ptr[64];
   for (int i = 0; i < 64; ++i)
   {
      slot3_ptr[i] = (char*)malloc(1020);
      eosio_assert(slot3_ptr[i] != nullptr, "should have allocated a 1020 char buf");
      if (i == 0)
         eosio_assert(loop_ptr1[3] == slot3_ptr[0], "loop_ptr1[3] should be very next pointer after slot3_ptr[0]");
      else
         eosio_assert(slot3_ptr[i - 1] + 1024 == slot3_ptr[i], "slot3_ptr[i] should be very next pointer after slot3_ptr[i-1]");
   }

   char* ptr7 = (char*)malloc(4);
   eosio_assert(ptr7 == nullptr, "should not have allocated a char buf");
}   

void test_memory::test_memset_memcpy()
{   
   char buf1[40] = {};
   char buf2[40] = {};

   verify_mem(buf1, 0, 40);
   verify_mem(buf2, 0, 40);

   memset(buf1, 0x22, 20);
   verify_mem(buf1, 0x22, 20);
   verify_mem(&buf1[20], 0, 20);

   memset(&buf2[20], 0xff, 20);
   verify_mem(buf2, 0, 20);
   verify_mem(&buf2[20], 0xff, 20);

   memcpy(&buf1[10], &buf2[10], 20);
   verify_mem(buf1, 0x22, 10);
   verify_mem(&buf1[10], 0, 10);
   verify_mem(&buf1[20], 0xff, 10);
   verify_mem(&buf1[30], 0, 10);

   memset(&buf1[1], 1, 1);
   verify_mem(buf1, 0x22, 1);
   verify_mem(&buf1[1], 1, 1);
   verify_mem(&buf1[2], 0x22, 8);

//验证相邻的非重叠缓冲区
   char buf3[50] = {};
   memset(&buf3[25], 0xee, 25);
   verify_mem(buf3, 0, 25);
   memcpy(buf3, &buf3[25], 25);
   verify_mem(buf3, 0xee, 50);

   memset(buf3, 0, 25);
   verify_mem(&buf3[25], 0xee, 25);
   memcpy(&buf3[25], buf3, 25);
   verify_mem(buf3, 0, 50);
}

void test_memory::test_memcpy_overlap_start()
{
   char buf3[99] = {};
   memset(buf3, 0xee, 50);
   memset(&buf3[50], 0xff, 49);
   memcpy(&buf3[49], buf3, 50);
}


void test_memory::test_memcpy_overlap_end()
{
   char buf3[99] = {};
   memset(buf3, 0xee, 50);
   memset(&buf3[50], 0xff, 49);
   memcpy(buf3, &buf3[49], 50);
}

void test_memory::test_memcmp()
{
   char buf1[] = "abcde";
   char buf2[] = "abcde";
   int32_t res1 = memcmp(buf1, buf2, 6);
   eosio_assert(res1 == 0, "first data should be equal to second data");

   char buf3[] = "abcde";
   char buf4[] = "fghij";
   int32_t res2 = memcmp(buf3, buf4, 6);
   eosio_assert(res2 < 0, "first data should be smaller than second data");

   char buf5[] = "fghij";
   char buf6[] = "abcde";
   int32_t res3 = memcmp(buf5, buf6, 6);
   eosio_assert(res3 > 0, "first data should be larger than second data");
}

void test_memory::test_outofbound_0()
{
memset((char *)0, 0xff, 1024 * 1024 * 1024); //大内存
}

void test_memory::test_outofbound_1()
{
memset((char *)16, 0xff, 0xffffffff); //内存环绕
}

void test_memory::test_outofbound_2()
{
    char buf[1024] = {0};
    char *ptr = (char *)malloc(1048576);
memcpy(buf, ptr, 1048576); //堆栈内存超出限制
}

void test_memory::test_outofbound_3()
{
    char *ptr = (char *)malloc(128);
memset(ptr, 0xcc, 1048576); //堆内存超出限制
}

template <typename T>
void test_memory_store() {
    T *ptr = (T *)(8192 * 1024 - 1);
    ptr[0] = (T)1;
}

template <typename T>
void test_memory_load() {
    T *ptr = (T *)(8192 * 1024 - 1);
    volatile T tmp = ptr[0];
    (void)tmp;
}

void test_memory::test_outofbound_4()
{
    test_memory_store<char>();
}
void test_memory::test_outofbound_5()
{
    test_memory_store<short>();
}
void test_memory::test_outofbound_6()
{
    test_memory_store<int>();
}
void test_memory::test_outofbound_7()
{
    test_memory_store<double>();
}
void test_memory::test_outofbound_8()
{
    test_memory_load<char>();
}
void test_memory::test_outofbound_9()
{
    test_memory_load<short>();
}
void test_memory::test_outofbound_10()
{
    test_memory_load<int>();
}
void test_memory::test_outofbound_11()
{
    test_memory_load<double>();
}

void test_memory::test_outofbound_12()
{
    volatile unsigned int a = 0xffffffff;
double *ptr = (double *)a; //与记忆包装一起储存
    ptr[0] = 1;
}

void test_memory::test_outofbound_13()
{
    volatile unsigned int a = 0xffffffff;
double *ptr = (double *)a; //使用内存环绕加载
    volatile double tmp = ptr[0];
    (void)tmp;
}
