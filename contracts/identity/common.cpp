
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "common.hpp"

#include <eosiolib/chain.h>

namespace identity {

   bool identity_base::is_trusted_by( account_name trusted, account_name by ) {
      trust_table t( _self, by );
      return t.find( trusted ) != t.end();
   }

   bool identity_base::is_trusted( account_name acnt ) {
      account_name active_producers[21];
      auto active_prod_size = get_active_producers( active_producers, sizeof(active_producers) );
      auto count = active_prod_size / sizeof(account_name);
      for( size_t i = 0; i < count; ++i ) {
         if( active_producers[i] == acnt )
            return true;
      }
      for( size_t i = 0; i < count; ++i ) {
         if( is_trusted_by( acnt, active_producers[i] ) )
            return true;
      }
      return false;
   }

}
