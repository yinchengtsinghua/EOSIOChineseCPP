
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include <eosio/chain/types.hpp>
#include <eosio/wallet_plugin/wallet_api.hpp>

using namespace std;
using namespace eosio::chain;

namespace eosio { namespace wallet {

namespace detail {
struct se_wallet_impl;
}

class se_wallet final : public wallet_api {
   public:
      se_wallet();
      ~se_wallet();

      private_key_type get_private_key(public_key_type pubkey) const override;

      bool is_locked() const override;
      void lock() override;
      void unlock(string password) override;
      void check_password(string password) override;
      void set_password(string password) override;

      map<public_key_type, private_key_type> list_keys() override;
      flat_set<public_key_type> list_public_keys() override;

      bool import_key(string wif_key) override;
      string create_key(string key_type) override;
      bool remove_key(string key) override;

      optional<signature_type> try_sign_digest(const digest_type digest, const public_key_type public_key) override;

   private:
      std::unique_ptr<detail::se_wallet_impl> my;
};

}}