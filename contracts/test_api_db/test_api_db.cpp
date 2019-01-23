
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

#include "test_db.cpp"

extern "C" {
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
      require_auth(code);
      WASM_TEST_HANDLER_EX(test_db, primary_i64_general);
      WASM_TEST_HANDLER_EX(test_db, primary_i64_lowerbound);
      WASM_TEST_HANDLER_EX(test_db, primary_i64_upperbound);
      WASM_TEST_HANDLER_EX(test_db, idx64_general);
      WASM_TEST_HANDLER_EX(test_db, idx64_lowerbound);
      WASM_TEST_HANDLER_EX(test_db, idx64_upperbound);
      WASM_TEST_HANDLER_EX(test_db, test_invalid_access);
      WASM_TEST_HANDLER_EX(test_db, idx_double_nan_create_fail);
      WASM_TEST_HANDLER_EX(test_db, idx_double_nan_modify_fail);
      WASM_TEST_HANDLER_EX(test_db, idx_double_nan_lookup_fail);
      WASM_TEST_HANDLER_EX(test_db, misaligned_secondary_key256_tests);

//未处理的测试调用
      eosio_assert(false, "Unknown Test");
   }

}
