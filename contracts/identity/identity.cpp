
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "common.hpp"

#include <eosiolib/contract.hpp>
#include <eosiolib/dispatcher.hpp>
#include <eosiolib/vector.hpp>

namespace identity {
   using eosio::action_meta;
   using eosio::singleton;
   using eosio::key256;
   using std::string;
   using std::vector;

   /*
    *本合同维护了关于
    *身份。身份与帐户的概念是分开的，因为
    *将身份映射到账户需经社区协商一致。
    *
    *有些用例需要全球信任源，这种信任源于选民
    *谁选择块生产商。区块生产商的意见是“可信的”，因此
    *是区块生产商标记为“受信任”的任何人的意见。
    *
    *当一个区块生产商被淘汰时，每一个认证中的隐含信任
    *他们制造或他们信任的制造被删除。所有用户都有责任
    *伪造认证。
    *
    *帐户需要声明身份，受信任的帐户必须证明
    *索赔。
    *
    *存储标识的数据：
    *
    *deployToAccount/identity/certs/[property，trusted，certifier]=>值
    *
    *问题数据库旨在回答：
    *
    * 1。$identity.$unique已通过“可信”认证
    * 2。$identity.$property已由$account认证
    * 3。$identity.$trusted已由“受信任的”证明者验证
    * 4。什么账户有权代表身份发言？
    *对于每个受信任的所有者认证
    *查看账户是否已认领。
    *
    * 5。账户有权代表什么身份发言？
    *-检查帐户拥有自我认证所有者的身份
    *-验证受信任的证书颁发者已确认所有者
    *
    *此数据库结构支持独立身份上的并行操作。
    *
    *当一个帐户认证一个属性时，我们会检查是否
    **/

   class contract : public identity_base {
      public:

        using identity_base::identity_base;

void settrust( const account_name trustor, ///<授权信托的账户
const account_name trusting, ///<接受信托的账户
const uint8_t      trust = 0 )///0删除，-1标记不受信任，1标记受信任
         {
            require_auth( trustor );
            require_recipient( trusting );

            trust_table table( _self, trustor );
            auto itr = table.find(trusting);
            if( itr == table.end() && trust > 0 ) {
               table.emplace( trustor, [&](trustrow& row) {
                     row.account = trusting;
                  });
            } else if( itr != table.end() && trust == 0 ) {
               table.erase(itr);
            }
         }

         /*
          *此操作创建新的全局唯一64位标识符，
          *为了最小化冲突，每个帐户都会自动分配
          *基于hash（帐户名）^ hash（tapos）的32位标识前缀。
          *
          *使用这种方法，没有两个账户可能被分配相同的账户。
          *由于tapos不断变化，因此32位前缀一致。这防止
          *滥用“创建者”选择与其他用户产生故意冲突。
          *
          *创建者可以使用自己选择的算法确定最后32位。我们
          *假设创建者的算法可以避免与自身的冲突。
          *
          *即使两个帐户在前32位中发生冲突，一个正确的创建者算法
          *应在最后32位中生成随机性，以尽量减少冲突。在事件中
          *冲突事务将失败，创建者可以重试。
          *
          *使用64位标识是因为密钥的使用频率很高，它可以提供更多
          *有效的表格/范围等。
          **/

         void create( const account_name creator, const uint64_t identity ) {
            require_auth( creator );
            idents_table t( _self, _self );
            auto itr = t.find( identity );
            eosio_assert( itr == t.end(), "identity already exists" );
            eosio_assert( identity != 0, "identity=0 is not allowed" );
            t.emplace(creator, [&](identrow& i) {
                  i.identity = identity;
                  i.creator = creator;
               });
         }

void certprop( const account_name       bill_storage_to, ///<支付存储费用的账户
                               const account_name       certifier,
                               const identity_name      identity,
                               const vector<certvalue>& values )
         {
            require_auth( certifier );
            if( bill_storage_to != certifier )
               require_auth( bill_storage_to );

            idents_table t( _self, _self );
            eosio_assert( t.find( identity ) != t.end(), "identity does not exist" );

///该表存在于标识的范围内。
            certs_table certs( _self, identity );
            bool trusted = is_trusted( certifier );

            for( const auto& value : values ) {
               auto idx = certs.template get_index<N(bytuple)>();
               if (value.confidence) {
                  eosio_assert(value.type.size() <= 32, "certrow::type should be not longer than 32 bytes");
                  auto itr = idx.lower_bound( certrow::key(value.property, trusted, certifier) );

                  if (itr != idx.end() && itr->property == value.property && itr->trusted == trusted && itr->certifier == certifier) {
                     idx.modify(itr, 0, [&](certrow& row) {
                           row.confidence = value.confidence;
                           row.type       = value.type;
                           row.data       = value.data;
                        });
                  } else {
                     auto pk = certs.available_primary_key();
                     certs.emplace(_self, [&](certrow& row) {
                           row.id = pk;
                           row.property   = value.property;
                           row.trusted    = trusted;
                           row.certifier  = certifier;
                           row.confidence = value.confidence;
                           row.type       = value.type;
                           row.data       = value.data;
                        });
                  }

                  auto itr_old = idx.lower_bound( certrow::key(value.property, !trusted, certifier) );
                  if (itr_old != idx.end() && itr_old->property == value.property && itr_old->trusted == !trusted && itr_old->certifier == certifier) {
                     idx.erase(itr_old);
                  }

//业主特殊处理
                  if (value.property == N(owner)) {
                     eosio_assert(sizeof(account_name) == value.data.size(), "data size doesn't match account_name size");
                     account_name acnt = *reinterpret_cast<const account_name*>(value.data.data());
if (certifier == acnt) { //只有自我认证才会影响账户表
                        accounts_table( _self, acnt ).set( identity, acnt );
                     }
                  }
               } else {
                  bool removed = false;
                  auto itr = idx.lower_bound( certrow::key(value.property, trusted, certifier) );
                  if (itr != idx.end() && itr->property == value.property && itr->trusted == trusted && itr->certifier == certifier) {
                     idx.erase(itr);
                  } else {
                     removed = true;
                  }
                  itr = idx.lower_bound( certrow::key(value.property, !trusted, certifier) );
                  if (itr != idx.end() && itr->property == value.property && itr->trusted == !trusted && itr->certifier == certifier) {
                     idx.erase(itr);
                  } else {
                     removed = true;
                  }
//业主特殊处理
                  if (value.property == N(owner)) {
                     eosio_assert(sizeof(account_name) == value.data.size(), "data size doesn't match account_name size");
                     account_name acnt = *reinterpret_cast<const account_name*>(value.data.data());
if (certifier == acnt) { //只有自我认证才会影响账户表
                        accounts_table( _self, acnt ).remove();
                     }
                  }
               }
            }
         }
   };

} ///命名空间标识

EOSIO_ABI( identity::contract, (create)(certprop)(settrust) );
