
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include <eosio/chain/webassembly/common.hpp>
#include <eosio/chain/webassembly/runtime_interface.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/apply_context.hpp>
#include <softfloat_types.h>

//WABT包括
#include <src/binary-reader.h>
#include <src/common.h>
#include <src/interp.h>

namespace eosio { namespace chain { namespace webassembly { namespace wabt_runtime {

using namespace fc;
using namespace wabt;
using namespace wabt::interp;
using namespace eosio::chain::webassembly::common;

struct wabt_apply_instance_vars {
   Memory* memory;
   apply_context& ctx;

   char* get_validated_pointer(uint32_t offset, uint32_t size) {
      EOS_ASSERT(memory, wasm_execution_error, "access violation");
      EOS_ASSERT(offset + size <= memory->data.size() && offset + size >= offset, wasm_execution_error, "access violation");
      return memory->data.data() + offset;
   }
};

struct intrinsic_registrator {
   using intrinsic_fn = TypedValue(*)(wabt_apply_instance_vars&, const TypedValues&);

   struct intrinsic_func_info {
      FuncSignature sig;
      intrinsic_fn func;
   }; 

   static auto& get_map(){
      static map<string, map<string, intrinsic_func_info>> _map;
      return _map;
   };

   intrinsic_registrator(const char* mod, const char* name, const FuncSignature& sig, intrinsic_fn fn) {
      get_map()[string(mod)][string(name)] = intrinsic_func_info{sig, fn};
   }
};

class wabt_runtime : public eosio::chain::wasm_runtime_interface {
   public:
      wabt_runtime();
      std::unique_ptr<wasm_instantiated_module_interface> instantiate_module(const char* code_bytes, size_t code_size, std::vector<uint8_t> initial_memory) override;

      void immediately_exit_currently_running_module() override;

   private:
wabt::ReadBinaryOptions read_binary_options;  //注意：默认ctor将查看feature.def中的每个选项，并将该功能的默认值设置为disabled。
};

/*
 *表示一个in-wasm内存数组的类
 *提示转录器下一个参数将
 *是一个大小（数据字节长度），并且对是一起验证的
 *这将触发内部调用程序的模板专门化
 *@ tPARAMT
 **/

template<typename T>
inline array_ptr<T> array_ptr_impl (wabt_apply_instance_vars& vars, uint32_t ptr, uint32_t length)
{
   EOS_ASSERT( length < INT_MAX/(uint32_t)sizeof(T), binaryen_exception, "length will overflow" );
   return array_ptr<T>((T*)(vars.get_validated_pointer(ptr, length * (uint32_t)sizeof(T))));
}

/*
 *表示必须以空结尾的WASM内存字符数组的类
 **/

inline null_terminated_ptr null_terminated_ptr_impl(wabt_apply_instance_vars& vars, uint32_t ptr)
{
   char *value = vars.get_validated_pointer(ptr, 1);
   const char* p = value;
   const char* const top_of_memory = vars.memory->data.data() + vars.memory->data.size();
   while(p < top_of_memory)
      if(*p++ == '\0')
         return null_terminated_ptr(value);

   FC_THROW_EXCEPTION(wasm_execution_error, "unterminated string");
}


template<typename T>
struct is_reference_from_value {
   static constexpr bool value = false;
};

template<>
struct is_reference_from_value<name> {
   static constexpr bool value = true;
};

template<>
struct is_reference_from_value<fc::time_point_sec> {
   static constexpr bool value = true;
};

template<typename T>
constexpr bool is_reference_from_value_v = is_reference_from_value<T>::value;

template<typename T>
T convert_literal_to_native(const TypedValue& v);

template<>
inline double convert_literal_to_native<double>(const TypedValue& v) {
   return v.get_f64();
}

template<>
inline float convert_literal_to_native<float>(const TypedValue& v) {
   return v.get_f32();
}

template<>
inline int64_t convert_literal_to_native<int64_t>(const TypedValue& v) {
   return v.get_i64();
}

template<>
inline uint64_t convert_literal_to_native<uint64_t>(const TypedValue& v) {
   return v.get_i64();
}

template<>
inline int32_t convert_literal_to_native<int32_t>(const TypedValue& v) {
   return v.get_i32();
}

template<>
inline uint32_t convert_literal_to_native<uint32_t>(const TypedValue& v) {
   return v.get_i32();
}

template<>
inline bool convert_literal_to_native<bool>(const TypedValue& v) {
   return v.get_i32();
}

template<>
inline name convert_literal_to_native<name>(const TypedValue& v) {
   int64_t val = v.get_i64();
   return name(val);
}

inline auto convert_native_to_literal(const wabt_apply_instance_vars&, const uint32_t &val) {
   TypedValue tv(Type::I32);
   tv.set_i32(val);
   return tv;
}

inline auto convert_native_to_literal(const wabt_apply_instance_vars&, const int32_t &val) {
   TypedValue tv(Type::I32);
   tv.set_i32(val);
   return tv;
}

inline auto convert_native_to_literal(const wabt_apply_instance_vars&, const uint64_t &val) {
   TypedValue tv(Type::I64);
   tv.set_i64(val);
   return tv;
}

inline auto convert_native_to_literal(const wabt_apply_instance_vars&, const int64_t &val) {
   TypedValue tv(Type::I64);
   tv.set_i64(val);
   return tv;
}

inline auto convert_native_to_literal(const wabt_apply_instance_vars&, const float &val) {
   TypedValue tv(Type::F32);
   tv.set_f32(val);
   return tv;
}

inline auto convert_native_to_literal(const wabt_apply_instance_vars&, const double &val) {
   TypedValue tv(Type::F64);
   tv.set_f64(val);
   return tv;
}

inline auto convert_native_to_literal(const wabt_apply_instance_vars&, const name &val) {
   TypedValue tv(Type::I64);
   tv.set_i64(val.value);
   return tv;
}

inline auto convert_native_to_literal(const wabt_apply_instance_vars& vars, char* ptr) {
   const char* base = vars.memory->data.data();
   const char* top_of_memory = base + vars.memory->data.size();
   EOS_ASSERT(ptr >= base && ptr < top_of_memory, wasm_execution_error, "returning pointer not in linear memory");
   Value v;
   v.i32 = (int)(ptr - base);
   return TypedValue(Type::I32, v);
}

struct void_type {
};

template<typename T>
struct wabt_to_value_type;

template<>
struct wabt_to_value_type<F32> {
   static constexpr auto value = Type::F32;
};

template<>
struct wabt_to_value_type<F64> {
   static constexpr auto value = Type::F64;
};
template<>
struct wabt_to_value_type<I32> {
   static constexpr auto value = Type::I32;
};
template<>
struct wabt_to_value_type<I64> {
   static constexpr auto value = Type::I64;
};

template<typename T>
constexpr auto wabt_to_value_type_v = wabt_to_value_type<T>::value;

template<typename T>
struct wabt_to_rvalue_type;
template<>
struct wabt_to_rvalue_type<F32> {
   static constexpr auto value = Type::F32;
};
template<>
struct wabt_to_rvalue_type<F64> {
   static constexpr auto value = Type::F64;
};
template<>
struct wabt_to_rvalue_type<I32> {
   static constexpr auto value = Type::I32;
};
template<>
struct wabt_to_rvalue_type<I64> {
   static constexpr auto value = Type::I64;
};
template<>
struct wabt_to_rvalue_type<const name&> {
   static constexpr auto value = Type::I64;
};
template<>
struct wabt_to_rvalue_type<name> {
   static constexpr auto value = Type::I64;
};

template<>
struct wabt_to_rvalue_type<char*> {
   static constexpr auto value = Type::I32;
};

template<typename T>
constexpr auto wabt_to_rvalue_type_v = wabt_to_rvalue_type<T>::value;

template<typename>
struct wabt_function_type_provider;

template<typename Ret, typename ...Args>
struct wabt_function_type_provider<Ret(Args...)> {
   static FuncSignature type() {
      return FuncSignature({wabt_to_value_type_v<Args> ...}, {wabt_to_rvalue_type_v<Ret>});
   }
};
template<typename ...Args>
struct wabt_function_type_provider<void(Args...)> {
   static FuncSignature type() {
      return FuncSignature({wabt_to_value_type_v<Args> ...}, {});
   }
};

/*
 *调用程序类型的转发声明，该类型将参数转录到本机方法或从本机方法转录参数。
 *并注入适当的检查
 *
 *@tparam ret-本机函数的返回类型
 *@tparam native参数-要转录的剩余本机参数的std:：tuple
 *@tparam wasmparameters-a std：：转换参数的元组
 **/

template<typename Ret, typename NativeParameters>
struct intrinsic_invoker_impl;

/*
 *完全转录签名的专业化
 *@tparam ret-本机函数的返回类型
 **/

template<typename Ret>
struct intrinsic_invoker_impl<Ret, std::tuple<>> {
   using next_method_type        = Ret (*)(wabt_apply_instance_vars&, const TypedValues&, int);

   template<next_method_type Method>
   static TypedValue invoke(wabt_apply_instance_vars& vars, const TypedValues& args) {
      return convert_native_to_literal(vars, Method(vars, args, args.size() - 1));
   }

   template<next_method_type Method>
   static const auto fn() {
      return invoke<Method>;
   }
};

/*
 *对空返回值的完全转录签名进行专门化
 *@tparam translated-wasm函数的参数
 **/

template<>
struct intrinsic_invoker_impl<void_type, std::tuple<>> {
   using next_method_type        = void_type (*)(wabt_apply_instance_vars&, const TypedValues&, int);

   template<next_method_type Method>
   static TypedValue invoke(wabt_apply_instance_vars& vars, const TypedValues& args) {
      Method(vars, args, args.size() - 1);
      return TypedValue(Type::Void);
   }

   template<next_method_type Method>
   static const auto fn() {
      return invoke<Method>;
   }
};

/*
 *用于在本机方法签名中转录简单类型的分离
 *@tparam ret-本机方法的返回类型
 *@tparam input-要转录的本机参数的类型
 *@tparam inputs-要转录的剩余本机参数
 **/

template<typename Ret, typename Input, typename... Inputs>
struct intrinsic_invoker_impl<Ret, std::tuple<Input, Inputs...>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<Inputs...>>;
   using then_type = Ret (*)(wabt_apply_instance_vars&, Input, Inputs..., const TypedValues&, int);

   template<then_type Then>
   static Ret translate_one(wabt_apply_instance_vars& vars, Inputs... rest, const TypedValues& args, int offset) {
      auto& last = args.at(offset);
      auto native = convert_literal_to_native<Input>(last);
      return Then(vars, native, rest..., args, (uint32_t)offset - 1);
   };

   template<then_type Then>
   static const auto fn() {
      return next_step::template fn<translate_one<Then>>();
   }
};

/*
 *专门用于在本机方法签名中转录数组指针类型
 *此类型转录为2个WASM参数：指针和字节长度，并检查该内存的有效性。
 *发送到本机方法之前的范围
 *
 *@tparam ret-本机方法的返回类型
 *@tparam inputs-要转录的剩余本机参数
 **/

template<typename T, typename Ret, typename... Inputs>
struct intrinsic_invoker_impl<Ret, std::tuple<array_ptr<T>, size_t, Inputs...>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<Inputs...>>;
   using then_type = Ret(*)(wabt_apply_instance_vars&, array_ptr<T>, size_t, Inputs..., const TypedValues&, int);

   template<then_type Then, typename U=T>
   static auto translate_one(wabt_apply_instance_vars& vars, Inputs... rest, const TypedValues& args, int offset) -> std::enable_if_t<std::is_const<U>::value, Ret> {
      static_assert(!std::is_pointer<U>::value, "Currently don't support array of pointers");
      uint32_t ptr = args.at((uint32_t)offset - 1).get_i32();
      size_t length = args.at((uint32_t)offset).get_i32();
      T* base = array_ptr_impl<T>(vars, ptr, length);
      if ( reinterpret_cast<uintptr_t>(base) % alignof(T) != 0 ) {
         if(vars.ctx.control.contracts_console())
            wlog( "misaligned array of const values" );
         std::vector<std::remove_const_t<T> > copy(length > 0 ? length : 1);
         T* copy_ptr = &copy[0];
         memcpy( (void*)copy_ptr, (void*)base, length * sizeof(T) );
         return Then(vars, static_cast<array_ptr<T>>(copy_ptr), length, rest..., args, (uint32_t)offset - 2);
      }
      return Then(vars, static_cast<array_ptr<T>>(base), length, rest..., args, (uint32_t)offset - 2);
   };

   template<then_type Then, typename U=T>
   static auto translate_one(wabt_apply_instance_vars& vars, Inputs... rest, const TypedValues& args, int offset) -> std::enable_if_t<!std::is_const<U>::value, Ret> {
      static_assert(!std::is_pointer<U>::value, "Currently don't support array of pointers");
      uint32_t ptr = args.at((uint32_t)offset - 1).get_i32();
      size_t length = args.at((uint32_t)offset).get_i32();
      T* base = array_ptr_impl<T>(vars, ptr, length);
      if ( reinterpret_cast<uintptr_t>(base) % alignof(T) != 0 ) {
         if(vars.ctx.control.contracts_console())
            wlog( "misaligned array of values" );
         std::vector<std::remove_const_t<T> > copy(length > 0 ? length : 1);
         T* copy_ptr = &copy[0];
         memcpy( (void*)copy_ptr, (void*)base, length * sizeof(T) );
         Ret ret = Then(vars, static_cast<array_ptr<T>>(copy_ptr), length, rest..., args, (uint32_t)offset - 2);  
         memcpy( (void*)base, (void*)copy_ptr, length * sizeof(T) );
         return ret;
      }
      return Then(vars, static_cast<array_ptr<T>>(base), length, rest..., args, (uint32_t)offset - 2);
   };
   
   template<then_type Then>
   static const auto fn() {
      return next_step::template fn<translate_one<Then>>();
   }
};

/*
 *用于在本机方法签名中转录以NULL结尾的指针类型的专门化
 *此类型转录1个WASM参数：一个经过验证包含
 *在分配的内存结束之前为空值。
 *
 *@tparam ret-本机方法的返回类型
 *@tparam inputs-要转录的剩余本机参数
 **/

template<typename Ret, typename... Inputs>
struct intrinsic_invoker_impl<Ret, std::tuple<null_terminated_ptr, Inputs...>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<Inputs...>>;
   using then_type = Ret(*)(wabt_apply_instance_vars&, null_terminated_ptr, Inputs..., const TypedValues&, int);

   template<then_type Then>
   static Ret translate_one(wabt_apply_instance_vars& vars, Inputs... rest, const TypedValues& args, int offset) {
      uint32_t ptr = args.at((uint32_t)offset).get_i32();
      return Then(vars, null_terminated_ptr_impl(vars, ptr), rest..., args, (uint32_t)offset - 1);
   };

   template<then_type Then>
   static const auto fn() {
      return next_step::template fn<translate_one<Then>>();
   }
};

/*
 *专门用于在共享大小的本机方法签名中转录一对数组指针类型
 *此类型转录为3个WASM参数：2个指针和字节长度，并检查这些内存的有效性。
 *发送到本机方法之前的范围
 *
 *@tparam ret-本机方法的返回类型
 *@tparam inputs-要转录的剩余本机参数
 **/

template<typename T, typename U, typename Ret, typename... Inputs>
struct intrinsic_invoker_impl<Ret, std::tuple<array_ptr<T>, array_ptr<U>, size_t, Inputs...>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<Inputs...>>;
   using then_type = Ret(*)(wabt_apply_instance_vars&, array_ptr<T>, array_ptr<U>, size_t, Inputs..., const TypedValues&, int);

   template<then_type Then>
   static Ret translate_one(wabt_apply_instance_vars& vars, Inputs... rest, const TypedValues& args, int offset) {
      uint32_t ptr_t = args.at((uint32_t)offset - 2).get_i32();
      uint32_t ptr_u = args.at((uint32_t)offset - 1).get_i32();
      size_t length = args.at((uint32_t)offset).get_i32();
      static_assert(std::is_same<std::remove_const_t<T>, char>::value && std::is_same<std::remove_const_t<U>, char>::value, "Currently only support array of (const)chars");
      return Then(vars, array_ptr_impl<T>(vars, ptr_t, length), array_ptr_impl<U>(vars, ptr_u, length), length, args, (uint32_t)offset - 3);
   };

   template<then_type Then>
   static const auto fn() {
      return next_step::template fn<translate_one<Then>>();
   }
};

/*
 *专门用于转录memset参数
 *
 *@tparam ret-本机方法的返回类型
 *@tparam inputs-要转录的剩余本机参数
 **/

template<typename Ret>
struct intrinsic_invoker_impl<Ret, std::tuple<array_ptr<char>, int, size_t>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<>>;
   using then_type = Ret(*)(wabt_apply_instance_vars&, array_ptr<char>, int, size_t, const TypedValues&, int);

   template<then_type Then>
   static Ret translate_one(wabt_apply_instance_vars& vars, const TypedValues& args, int offset) {
      uint32_t ptr = args.at((uint32_t)offset - 2).get_i32();
      uint32_t value = args.at((uint32_t)offset - 1).get_i32();
      size_t length = args.at((uint32_t)offset).get_i32();
      return Then(vars, array_ptr_impl<char>(vars, ptr, length), value, length, args, (uint32_t)offset - 3);
   };

   template<then_type Then>
   static const auto fn() {
      return next_step::template fn<translate_one<Then>>();
   }
};

/*
 *用于在本机方法签名中转录指针类型的专门化
 *此类型转换为Int32指针，检查该内存的有效性。
 *发送到本机方法之前的范围
 *
 *@tparam ret-本机方法的返回类型
 *@tparam inputs-要转录的剩余本机参数
 **/

template<typename T, typename Ret, typename... Inputs>
struct intrinsic_invoker_impl<Ret, std::tuple<T *, Inputs...>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<Inputs...>>;
   using then_type = Ret (*)(wabt_apply_instance_vars&, T *, Inputs..., const TypedValues&, int);

   template<then_type Then, typename U=T>
   static auto translate_one(wabt_apply_instance_vars& vars, Inputs... rest, const TypedValues& args, int offset) -> std::enable_if_t<std::is_const<U>::value, Ret> {
      uint32_t ptr = args.at((uint32_t)offset).get_i32();
      T* base = array_ptr_impl<T>(vars, ptr, 1);
      if ( reinterpret_cast<uintptr_t>(base) % alignof(T) != 0 ) {
         if(vars.ctx.control.contracts_console())
            wlog( "misaligned const pointer" );
         std::remove_const_t<T> copy;
         T* copy_ptr = &copy;
         memcpy( (void*)copy_ptr, (void*)base, sizeof(T) );
         return Then(vars, copy_ptr, rest..., args, (uint32_t)offset - 1);
      }
      return Then(vars, base, rest..., args, (uint32_t)offset - 1);
   };

   template<then_type Then, typename U=T>
   static auto translate_one(wabt_apply_instance_vars& vars, Inputs... rest, const TypedValues& args, int offset) -> std::enable_if_t<!std::is_const<U>::value, Ret> {
      uint32_t ptr = args.at((uint32_t)offset).get_i32();
      T* base = array_ptr_impl<T>(vars, ptr, 1);
      if ( reinterpret_cast<uintptr_t>(base) % alignof(T) != 0 ) {
         if(vars.ctx.control.contracts_console())
            wlog( "misaligned pointer" );
         T copy;
         memcpy( (void*)&copy, (void*)base, sizeof(T) );
         Ret ret = Then(vars, &copy, rest..., args, (uint32_t)offset - 1);
         memcpy( (void*)base, (void*)&copy, sizeof(T) );
         return ret; 
      }
      return Then(vars, base, rest..., args, (uint32_t)offset - 1);
   };

   template<then_type Then>
   static const auto fn() {
      return next_step::template fn<translate_one<Then>>();
   }
};

/*
 *用于将引用转码到可以作为本机值传递的名称的专门化
 *此类型转录为本机类型，由值加载到
 *堆栈上的变量，然后通过引用传递给内部变量。
 *
 *@tparam ret-本机方法的返回类型
 *@tparam inputs-要转录的剩余本机参数
 **/

template<typename Ret, typename... Inputs>
struct intrinsic_invoker_impl<Ret, std::tuple<const name&, Inputs...>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<Inputs...>>;
   using then_type = Ret (*)(wabt_apply_instance_vars&, const name&, Inputs..., const TypedValues&, int);

   template<then_type Then>
   static Ret translate_one(wabt_apply_instance_vars& vars, Inputs... rest, const TypedValues& args, int offset) {
      uint64_t wasm_value = args.at((uint32_t)offset).get_i64();
      auto value = name(wasm_value);
      return Then(vars, value, rest..., args, (uint32_t)offset - 1);
   }

   template<then_type Then>
   static const auto fn() {
      return next_step::template fn<translate_one<Then>>();
   }
};

/*
 *用于将引用转码到可以作为本机值传递的fc:：time_point_sec的专门化
 *此类型转录为本机类型，由值加载到
 *堆栈上的变量，然后通过引用传递给内部变量。
 *
 *@tparam ret-本机方法的返回类型
 *@tparam inputs-要转录的剩余本机参数
 **/

template<typename Ret, typename... Inputs>
struct intrinsic_invoker_impl<Ret, std::tuple<const fc::time_point_sec&, Inputs...>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<Inputs...>>;
   using then_type = Ret (*)(wabt_apply_instance_vars&, const fc::time_point_sec&, Inputs..., const TypedValues&, int);

   template<then_type Then>
   static Ret translate_one(wabt_apply_instance_vars& vars, Inputs... rest, const TypedValues& args, int offset) {
      uint32_t wasm_value = args.at((uint32_t)offset).get_i32();
      auto value = fc::time_point_sec(wasm_value);
      return Then(vars, value, rest..., args, (uint32_t)offset - 1);
   }

   template<then_type Then>
   static const auto fn() {
      return next_step::template fn<translate_one<Then>>();
   }
};


/*
 *用于在本机方法签名中转录引用类型的专门化
 *此类型转换为Int32指针，检查该内存的有效性。
 *发送到本机方法之前的范围
 *
 *@tparam ret-本机方法的返回类型
 *@tparam inputs-要转录的剩余本机参数
 **/

template<typename T, typename Ret, typename... Inputs>
struct intrinsic_invoker_impl<Ret, std::tuple<T &, Inputs...>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<Inputs...>>;
   using then_type = Ret (*)(wabt_apply_instance_vars&, T &, Inputs..., const TypedValues&, int);

   template<then_type Then, typename U=T>
   static auto translate_one(wabt_apply_instance_vars& vars, Inputs... rest, const TypedValues& args, int offset) -> std::enable_if_t<std::is_const<U>::value, Ret> {
//无法为空指针创建引用
      uint32_t ptr = args.at((uint32_t)offset).get_i32();
      EOS_ASSERT(ptr != 0, binaryen_exception, "references cannot be created for null pointers");
      T* base = array_ptr_impl<T>(vars, ptr, 1);
      if ( reinterpret_cast<uintptr_t>(base) % alignof(T) != 0 ) {
         if(vars.ctx.control.contracts_console())
            wlog( "misaligned const reference" );
         std::remove_const_t<T> copy;
         T* copy_ptr = &copy;
         memcpy( (void*)copy_ptr, (void*)base, sizeof(T) );
         return Then(vars, *copy_ptr, rest..., args, (uint32_t)offset - 1);
      }
      return Then(vars, *base, rest..., args, (uint32_t)offset - 1);
   }

   template<then_type Then, typename U=T>
   static auto translate_one(wabt_apply_instance_vars& vars, Inputs... rest, const TypedValues& args, int offset) -> std::enable_if_t<!std::is_const<U>::value, Ret> {
//无法为空指针创建引用
      uint32_t ptr = args.at((uint32_t)offset).get_i32();
      EOS_ASSERT(ptr != 0, binaryen_exception, "references cannot be created for null pointers");
      T* base = array_ptr_impl<T>(vars, ptr, 1);
      if ( reinterpret_cast<uintptr_t>(base) % alignof(T) != 0 ) {
         if(vars.ctx.control.contracts_console())
            wlog( "misaligned reference" );
         T copy;
         memcpy( (void*)&copy, (void*)base, sizeof(T) );
         Ret ret = Then(vars, copy, rest..., args, (uint32_t)offset - 1);
         memcpy( (void*)base, (void*)&copy, sizeof(T) );
         return ret; 
      }
      return Then(vars, *base, rest..., args, (uint32_t)offset - 1);
   }


   template<then_type Then>
   static const auto fn() {
      return next_step::template fn<translate_one<Then>>();
   }
};

extern apply_context* fixme_context;

/*
 *将包装类的声明转发给调用该类的方法
 **/

template<typename Ret, typename MethodSig, typename Cls, typename... Params>
struct intrinsic_function_invoker {
   using impl = intrinsic_invoker_impl<Ret, std::tuple<Params...>>;

   template<MethodSig Method>
   static Ret wrapper(wabt_apply_instance_vars& vars, Params... params, const TypedValues&, int) {
      class_from_wasm<Cls>::value(vars.ctx).checktime();
      return (class_from_wasm<Cls>::value(vars.ctx).*Method)(params...);
   }

   template<MethodSig Method>
   static const intrinsic_registrator::intrinsic_fn fn() {
      return impl::template fn<wrapper<Method>>();
   }
};

template<typename MethodSig, typename Cls, typename... Params>
struct intrinsic_function_invoker<void, MethodSig, Cls, Params...> {
   using impl = intrinsic_invoker_impl<void_type, std::tuple<Params...>>;

   template<MethodSig Method>
   static void_type wrapper(wabt_apply_instance_vars& vars, Params... params, const TypedValues& args, int offset) {
      class_from_wasm<Cls>::value(vars.ctx).checktime();
      (class_from_wasm<Cls>::value(vars.ctx).*Method)(params...);
      return void_type();
   }

   template<MethodSig Method>
   static const intrinsic_registrator::intrinsic_fn fn() {
      return impl::template fn<wrapper<Method>>();
   }

};

template<typename>
struct intrinsic_function_invoker_wrapper;

template<typename Cls, typename Ret, typename... Params>
struct intrinsic_function_invoker_wrapper<Ret (Cls::*)(Params...)> {
   using type = intrinsic_function_invoker<Ret, Ret (Cls::*)(Params...), Cls, Params...>;
};

template<typename Cls, typename Ret, typename... Params>
struct intrinsic_function_invoker_wrapper<Ret (Cls::*)(Params...) const> {
   using type = intrinsic_function_invoker<Ret, Ret (Cls::*)(Params...) const, Cls, Params...>;
};

template<typename Cls, typename Ret, typename... Params>
struct intrinsic_function_invoker_wrapper<Ret (Cls::*)(Params...) volatile> {
   using type = intrinsic_function_invoker<Ret, Ret (Cls::*)(Params...) volatile, Cls, Params...>;
};

template<typename Cls, typename Ret, typename... Params>
struct intrinsic_function_invoker_wrapper<Ret (Cls::*)(Params...) const volatile> {
   using type = intrinsic_function_invoker<Ret, Ret (Cls::*)(Params...) const volatile, Cls, Params...>;
};

#define __INTRINSIC_NAME(LABEL, SUFFIX) LABEL##SUFFIX
#define _INTRINSIC_NAME(LABEL, SUFFIX) __INTRINSIC_NAME(LABEL,SUFFIX)

#define _REGISTER_WABT_INTRINSIC(CLS, MOD, METHOD, WASM_SIG, NAME, SIG)\
   static eosio::chain::webassembly::wabt_runtime::intrinsic_registrator _INTRINSIC_NAME(__wabt_intrinsic_fn, __COUNTER__) (\
      MOD,\
      NAME,\
      eosio::chain::webassembly::wabt_runtime::wabt_function_type_provider<WASM_SIG>::type(),\
      eosio::chain::webassembly::wabt_runtime::intrinsic_function_invoker_wrapper<SIG>::type::fn<&CLS::METHOD>()\
   );\

} } } }//eosio:：chain:：webpassembly:：wabt_运行时
