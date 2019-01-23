
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
#include <eosio/chain/transaction.hpp>
#include <eosio/wallet_plugin/wallet_api.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <chrono>

namespace fc { class variant; }

namespace eosio {
namespace wallet {

///将钱包名称关联到钱包，并管理与每个钱包的交互。
///
///the name of the wallet is also used as part of the file name by software_wallet.请参见Wallet_Manager:：Create。
///no const方法，因为超时可能导致调用lock_all（）。
class wallet_manager {
public:
   wallet_manager();
   wallet_manager(const wallet_manager&) = delete;
   wallet_manager(wallet_manager&&) = delete;
   wallet_manager& operator=(const wallet_manager&) = delete;
   wallet_manager& operator=(wallet_manager&&) = delete;
   ~wallet_manager();

///设置钱包文件的位置路径。
///@param p覆盖默认值的路径。/钱包文件的位置。
   void set_dir(const boost::filesystem::path& p) {
      dir = p;
      initialize_lock();
   }

///设置锁定所有钱包的超时。
///if set then after t seconds of inactive then lock_all（）。
///activity定义为下面的任何wallet_manager方法调用。
   void set_timeout(const std::chrono::seconds& t);

///@请参阅wallet_manager:：set_timeout（const std:：chrono:：seconds&t）
//
   void set_timeout(int64_t secs) { set_timeout(std::chrono::seconds(secs)); }
      
///使用通过其公钥指定的私钥签署事务。
///使用chain_controller:：get_required_keys来确定txn需要哪些密钥。
///@param txn要签名的事务。
///@param键对应私钥的公钥，用于签署事务
///@param id要与之签署事务的链\u id。
///@返回已签名的txn
///@throws fc：：如果在未锁定的钱包中找不到相应的私钥，则出现异常
   chain::signed_transaction sign_transaction(const chain::signed_transaction& txn, const flat_set<public_key_type>& keys,
                                             const chain::chain_id_type& id);


///使用通过其公钥指定的私钥对摘要进行签名。
///@param消化要签名的摘要。
///@param key对应私钥的公钥，用于签署摘要
///@在摘要上返回签名
///@throws fc：：如果在未锁定的钱包中找不到相应的私钥，则出现异常
   chain::signature_type sign_digest(const chain::digest_type& digest, const public_key_type& key);

///创建新钱包。
///在文件dir/name中创建一个新钱包。钱包请参见设置_dir。
///新钱包在创建后解锁。
///@param钱包的名称和不带ext.wallet的文件名。
///@返回解锁钱包所需的明文密码。调用方负责保存密码，否则
///他们无法打开钱包。注意：不支持用户提供的密码。
///@throws fc:：exception如果名为的钱包已经存在（或文件名已经存在）
   std::string create(const std::string& name);

///打开现有钱包文件dir/name.wallet。
///注意，这不会解锁钱包，请参见钱包\u manager:：unlock。
///@param要打开的钱包文件名（减去ext.wallet）。
///@throws fc:：exception如果找不到/打开钱包文件。
   void open(const std::string& name);

///@如果钱包未锁定，返回一个附加“*”的钱包名称列表。
   std::vector<std::string> list_wallets();

///@从钱包中返回一份私人钥匙列表，前提是所述钱包的密码正确。
   map<public_key_type,private_key_type> list_keys(const string& name, const string& pw);

///@从所有未锁定的钱包中返回一组公钥，与链\控制器：：获取\所需的\密钥一起使用。
   flat_set<public_key_type> get_public_keys();

///锁定所有未锁定的钱包。
   void lock_all();

///锁定指定的钱包。
///no op如果钱包已经锁定。
///@param name要锁定的钱包的名称。
///@throws fc:：exception if wallet with name not found.
   void lock(const std::string& name);

///解锁指定的钱包。
///Wallet保持解锁状态，直到调用：：lock或程序退出。
///@param name要锁定的钱包的名称。
///@param password从：：create返回的明文密码。
///@throws fc：：如果钱包未找到或密码无效或已解锁，则出现异常。
   void unlock(const std::string& name, const std::string& password);

///将私钥导入指定的钱包。
///将WIF私钥导入指定的钱包。
///钱包必须打开并解锁。
///@param name要导入的钱包的名称。
///@param wif_key要导入的wif私钥，例如5KQWRPWDL6phxujxw37fssqz1jiwsst4cqzdeyxtp79zkvfd3
///@throws fc:：exception if wallet not found or locked.
   void import_key(const std::string& name, const std::string& wif_key);

///从指定的钱包中取出一把钥匙。
///钱包必须打开并解锁。
///@param name要从中删除密钥的钱包的名称。
///@param password从：：create返回的明文密码。
///@param key要删除的公钥，如eos6mryajqqq8ud7hvnycfnvpjqcpscn5so8bhthugyqet5gdw5cv
///@throws fc：：如果没有找到或锁定钱包或钥匙未被取下，则出现异常。
   void remove_key(const std::string& name, const std::string& password, const std::string& key);

///在指定的钱包中创建密钥。
///钱包必须打开和解锁
///@param要创建密钥的钱包的名称
//要创建的键的/@param类型
///@throws fc：：如果没有找到或锁定钱包，或钱包无法创建所述类型的密钥，则出现异常
///@返回创建的密钥的公钥
   string create_key(const std::string& name, const std::string& key_type);

///拥有钱包使用权
   void own_and_use_wallet(const string& name, std::unique_ptr<wallet_api>&& wallet);

private:
///verify未发生超时，否则重置超时。
///calls lock_all（）（如果超时时间已过）。
   void check_timeout();

private:
   using timepoint_t = std::chrono::time_point<std::chrono::system_clock>;
   std::map<std::string, std::unique_ptr<wallet_api>> wallets;
std::chrono::seconds timeout = std::chrono::seconds::max(); ///<在调用lock_all（）之前等待多长时间
mutable timepoint_t timeout_time = timepoint_t::max(); ///<何时调用lock_all（）
   boost::filesystem::path dir = ".";
   boost::filesystem::path lock_path = dir / "wallet.lock";
   std::unique_ptr<boost::interprocess::file_lock> wallet_dir_lock;

   void initialize_lock();
};

} //命名空间钱包
} //命名空间EOSIO


