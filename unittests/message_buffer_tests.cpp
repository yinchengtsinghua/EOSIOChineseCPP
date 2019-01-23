
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


#include <fc/network/message_buffer.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>


namespace eosio {
using namespace std;

size_t mb_size(boost::asio::mutable_buffer& mb) {
#if BOOST_VERSION >= 106600
   return mb.size();
#else
   return boost::asio::detail::buffer_size_helper(mb);
#endif
}

void* mb_data(boost::asio::mutable_buffer& mb) {
#if BOOST_VERSION >= 106600
   return mb.data();
#else
   return boost::asio::detail::buffer_cast_helper(mb);
#endif
}

BOOST_AUTO_TEST_SUITE(message_buffer_tests)

constexpr auto     def_buffer_size_mb = 4;
constexpr auto     def_buffer_size = 1024*1024*def_buffer_size_mb;

///test默认构造和缓冲序列生成
BOOST_AUTO_TEST_CASE(message_buffer_construction)
{
  try {
    fc::message_buffer<def_buffer_size> mb;
    BOOST_CHECK_EQUAL(mb.total_bytes(), def_buffer_size);
    BOOST_CHECK_EQUAL(mb.bytes_to_write(), def_buffer_size);
    BOOST_CHECK_EQUAL(mb.bytes_to_read(), 0);
    BOOST_CHECK_EQUAL(mb.read_ptr(), mb.write_ptr());

    auto mbs = mb.get_buffer_sequence_for_boost_async_read();
    auto mbsi = mbs.begin();
    BOOST_CHECK_EQUAL(mb_size(*mbsi), def_buffer_size);
    BOOST_CHECK_EQUAL(mb_data(*mbsi), mb.write_ptr());
    mbsi++;
    BOOST_CHECK(mbsi == mbs.end());
  }
  FC_LOG_AND_RETHROW()
}

///测试缓冲区增长和收缩
BOOST_AUTO_TEST_CASE(message_buffer_growth)
{
  try {
    fc::message_buffer<def_buffer_size> mb;
    mb.add_buffer_to_chain();
    BOOST_CHECK_EQUAL(mb.total_bytes(), 2 * def_buffer_size);
    BOOST_CHECK_EQUAL(mb.bytes_to_write(), 2 * def_buffer_size);
    BOOST_CHECK_EQUAL(mb.bytes_to_read(), 0);
    BOOST_CHECK_EQUAL(mb.read_ptr(), mb.write_ptr());

    {
      auto mbs = mb.get_buffer_sequence_for_boost_async_read();
      auto mbsi = mbs.begin();
      BOOST_CHECK_EQUAL(mb_size(*mbsi), def_buffer_size);
      BOOST_CHECK_EQUAL(mb_data(*mbsi), mb.write_ptr());
      mbsi++;
      BOOST_CHECK(mbsi != mbs.end());
      BOOST_CHECK_EQUAL(mb_size(*mbsi), def_buffer_size);
      BOOST_CHECK_NE(mb_data(*mbsi), nullptr);
      mbsi++;
      BOOST_CHECK(mbsi == mbs.end());
    }

    mb.advance_write_ptr(100);
    BOOST_CHECK_EQUAL(mb.total_bytes(), 2 * def_buffer_size);
    BOOST_CHECK_EQUAL(mb.bytes_to_write(), 2 * def_buffer_size - 100);
    BOOST_CHECK_EQUAL(mb.bytes_to_read(), 100);
    BOOST_CHECK_NE(mb.read_ptr(), nullptr);
    BOOST_CHECK_NE(mb.write_ptr(), nullptr);
    BOOST_CHECK_EQUAL((mb.read_ptr() + 100), mb.write_ptr());

    {
      auto mbs = mb.get_buffer_sequence_for_boost_async_read();
      auto mbsi = mbs.begin();
      BOOST_CHECK_EQUAL(mb_size(*mbsi), def_buffer_size - 100);
      BOOST_CHECK_EQUAL(mb_data(*mbsi), mb.write_ptr());
      mbsi++;
      BOOST_CHECK(mbsi != mbs.end());
      BOOST_CHECK_EQUAL(mb_size(*mbsi), def_buffer_size);
      BOOST_CHECK_NE(mb_data(*mbsi), nullptr);
      mbsi++;
      BOOST_CHECK(mbsi == mbs.end());
    }

    mb.advance_read_ptr(50);
    BOOST_CHECK_EQUAL(mb.total_bytes(), 2 * def_buffer_size);
    BOOST_CHECK_EQUAL(mb.bytes_to_write(), 2 * def_buffer_size - 100);
    BOOST_CHECK_EQUAL(mb.bytes_to_read(), 50);

    mb.advance_write_ptr(def_buffer_size);
    BOOST_CHECK_EQUAL(mb.total_bytes(), 2 * def_buffer_size);
    BOOST_CHECK_EQUAL(mb.bytes_to_write(), def_buffer_size - 100);
    BOOST_CHECK_EQUAL(mb.bytes_to_read(), 50 + def_buffer_size);

//将读指针移动到第二个块应将第二个块重置为第一个块
    mb.advance_read_ptr(def_buffer_size);
    BOOST_CHECK_EQUAL(mb.total_bytes(), def_buffer_size);
    BOOST_CHECK_EQUAL(mb.bytes_to_write(), def_buffer_size - 100);
    BOOST_CHECK_EQUAL(mb.bytes_to_read(), 50);

//将读指针移动到写指针应该收缩链并重置指针
    mb.advance_read_ptr(50);
    BOOST_CHECK_EQUAL(mb.total_bytes(), def_buffer_size);
    BOOST_CHECK_EQUAL(mb.bytes_to_write(), def_buffer_size);
    BOOST_CHECK_EQUAL(mb.bytes_to_read(), 0);

    mb.add_buffer_to_chain();
    BOOST_CHECK_EQUAL(mb.total_bytes(), 2 * def_buffer_size);
    BOOST_CHECK_EQUAL(mb.bytes_to_write(), 2 * def_buffer_size);
    BOOST_CHECK_EQUAL(mb.bytes_to_read(), 0);

    mb.advance_write_ptr(50);
    BOOST_CHECK_EQUAL(mb.total_bytes(), 2 * def_buffer_size);
    BOOST_CHECK_EQUAL(mb.bytes_to_write(), 2 * def_buffer_size - 50);
    BOOST_CHECK_EQUAL(mb.bytes_to_read(), 50);

//将读指针移动到写指针应该收缩链并重置指针
    mb.advance_read_ptr(50);
    BOOST_CHECK_EQUAL(mb.total_bytes(), def_buffer_size);
    BOOST_CHECK_EQUAL(mb.bytes_to_write(), def_buffer_size);
    BOOST_CHECK_EQUAL(mb.bytes_to_read(), 0);
  }
  FC_LOG_AND_RETHROW()
}

///test在多个缓冲区中查看和读取
BOOST_AUTO_TEST_CASE(message_buffer_peek_read)
{
  try {
    {
      const uint32_t small = 32;
      fc::message_buffer<small> mb;
      BOOST_CHECK_EQUAL(mb.total_bytes(), small);
      BOOST_CHECK_EQUAL(mb.bytes_to_write(), small);
      BOOST_CHECK_EQUAL(mb.bytes_to_read(), 0);
      BOOST_CHECK_EQUAL(mb.read_ptr(), mb.write_ptr());
      BOOST_CHECK_EQUAL(mb.read_index().first, 0);
      BOOST_CHECK_EQUAL(mb.read_index().second, 0);
      BOOST_CHECK_EQUAL(mb.write_index().first, 0);
      BOOST_CHECK_EQUAL(mb.write_index().second, 0);

      mb.add_space(100 - small);
      BOOST_CHECK_EQUAL(mb.total_bytes(), 4 * small);
      BOOST_CHECK_EQUAL(mb.bytes_to_write(), 4 * small);
      BOOST_CHECK_EQUAL(mb.bytes_to_read(), 0);
      BOOST_CHECK_EQUAL(mb.read_ptr(), mb.write_ptr());

      char* write_ptr = mb.write_ptr();
      for (char ind = 0; ind < 100; ) {
        *write_ptr = ind;
        ind++;
        if (ind % small == 0) {
          mb.advance_write_ptr(small);
          write_ptr = mb.write_ptr();
        } else {
          write_ptr++;
        }
      }
      mb.advance_write_ptr(100 % small);

      BOOST_CHECK_EQUAL(mb.total_bytes(), 4 * small);
      BOOST_CHECK_EQUAL(mb.bytes_to_write(), 4 * small - 100);
      BOOST_CHECK_EQUAL(mb.bytes_to_read(), 100);
      BOOST_CHECK_NE((void*) mb.read_ptr(), (void*) mb.write_ptr());
      BOOST_CHECK_EQUAL(mb.read_index().first, 0);
      BOOST_CHECK_EQUAL(mb.read_index().second, 0);
      BOOST_CHECK_EQUAL(mb.write_index().first, 3);
      BOOST_CHECK_EQUAL(mb.write_index().second, 4);

      char buffer[100];
      auto index = mb.read_index();
      mb.peek(buffer, 50, index);
      mb.peek(buffer+50, 50, index);
      for (int i=0; i < 100; i++) {
        BOOST_CHECK_EQUAL(i, buffer[i]);
      }

      BOOST_CHECK_EQUAL(mb.total_bytes(), 4 * small);
      BOOST_CHECK_EQUAL(mb.bytes_to_write(), 4 * small - 100);
      BOOST_CHECK_EQUAL(mb.bytes_to_read(), 100);
      BOOST_CHECK_NE((void*) mb.read_ptr(), (void*) mb.write_ptr());

      char buffer2[100];
      mb.read(buffer2, 100);
      for (int i=0; i < 100; i++) {
        BOOST_CHECK_EQUAL(i, buffer2[i]);
      }

      BOOST_CHECK_EQUAL(mb.total_bytes(), small);
      BOOST_CHECK_EQUAL(mb.bytes_to_write(), small);
      BOOST_CHECK_EQUAL(mb.bytes_to_read(), 0);
      BOOST_CHECK_EQUAL(mb.read_ptr(), mb.write_ptr());
    }
  }
  FC_LOG_AND_RETHROW()
}

///在将读取指针移到末尾时测试自动分配。
BOOST_AUTO_TEST_CASE(message_buffer_write_ptr_to_end)
{
  try {
    {
      const uint32_t small = 32;
      fc::message_buffer<small> mb;
      BOOST_CHECK_EQUAL(mb.total_bytes(), small);
      BOOST_CHECK_EQUAL(mb.bytes_to_write(), small);
      BOOST_CHECK_EQUAL(mb.bytes_to_read(), 0);
      BOOST_CHECK_EQUAL(mb.read_ptr(), mb.write_ptr());
      BOOST_CHECK_EQUAL(mb.read_index().first, 0);
      BOOST_CHECK_EQUAL(mb.read_index().second, 0);
      BOOST_CHECK_EQUAL(mb.write_index().first, 0);
      BOOST_CHECK_EQUAL(mb.write_index().second, 0);

      char* write_ptr = mb.write_ptr();
      for (char ind = 0; ind < small; ind++) {
        *write_ptr = ind;
        write_ptr++;
      }
      mb.advance_write_ptr(small);

      BOOST_CHECK_EQUAL(mb.total_bytes(), 2 * small);
      BOOST_CHECK_EQUAL(mb.bytes_to_write(), small);
      BOOST_CHECK_EQUAL(mb.bytes_to_read(), small);
      BOOST_CHECK_NE((void*) mb.read_ptr(), (void*) mb.write_ptr());
      BOOST_CHECK_EQUAL(mb.read_index().first, 0);
      BOOST_CHECK_EQUAL(mb.read_index().second, 0);
      BOOST_CHECK_EQUAL(mb.write_index().first, 1);
      BOOST_CHECK_EQUAL(mb.write_index().second, 0);

      auto mbs = mb.get_buffer_sequence_for_boost_async_read();
      auto mbsi = mbs.begin();
      BOOST_CHECK_EQUAL(mb_size(*mbsi), small);
      BOOST_CHECK_EQUAL(mb_data(*mbsi), mb.write_ptr());
      BOOST_CHECK_EQUAL(mb.read_ptr()+small, mb.write_ptr());
      mbsi++;
      BOOST_CHECK(mbsi == mbs.end());
    }
  }
  FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(message_buffer_read_peek_bounds) {
   using my_message_buffer_t = fc::message_buffer<1024>;
   my_message_buffer_t mbuff;
   unsigned char stuff[] = {
      0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
   };
   memcpy(mbuff.write_ptr(), stuff, sizeof(stuff));
   mbuff.advance_write_ptr(sizeof(stuff));

   my_message_buffer_t::index_t index = mbuff.read_index();
   uint8_t throw_away_buffer[4];
mbuff.peek(&throw_away_buffer, 4, index); //剩下8个字节供以后查看
mbuff.peek(&throw_away_buffer, 4, index); //留下4个字节供以后查看
mbuff.peek(&throw_away_buffer, 2, index); //留下2个字节供以后查看
   BOOST_CHECK_THROW(mbuff.peek(&throw_away_buffer, 3, index), fc::out_of_range_exception);
mbuff.peek(&throw_away_buffer, 1, index); //留下1个字节以便事后查看
mbuff.peek(&throw_away_buffer, 0, index); //留下1个字节以便事后查看
mbuff.peek(&throw_away_buffer, 1, index); //以后再也看不到字节了
   BOOST_CHECK_THROW(mbuff.peek(&throw_away_buffer, 1, index), fc::out_of_range_exception);

mbuff.read(&throw_away_buffer, 4); //之后还有8个字节需要读取
mbuff.read(&throw_away_buffer, 4); //之后还有4个字节需要读取
mbuff.read(&throw_away_buffer, 2); //之后还有2个字节需要读取
   BOOST_CHECK_THROW(mbuff.read(&throw_away_buffer, 4), fc::out_of_range_exception);
mbuff.read(&throw_away_buffer, 1); //剩余1个字节供以后读取
mbuff.read(&throw_away_buffer, 0); //剩余1个字节供以后读取
mbuff.read(&throw_away_buffer, 1); //之后没有剩余字节可读取
   BOOST_CHECK_THROW(mbuff.read(&throw_away_buffer, 1), fc::out_of_range_exception);
}

BOOST_AUTO_TEST_CASE(message_buffer_read_peek_bounds_multi) {
   using my_message_buffer_t = fc::message_buffer<5>;
   my_message_buffer_t mbuff;
   unsigned char stuff[] = {
      0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
   };
   memcpy(mbuff.write_ptr(), stuff, 5);
   mbuff.advance_write_ptr(5);
   memcpy(mbuff.write_ptr(), stuff+5, 5);
   mbuff.advance_write_ptr(5);
   memcpy(mbuff.write_ptr(), stuff+10, 2);
   mbuff.advance_write_ptr(2);

   my_message_buffer_t::index_t index = mbuff.read_index();
   uint8_t throw_away_buffer[4];
mbuff.peek(&throw_away_buffer, 4, index); //剩下8个字节供以后查看
mbuff.peek(&throw_away_buffer, 4, index); //留下4个字节供以后查看
mbuff.peek(&throw_away_buffer, 2, index); //留下2个字节供以后查看
   BOOST_CHECK_THROW(mbuff.peek(&throw_away_buffer, 3, index), fc::out_of_range_exception);
mbuff.peek(&throw_away_buffer, 1, index); //留下1个字节供以后查看
mbuff.peek(&throw_away_buffer, 0, index); //留下1个字节供以后查看
mbuff.peek(&throw_away_buffer, 1, index); //以后再也看不到字节了
   BOOST_CHECK_THROW(mbuff.peek(&throw_away_buffer, 1, index), fc::out_of_range_exception);

mbuff.read(&throw_away_buffer, 4); //之后还有8个字节需要读取
mbuff.read(&throw_away_buffer, 4); //之后还有4个字节需要读取
mbuff.read(&throw_away_buffer, 2); //之后还有2个字节需要读取
   BOOST_CHECK_THROW(mbuff.read(&throw_away_buffer, 4), fc::out_of_range_exception);
mbuff.read(&throw_away_buffer, 1); //之后还有1个字节需要读取
mbuff.read(&throw_away_buffer, 0); //之后还有1个字节需要读取
mbuff.read(&throw_away_buffer, 1); //之后没有剩余字节可读取
   BOOST_CHECK_THROW(mbuff.read(&throw_away_buffer, 1), fc::out_of_range_exception);
}

BOOST_AUTO_TEST_SUITE_END()

} //命名空间EOSIO

