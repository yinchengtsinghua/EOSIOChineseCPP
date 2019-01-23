
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once 
#include <eosiolib/varint.hpp>
#include <eosiolib/serialize.hpp>

namespace eosio {

   /*
   *@defgroup public key type公钥类型
   *@ingroup类型
   *@brief指定公钥类型
   *
   *@
   **/

   
   /*
    *eosio公钥
    *@brief eosio公钥
    **/

   struct public_key {
      /*
       *公钥的类型，可以是k1或r1
       *@brief公钥类型
       **/

      unsigned_int        type;

      /*
       *公钥的字节数
       *
       *@公钥的简短字节数
       **/

      std::array<char,33> data;

      friend bool operator == ( const public_key& a, const public_key& b ) {
        return std::tie(a.type,a.data) == std::tie(b.type,b.data);
      }
      friend bool operator != ( const public_key& a, const public_key& b ) {
        return std::tie(a.type,a.data) != std::tie(b.type,b.data);
      }
      EOSLIB_SERIALIZE( public_key, (type)(data) )
   };
   
}

///@公开密钥类型
