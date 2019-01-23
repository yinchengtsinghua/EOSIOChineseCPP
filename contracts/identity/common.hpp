
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include <eosiolib/singleton.hpp>
#include <eosiolib/multi_index.hpp>

namespace identity {

   typedef uint64_t identity_name;
   typedef uint64_t property_name;
   typedef uint64_t property_type_name;

   struct certvalue {
property_name     property; ///<属性名称，base32编码I64
std::string       type; ///<定义在数据中序列化的类型
std::vector<char> data; //<
std::string       memo; ///<元数据文件化认证基础
uint8_t           confidence = 1; ///<用于定义谎言责任，
//0删除

      EOSLIB_SERIALIZE( certvalue, (property)(type)(data)(memo)(confidence) )
   };

   struct certrow {
      uint64_t            id;
      property_name       property;
      uint64_t            trusted;
      account_name        certifier;
      uint8_t             confidence = 0;
      std::string         type;
      std::vector<char>   data;
      uint64_t primary_key() const { return id; }
      /*constexpr*/static eosio:：key256 key（uint64_t property，uint64_t trusted，uint64_t certifier）
         /*
           KEK256键；
           key.uint64s[0]=属性；
           key.uint64s[1]=可信；
           key.uint64s[2]=证明人；
           key.uint64s[3]=0；
         **/

         return eosio::key256::make_from_word_sequence<uint64_t>(property, trusted, certifier);
      }
      eosio::key256 get_key() const { return key(property, trusted, certifier); }

      EOSLIB_SERIALIZE( certrow , (property)(trusted)(certifier)(confidence)(type)(data)(id) )
   };

   struct identrow {
      uint64_t     identity;
      account_name creator;

      uint64_t primary_key() const { return identity; }

      EOSLIB_SERIALIZE( identrow , (identity)(creator) )
   };

   struct trustrow {
      account_name account;

      uint64_t primary_key() const { return account; }

      EOSLIB_SERIALIZE( trustrow, (account) )
   };

   typedef eosio::multi_index<N(certs), certrow,
                              eosio::indexed_by< N(bytuple), eosio::const_mem_fun<certrow, eosio::key256, &certrow::get_key> >
                              > certs_table;
   typedef eosio::multi_index<N(ident), identrow> idents_table;
   typedef eosio::singleton<N(account), identity_name>  accounts_table;
   typedef eosio::multi_index<N(trust), trustrow> trust_table;

   class identity_base {
      public:
         identity_base( account_name acnt) : _self( acnt ) {}

         bool is_trusted_by( account_name trusted, account_name by );

         bool is_trusted( account_name acnt );

      protected:
         account_name _self;
   };

}

