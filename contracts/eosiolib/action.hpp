
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
#include <eosiolib/action.h>
#include <eosiolib/datastream.hpp>
#include <eosiolib/serialize.hpp>

#include <boost/preprocessor/variadic/size.hpp>
#include <boost/preprocessor/variadic/to_tuple.hpp>
#include <boost/preprocessor/tuple/enum.hpp>
#include <boost/preprocessor/facilities/overload.hpp>

namespace eosio {

   /*
    *@ DeCopyActuppCPAPI行动C++ API
    *@ingroup操作api
    *@简要定义了用于查询动作和发送动作的类型安全C++文件
    *
    *@注意到，可以从C++直接使用的@ REF ActoCAPI中有一些方法
    *
    *@
    **/


   /*
    *
    *此方法解包T类型的当前操作。
    *
    *@brief将操作体解释为t类型。
    *@返回未打包的动作数据，转换为t。
    *
    *实例：
    *
    *@代码
    *结构虚拟动作
    *字符A；//1
    *无符号长B；//8
    *int c；//4
    *
    *eoslib-serialize（虚拟动作，（a）（b）（c））。
    *}；
    *dummy_action msg=unpack_action_data<dummy_action>（）；
    *@终结码
    **/

   template<typename T>
   T unpack_action_data() {
      constexpr size_t max_stack_buffer_size = 512;
      size_t size = action_data_size();
      const bool heap_allocation = max_stack_buffer_size < size;
      char* buffer = (char*)( heap_allocation ? malloc(size) : alloca(size) );
      read_action_data( buffer, size );
      auto res = unpack<T>( buffer, size );
//可用分配内存
      if ( heap_allocation ) {
         free(buffer);
      }
      return res;
   }

   using ::require_auth;
   using ::require_recipient;

   /*
    *所有列出的帐户都将添加到要通知的帐户集中。
    *
    *此帮助器方法允许您将多个帐户添加到要用单个帐户通知的帐户列表中。
    *不必多次调用类似的C API。
    *
    *@note action.code也被视为已通知帐户集的一部分。
    *
    *@brief通知此操作的帐户
    *@param name要通知的帐户
    *@param要通知的剩余帐户
    *
    *实例：
    *
    *@代码
    *要求收件人（n（account1），n（account2），n（account3））；//如果其中任何一个不在集合中，则抛出异常。
    *@终结码
    **/

   template<typename... accounts>
   void require_recipient( account_name name, accounts... remaining_accounts ){
      require_recipient( name );
      require_recipient( remaining_accounts... );
   }

   /*
    *权限级别的打包表示（授权）
    *
    *@权限级别的简短打包表示（授权）
    **/

   struct permission_level {
      /*
       *使用actor名称和权限名称构造新的权限级别对象
       *
       *@brief构造新的权限级别对象
       *@param a-拥有此授权的帐户的名称
       *@param p-权限名称
       **/

      permission_level( account_name a, permission_name p ):actor(a),permission(p){}

      /*
       *默认构造函数
       *
       *@brief构造新的权限级别对象
       **/

      permission_level(){}

      /*
       *拥有此权限的帐户的名称
       *
       *@拥有此权限的帐户的简要名称
       **/

      account_name    actor;
      /*
       *权限名称
       *
       *@权限的简短名称
       **/

      permission_name permission;

      /*
       *检查两个权限是否相等
       *
       *@brief检查两个权限是否相等
       *@param a-第一个要比较的权限
       *@param b-第二个比较权限
       *@等于时返回真
       *@如果不相等则返回false
       **/

      friend bool operator == ( const permission_level& a, const permission_level& b ) {
         return std::tie( a.actor, a.permission ) == std::tie( b.actor, b.permission );
      }

      EOSLIB_SERIALIZE( permission_level, (actor)(permission) )
   };

   /*
    *此操作需要指定的授权。如果此操作不包含指定的身份验证，它将失败。
    *
    *@brief需要此操作的指定授权
    *
    *@param level-需要授权
    **/

   void require_auth(const permission_level& level) {
      require_auth2( level.actor, level.permission );
   }

   /*
    *这是一个动作的打包表示，以及
    *授权级别的元数据。
    *
    *@动作的简短打包表示
    **/

   struct action {
      /*
       *操作的目标帐户名称
       *
       *@操作的目标帐户的简要名称
       **/

      account_name               account;

      /*
       *操作名称
       *
       *@动作的简要名称
       **/

      action_name                name;

      /*
       *授权此操作的权限列表
       *
       *@授权此操作的权限的简要列表
       **/

      vector<permission_level>   authorization;

      /*
       *有效载荷数据
       *
       *@brief有效载荷数据
       **/

      bytes                      data;

      /*
       *默认构造函数
       *
       *@brief构造新的操作对象
       **/

      action() = default;

      /*
       *使用给定的权限和操作结构构造新的操作对象
       *
       *@brief使用给定的权限和操作结构构造一个新的操作对象
       *@tparam action-操作结构类型
       *@param auth-授权此操作的权限
       *@param value-将通过pack into data序列化的操作结构
       **/

      template<typename Action>
      action( vector<permission_level>&& auth, const Action& value ) {
         account       = Action::get_account();
         name          = Action::get_name();
         authorization = move(auth);
         data          = pack(value);
      }

      /*
       *使用给定的权限列表和操作结构构造新的操作对象
       *
       *@brief使用给定的权限列表和操作结构构造一个新的操作对象
       *@tparam action-操作结构类型
       *@param auth-授权此操作的权限列表
       *@param value-将通过pack into data序列化的操作结构
       **/

      template<typename Action>
      action( const permission_level& auth, const Action& value )
      :authorization(1,auth) {
         account       = Action::get_account();
         name          = Action::get_name();
         data          = pack(value);
      }


      /*
       *使用给定的操作结构构造新的操作对象
       *
       *@brief用给定的action结构构造一个新的action对象
       *@tparam action-操作结构类型
       *@param value-将通过pack into data序列化的操作结构
       **/

      template<typename Action>
      action( const Action& value ) {
         account       = Action::get_account();
         name          = Action::get_name();
         data          = pack(value);
      }

      /*
       *使用给定的操作结构构造新的操作对象
       *
       *@brief用给定的权限构造一个新的action对象，action receiver，action name，action struct
       *@tparam t-操作结构的类型，必须可由“pack（…）”序列化
       *@param auth-授权此操作的权限
       *@param a-此操作的目标帐户名（操作接收器）
       *@param n-操作的名称
       *@param value-将通过pack into data序列化的操作结构
       **/

      template<typename T>
      action( const permission_level& auth, account_name a, action_name n, T&& value )
      :account(a), name(n), authorization(1,auth), data(pack(std::forward<T>(value))) {}

      /*
       *使用给定的操作结构构造新的操作对象
       *
       *@brief使用给定的权限列表、操作接收者、操作名称、操作结构构造一个新的操作对象
       *@tparam t-操作结构的类型，必须可由“pack（…）”序列化
       *@param auths-授权此操作的权限列表
       *@param a-此操作的目标帐户名（操作接收器）
       *@param n-操作的名称
       *@param value-将通过pack into data序列化的操作结构
       **/

      template<typename T>
      action( vector<permission_level> auths, account_name a, action_name n, T&& value )
      :account(a), name(n), authorization(std::move(auths)), data(pack(std::forward<T>(value))) {}

      EOSLIB_SERIALIZE( action, (account)(name)(authorization)(data) )

      /*
       *将操作作为内联操作发送
       *
       *@brief将操作作为内联操作发送
       **/

      void send() const {
         auto serialize = pack(*this);
         ::send_inline(serialize.data(), serialize.size());
      }

      /*
       *将操作作为内联上下文无关操作发送
       *
       *@brief将操作作为内联上下文无关操作发送
       *@pre此操作不应包含任何授权
       **/

      void send_context_free() const {
         eosio_assert( authorization.size() == 0, "context free actions cannot have authorizations");
         auto serialize = pack(*this);
         ::send_context_free_inline(serialize.data(), serialize.size());
      }

      /*
       *检索解包数据为t
       *
       *@brief检索解包数据为t
       *@tparam t需要数据类型
       *@返回动作数据
       **/

      template<typename T>
      T data_as() {
         eosio_assert( name == T::get_name(), "Invalid name" );
         eosio_assert( account == T::get_account(), "Invalid account" );
         return unpack<T>( &data[0], data.size() );
      }

   };

   /*
    *从中派生新定义的操作的基类，以便它可以利用调度程序
    *
    *@brief基类从中派生新定义的操作
    *@tparam account-此操作的目标帐户
    *@tparam name-操作的名称
    **/

   template<uint64_t Account, uint64_t Name>
   struct action_meta {
      /*
       *获取此操作的目标帐户
       *
       *@brief获取此操作的目标帐户
       *@return uint64使用此操作的目标帐户
       **/

      static uint64_t get_account() { return Account; }
      /*
       *获取此操作的名称
       *
       *@brief获取此操作的名称
       *@返回操作的名称
       **/

      static uint64_t get_name()  { return Name; }
   };

///@操作cpp API

   template<typename... Args>
   void dispatch_inline( account_name code, action_name act,
                         vector<permission_level> perms,
                         std::tuple<Args...> args ) {
      action( perms, code, act, std::move(args) ).send();
   }


   template<typename, uint64_t>
   struct inline_dispatcher;


   template<typename T, uint64_t Name, typename... Args>
   struct inline_dispatcher<void(T::*)(Args...), Name> {
      static void call(account_name code, const permission_level& perm, std::tuple<Args...> args) {
         dispatch_inline(code, Name, vector<permission_level>(1, perm), std::move(args));
      }
      static void call(account_name code, vector<permission_level> perms, std::tuple<Args...> args) {
         dispatch_inline(code, Name, std::move(perms), std::move(args));
      }
   };


} //命名空间EOSIO

#define INLINE_ACTION_SENDER3( CONTRACT_CLASS, FUNCTION_NAME, ACTION_NAME  )\
::eosio::inline_dispatcher<decltype(&CONTRACT_CLASS::FUNCTION_NAME), ACTION_NAME>::call

#define INLINE_ACTION_SENDER2( CONTRACT_CLASS, NAME )\
INLINE_ACTION_SENDER3( CONTRACT_CLASS, NAME, ::eosio::string_to_name(#NAME) )

#define INLINE_ACTION_SENDER(...) BOOST_PP_OVERLOAD(INLINE_ACTION_SENDER,__VA_ARGS__)(__VA_ARGS__)

/*
 *@addtogroup操作cppapi
 *集团的附加文件
 *@
 **/


/*
 *发送内联操作
 *
 *@brief发送内联操作
 *@param contract-此操作的目标帐户
 *@param name-操作的名称
 *PARAM…-指定为的操作的成员（“操作\u成员1_名称”，操作\u成员1_值）（“操作\u成员2_名称”，操作\u成员2_值）
 **/

#define SEND_INLINE_ACTION( CONTRACT, NAME, ... )\
INLINE_ACTION_SENDER(std::decay_t<decltype(CONTRACT)>, NAME)( (CONTRACT).get_self(),\
BOOST_PP_TUPLE_ENUM(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__)) );

/*
 *使用action meta扩展新定义的操作，以便它可以与调度程序一起工作。
 *
 *@brief使用action meta扩展新定义的操作
 *@param code-此操作的目标帐户
 *@param name-操作的名称
 **/

#define ACTION( CODE, NAME ) struct NAME : ::eosio::action_meta<CODE, ::eosio::string_to_name(#NAME) >

///@
