
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/subseq.hpp>
#include <boost/preprocessor/seq/remove.hpp>
#include <boost/preprocessor/seq/push_back.hpp>
#include <fc/reflect/reflect.hpp>
#include <eosio/chain/exceptions.hpp>

#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <fc/optional.hpp>
#include <fc/exception/exception.hpp>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "IR/Operators.h"
#include "IR/Module.h"

namespace eosio { namespace chain { namespace wasm_ops {

class instruction_stream {
   public:
      instruction_stream(size_t size) : idx(0) {
         data.resize(size);
      }
      void operator<< (const char c) { 
         if (idx >= data.size())
            data.resize(data.size()*2);
         data[idx++] = static_cast<U8>(c);
      }
      void set(size_t i, const char* arr) {
         if (i+idx >= data.size())
            data.resize(data.size()*2+i);
         memcpy((char*)&data[idx], arr, i);
         idx += i;
      }
      size_t get_index() { return idx; }
      std::vector<U8> get() {
         std::vector<U8> ret = data;
         ret.resize(idx);
         return ret;
      }
//私人：
      size_t idx;
      std::vector<U8> data;
};

//远期申报
struct instr;
using namespace fc;
using wasm_op_ptr   = std::unique_ptr<instr>;
using wasm_instr_ptr      = std::shared_ptr<instr>;
using wasm_return_t       = std::vector<uint8_t>; 
using wasm_instr_callback = std::function<std::vector<wasm_instr_ptr>(uint8_t)>;
using code_vector         = std::vector<uint8_t>;
using code_iterator       = std::vector<uint8_t>::iterator;
using wasm_op_generator   = std::function<wasm_instr_ptr(std::vector<uint8_t>, size_t)>;

#pragma pack (push)
struct memarg {
uint32_t a;   //排列
uint32_t o;   //抵消
};

struct blocktype {
uint8_t result = 0x40; //空（0x40）或valtype
};

struct memoryoptype {
   uint8_t end = 0x00;
};
struct branchtabletype {
   uint64_t target_depth;
   uint64_t table_index;
};

//用于无字段的指令
struct voidtype {};


inline std::string to_string( uint32_t field ) {
   return std::string("i32 : ")+std::to_string(field);
}
inline std::string to_string( uint64_t field ) {
   return std::string("i64 : ")+std::to_string(field);
}
inline std::string to_string( blocktype field ) {
   return std::string("blocktype : ")+std::to_string((uint32_t)field.result);
}
inline std::string to_string( memoryoptype field ) {
   return std::string("memoryoptype : ")+std::to_string((uint32_t)field.end);
}
inline std::string to_string( memarg field ) {
   return std::string("memarg : ")+std::to_string(field.a)+std::string(", ")+std::to_string(field.o);
}
inline std::string to_string( branchtabletype field ) {
   return std::string("branchtabletype : ")+std::to_string(field.target_depth)+std::string(", ")+std::to_string(field.table_index);
}

inline void pack( instruction_stream* stream, uint32_t field ) {
   const char packed[] = { char(field), char(field >> 8), char(field >> 16), char(field >> 24) };
   stream->set(sizeof(packed), packed);
}
inline void pack( instruction_stream* stream, uint64_t field ) {
   const char packed[] = { char(field), char(field >> 8), char(field >> 16), char(field >> 24), 
                           char(field >> 32), char(field >> 40), char(field >> 48), char(field >> 56) };
   stream->set(sizeof(packed), packed);
}
inline void pack( instruction_stream* stream, blocktype field ) {
   const char packed[] = { char(field.result) };
   stream->set(sizeof(packed), packed);
}
inline void pack( instruction_stream* stream,  memoryoptype field ) {
   const char packed[] = { char(field.end) };
   stream->set(sizeof(packed), packed);
}
inline void pack( instruction_stream* stream, memarg field ) {
   const char packed[] = { char(field.a), char(field.a >> 8), char(field.a >> 16), char(field.a >> 24), 
                           char(field.o), char(field.o >> 8), char(field.o >> 16), char(field.o >> 24)};
   stream->set(sizeof(packed), packed);

}
inline void pack( instruction_stream* stream, branchtabletype field ) {
   const char packed[] = { char(field.target_depth), char(field.target_depth >> 8), char(field.target_depth >> 16), char(field.target_depth >> 24), 
            char(field.target_depth >> 32), char(field.target_depth >> 40), char(field.target_depth >> 48), char(field.target_depth >> 56), 
            char(field.table_index), char(field.table_index >> 8), char(field.table_index >> 16), char(field.table_index >> 24), 
            char(field.table_index >> 32), char(field.table_index >> 40), char(field.table_index >> 48), char(field.table_index >> 56) };
   stream->set(sizeof(packed), packed);
}

template <typename Field>
struct field_specific_params {
   static constexpr int skip_ahead = sizeof(uint16_t) + sizeof(Field);
   static auto unpack( char* opcode, Field& f ) { f = *reinterpret_cast<Field*>(opcode); }
   static void pack(instruction_stream* stream, Field& f) { return eosio::chain::wasm_ops::pack(stream, f); }
   static auto to_string(Field& f) { return std::string(" ")+
                                       eosio::chain::wasm_ops::to_string(f); }
};
template <>
struct field_specific_params<voidtype> {
   static constexpr int skip_ahead = sizeof(uint16_t);
   static auto unpack( char* opcode, voidtype& f ) {}
   static void pack(instruction_stream* stream, voidtype& f) {}
   static auto to_string(voidtype& f) { return ""; }
}; 

#define CONSTRUCT_OP_HAS_DATA( r, DATA, OP )                                                        \
template <typename ... Mutators>                                                                    \
struct OP final : instr_base<Mutators...> {                                                         \
   uint16_t code = BOOST_PP_CAT(OP,_code);                                                          \
   DATA field;                                                                                      \
   uint16_t get_code() override { return BOOST_PP_CAT(OP,_code); }                                  \
   int skip_ahead() override { return field_specific_params<DATA>::skip_ahead; }                    \
   void unpack( char* opcode ) override {                                                           \
      field_specific_params<DATA>::unpack( opcode, field );                                         \
   }                                                                                                \
   void pack(instruction_stream* stream) override {                                                 \
      stream->set(2, (const char*)&code);                                                           \
      field_specific_params<DATA>::pack( stream, field );                                           \
   }                                                                                                \
   std::string to_string() override {                                                               \
      return std::string(BOOST_PP_STRINGIZE(OP))+field_specific_params<DATA>::to_string( field );   \
   }                                                                                                \
};

#define WASM_OP_SEQ  (error)                \
                     (end)                  \
                     (unreachable)          \
                     (nop)                  \
                     (else_)                \
                     (return_)              \
                     (drop)                 \
                     (select)               \
                     (i32_eqz)              \
                     (i32_eq)               \
                     (i32_ne)               \
                     (i32_lt_s)             \
                     (i32_lt_u)             \
                     (i32_gt_s)             \
                     (i32_gt_u)             \
                     (i32_le_s)             \
                     (i32_le_u)             \
                     (i32_ge_s)             \
                     (i32_ge_u)             \
                     (i64_eqz)              \
                     (i64_eq)               \
                     (i64_ne)               \
                     (i64_lt_s)             \
                     (i64_lt_u)             \
                     (i64_gt_s)             \
                     (i64_gt_u)             \
                     (i64_le_s)             \
                     (i64_le_u)             \
                     (i64_ge_s)             \
                     (i64_ge_u)             \
                     (f32_eq)               \
                     (f32_ne)               \
                     (f32_lt)               \
                     (f32_gt)               \
                     (f32_le)               \
                     (f32_ge)               \
                     (f64_eq)               \
                     (f64_ne)               \
                     (f64_lt)               \
                     (f64_gt)               \
                     (f64_le)               \
                     (f64_ge)               \
                     (i32_clz)              \
                     (i32_ctz)              \
                     (i32_popcnt)           \
                     (i32_add)              \
                     (i32_sub)              \
                     (i32_mul)              \
                     (i32_div_s)            \
                     (i32_div_u)            \
                     (i32_rem_s)            \
                     (i32_rem_u)            \
                     (i32_and)              \
                     (i32_or)               \
                     (i32_xor)              \
                     (i32_shl)              \
                     (i32_shr_s)            \
                     (i32_shr_u)            \
                     (i32_rotl)             \
                     (i32_rotr)             \
                     (i64_clz)              \
                     (i64_ctz)              \
                     (i64_popcnt)           \
                     (i64_add)              \
                     (i64_sub)              \
                     (i64_mul)              \
                     (i64_div_s)            \
                     (i64_div_u)            \
                     (i64_rem_s)            \
                     (i64_rem_u)            \
                     (i64_and)              \
                     (i64_or)               \
                     (i64_xor)              \
                     (i64_shl)              \
                     (i64_shr_s)            \
                     (i64_shr_u)            \
                     (i64_rotl)             \
                     (i64_rotr)             \
                     (f32_abs)              \
                     (f32_neg)              \
                     (f32_ceil)             \
                     (f32_floor)            \
                     (f32_trunc)            \
                     (f32_nearest)          \
                     (f32_sqrt)             \
                     (f32_add)              \
                     (f32_sub)              \
                     (f32_mul)              \
                     (f32_div)              \
                     (f32_min)              \
                     (f32_max)              \
                     (f32_copysign)         \
                     (f64_abs)              \
                     (f64_neg)              \
                     (f64_ceil)             \
                     (f64_floor)            \
                     (f64_trunc)            \
                     (f64_nearest)          \
                     (f64_sqrt)             \
                     (f64_add)              \
                     (f64_sub)              \
                     (f64_mul)              \
                     (f64_div)              \
                     (f64_min)              \
                     (f64_max)              \
                     (f64_copysign)         \
                     (i32_wrap_i64)         \
                     (i32_trunc_s_f32)      \
                     (i32_trunc_u_f32)      \
                     (i32_trunc_s_f64)      \
                     (i32_trunc_u_f64)      \
                     (i64_extend_s_i32)     \
                     (i64_extend_u_i32)     \
                     (i64_trunc_s_f32)      \
                     (i64_trunc_u_f32)      \
                     (i64_trunc_s_f64)      \
                     (i64_trunc_u_f64)      \
                     (f32_convert_s_i32)    \
                     (f32_convert_u_i32)    \
                     (f32_convert_s_i64)    \
                     (f32_convert_u_i64)    \
                     (f32_demote_f64)       \
                     (f64_convert_s_i32)    \
                     (f64_convert_u_i32)    \
                     (f64_convert_s_i64)    \
                     (f64_convert_u_i64)    \
                     (f64_promote_f32)      \
                     (i32_reinterpret_f32)  \
                     (i64_reinterpret_f64)  \
                     (f32_reinterpret_i32)  \
                     (f64_reinterpret_i64)  \
                     (grow_memory)          \
                     (current_memory)       \
/*块类型ops*/\
                     （块）\
                     （循环）
                     （如果\
/*32位操作系统*/                             \

                     (br)                   \
                     (br_if)                \
                     (call)                 \
                     (call_indirect)        \
                     (get_local)            \
                     (set_local)            \
                     (tee_local)            \
                     (get_global)           \
                     (set_global)           \
                     (i32_const)            \
                     (f32_const)            \
/*memarg操作*/\
                     （I32_负载）
                     （I64_负载）
                     （F32_负载）
                     （F64_负载）
                     （I32_负荷8_s）
                     （I32_负载8_） \
                     （I32_负荷16_s）
                     （I32_负载16_） \
                     （I64英寸负载8英寸）
                     （I64_负载8_） \
                     （I64英寸负载16英寸）
                     （I64_负载16_）
                     （I64英寸负载32英寸）
                     （I64_负载32_） \
                     （I32_商店）
                     （I64_商店）
                     （F32_商店）
                     （F64_商店）
                     （I32_商店8）
                     （I32_商店16）
                     （I64楼8楼）
                     （I64楼16楼）
                     （I64商店32）
/*64位操作系统*/                             \

                     (i64_const)            \
                     (f64_const)            \
/*分支表op*/\
                     （Br_表）

枚举代码{
   无法访问的代码=0x00，
   nop代码=0x01，
   块代码=0x02，
   循环代码=0x03，
   如果_uuu code=0x04，
   否则代码=0x05，
   结束代码=0x0B，
   br_code=0x0c，
   如果代码=0x0D，
   br_table_code=0x0e，
   返回代码=0x0F，
   调用代码=0x10，
   调用间接代码=0x11，
   丢弃代码=0x1A，
   选择_code=0x1B，
   获取本地代码=0x20，
   设置本地代码=0x21，
   tee_local_code=0x22，
   获取全局代码=0x23，
   设置全局代码=0x24，
   I32_负载_代码=0x28，
   I64_负载_代码=0x29，
   f32_加载代码=0x2A，
   F64_加载代码=0x2b，
   I32_load8_s_code=0x2c，
   I32_负载8_u_代码=0x2d，
   I32_load16_s_code=0x2e，
   I32_负载16_u_代码=0x2f，
   I64_负载8_s_代码=0x30，
   I64_负载8_u代码=0x31，
   I64_load16_s_code=0x32，
   I64_负载16_u代码=0x33，
   I64_load32_s_code=0x34，
   I64_负载32_u代码=0x35，
   I32_store_code=0x36，
   I64_store_code=0x37，
   f32_store_code=0x38，
   f64_store_code=0x39，
   I32_store8_code=0x3a，
   I32_Store16_code=0x3b，
   I64存储8代码=0x3c，
   I64商店16代码=0x3d，
   I64瓒store32瓒code=0x3e，
   当前存储器代码=0x3F，
   增长内存代码=0x40，
   I32常数代码=0x41，
   I64_常量代码=0x42，
   f32常数代码=0x43，
   F64常数代码=0x44，
   I32_eqz_code=0x45，
   I32_eq_code=0x46，
   I32_ne_code=0x47，
   I32_lt_s_code=0x48，
   I32_lt__代码=0x49，
   I32_gt_s_code=0x4a，
   I32_gt__代码=0x4b，
   I32_le_s_code=0x4c，
   I32_le_u_代码=0x4d，
   I32_ge_s_code=0x4e，
   I32_ge_u_代码=0x4f，
   I64_eqz_code=0x50，
   I64_eq_code=0x51，
   I64_ne_代码=0x52，
   I64_lt_s_code=0x53，
   I64_lt_u_code=0x54，
   I64_gt_s_代码=0x55，
   I64_gt___代码=0x56，
   I64_le_s_code=0x57，
   I64_le_u u_代码=0x58，
   I64_ge_s_code=0x59，
   I64_ge_u_代码=0x5a，
   f32_eq_code=0x5b，
   f32_ne_code=0x5c，
   f32_lt_code=0x5d，
   f32_gt_code=0x5e，
   f32_le_code=0x5f，
   f32_ge_code=0x60，
   f64_eq_code=0x61，
   f64_ne_code=0x62，
   f64_lt_code=0x63，
   f64_gt_code=0x64，
   f64_le_code=0x65，
   f64_ge_code=0x66，
   I32_clz_code=0x67，
   I32_ctz_code=0x68，
   I32_popcnt_code=0x69，
   I32_添加_code=0x6a，
   I32_子代码=0x6b，
   I32_mul_code=0x6c，
   I32_div_s_code=0x6d，
   I32_Div__代码=0x6e，
   I32_-Rem_-S_代码=0x6F，
   I32_-rem_-u代码=0x70，
   I32_和_代码=0x71，
   I32_或_代码=0x72，
   I32_xor_code=0x73，
   I32_shl_code=0x74，
   I32_shr_s_code=0x75，
   I32_Shr__代码=0x76，
   I32_Rotl_code=0x77，
   I32_rotor_code=0x78，
   I64_clz_code=0x79，
   I64_ctz_code=0x7a，
   I64_popcnt_code=0x7b，
   I64_添加代码=0x7c，
   I64_子代码=0x7d，
   I64_Mul_代码=0x7e，
   I64_div_s_code=0x7f，
   I64_Div__代码=0x80，
   I64_-Rem_-S_代码=0x81，
   I64_-Rem_-u代码=0x82，
   I64_和_代码=0x83，
   I64_或_代码=0x84，
   I64_xor_code=0x85，
   I64_shl_code=0x86，
   I64_shr_s_code=0x87，
   I64_Shr__代码=0x88，
   I64_旋转代码=0x89，
   I64旋转代码=0x8A，
   f32_abs_code=0x8b，
   f32_neg_code=0x8c，
   f32_ceil_code=0x8d，
   f32_地板_代码=0x8e，
   f32_轴代码=0x8F，
   f32_最近的_代码=0x90，
   f32 sqrt code=0x91，
   f32_添加代码=0x92，
   f32_子代码=0x93，
   f32_mul_code=0x94，
   f32_div_code=0x95，
   f32_最小值代码=0x96，
   f32_max_code=0x97，
   f32_copysign_code=0x98，
   f64_abs_code=0x99，
   f64_neg_code=0x9a，
   f64_ceil_code=0x9b，
   F64地板代码=0x9C，
   F64轴代码=0x9D，
   f64_最近的_code=0x9e，
   f64_sqrt_code=0x9f，
   f64_add_code=0xa0，
   F64_子代码=0XA1，
   f64_mul_code=0xa2，
   f64_div_code=0xa3，
   f64_最小值代码=0xA4，
   f64_max_code=0xA5，
   f64_copysign_code=0xA6，
   I32_wrap_I64_code=0xA7，
   I32_-trunc_-s_-f32_-code=0xA8，
   I32_-Trunc_-F32_代码=0xA9，
   I32_Trunc_S_F64_code=0XAA，
   I32_Trunc__F64_code=0XAB，
   I64_扩展_s_I32_代码=0xAC，
   I64_扩展__I32_代码=0XAD，
   I64_-trunc_-s_-f32_-code=0Xae，
   I64_-trunc_-f32_-code=0XAF，
   I64_-trunc_-s_-f64_-code=0xb0，
   I64_-Trunc_-F64_代码=0xB1，
   f32_convert_s_i32_code=0xb2，
   f32_convert__i32_code=0xb3，
   f32_转换_s_i64_代码=0xb4，
   f32_转换__I64_代码=0xb5，
   f32_降级_f64_代码=0xb6，
   f64_convert_s_i32_code=0xb7，
   f64_convert__i32_code=0xb8，
   f64_convert_s_i64_code=0xb9，
   f64_convert__I64_code=0xba，
   f64_promote_f32_code=0xbb，
   I32_reinterpret_f32_code=0XBC，
   I64_重新解释_f64_code=0xbd，
   f32_reinterpret_i32_code=0xbe，
   f64_reinterpret_i64_code=0xbf，
   错误代码=0xFF，
}；/ /代码

结构访问者参数
   IR：：模块*模块；
   指令流*新代码；
   ir:：function def*函数定义；
   大小开始索引；
}；

结构图
   virtual std:：string to_string（）返回“instr”；
   虚拟uint16_t get_code（）=0；
   虚拟无效访问（访问者参数和参数）=0；
   virtual int skip_ahead（）=0；
   虚空解包（char*opcode）=0；
   虚拟空包（指令流*流）=0；
   虚拟bool为_kill（）=0；
   虚拟bool为_post（）=0；
}；

//如果我们应该将instr反映到新的代码块中，则用于编码的基注入器和实用程序类
template<typename mutator，typename…突变体>
结构传播应该杀死
   static constexpr bool value=mutator:：kills propagate_should_kill<mutators…>：value；
}；
模板<typename mutator>
结构传播_should_kill<mutator>
   静态constexpr bool value=mutator:：kills；
}；
template<typename mutator，typename…突变体>
结构传播注入后
   static constexpr bool value=mutator:：post propagate post injection<mutators…>：value；
}；
模板<typename mutator>
结构传播
   static constexpr bool value=mutator:：post；
}；
模板<类型名…突变体>
结构指令库：指令库
   bool is_post（）override return propagate_post_injection<mutators…>：value；
   bool is_kill（）override return propagate_should_kill<mutators…>：：value；
   虚拟无效访问（访问者参数和参数）覆盖
      对于（auto m：mutators：：accept…} {
         M（这个，ARG）；
      }
   }
}；

//构造指令
boost_pp_seq_for_each（construct_op_has_data，voidtype，boost_pp_seq_subseq（wasm_op_seq，0，133））。
Boost_pp_seq_for_each（construct_op_has_data，blocktype，Boost_pp_seq_subseq（wasm_op_seq，133，3））。
Boost_pp_seq_for_each（construct_op_has_data，uint32_t，Boost_pp_seq_subsq（wasm_op_seq，136，11））。
Boost_pp_seq_for_each（construct_op_has_data，memarg，Boost_pp_seq_subsq（wasm_op_seq，147，23））。
Boost_pp_seq_for_each（construct_op_has_data，uint64_t，Boost_pp_seq_subsq（wasm_op_seq，170，2））。
boost_pp_seq_for_each（construct_op_has_data，branchtabletype，boost_pp_seq_subsq（wasm_op_seq，172，1））。
undef结构_op_有_数据

杂注包（pop）

结构nop突变
   static constexpr bool kills=false；
   static constexpr bool post=false；
   静态无效接受（instr*inst，visitor_arg&arg）
}；

//要从中继承以附加特定的赋值函数的类，并且具有所有指定类型的默认行为
template<typename mutator=nop mutator，typename…突变体>
结构类型
定义Gen_类型（R、T、OP）
   使用boost_pp_cat（op，_t）=op<t，boost_pp_cat（t，s）…>
   每种类型的增强序列
nEnf基因型
}；/ / oppe类型


/**
 *缓存操作部分
 **/

template <class Op_Types>
class cached_ops {
#define GEN_FIELD( r, P, OP ) \
   static std::unique_ptr<typename Op_Types::BOOST_PP_CAT(OP,_t)> BOOST_PP_CAT(P, OP);
   BOOST_PP_SEQ_FOR_EACH( GEN_FIELD, cached_, WASM_OP_SEQ )
#undef GEN_FIELD

   static std::vector<instr*> _cached_ops;
   public:
   static std::vector<instr*>* get_cached_ops() {
#define PUSH_BACK_OP( r, T, OP ) \
         _cached_ops[BOOST_PP_CAT(OP,_code)] = BOOST_PP_CAT(T, OP).get();
      if ( _cached_ops.empty() ) {
//有错误的预填充
         _cached_ops.resize( 256, cached_error.get() );
         BOOST_PP_SEQ_FOR_EACH( PUSH_BACK_OP, cached_ , WASM_OP_SEQ )
      }
#undef PUSH_BACK_OP
      return &_cached_ops;
   }
};

template <class Op_Types>
std::vector<instr*> cached_ops<Op_Types>::_cached_ops; 

#define INIT_FIELD( r, P, OP ) \
   template <class Op_Types>   \
   std::unique_ptr<typename Op_Types::BOOST_PP_CAT(OP,_t)> cached_ops<Op_Types>::BOOST_PP_CAT(P, OP) = std::make_unique<typename Op_Types::BOOST_PP_CAT(OP,_t)>();
   BOOST_PP_SEQ_FOR_EACH( INIT_FIELD, cached_, WASM_OP_SEQ )

template <class Op_Types>
std::vector<instr*>* get_cached_ops_vec() {
 #define GEN_FIELD( r, P, OP ) \
   static std::unique_ptr<typename Op_Types::BOOST_PP_CAT(OP,_t)> BOOST_PP_CAT(P, OP) = std::make_unique<typename Op_Types::BOOST_PP_CAT(OP,_t)>();
   BOOST_PP_SEQ_FOR_EACH( GEN_FIELD, cached_, WASM_OP_SEQ )
 #undef GEN_FIELD
   static std::vector<instr*> _cached_ops;

#define PUSH_BACK_OP( r, T, OP ) \
      _cached_ops[BOOST_PP_CAT(OP,_code)] = BOOST_PP_CAT(T, OP).get();

   if ( _cached_ops.empty() ) {
//有错误的预填充
      _cached_ops.resize( 256, cached_error.get() );
      BOOST_PP_SEQ_FOR_EACH( PUSH_BACK_OP, cached_ , WASM_OP_SEQ )
   }
#undef PUSH_BACK_OP
   return &_cached_ops;
}
using namespace IR;

//从输入流中解码运算符并通过操作码发送。
//此代码来自wasm jit/include/ir/operators.h。
template <class Op_Types>
struct EOSIO_OperatorDecoderStream
{
   EOSIO_OperatorDecoderStream(const std::vector<U8>& codeBytes)
   : start(codeBytes.data()), nextByte(codeBytes.data()), end(codeBytes.data()+codeBytes.size()) {
     if(!_cached_ops)
        _cached_ops = cached_ops<Op_Types>::get_cached_ops();
   }

   operator bool() const { return nextByte < end; }

   instr* decodeOp() {
      EOS_ASSERT(nextByte + sizeof(IR::Opcode) <= end, wasm_exception, "");
      IR::Opcode opcode = *(IR::Opcode*)nextByte;  
      switch(opcode)
      {
      #define VISIT_OPCODE(opcode,name,nameString,Imm,...) \
         case IR::Opcode::name: \
         { \
            EOS_ASSERT(nextByte + sizeof(IR::OpcodeAndImm<IR::Imm>) <= end, wasm_exception, ""); \
            IR::OpcodeAndImm<IR::Imm>* encodedOperator = (IR::OpcodeAndImm<IR::Imm>*)nextByte; \
            nextByte += sizeof(IR::OpcodeAndImm<IR::Imm>); \
            auto op = _cached_ops->at(BOOST_PP_CAT(name, _code)); \
            op->unpack( reinterpret_cast<char*>(&(encodedOperator->imm)) ); \
            return op;  \
         }
      ENUM_OPERATORS(VISIT_OPCODE)
      #undef VISIT_OPCODE
      default:
         nextByte += sizeof(IR::Opcode);
         return _cached_ops->at(error_code); 
      }
   }

   instr* decodeOpWithoutConsume() {
      const U8* savedNextByte = nextByte;
      instr* result = decodeOp();
      nextByte = savedNextByte;
      return result;
   }
   inline uint32_t index() { return nextByte - start; }
private:
//缓存的操作以获取的地址
   static const std::vector<instr*>* _cached_ops;
   const U8* start;
   const U8* nextByte;
   const U8* end;
};

template <class Op_Types>
const std::vector<instr*>* EOSIO_OperatorDecoderStream<Op_Types>::_cached_ops;

}}} //名称空间eosio，chain，wasm_ops

FC_REFLECT_TEMPLATE( (typename T), eosio::chain::wasm_ops::block< T >, (code)(rt) )
FC_REFLECT( eosio::chain::wasm_ops::memarg, (a)(o) )
FC_REFLECT( eosio::chain::wasm_ops::blocktype, (result) )
FC_REFLECT( eosio::chain::wasm_ops::memoryoptype, (end) )
