
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <eosiolib/eosio.hpp>
using namespace eosio;

struct integration_test : public eosio::contract {
      using contract::contract;

      struct payload {
         uint64_t            key;
         vector<uint64_t>    data;

         uint64_t primary_key()const { return key; }
      };
      typedef eosio::multi_index<N(payloads), payload> payloads;

///ABI行动
      void store( account_name from,
                  account_name to,
                  uint64_t     num ) {
         require_auth( from );
         eosio_assert( is_account( to ), "to account does not exist");
         eosio_assert( num < std::numeric_limits<size_t>::max(), "num to large");
         payloads data ( _self, from );
         uint64_t key = 0;
         const uint64_t num_keys = 5;
         while (data.find( key ) != data.end()) {
            key += num_keys;
         }
         for (size_t i = 0; i < num_keys; ++i) {
            data.emplace(from, [&]( auto& g ) {
               g.key = key + i;
               g.data = vector<uint64_t>( static_cast<size_t>(num), 5);
            });
         }
      }
};

EOSIO_ABI( integration_test, (store) )
