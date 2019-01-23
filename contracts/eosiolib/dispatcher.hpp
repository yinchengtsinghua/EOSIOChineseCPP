
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <eosiolib/print.hpp>
#include <eosiolib/action.hpp>

#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/include/std_tuple.hpp>

#include <boost/mp11/tuple.hpp>
#define N(X) ::eosio::string_to_name(#X)
namespace eosio {

   template<typename Contract, typename FirstAction>
   bool dispatch( uint64_t code, uint64_t act ) {
      if( code == FirstAction::get_account() && FirstAction::get_name() == act ) {
         Contract().on( unpack_action_data<FirstAction>() );
         return true;
      }
      return false;
   }


   /*
    *此方法将动态地将传入的一组操作分派给
    *
    ＊` `
    *静态协定：：打开（actionType）
    ＊` `
    *
    *为了实现这一点，必须从eosio:：contract派生操作。
    *
    **/

   template<typename Contract, typename FirstAction, typename SecondAction, typename... Actions>
   bool dispatch( uint64_t code, uint64_t act ) {
      if( code == FirstAction::get_account() && FirstAction::get_name() == act ) {
         Contract().on( unpack_action_data<FirstAction>() );
         return true;
      }
      return eosio::dispatch<Contract,SecondAction,Actions...>( code, act );
   }

   /*
    *@defgroup调度器调度程序API
    *@brief定义了将操作分派给合同内适当的操作处理程序的函数
    *@ingroup contractdev公司
    **/

   
   /*
    *DePoxCopyService CPP调度器C++ API
    *@简要定义C++函数以将动作分发到契约内的适当动作处理程序中
    *@ingroup调度员
    *@
    **/


   /*
    *解包接收到的操作并执行相应的操作处理程序
    *
    *@brief解包接收到的操作并执行相应的操作处理程序
    *@tparam t-具有相应操作处理程序的合同类，此合同应派生自eosio:：contract
    *@tparam q-操作处理程序函数的命名空间
    *@tparam args-操作处理程序接受的参数，即操作的成员
    *@param obj-具有相应操作处理程序的协定对象
    *@param func-操作处理程序
    *@返回真
    **/

   template<typename T, typename Q, typename... Args>
   bool execute_action( T* obj, void (Q::*func)(Args...)  ) {
      size_t size = action_data_size();

//在这里使用malloc/free可能不是异常安全的，尽管wasm不支持异常
      constexpr size_t max_stack_buffer_size = 512;
      void* buffer = nullptr;
      if( size > 0 ) {
         buffer = max_stack_buffer_size < size ? malloc(size) : alloca(size);
         read_action_data( buffer, size );
      }

      auto args = unpack<std::tuple<std::decay_t<Args>...>>( (char*)buffer, size );

      if ( max_stack_buffer_size < size ) {
         free(buffer);
      }

      auto f2 = [&]( auto... a ){  
         (obj->*func)( a... ); 
      };

      boost::mp11::tuple_apply( f2, args );
      return true;
   }
///@调度员

//eosio_api的helper宏
#define EOSIO_API_CALL( r, OP, elem ) \
   case ::eosio::string_to_name( BOOST_PP_STRINGIZE(elem) ): \
      eosio::execute_action( &thiscontract, &OP::elem ); \
      break;

//eosio-abi的helper宏
#define EOSIO_API( TYPE,  MEMBERS ) \
   BOOST_PP_SEQ_FOR_EACH( EOSIO_API_CALL, TYPE, MEMBERS )

/*
 *@addtogroup调度员
 *@
 **/


/*
 *创建合同应用处理程序的便捷宏
 *要使用此宏，需要从eosio:：contract派生合同。
 *
 *@brief方便宏创建合同应用处理程序
 *@param type-合同的类名
 *@param members-此合同支持的可用操作序列
 *
 *实例：
 *@代码
 *eosio_abi（eosio:：bios，（setpriv）（setalimits）（setglmits）（setprods）（reqauth））。
 *@终结码
 **/

#define EOSIO_ABI( TYPE, MEMBERS ) \
extern "C" { \
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
      auto self = receiver; \
      if( action == N(onerror)) { \
         /*OnerRor仅在用于“eosio”代码帐户并由“eosio”的“活动权限”*/\
         eosio_assert（code==n（eosio），“onerror action's仅从\“eosio \”系统帐户中有效”）；
      }
      如果（code==self action==n（onerror））
         键入此合同（self）；\
         开关（动作）
            eosio_api（类型，成员）
         }
         /*不允许运行此合同的析构函数：eosio_exit（0）；*/ \

      } \
   } \
} \
///@调度员


   /*
   模板<typename t>
   结构调度员
      调度员（账户名称代码）：合同（代码）

      模板<typename funcptr>
      void dispatch（账户名称操作，funcptr）
      }

      T合同；
   }；

   无效调度（账户名称代码、账户名称操作，
   **/


}
