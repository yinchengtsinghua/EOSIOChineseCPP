
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include <functional>
#include <vector>

namespace IR {
   struct Module;
}

namespace eosio { namespace chain { namespace wasm_constraints {
constexpr unsigned maximum_linear_memory      = 33*1024*1024;//字节
constexpr unsigned maximum_mutable_globals    = 1024;        //字节
constexpr unsigned maximum_table_elements     = 1024;        //元素
constexpr unsigned maximum_section_elements   = 1024;        //元素
constexpr unsigned maximum_linear_memory_init = 64*1024;     //字节
constexpr unsigned maximum_func_local_bytes   = 8192;        //字节
constexpr unsigned maximum_call_depth         = 250;         //嵌套调用
   constexpr unsigned maximum_code_size          = 20*1024*1024; 

   static constexpr unsigned wasm_page_size      = 64*1024;

   static_assert(maximum_linear_memory%wasm_page_size      == 0, "maximum_linear_memory must be mulitple of wasm page size");
   static_assert(maximum_mutable_globals%4                 == 0, "maximum_mutable_globals must be mulitple of 4");
   static_assert(maximum_table_elements*8%4096             == 0, "maximum_table_elements*8 must be mulitple of 4096");
   static_assert(maximum_linear_memory_init%wasm_page_size == 0, "maximum_linear_memory_init must be mulitple of wasm page size");
   static_assert(maximum_func_local_bytes%8                == 0, "maximum_func_local_bytes must be mulitple of 8");
   static_assert(maximum_func_local_bytes>32                   , "maximum_func_local_bytes must be greater than 32");
} //命名空间wasm_约束

}} //命名空间eosio，链
