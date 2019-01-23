
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

#include <eosiolib/eosio.hpp>
#include "../test_api/test_api.hpp"

#include "test_extended_memory.cpp"
#include "test_memory.cpp"

extern "C" {
   /*D应用（uint64_t/*接收器*/，uint64_t代码，uint64_t操作）
      需要授权（代码）；

      //测试扩展内存
      wasm_test_处理程序（test_extended_memory，test_initial_buffer）；
      wasm_test_处理程序（test_extended_memory，test_page_memory）；
      wasm_test_处理程序（test_extended_memory，test_page_memory_exceeded）；
      wasm_test_处理程序（test_extended_memory，test_page_memory_negative_bytes）；

      //测试内存
      wasm-test-handler（测试内存，测试内存分配）；
      wasm_test_handler（test_memory，test_memory_hunk）；
      wasm-test-handler（测试内存，测试内存块）；
      wasm_test_handler（测试内存，测试内存Hunks_disjoint）；
      wasm_test_处理程序（test_memory，test_memset_memcpy）；
      wasm_test_处理程序（test_memory，test_memcpy_overlap_start）；
      wasm_test_处理程序（test_memory，test_memcpy_overlap_end）；
      wasm_test_处理程序（test_memory，test_memcmp）；
      wasm_test_处理程序（test_memory，test_outofbound_0）；
      wasm_test_处理程序（test_memory，test_outofbound_1）；
      wasm_test_处理程序（test_memory，test_outofbound_2）；
      wasm_test_处理程序（test_memory，test_outofbound_3）；
      wasm_test_处理程序（test_memory，test_outofbound_4）；
      wasm_test_处理程序（test_memory，test_outofbound_5）；
      wasm_test_处理程序（test_memory，test_outofbound_6）；
      wasm_test_处理程序（test_memory，test_outofbound_7）；
      wasm_test_处理程序（test_memory，test_outofbound_8）；
      wasm_test_处理程序（test_memory，test_outofbound_9）；
      wasm_test_处理程序（test_memory，test_outofbound_10）；
      wasm_test_处理程序（test_memory，test_outofbound_11）；
      wasm_test_处理程序（test_memory，test_outofbound_12）；
      wasm_test_处理程序（test_memory，test_outofbound_13）；
      
      //未处理的测试调用
      eosio_断言（错误，“未知测试”）；
   }

}
