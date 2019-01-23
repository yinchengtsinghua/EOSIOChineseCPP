
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

#include <eosio/wallet_plugin/wallet_manager.hpp>
#include <eosio/wallet_plugin/wallet.hpp>
#include <eosio/wallet_plugin/se_wallet.hpp>
#include <eosio/chain/exceptions.hpp>
#include <boost/algorithm/string.hpp>
namespace eosio {
namespace wallet {

constexpr auto file_ext = ".wallet";
constexpr auto password_prefix = "PW";

std::string gen_password() {
   auto key = private_key_type::generate();
   return password_prefix + string(key);

}

bool valid_filename(const string& name) {
   if (name.empty()) return false;
   if (std::find_if(name.begin(), name.end(), !boost::algorithm::is_alnum() && !boost::algorithm::is_any_of("._-")) != name.end()) return false;
   return boost::filesystem::path(name).filename().string() == name;
}

wallet_manager::wallet_manager() {
#ifdef __APPLE__
   try {
      wallets.emplace("SecureEnclave", std::make_unique<se_wallet>());
   } catch(fc::exception& ) {}
#endif
}

wallet_manager::~wallet_manager() {
//不需要，但可能会吓唬用户
   if(wallet_dir_lock)
      boost::filesystem::remove(lock_path);
}

void wallet_manager::set_timeout(const std::chrono::seconds& t) {
   timeout = t;
   auto now = std::chrono::system_clock::now();
   timeout_time = now + timeout;
   EOS_ASSERT(timeout_time >= now && timeout_time.time_since_epoch().count() > 0, invalid_lock_timeout_exception, "Overflow on timeout_time, specified ${t}, now ${now}, timeout_time ${timeout_time}",
             ("t", t.count())("now", now.time_since_epoch().count())("timeout_time", timeout_time.time_since_epoch().count()));
}

void wallet_manager::check_timeout() {
   if (timeout_time != timepoint_t::max()) {
      const auto& now = std::chrono::system_clock::now();
      if (now >= timeout_time) {
         lock_all();
      }
      timeout_time = now + timeout;
   }
}

std::string wallet_manager::create(const std::string& name) {
   check_timeout();

   EOS_ASSERT(valid_filename(name), wallet_exception, "Invalid filename, path not allowed in wallet name ${n}", ("n", name));

   auto wallet_filename = dir / (name + file_ext);

   if (fc::exists(wallet_filename)) {
      EOS_THROW(chain::wallet_exist_exception, "Wallet with name: '${n}' already exists at ${path}", ("n", name)("path",fc::path(wallet_filename)));
   }

   std::string password = gen_password();
   wallet_data d;
   auto wallet = make_unique<soft_wallet>(d);
   wallet->set_password(password);
   wallet->set_wallet_filename(wallet_filename.string());
   wallet->unlock(password);
   wallet->lock();
   wallet->unlock(password);

//在此处显式保存钱包文件，以确保它现在存在。
   wallet->save_wallet_file();

//如果我们在地图中有名字，那么删除它，因为我们希望替换下面的模板。
//如果在运行eos walletd时删除钱包文件，则可能发生这种情况。
   auto it = wallets.find(name);
   if (it != wallets.end()) {
      wallets.erase(it);
   }
   wallets.emplace(name, std::move(wallet));

   return password;
}

void wallet_manager::open(const std::string& name) {
   check_timeout();

   EOS_ASSERT(valid_filename(name), wallet_exception, "Invalid filename, path not allowed in wallet name ${n}", ("n", name));

   wallet_data d;
   auto wallet = std::make_unique<soft_wallet>(d);
   auto wallet_filename = dir / (name + file_ext);
   wallet->set_wallet_filename(wallet_filename.string());
   if (!wallet->load_wallet_file()) {
      EOS_THROW(chain::wallet_nonexistent_exception, "Unable to open file: ${f}", ("f", wallet_filename.string()));
   }

//如果我们在地图中有名字，那么删除它，因为我们希望替换下面的模板。
//如果在运行eos walletd时添加钱包文件，就会发生这种情况。
   auto it = wallets.find(name);
   if (it != wallets.end()) {
      wallets.erase(it);
   }
   wallets.emplace(name, std::move(wallet));
}

std::vector<std::string> wallet_manager::list_wallets() {
   check_timeout();
   std::vector<std::string> result;
   for (const auto& i : wallets) {
      if (i.second->is_locked()) {
         result.emplace_back(i.first);
      } else {
         result.emplace_back(i.first + " *");
      }
   }
   return result;
}

map<public_key_type,private_key_type> wallet_manager::list_keys(const string& name, const string& pw) {
   check_timeout();

   if (wallets.count(name) == 0)
      EOS_THROW(chain::wallet_nonexistent_exception, "Wallet not found: ${w}", ("w", name));
   auto& w = wallets.at(name);
   if (w->is_locked())
      EOS_THROW(chain::wallet_locked_exception, "Wallet is locked: ${w}", ("w", name));
w->check_password(pw); //密码错误时引发
   return w->list_keys();
}

flat_set<public_key_type> wallet_manager::get_public_keys() {
   check_timeout();
   EOS_ASSERT(!wallets.empty(), wallet_not_available_exception, "You don't have any wallet!");
   flat_set<public_key_type> result;
   bool is_all_wallet_locked = true;
   for (const auto& i : wallets) {
      if (!i.second->is_locked()) {
         result.merge(i.second->list_public_keys());
      }
      is_all_wallet_locked &= i.second->is_locked();
   }
   EOS_ASSERT(!is_all_wallet_locked, wallet_locked_exception, "You don't have any unlocked wallet!");
   return result;
}


void wallet_manager::lock_all() {
//没有呼叫检查超时，因为我们无论如何都锁定了
   for (auto& i : wallets) {
      if (!i.second->is_locked()) {
         i.second->lock();
      }
   }
}

void wallet_manager::lock(const std::string& name) {
   check_timeout();
   if (wallets.count(name) == 0) {
      EOS_THROW(chain::wallet_nonexistent_exception, "Wallet not found: ${w}", ("w", name));
   }
   auto& w = wallets.at(name);
   if (w->is_locked()) {
      return;
   }
   w->lock();
}

void wallet_manager::unlock(const std::string& name, const std::string& password) {
   check_timeout();
   if (wallets.count(name) == 0) {
      open( name );
   }
   auto& w = wallets.at(name);
   if (!w->is_locked()) {
      EOS_THROW(chain::wallet_unlocked_exception, "Wallet is already unlocked: ${w}", ("w", name));
      return;
   }
   w->unlock(password);
}

void wallet_manager::import_key(const std::string& name, const std::string& wif_key) {
   check_timeout();
   if (wallets.count(name) == 0) {
      EOS_THROW(chain::wallet_nonexistent_exception, "Wallet not found: ${w}", ("w", name));
   }
   auto& w = wallets.at(name);
   if (w->is_locked()) {
      EOS_THROW(chain::wallet_locked_exception, "Wallet is locked: ${w}", ("w", name));
   }
   w->import_key(wif_key);
}

void wallet_manager::remove_key(const std::string& name, const std::string& password, const std::string& key) {
   check_timeout();
   if (wallets.count(name) == 0) {
      EOS_THROW(chain::wallet_nonexistent_exception, "Wallet not found: ${w}", ("w", name));
   }
   auto& w = wallets.at(name);
   if (w->is_locked()) {
      EOS_THROW(chain::wallet_locked_exception, "Wallet is locked: ${w}", ("w", name));
   }
w->check_password(password); //密码错误时引发
   w->remove_key(key);
}

string wallet_manager::create_key(const std::string& name, const std::string& key_type) {
   check_timeout();
   if (wallets.count(name) == 0) {
      EOS_THROW(chain::wallet_nonexistent_exception, "Wallet not found: ${w}", ("w", name));
   }
   auto& w = wallets.at(name);
   if (w->is_locked()) {
      EOS_THROW(chain::wallet_locked_exception, "Wallet is locked: ${w}", ("w", name));
   }

   string upper_key_type = boost::to_upper_copy<std::string>(key_type);
   return w->create_key(upper_key_type);
}

chain::signed_transaction
wallet_manager::sign_transaction(const chain::signed_transaction& txn, const flat_set<public_key_type>& keys, const chain::chain_id_type& id) {
   check_timeout();
   chain::signed_transaction stxn(txn);

   for (const auto& pk : keys) {
      bool found = false;
      for (const auto& i : wallets) {
         if (!i.second->is_locked()) {
            optional<signature_type> sig = i.second->try_sign_digest(stxn.sig_digest(id, stxn.context_free_data), pk);
            if (sig) {
               stxn.signatures.push_back(*sig);
               found = true;
break; //内供
            }
         }
      }
      if (!found) {
         EOS_THROW(chain::wallet_missing_pub_key_exception, "Public key not found in unlocked wallets ${k}", ("k", pk));
      }
   }

   return stxn;
}

chain::signature_type
wallet_manager::sign_digest(const chain::digest_type& digest, const public_key_type& key) {
   check_timeout();

   try {
      for (const auto& i : wallets) {
         if (!i.second->is_locked()) {
            optional<signature_type> sig = i.second->try_sign_digest(digest, key);
            if (sig)
               return *sig;
         }
      }
   } FC_LOG_AND_RETHROW();

   EOS_THROW(chain::wallet_missing_pub_key_exception, "Public key not found in unlocked wallets ${k}", ("k", key));
}

void wallet_manager::own_and_use_wallet(const string& name, std::unique_ptr<wallet_api>&& wallet) {
   if(wallets.find(name) != wallets.end())
      FC_THROW("tried to use wallet name the already existed");
   wallets.emplace(name, std::move(wallet));
}

void wallet_manager::initialize_lock() {
//这在技术上有点激烈——如果同时有多个keosd在这个函数中。
//我认为在这里维护跨平台的Boost结构是一个可以接受的折衷。
   lock_path = dir / "wallet.lock";
   {
      std::ofstream x(lock_path.string());
      EOS_ASSERT(!x.fail(), wallet_exception, "Failed to open wallet lock file at ${f}", ("f", lock_path.string()));
   }
   wallet_dir_lock = std::make_unique<boost::interprocess::file_lock>(lock_path.string().c_str());
   if(!wallet_dir_lock->try_lock()) {
      wallet_dir_lock.reset();
      EOS_THROW(wallet_exception, "Failed to lock access to wallet directory; is another keosd running?");
   }
}

} //命名空间钱包
} //命名空间EOSIO
