
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

#include <proxy/proxy.hpp>
#include <eosiolib/transaction.hpp>
#include <eosio.token/eosio.token.hpp>

namespace proxy {
   using namespace eosio;

   namespace configs {

      bool get(config &out, const account_name &self) {
         auto it = db_find_i64(self, self, N(config), config::key);
         if (it != -1) {
            auto size = db_get_i64(it, (char*)&out, sizeof(config));
            eosio_assert(size == sizeof(config), "Wrong record size");
            return true;
         } else {
            return false;
         }
      }

      void store(const config &in, const account_name &self) {
         auto it = db_find_i64(self, self, N(config), config::key);
         if (it != -1) {
            db_update_i64(it, self, (const char *)&in, sizeof(config));
         } else {
            db_store_i64(self, N(config), self, config::key, (const char *)&in, sizeof(config));
         }
      }
   };

   template<typename T>
   /*D申请转账（uint64_t receiver，account_name/*code*/，const&transfer）
      配置代码配置；
      const auto self=接收器；
      auto get_res=configs:：get（code_config，self）；
      eosioou assert（getou res，“尝试使用未配置的代理”）；
      if（transfer.from==self）
         eosio_assert（transfer.to==code_config.owner，“代理只能支付其所有者”）；
      }否则{
         eosio_断言（transfer.to==self，“代理不参与此传输”）；
         T new_transfer=T（传输）；
         new_transfer.from=自我；
         new_transfer.to=code_config.owner；

         auto id=code_config.next_id++；
         configs：：存储（code_config，self）；

         交易结束；
         out.actions.emplace_back（permission_level_self，n（active），n（eosio.token），n（transfer），new_transfer）；
         out.delay_sec=代码配置延迟；
         发送（id，self）；
      }
   }

   void apply_set owner（uint64_t receiver，set_owner params）
      const auto self=接收器；
      需要授权（params.owner）；
      配置代码配置；
      configs：：get（code_config，self）；
      code_config.owner=params.owner；
      code_config.delay=params.delay；
      eosio：：print（“设置所有者为：”，name params.owner，，“with delay：”，params.delay，“\n”）；
      configs：：存储（code_config，self）；
   }

   template<size_t…args>
   无效应用错误（uint64接收器，const onerror&error）
      eosio:：print（“starting onerror \n”）；
      const auto self=接收器；
      配置代码配置；
      eosio_assert（configs:：get（code_config，self），“尝试使用未配置的代理”）；

      auto id=code_config.next_id++；
      configs：：存储（code_config，self）；

      eosio:：print（“重新发送事务：”，error.sender_id，“as”，id，“\n”）；
      事务dtrx=error.unpack_sent_trx（）；
      dtrx.delay_sec=代码_配置延迟；
      发送（id，self）；
   }
}

使用名称空间代理；
使用名称空间eosio；

外部“C”{

    ///Apply方法实现了向此协定发送事件
    无效应用（uint64_t receiver，uint64_t code，uint64_t action）
      if（code==n（eosio）&&action==n（onerror））
         应用OnError（Receiver，OnError:：from_current_action（））；
      else if（code==n（eosio.token））
         if（action==n（transfer））
            应用传输（receiver，code，unpack_action_data<eosio:：token:：transfer_args>（））；
         }
      否则，如果（code==receiver）
         if（action==n（setowner））
            应用_set owner（receiver，unpack_action_data<set_owner>（））；
         }
      }
   }
}
