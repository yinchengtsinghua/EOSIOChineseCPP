
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

#include <eosio/chain/types.hpp>
#include <eosio/wallet_plugin/wallet_api.hpp>

#include <fc/real128.hpp>
#include <fc/crypto/base58.hpp>

using namespace std;
using namespace eosio::chain;

namespace eosio { namespace wallet {

typedef uint16_t transaction_handle_type;

struct wallet_data
{
   /*tor<char>cipher_keys；/**加密密钥*/
}；

命名空间详细信息
类软钱包；
}

/**
 *此钱包假设它以高带宽、低延迟连接和
 *执行最小缓存。
 **/

class soft_wallet final : public wallet_api
{
   public:
      soft_wallet( const wallet_data& initial_data );

      ~soft_wallet();

      bool copy_wallet_file( string destination_filename );

      /*返回当前钱包文件名。
       *
       *这是自动保存钱包时使用的文件名。
       *
       *@请参见设置\u钱包\u文件名（）
       *@返回钱包文件名
       **/

      string                            get_wallet_filename() const;

      /*
       *获取与公钥对应的WIF私钥。这个
       *私钥必须已经在钱包中。
       **/

      private_key_type get_private_key( public_key_type pubkey )const override;

      /*
       *@param role-active owner过账备忘录
       **/

      pair<public_key_type,private_key_type>  get_private_key_from_password( string account, string role, string password )const;

      /*检查钱包是否刚刚创建，并且尚未设置密码。
       *
       *调用\c set \u密码将钱包转换为锁定状态。
       *@如果钱包是新的，则返回“真”
       *InGroup钱包管理
       **/

      bool    is_new()const;

      /*检查钱包是否上锁（无法使用其私人钥匙）。
       *
       *此状态可以通过调用\c lock（）或\c unlock（）来更改。
       *@如果钱包被锁定，返回“真”
       *InGroup钱包管理
       **/

      bool    is_locked()const override;

      /*立即锁上钱包。
       *InGroup钱包管理
       **/

      void    lock() override;

      /*打开钱包。
       *
       *钱包保持解锁状态，直到调用\c锁
       *或程序退出。
       *@param password以前用\c set_password（）设置的密码
       *InGroup钱包管理
       **/

      void    unlock(string password) override;

      /*检查钱包密码
       *
       *验证钱包上的密码，即使钱包已解锁，
       *如果密码错误，则抛出。
       *@param password以前用\c set_password（）设置的密码
       *InGroup钱包管理
       **/

      void    check_password(string password) override;

      /*在钱包上设置新密码。
       *
       *钱包必须是“新的”或“解锁的”
       *执行此命令。
       *InGroup钱包管理
       **/

      void    set_password(string password) override;

      /*把钱包里所有的私人钥匙都扔掉。
       *
       *按键以WIF格式打印。你可以把这些钥匙导入另一个钱包
       *使用\c import_key（）。
       *@返回一个包含私钥的映射，由其公钥索引
       **/

      map<public_key_type, private_key_type> list_keys() override;
      
      /*把钱包里所有的公共钥匙都扔掉。
       *@返回包含公钥的向量
       **/

      flat_set<public_key_type> list_public_keys() override;

      /*装入指定的石墨烯钱包。
       *
       *当前钱包在加载新钱包之前关闭。
       *
       *@警告这不会更改将来使用的文件名
       *钱包会写，因此这可能会导致您改写原始文件。
       *钱包，除非您还调用\c set_wallet_filename（）。
       *
       *@param wallet_filename要加载的wallet json文件的文件名。
       *如果\c wallet\u文件名为空，则重新加载
       *现有钱包文件
       *@如果指定的钱包已加载，则返回“真”
       **/

      bool    load_wallet_file(string wallet_filename = "");

      /*将当前钱包保存到给定的文件名。
       *
       *@警告这不会更改将来使用的钱包文件名
       *写入，因此将此函数视为“将副本另存为…”，而不是
       *“另存为…”。使用\c set \u wallet \u filename（）生成文件名
       *坚持。
       *@param wallet_filename要创建的新wallet json文件的文件名
       *或覆盖。如果\c wallet\u文件名为空，
       *保存到当前文件名。
       **/

      void    save_wallet_file(string wallet_filename = "");

      /*设置用于将来写入的钱包文件名。
       *
       *这不会触发保存，只会更改默认文件名。
       *这将在下次触发保存时使用。
       *
       *@param wallet_filename用于将来保存的新文件名
       **/

      void    set_wallet_filename(string wallet_filename);

      /*将WIF私钥导入钱包，用于通过帐户签署交易。
       *
       *示例：导入键5KQWRPWDL6phxujxw37fssqz1jiwsst4cqzdeyxtp79zkvfd3
       *
       *@param wif_key要导入的wif私钥
       **/

      bool import_key( string wif_key ) override;

      /*从钱包中取出钥匙。
       *
       *示例：删除键eos6mryajqq8ud7hvnycfnvpjqcpscn5so8bhthugyqet5gdw5cv
       *
       *@param key要删除的公钥
       **/

      bool remove_key( string key ) override;

       /*在钱包中创建一个密钥，用于对帐户的交易进行签名。
       *
       *示例：创建键K1
       *
       *@param key_键入要创建的密钥类型。可能是空的，以允许钱包选择合适的/最佳的密钥类型
       **/

      string create_key( string key_type ) override;

      /*尝试通过给定的公钥签署摘要
      **/

      optional<signature_type> try_sign_digest( const digest_type digest, const public_key_type public_key ) override;

      std::shared_ptr<detail::soft_wallet_impl> my;
      void encrypt_keys();
};

struct plain_keys {
   fc::sha512                            checksum;
   map<public_key_type,private_key_type> keys;
};

} }

FC_REFLECT( eosio::wallet::wallet_data, (cipher_keys) )

FC_REFLECT( eosio::wallet::plain_keys, (checksum)(keys) )
