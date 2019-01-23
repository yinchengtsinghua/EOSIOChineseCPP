
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <eosiolib/eosio.hpp>
#include <eosiolib/memory.hpp>
#include "../test_api/test_api.hpp"

//使用名称空间eosio；

void verify( const void* const ptr, const uint32_t val, const uint32_t size) {
	const char* char_ptr = (const char*)ptr;
	for (uint32_t i = 0; i < size; ++i)
		eosio_assert(static_cast<uint32_t>(static_cast<unsigned char>(char_ptr[i])) == val, "buffer slot doesn't match");
}

#define PRINT_PTR(x) prints("PTR : "); print((uint32_t)x, 4); prints("\n");

void test_extended_memory::test_page_memory() {
	constexpr uint32_t _64K = 64*1024;
   /*
    *test test_extended_memory:：test_page_memory`确保初始页面大小`
    *鉴于我还没有尝试增加“程序中断”的次数，
    *当我调用sbrk（0）时，我应该得到第一页的结尾，应该是64k。
    **/

	auto prev = sbrk(0);
	eosio_assert(reinterpret_cast<uint32_t>(prev) == _64K, "Should initially have 1 64K page allocated");

   /*
    *test test_extended_memory:：test_page_memory`确保SBRK返回程序中断的前一个结束'
    *鉴于我没有试图增加记忆，
    *当我调用sbrk（1）时，我应该得到第一页的结尾，应该是64k。
    **/

	prev = sbrk(1);
	eosio_assert(reinterpret_cast<uint32_t>(prev) == _64K, "Should still be pointing to the end of the 1st 64K page");

   /*
    *test test_extended_memory:：test_page_memory`确保SBRK对齐分配'
    *假设我通过sbrk分配了1个字节，
    *当我调用sbrk（2）时，由于保持了8字节的对齐，所以应该比上一个末尾多8字节。
    **/

	prev = sbrk(2);
	eosio_assert(reinterpret_cast<uint32_t>(prev) == _64K+8, "Should point to 8 past the end of 1st 64K page");

   /*
    *test test_extended_memory:：test_page_memory`确保SBRK对齐分配2`
    *考虑到我通过sbrk分配了2个字节，
    *当我调用sbrk（64K-17）时，由于保持了8字节的对齐，所以我应该超过前一个末尾8字节。
    **/

	prev = sbrk(_64K - 17);
	eosio_assert(reinterpret_cast<uint32_t>(prev) == _64K+16, "Should point to 16 past the end of the 1st 64K page");

	prev = sbrk(1);
	eosio_assert(reinterpret_cast<uint32_t>(prev) == 2*_64K, "Should point to the end of the 2nd 64K page");

	prev = sbrk(_64K);
	eosio_assert(reinterpret_cast<uint32_t>(prev) == 2*_64K+8, "Should point to 8 past the end of the 2nd 64K page");

	prev = sbrk(_64K - 15);
	eosio_assert(reinterpret_cast<uint32_t>(prev) == 3*_64K+8, "Should point to 8 past the end of the 3rd 64K page");

	prev = sbrk(2*_64K-1);
	eosio_assert(reinterpret_cast<uint32_t>(prev) == 4*_64K, "Should point to the end of the 4th 64K page");

	prev = sbrk(2*_64K);
	eosio_assert(reinterpret_cast<uint32_t>(prev) == 6*_64K, "Should point to the end of the 6th 64K page");

	prev = sbrk(2*_64K+1);
	eosio_assert(reinterpret_cast<uint32_t>(prev) == 8*_64K, "Should point to the end of the 8th 64K page");

	prev = sbrk(6*_64K-15);
	eosio_assert(reinterpret_cast<uint32_t>(prev) == 10*_64K+8, "Should point to 8 past the end of the 10th 64K page");

	prev = sbrk(0);
	eosio_assert(reinterpret_cast<uint32_t>(prev) == 16*_64K, "Should point to 8 past the end of the 16th 64K page");
}

void test_extended_memory::test_page_memory_exceeded() {
   /*
    *test test_extended_memory:：test_page_memory_exceeded`确保SBRK分配的内存不超过1百万`
    *考虑到我没有尝试增加分配的内存，
    *当我用sbrk（15*64k）增加分配的内存时，我应该得到第一页的结尾。
    **/

	auto prev = sbrk(15*64*1024);
	eosio_assert(reinterpret_cast<uint32_t>(prev) == 64*1024, "Should have allocated 1M of memory");

   /*
    *test test_extended_memory:：test_page_memory_exceeded`确保SBRK分配的内存不会超过1米2`
    **/

   prev = sbrk(0);
	eosio_assert(reinterpret_cast<uint32_t>(prev) == (1024*1024), "Should have allocated 1M of memory");

	eosio_assert(reinterpret_cast<int32_t>(sbrk(1)) == -1, "sbrk should have failed for trying to allocate too much memory");
}

void test_extended_memory::test_page_memory_negative_bytes() {
   eosio_assert(reinterpret_cast<int32_t>(sbrk((uint32_t)-1)) == -1, "Should have errored for trying to remove memory");
}

void test_extended_memory::test_initial_buffer() {
//初始缓冲区应在8192字节时耗尽
//8176左（12+ptr头段）
	char* ptr1 = (char*)malloc(12);
	eosio_assert(ptr1 != nullptr, "should have allocated 12 char buffer");

	char* ptr2 = (char*)malloc(8159);
	eosio_assert(ptr2 != nullptr, "should have allocate 8159 char buffer");

//应该溢出初始堆，在第二个堆中分配
	char* ptr3 = (char*)malloc(20);
	eosio_assert(ptr3 != nullptr, "should have allocated a 20 char buffer");
   verify(ptr3, 0, 20);

}
