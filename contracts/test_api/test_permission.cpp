
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
/*
 *@文件操作\test.cpp
 *@eos/license中定义的版权
 **/

#include <eosiolib/permission.h>
#include <eosiolib/db.h>

#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/compiler_builtins.h>
#include <eosiolib/serialize.hpp>
#include <eosiolib/action.hpp>

#include "test_api.hpp"

#include <limits>

struct check_auth_msg {
   account_name         account;
   permission_name      permission;
   std::vector<public_key>   pubkeys;

   EOSLIB_SERIALIZE( check_auth_msg, (account)(permission)(pubkeys)  )
};

void test_permission::check_authorization(uint64_t receiver, uint64_t code, uint64_t action) {
   (void)code;
   (void)action;
   using namespace eosio;

   auto self = receiver;
   auto params = unpack_action_data<check_auth_msg>();
   auto packed_pubkeys = pack(params.pubkeys);
   int64_t res64 = ::check_permission_authorization( params.account,
                                                     params.permission,
                                                     packed_pubkeys.data(), packed_pubkeys.size(),
                                                     (const char*)0,        0,
                                                     static_cast<uint64_t>(std::numeric_limits<int64_t>::max())
                                                   );
   /*
   uint64_t res64=（uint64_t）：检查授权（params.account，params.permission，
        （char*）params.pubkeys.data（），params.pubkeys.size（）*sizeof（public_key））；
   **/


   auto itr = db_lowerbound_i64(self, self, self, 1);
   if(itr == -1) {
      db_store_i64(self, self, self, 1, &res64, sizeof(int64_t));
   } else {
      db_update_i64(itr, self, &res64, sizeof(int64_t));
   }
}

struct test_permission_last_used_msg {
   account_name     account;
   permission_name  permission;
   int64_t          last_used_time;

   EOSLIB_SERIALIZE( test_permission_last_used_msg, (account)(permission)(last_used_time) )
};

/*d test_permission：：test_permission_last used（uint64_t/*receiver*/，uint64_t code，uint64_t action）
   （无效）代码；
   （无效）行动；
   使用名称空间eosio；

   auto params=unpack_action_data<test_permission_last_used_msg>（）；

   eosio_assert（get_permission_last_used（params.account，params.permission）==params.last_used_time，“unexpected last used permission time”）；
}

void test_permission:：test_account_creation_time（uint64_t/*接收器*/, uint64_t code, uint64_t action) {

   (void)code;
   (void)action;
   using namespace eosio;

   auto params = unpack_action_data<test_permission_last_used_msg>();

   eosio_assert( get_account_creation_time(params.account) == params.last_used_time, "unexpected account creation time" );
}
