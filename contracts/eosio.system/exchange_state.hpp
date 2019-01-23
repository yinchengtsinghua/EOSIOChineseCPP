
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include <eosiolib/asset.hpp>

namespace eosiosystem {
   using eosio::asset;
   using eosio::symbol_type;

   typedef double real_type;

   /*
    *使用Bancor Math在两种资产类型之间创建50/50中继。的状态
    *Bancor Exchange完全包含在此结构中。没有外部
    *与使用此API相关的副作用。
    **/

   struct exchange_state {
      asset    supply;

      struct connector {
         asset balance;
         double weight = .5;

         EOSLIB_SERIALIZE( connector, (balance)(weight) )
      };

      connector base;
      connector quote;

      uint64_t primary_key()const { return supply.symbol; }

      asset convert_to_exchange( connector& c, asset in ); 
      asset convert_from_exchange( connector& c, asset in );
      asset convert( asset from, symbol_type to );

      EOSLIB_SERIALIZE( exchange_state, (supply)(base)(quote) )
   };

   typedef eosio::multi_index<N(rammarket), exchange_state> rammarket;

} ///namespace eosios系统
