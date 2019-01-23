
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

#pragma once

#include <array>
#include <algorithm>
#include <type_traits>

#include <eosiolib/system.h>

namespace eosio {

   template<size_t Size>
   class fixed_key;

   template<size_t Size>
   bool operator==(const fixed_key<Size> &c1, const fixed_key<Size> &c2);

   template<size_t Size>
   bool operator!=(const fixed_key<Size> &c1, const fixed_key<Size> &c2);

   template<size_t Size>
   bool operator>(const fixed_key<Size> &c1, const fixed_key<Size> &c2);

   template<size_t Size>
   bool operator<(const fixed_key<Size> &c1, const fixed_key<Size> &c2);

    /*
    *@defgroup固定\键固定大小键
    *@brief固定大小键，按字典顺序为多索引表排序
    *@ingroup类型
    *@
    **/


   /*
    *多索引表按字典顺序排序的固定大小键
    *
    *@brief固定大小键，按字典顺序为多索引表排序
    *@tparam size-固定键对象的大小
    *@ingroup类型
    **/

   template<size_t Size>
   class fixed_key {
      private:

         template<bool...> struct bool_pack;
         template<bool... bs>
         using all_true = std::is_same< bool_pack<bs..., true>, bool_pack<true, bs...> >;

         template<typename Word, size_t NumWords>
         static void set_from_word_sequence(const std::array<Word, NumWords>& arr, fixed_key<Size>& key)
         {
            auto itr = key._data.begin();
            word_t temp_word = 0;
            const size_t sub_word_shift = 8 * sizeof(Word);
            const size_t num_sub_words = sizeof(word_t) / sizeof(Word);
            auto sub_words_left = num_sub_words;
            for( auto&& w : arr ) {
               if( sub_words_left > 1 ) {
                   temp_word |= static_cast<word_t>(w);
                   temp_word <<= sub_word_shift;
                   --sub_words_left;
                   continue;
               }

               eosio_assert( sub_words_left == 1, "unexpected error in fixed_key constructor" );
               temp_word |= static_cast<word_t>(w);
               sub_words_left = num_sub_words;

               *itr = temp_word;
               temp_word = 0;
               ++itr;
            }
            if( sub_words_left != num_sub_words ) {
               if( sub_words_left > 1 )
                  temp_word <<= 8 * (sub_words_left-1);
               *itr = temp_word;
            }
         }

      public:

         typedef uint128_t word_t;
         
         /*
          *获取此固定关键字对象中包含的字数。一个字的大小被定义为16个字节
          *
          *@brief获取此固定关键字对象中包含的字数
          **/


         static constexpr size_t num_words() { return (Size + sizeof(word_t) - 1) / sizeof(word_t); }

         /*
          *获取此固定\键对象中包含的填充字节数。填充字节是剩余的字节
          *分配完所有单词后，在固定键对象内
          *
          *@brief获取此固定关键字对象中包含的填充字节数
          **/

         static constexpr size_t padded_bytes() { return num_words() * sizeof(word_t) - Size; }

         /*
         *@brief固定\key对象的默认构造函数
         *
         *@详细说明将所有字节初始化为零的固定\键对象的默认构造函数
         **/

         constexpr fixed_key() : _data() {}

         /*
         *@brief构造器从std：：num_words（）words数组固定_key对象
         *
         *@details构造函数从std：：num_words（）words数组固定了_key对象
         *@param arr数据
         **/

         fixed_key(const std::array<word_t, num_words()>& arr)
         {
           std::copy(arr.begin(), arr.end(), _data.begin());
         }

         /*
         *@brief构造器从std：：num_words（）words数组固定_key对象
         *
         *@details构造函数从std：：num_words（）words数组固定了_key对象
         *@param arr-源数据
         **/

         template<typename Word, size_t NumWords,
                  typename Enable = typename std::enable_if<std::is_integral<Word>::value &&
                                                             !std::is_same<Word, bool>::value &&
                                                             sizeof(Word) < sizeof(word_t)>::type >
         fixed_key(const std::array<Word, NumWords>& arr)
         {
            static_assert( sizeof(word_t) == (sizeof(word_t)/sizeof(Word)) * sizeof(Word),
                           "size of the backing word size is not divisible by the size of the array element" );
            static_assert( sizeof(Word) * NumWords <= Size, "too many words supplied to fixed_key constructor" );

            set_from_word_sequence(arr, *this);
         }

         /*
         *@brief从一系列单词创建一个新的固定\键对象
         *
         *details从一系列单词创建一个新的固定\键对象
         *@tparam first word-序列中第一个单词的类型
         *@tparam rest-序列中剩余单词的类型
         *@param first_word-序列中的第一个词
         *@param rest-序列中的剩余单词
         **/

         template<typename FirstWord, typename... Rest>
         static
         fixed_key<Size>
         make_from_word_sequence(typename std::enable_if<std::is_integral<FirstWord>::value &&
                                                          !std::is_same<FirstWord, bool>::value &&
                                                          sizeof(FirstWord) <= sizeof(word_t) &&
                                                          all_true<(std::is_same<FirstWord, Rest>::value)...>::value,
                                                         FirstWord>::type first_word,
                                 Rest... rest)
         {
            static_assert( sizeof(word_t) == (sizeof(word_t)/sizeof(FirstWord)) * sizeof(FirstWord),
                           "size of the backing word size is not divisible by the size of the words supplied as arguments" );
            static_assert( sizeof(FirstWord) * (1 + sizeof...(Rest)) <= Size, "too many words supplied to make_from_word_sequence" );

            fixed_key<Size> key;
            set_from_word_sequence(std::array<FirstWord, 1+sizeof...(Rest)>{{ first_word, rest... }}, key);
            return key;
         }

         /*
          *获取包含的std:：array
          *@brief获取包含的std:：array
          **/

         const auto& get_array()const { return _data; }

         /*
          *获取包含的std:：array的基础数据
          *@brief获取包含的std:：array的基础数据
          **/

         auto data() { return _data.data(); }

         /*
          *获取包含的std:：array的基础数据
          *@brief获取包含的std:：array的基础数据
          **/

         auto data()const { return _data.data(); }

         /*
          *获取包含的std:：array的大小
          *@brief获取包含的std:：array的大小
          **/

         auto size()const { return _data.size(); }


         /*
          *提取包含的数据作为字节数组
          *@brief将包含的数据提取为字节数组
          *@return-以字节数组形式提取的数据
          **/

         std::array<uint8_t, Size> extract_as_byte_array()const {
            std::array<uint8_t, Size> arr;

            const size_t num_sub_words = sizeof(word_t);

            auto arr_itr  = arr.begin();
            auto data_itr = _data.begin();

            for( size_t counter = _data.size(); counter > 0; --counter, ++data_itr ) {
               size_t sub_words_left = num_sub_words;

if( counter == 1 ) { //如果数据数组中的最后一个字…
                  sub_words_left -= padded_bytes();
               }
               auto temp_word = *data_itr;
               for( ; sub_words_left > 0; --sub_words_left ) {
                  *(arr_itr + sub_words_left - 1) = static_cast<uint8_t>(temp_word & 0xFF);
                  temp_word >>= 8;
               }
               arr_itr += num_sub_words;
            }

            return arr;
         }

//比较运算符
         friend bool operator== <>(const fixed_key<Size> &c1, const fixed_key<Size> &c2);

         friend bool operator!= <>(const fixed_key<Size> &c1, const fixed_key<Size> &c2);

         friend bool operator> <>(const fixed_key<Size> &c1, const fixed_key<Size> &c2);

         friend bool operator< <>(const fixed_key<Size> &c1, const fixed_key<Size> &c2);

      private:

         std::array<word_t, num_words()> _data;
    };

   /*
    *@brief比较两个固定的关键字变量c1和c2
    *
    *@details在词典中比较了两个固定的关键字变量c1和c2
    *@param c1-要比较的第一个固定\键对象
    *@param c2-要比较的第二个固定\键对象
    *@return如果c1==c2，返回true，否则返回false
    **/

   template<size_t Size>
   bool operator==(const fixed_key<Size> &c1, const fixed_key<Size> &c2) {
      return c1._data == c2._data;
   }

   /*
    *@brief比较两个固定的关键字变量c1和c2
    *
    *@details在词典中比较了两个固定的关键字变量c1和c2
    *@param c1-要比较的第一个固定\键对象
    *@param c2-要比较的第二个固定\键对象
    *@如果是c1，返回！=c2，返回真，否则返回假
    **/

   template<size_t Size>
   bool operator!=(const fixed_key<Size> &c1, const fixed_key<Size> &c2) {
      return c1._data != c2._data;
   }

   /*
    *@brief比较两个固定的关键字变量c1和c2
    *
    *@details在词典中比较了两个固定的关键字变量c1和c2
    *@param c1-要比较的第一个固定\键对象
    *@param c2-要比较的第二个固定\键对象
    *@return如果c1>c2，返回true，否则返回false
    **/

   template<size_t Size>
   bool operator>(const fixed_key<Size>& c1, const fixed_key<Size>& c2) {
      return c1._data > c2._data;
   }

   /*
    *@brief比较两个固定的关键字变量c1和c2
    *
    *@details在词典中比较了两个固定的关键字变量c1和c2
    *@param c1-要比较的第一个固定\键对象
    *@param c2-要比较的第二个固定\键对象
    *@return如果c1<c2，返回true，否则返回false
    **/

   template<size_t Size>
   bool operator<(const fixed_key<Size> &c1, const fixed_key<Size> &c2) {
      return c1._data < c2._data;
   }
///@ }

   typedef fixed_key<32> key256;
}
