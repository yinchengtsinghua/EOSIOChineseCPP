
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <eosio/chain/webassembly/common.hpp>

static const char huge_tables_wast[] = R"=====(
(module
 (import "env" "sha256" (func $sha256 (param i32 i32 i32)))
 (table 0 anyfunc)
 (memory $0 32)
 (data (i32.const 4) "hello")
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
  (call $sha256
   (i32.const 4)
   (i32.const 5)
   (i32.const 16)
  )
 )
)
)=====";

static const char aligned_ref_wast[] = R"=====(
(module
 (import "env" "sha256" (func $sha256 (param i32 i32 i32)))
 (table 0 anyfunc)
 (memory $0 32)
 (data (i32.const 4) "hello")
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
  (call $sha256
   (i32.const 4)
   (i32.const 5)
   (i32.const 16)
  )
 )
)
)=====";

static const char aligned_const_ref_wast[] = R"=====(
(module
 (import "env" "sha256" (func $sha256 (param i32 i32 i32)))
 (import "env" "assert_sha256" (func $assert_sha256 (param i32 i32 i32)))
 (table 0 anyfunc)
 (memory $0 32)
 (data (i32.const 4) "hello")
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
  (local $3 i32)
  (call $sha256
   (i32.const 4)
   (i32.const 5)
   (i32.const 16)
  )
  (call $assert_sha256
   (i32.const 4)
   (i32.const 5)
   (i32.const 16)
  )
 )
)
)=====";

static const char misaligned_ref_wast[] = R"=====(
(module
 (import "env" "sha256" (func $sha256 (param i32 i32 i32)))
 (table 0 anyfunc)
 (memory $0 32)
 (data (i32.const 4) "hello")
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
  (call $sha256
   (i32.const 4)
   (i32.const 5)
   (i32.const 5)
  )
 )
)
)=====";

static const char misaligned_const_ref_wast[] = R"=====(
(module
 (import "env" "sha256" (func $sha256 (param i32 i32 i32)))
 (import "env" "assert_sha256" (func $assert_sha256 (param i32 i32 i32)))
 (import "env" "memmove" (func $memmove (param i32 i32 i32) (result i32)))
 (table 0 anyfunc)
 (memory $0 32)
 (data (i32.const 4) "hello")
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
  (local $3 i32)
  (call $sha256
   (i32.const 4)
   (i32.const 5)
   (i32.const 16)
  )
  (set_local $3
   (call $memmove
    (i32.const 17)
    (i32.const 16)
    (i32.const 64) 
   )
  )
  (call $assert_sha256
   (i32.const 4)
   (i32.const 5)
   (i32.const 17)
  )
 )
)
)=====";

static const char entry_wast[] = R"=====(
(module
 (import "env" "require_auth" (func $require_auth (param i64)))
 (import "env" "eosio_assert" (func $eosio_assert (param i32 i32)))
 (import "env" "current_time" (func $current_time (result i64)))
 (table 0 anyfunc)
 (memory $0 1)
 (export "memory" (memory $0))
 (export "entry" (func $entry))
 (export "apply" (func $apply))
 (func $entry
  (block
   (i64.store offset=4
    (i32.const 0)
    (call $current_time)
   )
  )
 )
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
  (block
   (call $require_auth (i64.const 6121376101093867520))
   (call $eosio_assert
    (i64.eq
     (i64.load offset=4
      (i32.const 0)
     )
     (call $current_time)
    )
    (i32.const 0)
   )
  )
 )
 (start $entry)
)
)=====";

static const char entry_wast_2[] = R"=====(
(module
 (import "env" "require_auth" (func $require_auth (param i64)))
 (import "env" "eosio_assert" (func $eosio_assert (param i32 i32)))
 (import "env" "current_time" (func $current_time (result i64)))
 (table 0 anyfunc)
 (memory $0 1)
 (export "memory" (memory $0))
 (export "apply" (func $apply))
 (start $entry)
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
  (block
   (call $require_auth (i64.const 6121376101093867520))
   (call $eosio_assert
    (i64.eq
     (i64.load offset=4
      (i32.const 0)
     )
     (call $current_time)
    )
    (i32.const 0)
   )
  )
 )
 (func $entry
  (block
   (i64.store offset=4
    (i32.const 0)
    (call $current_time)
   )
  )
 )
)
)=====";

static const char simple_no_memory_wast[] = R"=====(
(module
 (import "env" "require_auth" (func $require_auth (param i64)))
 (import "env" "memcpy" (func $memcpy (param i32 i32 i32) (result i32)))
 (table 0 anyfunc)
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
    (call $require_auth (i64.const 11323361180581363712))
    (drop
       (call $memcpy
          (i32.const 0)
          (i32.const 1024)
          (i32.const 1024)
       )
    )
 )
)
)=====";

static const char mutable_global_wast[] = R"=====(
(module
 (import "env" "require_auth" (func $require_auth (param i64)))
 (import "env" "eosio_assert" (func $eosio_assert (param i32 i32)))
 (table 0 anyfunc)
 (memory $0 1)
 (export "memory" (memory $0))
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
  (call $require_auth (i64.const 7235159549794234880))
  (if (i64.eq (get_local $2) (i64.const 0)) (then
    (set_global $g0 (i64.const 444))
    (return)
  ))
  (if (i64.eq (get_local $2) (i64.const 1)) (then
    (call $eosio_assert (i64.eq (get_global $g0) (i64.const 2)) (i32.const 0))
    (return)
  ))
  (call $eosio_assert (i32.const 0) (i32.const 0))
 )
 (global $g0 (mut i64) (i64.const 2))
)
)=====";

static const char biggest_memory_wast[] = R"=====(
(module
 (import "env" "eosio_assert" (func $$eosio_assert (param i32 i32)))
 (import "env" "require_auth" (func $$require_auth (param i64)))
 (table 0 anyfunc)
 (memory $$0 ${MAX_WASM_PAGES})
 (export "memory" (memory $$0))
 (export "apply" (func $$apply))
 (func $$apply (param $$0 i64) (param $$1 i64) (param $$2 i64)
  (call $$require_auth (i64.const 4294504710842351616))
  (call $$eosio_assert
   (i32.eq
     (grow_memory (i32.const 1))
     (i32.const -1)
   )
   (i32.const 0)
  )
 )
)
)=====";

static const char too_big_memory_wast[] = R"=====(
(module
 (table 0 anyfunc)
 (memory $$0 ${MAX_WASM_PAGES_PLUS_ONE})
 (export "memory" (memory $$0))
 (export "apply" (func $$apply))
 (func $$apply (param $$0 i64) (param $$1 i64) (param $$2 i64))
)
)=====";

static const char valid_sparse_table[] = R"=====(
(module
 (table 1024 anyfunc)
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64))
 (elem (i32.const 0) $apply)
 (elem (i32.const 1022) $apply $apply)
)
)=====";

static const char too_big_table[] = R"=====(
(module
 (table 1025 anyfunc)
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64))
 (elem (i32.const 0) $apply)
 (elem (i32.const 1022) $apply $apply)
)
)=====";

static const char memory_init_borderline[] = R"=====(
(module
 (memory $0 16)
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64))
 (data (i32.const 65532) "sup!")
)
)=====";

static const char memory_init_toolong[] = R"=====(
(module
 (memory $0 16)
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64))
 (data (i32.const 65533) "sup!")
)
)=====";

static const char memory_init_negative[] = R"=====(
(module
 (memory $0 16)
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64))
 (data (i32.const -1) "sup!")
)
)=====";

static const char memory_table_import[] = R"=====(
(module
 (table  (import "foo" "table") 10 anyfunc)
 (memory (import "nom" "memory") 0)
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64))
)
)=====";

static const char table_checker_wast[] = R"=====(
(module
 (import "env" "require_auth" (func $require_auth (param i64)))
 (import "env" "eosio_assert" (func $assert (param i32 i32)))
 (type $SIG$vj (func (param i64)))
 (table 1024 anyfunc)
 (memory $0 1)
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
   (call $require_auth (i64.const 14547189746360123392))
   (call_indirect $SIG$vj
     (i64.shr_u
       (get_local $2)
       (i64.const 32)
     )
     (i32.wrap/i64
       (get_local $2)
     )
   )
 )
 (func $apple (type $SIG$vj) (param $0 i64)
   (call $assert
     (i64.eq
       (get_local $0)
       (i64.const 555)
     )
     (i32.const 0)
   )
 )
 (func $bannna (type $SIG$vj) (param $0 i64)
   (call $assert
     (i64.eq
       (get_local $0)
       (i64.const 7777)
     )
     (i32.const 0)
   )
 )
 (elem (i32.const 0) $apple)
 (elem (i32.const 1022) $apple $bannna)
)
)=====";

static const char table_checker_proper_syntax_wast[] = R"=====(
(module
 (import "env" "require_auth" (func $require_auth (param i64)))
 (import "env" "eosio_assert" (func $assert (param i32 i32)))
 (import "env" "printi" (func $printi (param i64)))
 (type $SIG$vj (func (param i64)))
 (table 1024 anyfunc)
 (memory $0 1)
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
   (call $require_auth (i64.const 14547189746360123392))
   (call_indirect (type $SIG$vj)
     (i64.shr_u
       (get_local $2)
       (i64.const 32)
     )
     (i32.wrap/i64
       (get_local $2)
     )
   )
 )
 (func $apple (type $SIG$vj) (param $0 i64)
   (call $assert
     (i64.eq
       (get_local $0)
       (i64.const 555)
     )
     (i32.const 0)
   )
 )
 (func $bannna (type $SIG$vj) (param $0 i64)
   (call $assert
     (i64.eq
       (get_local $0)
       (i64.const 7777)
     )
     (i32.const 0)
   )
 )
 (elem (i32.const 0) $apple)
 (elem (i32.const 1022) $apple $bannna)
)
)=====";

static const char table_checker_small_wast[] = R"=====(
(module
 (import "env" "require_auth" (func $require_auth (param i64)))
 (import "env" "eosio_assert" (func $assert (param i32 i32)))
 (import "env" "printi" (func $printi (param i64)))
 (type $SIG$vj (func (param i64)))
 (table 128 anyfunc)
 (memory $0 1)
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
   (call $require_auth (i64.const 14547189746360123392))
   (call_indirect (type $SIG$vj)
     (i64.shr_u
       (get_local $2)
       (i64.const 32)
     )
     (i32.wrap/i64
       (get_local $2)
     )
   )
 )
 (func $apple (type $SIG$vj) (param $0 i64)
   (call $assert
     (i64.eq
       (get_local $0)
       (i64.const 555)
     )
     (i32.const 0)
   )
 )
 (elem (i32.const 0) $apple)
)
)=====";

static const char global_protection_none_get_wast[] = R"=====(
(module
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
   (drop (get_global 0))
 )
)
)=====";

static const char global_protection_some_get_wast[] = R"=====(
(module
 (global i32 (i32.const -11))
 (global i32 (i32.const 56))
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
   (drop (get_global 1))
   (drop (get_global 2))
 )
)
)=====";

static const char global_protection_none_set_wast[] = R"=====(
(module
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
   (set_global 0 (get_local 1))
 )
)
)=====";

static const char global_protection_some_set_wast[] = R"=====(
(module
 (global i64 (i64.const -11))
 (global i64 (i64.const 56))
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64)
   (set_global 2 (get_local 1))
 )
)
)=====";

static const std::vector<uint8_t> global_protection_okay_get_wasm{
   0x00, 'a', 's', 'm', 0x01, 0x00, 0x00, 0x00,
0x01, 0x07, 0x01, 0x60, 0x03, 0x7e, 0x7e, 0x7e, 0x00,            //包含void函数的类型节（I64、I64、I64）
0x03, 0x02, 0x01, 0x00,                                          //函数

0x06, 0x06, 0x01, 0x7f, 0x00, 0x41, 0x75, 0x0b,                  //全球的

0x07, 0x09, 0x01, 0x05, 'a', 'p', 'p', 'l', 'y', 0x00, 0x00,     //将函数0导出为“apply”
0x0a, 0x07, 0x01,                                                //代码段
0x05, 0x00,                                                      //函数体从长度5开始；无局部变量
0x23, 0x00,                                                      //获得全球0
0x1a,                                                            //滴
0x0b                                                             //结束
};

static const std::vector<uint8_t> global_protection_none_get_wasm{
   0x00, 'a', 's', 'm', 0x01, 0x00, 0x00, 0x00,
0x01, 0x07, 0x01, 0x60, 0x03, 0x7e, 0x7e, 0x7e, 0x00,            //包含void函数的类型节（I64、I64、I64）
0x03, 0x02, 0x01, 0x00,                                          //函数

0x07, 0x09, 0x01, 0x05, 'a', 'p', 'p', 'l', 'y', 0x00, 0x00,     //将函数0导出为“apply”
0x0a, 0x07, 0x01,                                                //代码段
0x05, 0x00,                                                      //函数体从长度5开始；无局部变量
0x23, 0x00,                                                      //获得全球0
0x1a,                                                            //滴
0x0b                                                             //结束
};

static const std::vector<uint8_t> global_protection_some_get_wasm{
   0x00, 'a', 's', 'm', 0x01, 0x00, 0x00, 0x00,
0x01, 0x07, 0x01, 0x60, 0x03, 0x7e, 0x7e, 0x7e, 0x00,            //包含void函数的类型节（I64、I64、I64）
0x03, 0x02, 0x01, 0x00,                                          //函数

0x06, 0x06, 0x01, 0x7f, 0x00, 0x41, 0x75, 0x0b,                  //全球的

0x07, 0x09, 0x01, 0x05, 'a', 'p', 'p', 'l', 'y', 0x00, 0x00,     //将函数0导出为“apply”
0x0a, 0x07, 0x01,                                                //代码段
0x05, 0x00,                                                      //函数体从长度5开始；无局部变量
0x23, 0x01,                                                      //获得全球1
0x1a,                                                            //滴
0x0b                                                             //结束
};

static const std::vector<uint8_t> global_protection_okay_set_wasm{
   0x00, 'a', 's', 'm', 0x01, 0x00, 0x00, 0x00,
0x01, 0x07, 0x01, 0x60, 0x03, 0x7e, 0x7e, 0x7e, 0x00,            //包含void函数的类型节（I64、I64、I64）
0x03, 0x02, 0x01, 0x00,                                          //函数

0x06, 0x06, 0x01, 0x7e, 0x01, 0x42, 0x75, 0x0b,                  //全球的。。这次是i64和全局可变

0x07, 0x09, 0x01, 0x05, 'a', 'p', 'p', 'l', 'y', 0x00, 0x00,     //将函数0导出为“apply”
0x0a, 0x08, 0x01,                                                //代码段
0x06, 0x00,                                                      //函数体从长度6开始；无局部变量
0x20, 0x00,                                                      //获得本地0
0x24, 0x00,                                                      //设置全局0
0x0b                                                             //结束
};

static const std::vector<uint8_t> global_protection_some_set_wasm{
   0x00, 'a', 's', 'm', 0x01, 0x00, 0x00, 0x00,
0x01, 0x07, 0x01, 0x60, 0x03, 0x7e, 0x7e, 0x7e, 0x00,            //包含void函数的类型节（I64、I64、I64）
0x03, 0x02, 0x01, 0x00,                                          //函数

0x06, 0x06, 0x01, 0x7e, 0x01, 0x42, 0x75, 0x0b,                  //全球的。。这次是i64和全局可变

0x07, 0x09, 0x01, 0x05, 'a', 'p', 'p', 'l', 'y', 0x00, 0x00,     //将函数0导出为“apply”
0x0a, 0x08, 0x01,                                                //代码段
0x06, 0x00,                                                      //函数体从长度6开始；无局部变量
0x20, 0x00,                                                      //获得本地0
0x24, 0x01,                                                      //设置全局1
0x0b                                                             //结束
};

static const char no_apply_wast[] = R"=====(
(module
 (func $apply (param $0 i64) (param $1 i64) (param $2 i64))
)
)=====";

static const char no_apply_2_wast[] = R"=====(
(module
  (type (;0;) (func))
  (type (;1;) (func (result i64)))
  (type (;2;) (func (param i64 i64)))
  (type (;3;) (func (param i64)))
  (type (;4;) (func (param i32 i32)))
  (type (;5;) (func (result i32)))
  (type (;6;) (func (param i32 i32) (result i32)))
  (type (;7;) (func (param i32 i32 i32) (result i32)))
  (type (;8;) (func (param i64 i64 i64)))
  (func (;0;) (type 8) (param i64 i64 i64))
  (global (;0;) f32 (f32.const -0x1.8008p+2 (;=-6.00049;)))
  (export "llp/y" (func 0)))
)=====";

static const char no_apply_3_wast[] = R"=====(
(module
  (type (;0;) (func (param i64)))
  (type (;1;) (func (param i64)))
  (type (;2;) (func (param i64)))
  (type (;3;) (func (param i64 i64 i64)))
  (global (;0;) f32 (f32.const -0x1.8008p+2 (;=-6.00049;)))
  (global (;1;) f32 (f32.const -0x1.8008p+2 (;=-6.00049;)))
  (global (;2;) f32 (f32.const -0x1.8008p+2 (;=-6.00049;)))
  (global (;3;) f32 (f32.const -0x1.8008p+2 (;=-6.00049;)))
  (global (;4;) f32 (f32.const -0x1.8008p+2 (;=-6.00049;)))
  (global (;5;) f32 (f32.const -0x1.8008p+2 (;=-6.00049;)))
  (func (;0;) (type 3) (param i64 i64 i64))
  (export "apply" (global 5))
)
)=====";

static const char apply_wrong_signature_wast[] = R"=====(
(module
 (export "apply" (func $apply))
 (func $apply (param $0 i64) (param $1 f64))
)
)=====";

static const char import_injected_wast[] =                                            \
"(module"                                                                             \
" (export \"apply\" (func $apply))"                                                   \
" (import \"" EOSIO_INJECTED_MODULE_NAME "\" \"checktime\" (func $inj (param i32)))"  \
" (func $apply (param $0 i64) (param $1 i64) (param $2 i64))"                         \
")";

static const char memory_growth_memset_store[] = R"=====(
(module
 (export "apply" (func $apply))
 (memory $0 1)
 (func $apply (param $0 i64)(param $1 i64)(param $2 i64)
    (drop (grow_memory (i32.const 2)))
    (i32.store (i32.const 80000) (i32.const 2))
    (i32.store (i32.const 140000) (i32.const 3))
 )
)
)=====";

static const char memory_growth_memset_test[] = R"=====(
(module
 (export "apply" (func $apply))
 (import "env" "eosio_assert" (func $eosio_assert (param i32 i32)))
 (memory $0 1)
 (func $apply (param $0 i64)(param $1 i64)(param $2 i64)
   (drop (grow_memory (i32.const 2)))
   (call $eosio_assert
     (i32.eq
       (i32.load offset=80000 (i32.const 0))
       (i32.const 0)
     )
     (i32.const 0)
   )
   (call $eosio_assert
     (i32.eq
       (i32.load offset=140000 (i32.const 0))
       (i32.const 0)
     )
     (i32.const 0)
   )
 )
)
)=====";
