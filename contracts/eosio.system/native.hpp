
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

#include <eosiolib/action.hpp>
#include <eosiolib/public_key.hpp>
#include <eosiolib/types.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/privileged.h>
#include <eosiolib/optional.hpp>
#include <eosiolib/producer_schedule.hpp>
#include <eosiolib/contract.hpp>

namespace eosiosystem {
   using eosio::permission_level;
   using eosio::public_key;

   typedef std::vector<char> bytes;

   struct permission_level_weight {
      permission_level  permission;
      weight_type       weight;

//不需要显式序列化宏，此处仅用于提高编译时间
      EOSLIB_SERIALIZE( permission_level_weight, (permission)(weight) )
   };

   struct key_weight {
      public_key   key;
      weight_type  weight;

//不需要显式序列化宏，此处仅用于提高编译时间
      EOSLIB_SERIALIZE( key_weight, (key)(weight) )
   };

   struct authority {
      uint32_t                              threshold;
      uint32_t                              delay_sec;
      std::vector<key_weight>               keys;
      std::vector<permission_level_weight>  accounts;

//不需要显式序列化宏，此处仅用于提高编译时间
      EOSLIB_SERIALIZE( authority, (threshold)(delay_sec)(keys)(accounts) )
   };

   struct block_header {
      uint32_t                                  timestamp;
      account_name                              producer;
      uint16_t                                  confirmed = 0;
      block_id_type                             previous;
      checksum256                               transaction_mroot;
      checksum256                               action_mroot;
      uint32_t                                  schedule_version = 0;
      eosio::optional<eosio::producer_schedule> new_producers;

//不需要显式序列化宏，此处仅用于提高编译时间
      EOSLIB_SERIALIZE(block_header, (timestamp)(producer)(confirmed)(previous)(transaction_mroot)(action_mroot)
                                     (schedule_version)(new_producers))
   };


   /*
    *方法参数被注释掉，以防止生成解析输入数据的代码。
    **/

   class native : public eosio::contract {
      public:

         using eosio::contract::contract;

         /*
          *在创建新帐户后调用。此代码强制执行资源限制规则
          *对于新帐户和新帐户命名约定。
          *
          * 1。帐户不能包含“.”符号，该符号强制所有帐户数为12
          *字符长，不带“.”，直到实现未来的帐户拍卖过程
          *防止名字蹲下。
          *
          * 2。新帐户必须使用最少数量的令牌（如系统参数中设置的那样）
          *因此，此方法将从接收端为newacnt执行一个内联BuyRAM。
          *等于当前新账户创建费的金额。
          **/

         void newaccount( account_name     creator,
                          account_name     newact
                          /*不需要解析权限
                          施工当局和业主，
                          常量权限和活动*/ );



         /*d updateauth（/*帐户\名称帐户，
                                 权限名称权限，
                                 权限名称父级，
                                 施工权限和数据*/ ) {}


         /*d删除身份验证（/*帐户名帐户，权限名权限*/）

         void linkauth（/*帐户名帐户，
                               账户名称代码，
                               动作名称类型，
                               权限名称要求*/ ) {}


         /*取消链接身份验证（/*帐户名帐户，
                                 账户名称代码，
                                 动作名称类型*/ ) {}


         /*d取消延迟（/*权限级别取消验证，事务处理类型trx_id*/）

         void onerror（/*const bytes*/ ) {}


   };
}
