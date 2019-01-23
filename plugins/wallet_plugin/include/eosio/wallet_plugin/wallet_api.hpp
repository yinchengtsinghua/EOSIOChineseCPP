
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
#include <eosio/chain/transaction.hpp>

using namespace std;
using namespace eosio::chain;

namespace eosio { namespace wallet {

class wallet_api
{
   public:
      virtual ~wallet_api() {}

      /*
       *获取对应于公钥的私钥。这个
       *私钥必须已经在钱包中。
       **/

      virtual private_key_type get_private_key( public_key_type pubkey ) const = 0;

      /*检查钱包是否上锁（无法使用其私人钥匙）。
       *
       *此状态可以通过调用\c lock（）或\c unlock（）来更改。
       *@如果钱包被锁定，返回“真”
       *InGroup钱包管理
       **/

      virtual bool    is_locked() const = 0;

      /*立即锁上钱包
       *InGroup钱包管理
       **/

      virtual void    lock() = 0;

      /*打开钱包。
       *
       *钱包保持解锁状态，直到调用\c锁
       *或程序退出。
       *@param password以前用\c set_password（）设置的密码
       *InGroup钱包管理
       **/

      virtual void    unlock(string password) = 0;

      /*检查钱包密码
       *
       *验证钱包上的密码，即使钱包已解锁，
       *如果密码错误，则抛出。
       *@param password以前用\c set_password（）设置的密码
       *InGroup钱包管理
       **/

      virtual void    check_password(string password) = 0;

      /*在钱包上设置新密码。
       *
       *钱包必须是“新的”或“解锁的”
       *执行此命令。
       *InGroup钱包管理
       **/

      virtual void    set_password(string password) = 0;

      /*把钱包里所有的私人钥匙都扔掉。
       *
       *按键以WIF格式打印。你可以把这些钥匙导入另一个钱包
       *使用\c import_key（）。
       *@返回一个包含私钥的映射，由其公钥索引
       **/

      virtual map<public_key_type, private_key_type> list_keys() = 0;

      /*把钱包里所有的公共钥匙都扔掉。
       *@返回包含公钥的向量
       **/

      virtual flat_set<public_key_type> list_public_keys() = 0;

      /*将WIF私钥导入钱包，用于通过帐户签署交易。
       *
       *示例：导入键5KQWRPWDL6phxujxw37fssqz1jiwsst4cqzdeyxtp79zkvfd3
       *
       *@param wif_key要导入的wif私钥
       **/

      virtual bool import_key( string wif_key ) = 0;

      /*从钱包中取出钥匙。
       *
       *示例：删除键eos6mryajqq8ud7hvnycfnvpjqcpscn5so8bhthugyqet5gdw5cv
       *
       *@param key要删除的公钥
       **/

      virtual bool remove_key( string key ) = 0;

       /*在钱包中创建一个密钥，用于对帐户的交易进行签名。
       *
       *示例：创建键K1
       *
       *@param key_键入要创建的密钥类型。可能是空的，以允许钱包选择合适的/最佳的密钥类型
       **/

      virtual string create_key( string key_type ) = 0;

      /*返回给定摘要和公钥的签名（如果此钱包可以通过该公钥签名）
       **/

      virtual optional<signature_type> try_sign_digest( const digest_type digest, const public_key_type public_key ) = 0;
};

}}
