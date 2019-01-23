
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <eosio.sudo/eosio.sudo.hpp>
#include <eosiolib/transaction.hpp>

namespace eosio {

/*
exec函数手动分析输入数据（而不是从调度器获取已分析的参数）
因为如果包含的事务非常大，那么在分派器中分析数据会使用太多的CPU

如果我们使用Dispatcher，函数签名应该是：

void sudo:：exec（帐户名执行器，
                 事务处理trx）
**/


void sudo::exec() {
   require_auth( _self );

   constexpr size_t max_stack_buffer_size = 512;
   size_t size = action_data_size();
   char* buffer = (char*)( max_stack_buffer_size < size ? malloc(size) : alloca(size) );
   read_action_data( buffer, size );

   account_name executer;

   datastream<const char*> ds( buffer, size );
   ds >> executer;

   require_auth( executer );

   size_t trx_pos = ds.tellp();
   send_deferred( (uint128_t(executer) << 64) | current_time(), executer, buffer+trx_pos, size-trx_pos );
}

} ///命名空间eosio

EOSIO_ABI( eosio::sudo, (exec) )
