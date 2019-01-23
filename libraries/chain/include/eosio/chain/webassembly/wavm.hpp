
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include <eosio/chain/webassembly/common.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/webassembly/runtime_interface.hpp>
#include <eosio/chain/apply_context.hpp>
#include <softfloat.hpp>
#include "Runtime/Runtime.h"
#include "IR/Types.h"


namespace eosio { namespace chain { namespace webassembly { namespace wavm {

using namespace IR;
using namespace Runtime;
using namespace fc;
using namespace eosio::chain::webassembly::common;

class wavm_runtime : public eosio::chain::wasm_runtime_interface {
   public:
      wavm_runtime();
      ~wavm_runtime();
      std::unique_ptr<wasm_instantiated_module_interface> instantiate_module(const char* code_bytes, size_t code_size, std::vector<uint8_t> initial_memory) override;

      void immediately_exit_currently_running_module() override;

      struct runtime_guard {
         runtime_guard();
         ~runtime_guard();
      };

   private:
      std::shared_ptr<runtime_guard> _runtime_guard;
};

//这是单线程实现的临时黑客
struct running_instance_context {
   MemoryInstance* memory;
   apply_context*  apply_ctx;
};
extern running_instance_context the_running_instance_context;

/*
 *表示一个in-wasm内存数组的类
 *提示转录器下一个参数将
 *尺寸（单位：ts），并将两个尺寸对一起验证。
 *这将触发内部调用程序的模板专门化
 *@ tPARAMT
 **/

template<typename T>
inline array_ptr<T> array_ptr_impl (running_instance_context& ctx, U32 ptr, size_t length)
{
   MemoryInstance* mem = ctx.memory;
   if (!mem) 
      Runtime::causeException(Exception::Cause::accessViolation);

   size_t mem_total = IR::numBytesPerPage * Runtime::getMemoryNumPages(mem);
   if (ptr >= mem_total || length > (mem_total - ptr) / sizeof(T))
      Runtime::causeException(Exception::Cause::accessViolation);
   
   T* ret_ptr = (T*)(getMemoryBaseAddress(mem) + ptr);

   return array_ptr<T>((T*)(getMemoryBaseAddress(mem) + ptr));
}

/*
 *表示必须以空结尾的WASM内存字符数组的类
 **/

inline null_terminated_ptr null_terminated_ptr_impl(running_instance_context& ctx, U32 ptr)
{
   MemoryInstance* mem = ctx.memory;
   if(!mem)
      Runtime::causeException(Exception::Cause::accessViolation);

   char *value                     = (char*)(getMemoryBaseAddress(mem) + ptr);
   const char* p                   = value;
   const char* const top_of_memory = (char*)(getMemoryBaseAddress(mem) + IR::numBytesPerPage*Runtime::getMemoryNumPages(mem));
   while(p < top_of_memory)
      if(*p++ == '\0')
         return null_terminated_ptr(value);

   Runtime::causeException(Exception::Cause::accessViolation);
}


/*
 *将本机类型映射到WASM VM类型的模板
 *@tparam t本机类型
 **/

template<typename T>
struct native_to_wasm {
   using type = void;
};

/*
 *专门用于将指针映射到Int32
 **/

template<typename T>
struct native_to_wasm<T *> {
   using type = I32;
};

/*
 *本机类型的映射
 **/

template<>
struct native_to_wasm<float> {
   using type = F32;
};
template<>
struct native_to_wasm<double> {
   using type = F64;
};
template<>
struct native_to_wasm<int32_t> {
   using type = I32;
};
template<>
struct native_to_wasm<uint32_t> {
   using type = I32;
};
template<>
struct native_to_wasm<int64_t> {
   using type = I64;
};
template<>
struct native_to_wasm<uint64_t> {
   using type = I64;
};
template<>
struct native_to_wasm<bool> {
   using type = I32;
};
template<>
struct native_to_wasm<const name &> {
   using type = I64;
};
template<>
struct native_to_wasm<name> {
   using type = I64;
};
template<>
struct native_to_wasm<const fc::time_point_sec &> {
   using type = I32;
};

template<>
struct native_to_wasm<fc::time_point_sec> {
   using type = I32;
};

//方便别名
template<typename T>
using native_to_wasm_t = typename native_to_wasm<T>::type;

template<typename T>
inline auto convert_native_to_wasm(const running_instance_context& ctx, T val) {
   return native_to_wasm_t<T>(val);
}

inline auto convert_native_to_wasm(const running_instance_context& ctx, const name &val) {
   return native_to_wasm_t<const name &>(val.value);
}

inline auto convert_native_to_wasm(const running_instance_context& ctx, const fc::time_point_sec& val) {
   return native_to_wasm_t<const fc::time_point_sec &>(val.sec_since_epoch());
}

inline auto convert_native_to_wasm(running_instance_context& ctx, char* ptr) {
   MemoryInstance* mem = ctx.memory;
   if(!mem)
      Runtime::causeException(Exception::Cause::accessViolation);
   char* base = (char*)getMemoryBaseAddress(mem);
   char* top_of_memory = base + IR::numBytesPerPage*Runtime::getMemoryNumPages(mem);
   if(ptr < base || ptr >= top_of_memory)
      Runtime::causeException(Exception::Cause::accessViolation);
   return (U32)(ptr - base);
}

template<typename T>
inline auto convert_wasm_to_native(native_to_wasm_t<T> val) {
   return T(val);
}

template<typename T>
struct wasm_to_value_type;

template<>
struct wasm_to_value_type<F32> {
   static constexpr auto value = ValueType::f32;
};

template<>
struct wasm_to_value_type<F64> {
   static constexpr auto value = ValueType::f64;
};
template<>
struct wasm_to_value_type<I32> {
   static constexpr auto value = ValueType::i32;
};
template<>
struct wasm_to_value_type<I64> {
   static constexpr auto value = ValueType::i64;
};

template<typename T>
constexpr auto wasm_to_value_type_v = wasm_to_value_type<T>::value;

template<typename T>
struct wasm_to_rvalue_type;
template<>
struct wasm_to_rvalue_type<F32> {
   static constexpr auto value = ResultType::f32;
};
template<>
struct wasm_to_rvalue_type<F64> {
   static constexpr auto value = ResultType::f64;
};
template<>
struct wasm_to_rvalue_type<I32> {
   static constexpr auto value = ResultType::i32;
};
template<>
struct wasm_to_rvalue_type<I64> {
   static constexpr auto value = ResultType::i64;
};
template<>
struct wasm_to_rvalue_type<void> {
   static constexpr auto value = ResultType::none;
};
template<>
struct wasm_to_rvalue_type<const name&> {
   static constexpr auto value = ResultType::i64;
};
template<>
struct wasm_to_rvalue_type<name> {
   static constexpr auto value = ResultType::i64;
};

template<>
struct wasm_to_rvalue_type<char*> {
   static constexpr auto value = ResultType::i32;
};

template<>
struct wasm_to_rvalue_type<fc::time_point_sec> {
   static constexpr auto value = ResultType::i32;
};


template<typename T>
constexpr auto wasm_to_rvalue_type_v = wasm_to_rvalue_type<T>::value;

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



struct void_type {
};

/*
 *提供了所需的C ABI签名的函数类型的提供程序的转发声明
 **/

template<typename>
struct wasm_function_type_provider;

/*
 *专门化销毁返回和参数
 **/

template<typename Ret, typename ...Args>
struct wasm_function_type_provider<Ret(Args...)> {
   static const FunctionType *type() {
      return FunctionType::get(wasm_to_rvalue_type_v<Ret>, {wasm_to_value_type_v<Args> ...});
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

template<typename Ret, typename NativeParameters, typename WasmParameters>
struct intrinsic_invoker_impl;

/*
 *完全转录签名的专业化
 *@tparam ret-本机函数的返回类型
 *@tparam translated-wasm函数的参数
 **/

template<typename Ret, typename ...Translated>
struct intrinsic_invoker_impl<Ret, std::tuple<>, std::tuple<Translated...>> {
   using next_method_type        = Ret (*)(running_instance_context &, Translated...);

   template<next_method_type Method>
   static native_to_wasm_t<Ret> invoke(Translated... translated) {
      return convert_native_to_wasm(the_running_instance_context, Method(the_running_instance_context, translated...));
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

template<typename ...Translated>
struct intrinsic_invoker_impl<void_type, std::tuple<>, std::tuple<Translated...>> {
   using next_method_type        = void_type (*)(running_instance_context &, Translated...);

   template<next_method_type Method>
   static void invoke(Translated... translated) {
      Method(the_running_instance_context, translated...);
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
 *@tparam translated-转录的wasm参数列表
 **/

template<typename Ret, typename Input, typename... Inputs, typename... Translated>
struct intrinsic_invoker_impl<Ret, std::tuple<Input, Inputs...>, std::tuple<Translated...>> {
   using translated_type = native_to_wasm_t<Input>;
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<Inputs...>, std::tuple<Translated..., translated_type>>;
   using then_type = Ret (*)(running_instance_context &, Input, Inputs..., Translated...);

   template<then_type Then>
   static Ret translate_one(running_instance_context& ctx, Inputs... rest, Translated... translated, translated_type last) {
      auto native = convert_wasm_to_native<Input>(last);
      return Then(ctx, native, rest..., translated...);
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
 *@tparam translated-转录的wasm参数列表
 **/

template<typename T, typename Ret, typename... Inputs, typename ...Translated>
struct intrinsic_invoker_impl<Ret, std::tuple<array_ptr<T>, size_t, Inputs...>, std::tuple<Translated...>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<Inputs...>, std::tuple<Translated..., I32, I32>>;
   using then_type = Ret(*)(running_instance_context&, array_ptr<T>, size_t, Inputs..., Translated...);

   template<then_type Then, typename U=T>
   static auto translate_one(running_instance_context& ctx, Inputs... rest, Translated... translated, I32 ptr, I32 size) -> std::enable_if_t<std::is_const<U>::value, Ret> {
      static_assert(!std::is_pointer<U>::value, "Currently don't support array of pointers");
      const auto length = size_t(size);
      T* base = array_ptr_impl<T>(ctx, (U32)ptr, length);
      if ( reinterpret_cast<uintptr_t>(base) % alignof(T) != 0 ) {
         if(ctx.apply_ctx->control.contracts_console())
            wlog( "misaligned array of const values" );
         std::vector<std::remove_const_t<T> > copy(length > 0 ? length : 1);
         T* copy_ptr = &copy[0];
         memcpy( (void*)copy_ptr, (void*)base, length * sizeof(T) );
         return Then(ctx, static_cast<array_ptr<T>>(copy_ptr), length, rest..., translated...);
      }
      return Then(ctx, static_cast<array_ptr<T>>(base), length, rest..., translated...);
   };

   template<then_type Then, typename U=T>
   static auto translate_one(running_instance_context& ctx, Inputs... rest, Translated... translated, I32 ptr, I32 size) -> std::enable_if_t<!std::is_const<U>::value, Ret> {
      static_assert(!std::is_pointer<U>::value, "Currently don't support array of pointers");
      const auto length = size_t(size);
      T* base = array_ptr_impl<T>(ctx, (U32)ptr, length);
      if ( reinterpret_cast<uintptr_t>(base) % alignof(T) != 0 ) {
         if(ctx.apply_ctx->control.contracts_console())
            wlog( "misaligned array of values" );
         std::vector<std::remove_const_t<T> > copy(length > 0 ? length : 1);
         T* copy_ptr = &copy[0];
         memcpy( (void*)copy_ptr, (void*)base, length * sizeof(T) );
         Ret ret = Then(ctx, static_cast<array_ptr<T>>(copy_ptr), length, rest..., translated...);  
         memcpy( (void*)base, (void*)copy_ptr, length * sizeof(T) );
         return ret;
      }
      return Then(ctx, static_cast<array_ptr<T>>(base), length, rest..., translated...);
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
 *@tparam translated-转录的wasm参数列表
 **/

template<typename Ret, typename... Inputs, typename ...Translated>
struct intrinsic_invoker_impl<Ret, std::tuple<null_terminated_ptr, Inputs...>, std::tuple<Translated...>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<Inputs...>, std::tuple<Translated..., I32>>;
   using then_type = Ret(*)(running_instance_context&, null_terminated_ptr, Inputs..., Translated...);

   template<then_type Then>
   static Ret translate_one(running_instance_context& ctx, Inputs... rest, Translated... translated, I32 ptr) {
      return Then(ctx, null_terminated_ptr_impl(ctx, (U32)ptr), rest..., translated...);
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
 *@tparam translated-转录的wasm参数列表
 **/

template<typename T, typename U, typename Ret, typename... Inputs, typename ...Translated>
struct intrinsic_invoker_impl<Ret, std::tuple<array_ptr<T>, array_ptr<U>, size_t, Inputs...>, std::tuple<Translated...>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<Inputs...>, std::tuple<Translated..., I32, I32, I32>>;
   using then_type = Ret(*)(running_instance_context&, array_ptr<T>, array_ptr<U>, size_t, Inputs..., Translated...);

   template<then_type Then>
   static Ret translate_one(running_instance_context& ctx, Inputs... rest, Translated... translated, I32 ptr_t, I32 ptr_u, I32 size) {
      static_assert(std::is_same<std::remove_const_t<T>, char>::value && std::is_same<std::remove_const_t<U>, char>::value, "Currently only support array of (const)chars");
      const auto length = size_t(size);
      return Then(ctx, array_ptr_impl<T>(ctx, (U32)ptr_t, length), array_ptr_impl<U>(ctx, (U32)ptr_u, length), length, rest..., translated...);
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
 *@tparam translated-转录的wasm参数列表
 **/

template<typename Ret>
struct intrinsic_invoker_impl<Ret, std::tuple<array_ptr<char>, int, size_t>, std::tuple<>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<>, std::tuple<I32, I32, I32>>;
   using then_type = Ret(*)(running_instance_context&, array_ptr<char>, int, size_t);

   template<then_type Then>
   static Ret translate_one(running_instance_context& ctx, I32 ptr, I32 value, I32 size) {
      const auto length = size_t(size);
      return Then(ctx, array_ptr_impl<char>(ctx, (U32)ptr, length), value, length);
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
 *@tparam translated-转录的wasm参数列表
 **/

template<typename T, typename Ret, typename... Inputs, typename ...Translated>
struct intrinsic_invoker_impl<Ret, std::tuple<T *, Inputs...>, std::tuple<Translated...>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<Inputs...>, std::tuple<Translated..., I32>>;
   using then_type = Ret (*)(running_instance_context&, T *, Inputs..., Translated...);

   template<then_type Then, typename U=T>
   static auto translate_one(running_instance_context& ctx, Inputs... rest, Translated... translated, I32 ptr) -> std::enable_if_t<std::is_const<U>::value, Ret> {
      T* base = array_ptr_impl<T>(ctx, (U32)ptr, 1);
      if ( reinterpret_cast<uintptr_t>(base) % alignof(T) != 0 ) {
         if(ctx.apply_ctx->control.contracts_console())
            wlog( "misaligned const pointer" );
         std::remove_const_t<T> copy;
         T* copy_ptr = &copy;
         memcpy( (void*)copy_ptr, (void*)base, sizeof(T) );
         return Then(ctx, copy_ptr, rest..., translated...);
      }
      return Then(ctx, base, rest..., translated...);
   };

   template<then_type Then, typename U=T>
   static auto translate_one(running_instance_context& ctx, Inputs... rest, Translated... translated, I32 ptr) -> std::enable_if_t<!std::is_const<U>::value, Ret> {
      T* base = array_ptr_impl<T>(ctx, (U32)ptr, 1);
      if ( reinterpret_cast<uintptr_t>(base) % alignof(T) != 0 ) {
         if(ctx.apply_ctx->control.contracts_console())
            wlog( "misaligned pointer" );
         std::remove_const_t<T> copy;
         T* copy_ptr = &copy;
         memcpy( (void*)copy_ptr, (void*)base, sizeof(T) );
         Ret ret = Then(ctx, copy_ptr, rest..., translated...);
         memcpy( (void*)base, (void*)copy_ptr, sizeof(T) );
         return ret;
      }
      return Then(ctx, base, rest..., translated...);
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
 *@tparam translated-转录的wasm参数列表
 **/

template<typename Ret, typename... Inputs, typename ...Translated>
struct intrinsic_invoker_impl<Ret, std::tuple<const name&, Inputs...>, std::tuple<Translated...>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<Inputs...>, std::tuple<Translated..., native_to_wasm_t<const name&> >>;
   using then_type = Ret (*)(running_instance_context&, const name&, Inputs..., Translated...);

   template<then_type Then>
   static Ret translate_one(running_instance_context& ctx, Inputs... rest, Translated... translated, native_to_wasm_t<const name&> wasm_value) {
      auto value = name(wasm_value);
      return Then(ctx, value, rest..., translated...);
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
 *@tparam translated-转录的wasm参数列表
 **/

template<typename T, typename Ret, typename... Inputs, typename ...Translated>
struct intrinsic_invoker_impl<Ret, std::tuple<T &, Inputs...>, std::tuple<Translated...>> {
   using next_step = intrinsic_invoker_impl<Ret, std::tuple<Inputs...>, std::tuple<Translated..., I32>>;
   using then_type = Ret (*)(running_instance_context &, T &, Inputs..., Translated...);

   template<then_type Then, typename U=T>
   static auto translate_one(running_instance_context& ctx, Inputs... rest, Translated... translated, I32 ptr) -> std::enable_if_t<std::is_const<U>::value, Ret> {
//无法为空指针创建引用
      EOS_ASSERT((U32)ptr != 0, wasm_exception, "references cannot be created for null pointers");
      MemoryInstance* mem = ctx.memory;
      if(!mem || (U32)ptr+sizeof(T) >= IR::numBytesPerPage*Runtime::getMemoryNumPages(mem))
         Runtime::causeException(Exception::Cause::accessViolation);
      T &base = *(T*)(getMemoryBaseAddress(mem)+(U32)ptr);
      if ( reinterpret_cast<uintptr_t>(&base) % alignof(T) != 0 ) {
         if(ctx.apply_ctx->control.contracts_console())
            wlog( "misaligned const reference" );
         std::remove_const_t<T> copy;
         T* copy_ptr = &copy;
         memcpy( (void*)copy_ptr, (void*)&base, sizeof(T) );
         return Then(ctx, *copy_ptr, rest..., translated...);
      }
      return Then(ctx, base, rest..., translated...);
   }

   template<then_type Then, typename U=T>
   static auto translate_one(running_instance_context& ctx, Inputs... rest, Translated... translated, I32 ptr) -> std::enable_if_t<!std::is_const<U>::value, Ret> {
//无法为空指针创建引用
      EOS_ASSERT((U32)ptr != 0, wasm_exception, "reference cannot be created for null pointers");
      MemoryInstance* mem = ctx.memory;
      if(!mem || (U32)ptr+sizeof(T) >= IR::numBytesPerPage*Runtime::getMemoryNumPages(mem))
         Runtime::causeException(Exception::Cause::accessViolation);
      T &base = *(T*)(getMemoryBaseAddress(mem)+(U32)ptr);
      if ( reinterpret_cast<uintptr_t>(&base) % alignof(T) != 0 ) {
         if(ctx.apply_ctx->control.contracts_console())
            wlog( "misaligned reference" );
         std::remove_const_t<T> copy;
         T* copy_ptr = &copy;
         memcpy( (void*)copy_ptr, (void*)&base, sizeof(T) );
         Ret ret = Then(ctx, *copy_ptr, rest..., translated...);
         memcpy( (void*)&base, (void*)copy_ptr, sizeof(T) );
         return ret;
      }
      return Then(ctx, base, rest..., translated...);
   }

   template<then_type Then>
   static const auto fn() {
      return next_step::template fn<translate_one<Then>>();
   }
};

/*
 *将包装类的声明转发给调用该类的方法
 **/

template<typename WasmSig, typename Ret, typename MethodSig, typename Cls, typename... Params>
struct intrinsic_function_invoker {
   using impl = intrinsic_invoker_impl<Ret, std::tuple<Params...>, std::tuple<>>;

   template<MethodSig Method>
   static Ret wrapper(running_instance_context& ctx, Params... params) {
      class_from_wasm<Cls>::value(*ctx.apply_ctx).checktime();
      return (class_from_wasm<Cls>::value(*ctx.apply_ctx).*Method)(params...);
   }

   template<MethodSig Method>
   static const WasmSig *fn() {
      auto fn = impl::template fn<wrapper<Method>>();
      static_assert(std::is_same<WasmSig *, decltype(fn)>::value,
                    "Intrinsic function signature does not match the ABI");
      return fn;
   }
};

template<typename WasmSig, typename MethodSig, typename Cls, typename... Params>
struct intrinsic_function_invoker<WasmSig, void, MethodSig, Cls, Params...> {
   using impl = intrinsic_invoker_impl<void_type, std::tuple<Params...>, std::tuple<>>;

   template<MethodSig Method>
   static void_type wrapper(running_instance_context& ctx, Params... params) {
      class_from_wasm<Cls>::value(*ctx.apply_ctx).checktime();
      (class_from_wasm<Cls>::value(*ctx.apply_ctx).*Method)(params...);
      return void_type();
   }

   template<MethodSig Method>
   static const WasmSig *fn() {
      auto fn = impl::template fn<wrapper<Method>>();
      static_assert(std::is_same<WasmSig *, decltype(fn)>::value,
                    "Intrinsic function signature does not match the ABI");
      return fn;
   }
};

template<typename, typename>
struct intrinsic_function_invoker_wrapper;

template<typename WasmSig, typename Cls, typename Ret, typename... Params>
struct intrinsic_function_invoker_wrapper<WasmSig, Ret (Cls::*)(Params...)> {
   using type = intrinsic_function_invoker<WasmSig, Ret, Ret (Cls::*)(Params...), Cls, Params...>;
};

template<typename WasmSig, typename Cls, typename Ret, typename... Params>
struct intrinsic_function_invoker_wrapper<WasmSig, Ret (Cls::*)(Params...) const> {
   using type = intrinsic_function_invoker<WasmSig, Ret, Ret (Cls::*)(Params...) const, Cls, Params...>;
};

template<typename WasmSig, typename Cls, typename Ret, typename... Params>
struct intrinsic_function_invoker_wrapper<WasmSig, Ret (Cls::*)(Params...) volatile> {
   using type = intrinsic_function_invoker<WasmSig, Ret, Ret (Cls::*)(Params...) volatile, Cls, Params...>;
};

template<typename WasmSig, typename Cls, typename Ret, typename... Params>
struct intrinsic_function_invoker_wrapper<WasmSig, Ret (Cls::*)(Params...) const volatile> {
   using type = intrinsic_function_invoker<WasmSig, Ret, Ret (Cls::*)(Params...) const volatile, Cls, Params...>;
};

#define _ADD_PAREN_1(...) ((__VA_ARGS__)) _ADD_PAREN_2
#define _ADD_PAREN_2(...) ((__VA_ARGS__)) _ADD_PAREN_1
#define _ADD_PAREN_1_END
#define _ADD_PAREN_2_END
#define _WRAPPED_SEQ(SEQ) BOOST_PP_CAT(_ADD_PAREN_1 SEQ, _END)

#define __INTRINSIC_NAME(LABEL, SUFFIX) LABEL##SUFFIX
#define _INTRINSIC_NAME(LABEL, SUFFIX) __INTRINSIC_NAME(LABEL,SUFFIX)

#define _REGISTER_WAVM_INTRINSIC(CLS, MOD, METHOD, WASM_SIG, NAME, SIG)\
   static Intrinsics::Function _INTRINSIC_NAME(__intrinsic_fn, __COUNTER__) (\
      MOD "." NAME,\
      eosio::chain::webassembly::wavm::wasm_function_type_provider<WASM_SIG>::type(),\
      (void *)eosio::chain::webassembly::wavm::intrinsic_function_invoker_wrapper<WASM_SIG, SIG>::type::fn<&CLS::METHOD>()\
   );\


} } } }//eosio:：chain:：webpassembly:：wavm
