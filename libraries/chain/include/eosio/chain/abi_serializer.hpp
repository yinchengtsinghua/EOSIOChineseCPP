
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

#pragma once
#include <eosio/chain/abi_def.hpp>
#include <eosio/chain/trace.hpp>
#include <eosio/chain/exceptions.hpp>
#include <fc/variant_object.hpp>
#include <fc/scoped_exit.hpp>

namespace eosio { namespace chain {

using std::map;
using std::string;
using std::function;
using std::pair;
using namespace fc;

namespace impl {
   struct abi_from_variant;
   struct abi_to_variant;

   struct abi_traverse_context;
   struct abi_traverse_context_with_path;
   struct binary_to_variant_context;
   struct variant_to_binary_context;
}

/*
 *描述二进制表示消息和表内容，以便
 *转换为JSON和JSON。
 **/

struct abi_serializer {
   abi_serializer(){ configure_built_in_types(); }
   abi_serializer( const abi_def& abi, const fc::microseconds& max_serialization_time );
   void set_abi(const abi_def& abi, const fc::microseconds& max_serialization_time);

   type_name resolve_type(const type_name& t)const;
   bool      is_array(const type_name& type)const;
   bool      is_optional(const type_name& type)const;
   bool      is_type(const type_name& type, const fc::microseconds& max_serialization_time)const;
   bool      is_builtin_type(const type_name& type)const;
   bool      is_integer(const type_name& type) const;
   int       get_integer_size(const type_name& type) const;
   bool      is_struct(const type_name& type)const;
   type_name fundamental_type(const type_name& type)const;

   const struct_def& get_struct(const type_name& type)const;

   type_name get_action_type(name action)const;
   type_name get_table_type(name action)const;

   optional<string>  get_error_message( uint64_t error_code )const;

   fc::variant binary_to_variant( const type_name& type, const bytes& binary, const fc::microseconds& max_serialization_time, bool short_path = false )const;
   fc::variant binary_to_variant( const type_name& type, fc::datastream<const char*>& binary, const fc::microseconds& max_serialization_time, bool short_path = false )const;

   bytes       variant_to_binary( const type_name& type, const fc::variant& var, const fc::microseconds& max_serialization_time, bool short_path = false )const;
   void        variant_to_binary( const type_name& type, const fc::variant& var, fc::datastream<char*>& ds, const fc::microseconds& max_serialization_time, bool short_path = false )const;

   template<typename T, typename Resolver>
   static void to_variant( const T& o, fc::variant& vo, Resolver resolver, const fc::microseconds& max_serialization_time );

   template<typename T, typename Resolver>
   static void from_variant( const fc::variant& v, T& o, Resolver resolver, const fc::microseconds& max_serialization_time );

   template<typename Vec>
   static bool is_empty_abi(const Vec& abi_vec)
   {
      return abi_vec.size() <= 4;
   }

   template<typename Vec>
   static bool to_abi(const Vec& abi_vec, abi_def& abi)
   {
if( !is_empty_abi(abi_vec) ) { ///4==空ABI的packsize
         fc::datastream<const char*> ds( abi_vec.data(), abi_vec.size() );
         fc::raw::unpack( ds, abi );
         return true;
      }
      return false;
   }

   typedef std::function<fc::variant(fc::datastream<const char*>&, bool, bool)>  unpack_function;
   typedef std::function<void(const fc::variant&, fc::datastream<char*>&, bool, bool)>  pack_function;

   void add_specialized_unpack_pack( const string& name, std::pair<abi_serializer::unpack_function, abi_serializer::pack_function> unpack_pack );

static const size_t max_recursion_depth = 32; //任意深度以防止无限递归

private:

   map<type_name, type_name>     typedefs;
   map<type_name, struct_def>    structs;
   map<name,type_name>           actions;
   map<name,type_name>           tables;
   map<uint64_t, string>         error_messages;
   map<type_name, variant_def>   variants;

   map<type_name, pair<unpack_function, pack_function>> built_in_types;
   void configure_built_in_types();

   fc::variant _binary_to_variant( const type_name& type, const bytes& binary, impl::binary_to_variant_context& ctx )const;
   fc::variant _binary_to_variant( const type_name& type, fc::datastream<const char*>& binary, impl::binary_to_variant_context& ctx )const;
   void        _binary_to_variant( const type_name& type, fc::datastream<const char*>& stream,
                                   fc::mutable_variant_object& obj, impl::binary_to_variant_context& ctx )const;

   bytes       _variant_to_binary( const type_name& type, const fc::variant& var, impl::variant_to_binary_context& ctx )const;
   void        _variant_to_binary( const type_name& type, const fc::variant& var,
                                   fc::datastream<char*>& ds, impl::variant_to_binary_context& ctx )const;

   static type_name _remove_bin_extension(const type_name& type);
   bool _is_type( const type_name& type, impl::abi_traverse_context& ctx )const;

   void validate( impl::abi_traverse_context& ctx )const;

   friend struct impl::abi_from_variant;
   friend struct impl::abi_to_variant;
   friend struct impl::abi_traverse_context_with_path;
};

namespace impl {

   struct abi_traverse_context {
      abi_traverse_context( fc::microseconds max_serialization_time )
      : max_serialization_time( max_serialization_time ), deadline( fc::time_point::now() + max_serialization_time ), recursion_depth(0)
      {}

      abi_traverse_context( fc::microseconds max_serialization_time, fc::time_point deadline )
      : max_serialization_time( max_serialization_time ), deadline( deadline ), recursion_depth(0)
      {}

      void check_deadline()const;

      fc::scoped_exit<std::function<void()>> enter_scope();

   protected:
      fc::microseconds max_serialization_time;
      fc::time_point   deadline;
      size_t           recursion_depth;
   };

   struct empty_path_root {};

   struct array_type_path_root {
   };

   struct struct_type_path_root {
      map<type_name, struct_def>::const_iterator  struct_itr;
   };

   struct variant_type_path_root {
      map<type_name, variant_def>::const_iterator variant_itr;
   };

   using path_root = static_variant<empty_path_root, array_type_path_root, struct_type_path_root, variant_type_path_root>;

   struct empty_path_item {};

   struct array_index_path_item {
      path_root                                   type_hint;
      uint32_t                                    array_index = 0;
   };

   struct field_path_item {
      map<type_name, struct_def>::const_iterator  parent_struct_itr;
      uint32_t                                    field_ordinal = 0;
   };

   struct variant_path_item {
      map<type_name, variant_def>::const_iterator variant_itr;
      uint32_t                                    variant_ordinal = 0;
   };

   using path_item = static_variant<empty_path_item, array_index_path_item, field_path_item, variant_path_item>;

   struct abi_traverse_context_with_path : public abi_traverse_context {
      abi_traverse_context_with_path( const abi_serializer& abis, fc::microseconds max_serialization_time, const type_name& type )
      : abi_traverse_context( max_serialization_time ), abis(abis)
      {
         set_path_root(type);
      }

      abi_traverse_context_with_path( const abi_serializer& abis, fc::microseconds max_serialization_time, fc::time_point deadline, const type_name& type )
      : abi_traverse_context( max_serialization_time, deadline ), abis(abis)
      {
         set_path_root(type);
      }

      abi_traverse_context_with_path( const abi_serializer& abis, const abi_traverse_context& ctx, const type_name& type )
      : abi_traverse_context(ctx), abis(abis)
      {
         set_path_root(type);
      }

      void set_path_root( const type_name& type );

      fc::scoped_exit<std::function<void()>> push_to_path( const path_item& item );

      void set_array_index_of_path_back( uint32_t i );
      void hint_array_type_if_in_array();
      void hint_struct_type_if_in_array( const map<type_name, struct_def>::const_iterator& itr );
      void hint_variant_type_if_in_array( const map<type_name, variant_def>::const_iterator& itr );

      string get_path_string()const;

      string maybe_shorten( const string& str );

   protected:
      const abi_serializer&  abis;
      path_root              root_of_path;
      vector<path_item>      path;
   public:
      bool                   short_path = false;
   };

   struct binary_to_variant_context : public abi_traverse_context_with_path {
      using abi_traverse_context_with_path::abi_traverse_context_with_path;
   };

   struct variant_to_binary_context : public abi_traverse_context_with_path {
      using abi_traverse_context_with_path::abi_traverse_context_with_path;

      fc::scoped_exit<std::function<void()>> disallow_extensions_unless( bool condition );

      bool extensions_allowed()const { return allow_extensions; }

   protected:
      bool                   allow_extensions = true;
   };

   /*
    *确定类型是否包含与ABI相关的信息，可能是深嵌套的
    *@tparam t-要检查的类型
    **/

   template<typename T>
   constexpr bool single_type_requires_abi_v() {
      return std::is_base_of<transaction, T>::value ||
             std::is_same<T, packed_transaction>::value ||
             std::is_same<T, transaction_trace>::value ||
             std::is_same<T, transaction_receipt>::value ||
             std::is_same<T, base_action_trace>::value ||
             std::is_same<T, action_trace>::value ||
             std::is_same<T, signed_transaction>::value ||
             std::is_same<T, signed_block>::value ||
             std::is_same<T, action>::value;
   }

   /*
    *basic constexpr对于类型，直接为基本检查起别名。
    *@tparam t-要检查的类型
    **/

   template<typename T>
   struct type_requires_abi {
      static constexpr bool value() {
         return single_type_requires_abi_v<T>();
      }
   };

   /*
    *捕获常见容器模式并检查其包含类型的专门化
    *@tparam container-其第一个参数是包含类型的模板化容器类型
    **/

   template<template<typename ...> class Container, typename T, typename ...Args >
   struct type_requires_abi<Container<T, Args...>> {
      static constexpr bool value() {
         return single_type_requires_abi_v<T>();
      }
   };

   template<typename T>
   constexpr bool type_requires_abi_v() {
      return type_requires_abi<T>::value();
   }

   /*
    *根据类型是否包含ABI相关信息创建过载保护的方便别名
    **/

   template<typename T>
   using not_require_abi_t = std::enable_if_t<!type_requires_abi_v<T>(), int>;

   template<typename T>
   using require_abi_t = std::enable_if_t<type_requires_abi_v<T>(), int>;

   struct abi_to_variant {
      /*
       *重载为不依赖于ABI信息的类型添加的模板
       *并且可以降级为普通：：到_变量（…）处理
       **/

      template<typename M, typename Resolver, not_require_abi_t<M> = 1>
      static void add( mutable_variant_object &mvo, const char* name, const M& v, Resolver, abi_traverse_context& ctx )
      {
         auto h = ctx.enter_scope();
         mvo(name,v);
      }

      /*
       *为树中包含ABI信息的类型添加重载的模板
       *对于这些类型，我们创建新的ABI感知访客
       **/

      template<typename M, typename Resolver, require_abi_t<M> = 1>
      static void add( mutable_variant_object &mvo, const char* name, const M& v, Resolver resolver, abi_traverse_context& ctx );

      /*
       *重载添加的模板用于在树中包含ABI信息的类型的向量
       *对于这些成员，我们调用：：add以触发进一步的处理。
       **/

      template<typename M, typename Resolver, require_abi_t<M> = 1>
      static void add( mutable_variant_object &mvo, const char* name, const vector<M>& v, Resolver resolver, abi_traverse_context& ctx )
      {
         auto h = ctx.enter_scope();
         vector<variant> array;
         array.reserve(v.size());

         for (const auto& iter: v) {
            mutable_variant_object elem_mvo;
            add(elem_mvo, "_", iter, resolver, ctx);
            array.emplace_back(std::move(elem_mvo["_"]));
         }
         mvo(name, std::move(array));
      }

      /*
       *重载添加的模板用于共享类型，这些类型在其树中包含ABI信息。
       *对于这些成员，我们调用：：add以触发进一步的处理。
       **/

      template<typename M, typename Resolver, require_abi_t<M> = 1>
      static void add( mutable_variant_object &mvo, const char* name, const std::shared_ptr<M>& v, Resolver resolver, abi_traverse_context& ctx )
      {
         auto h = ctx.enter_scope();
         if( !v ) return;
         mutable_variant_object obj_mvo;
         add(obj_mvo, "_", *v, resolver, ctx);
         mvo(name, std::move(obj_mvo["_"]));
      }

      template<typename Resolver>
      struct add_static_variant
      {
         mutable_variant_object& obj_mvo;
         Resolver& resolver;
         abi_traverse_context& ctx;

         add_static_variant( mutable_variant_object& o, Resolver& r, abi_traverse_context& ctx )
               :obj_mvo(o), resolver(r), ctx(ctx) {}

         typedef void result_type;
         template<typename T> void operator()( T& v )const
         {
            add(obj_mvo, "_", v, resolver, ctx);
         }
      };

      template<typename Resolver, typename... Args>
      static void add( mutable_variant_object &mvo, const char* name, const fc::static_variant<Args...>& v, Resolver resolver, abi_traverse_context& ctx )
      {
         auto h = ctx.enter_scope();
         mutable_variant_object obj_mvo;
         add_static_variant<Resolver> adder(obj_mvo, resolver, ctx);
         v.visit(adder);
         mvo(name, std::move(obj_mvo["_"]));
      }

      /*
       *操作的to_variant_对象过载
       *@t参数分解器
       *@帕拉姆法案
       *@参数分解器
       *@返回
       **/

      template<typename Resolver>
      static void add( mutable_variant_object &out, const char* name, const action& act, Resolver resolver, abi_traverse_context& ctx )
      {
         auto h = ctx.enter_scope();
         mutable_variant_object mvo;
         mvo("account", act.account);
         mvo("name", act.name);
         mvo("authorization", act.authorization);

         try {
            auto abi = resolver(act.account);
            if (abi.valid()) {
               auto type = abi->get_action_type(act.name);
               if (!type.empty()) {
                  try {
                     binary_to_variant_context _ctx(*abi, ctx, type);
_ctx.short_path = true; //为了安全起见，同时避免在整个地方执行重写布尔值的复杂性
                     mvo( "data", abi->_binary_to_variant( type, act.data, _ctx ));
                     mvo("hex_data", act.data);
                  } catch(...) {
//序列化数据失败，则保留为未序列化
                     mvo("data", act.data);
                  }
               } else {
                  mvo("data", act.data);
               }
            } else {
               mvo("data", act.data);
            }
         } catch(...) {
            mvo("data", act.data);
         }
         out(name, std::move(mvo));
      }

      /*
       *压缩事务的to_variant_对象过载
       *@t参数分解器
       *@帕拉姆法案
       *@参数分解器
       *@返回
       **/

      template<typename Resolver>
      static void add( mutable_variant_object &out, const char* name, const packed_transaction& ptrx, Resolver resolver, abi_traverse_context& ctx )
      {
         auto h = ctx.enter_scope();
         mutable_variant_object mvo;
         auto trx = ptrx.get_transaction();
         mvo("id", trx.id());
         mvo("signatures", ptrx.get_signatures());
         mvo("compression", ptrx.get_compression());
         mvo("packed_context_free_data", ptrx.get_packed_context_free_data());
         mvo("context_free_data", ptrx.get_context_free_data());
         mvo("packed_trx", ptrx.get_packed_transaction());
         add(mvo, "transaction", trx, resolver, ctx);

         out(name, std::move(mvo));
      }
   };

   /*
    *使用冲突解决程序为嵌套类型解析ABI的反射访问者
    *一旦类型不再包含
    *ABI相关信息
    *
    *@tparam resvorer-可通过签名调用（const name&code_account）->可选<abi_def>
    **/

   template<typename T, typename Resolver>
   class abi_to_variant_visitor
   {
      public:
         abi_to_variant_visitor( mutable_variant_object& _mvo, const T& _val, Resolver _resolver, abi_traverse_context& _ctx )
         :_vo(_mvo)
         ,_val(_val)
         ,_resolver(_resolver)
         ,_ctx(_ctx)
         {}

         /*
          *访问单个成员并将其添加到variant对象
          *@tparam成员-要访问的成员
          *@tparam类-我们正在遍历的类
          *@tparam member-指向成员的指针
          *@param name-成员的名称
          **/

         template<typename Member, class Class, Member (Class::*member) >
         void operator()( const char* name )const
         {
            abi_to_variant::add( _vo, name, (_val.*member), _resolver, _ctx );
         }

      private:
         mutable_variant_object& _vo;
         const T& _val;
         Resolver _resolver;
         abi_traverse_context& _ctx;
   };

   struct abi_from_variant {
      /*
       *重载提取不依赖于ABI信息的类型的模板
       *并且可以从_variant（…）处理降级到正常：：。
       **/

      template<typename M, typename Resolver, not_require_abi_t<M> = 1>
      static void extract( const variant& v, M& o, Resolver, abi_traverse_context& ctx )
      {
         auto h = ctx.enter_scope();
         from_variant(v, o);
      }

      /*
       *为树中包含ABI信息的类型重载提取的模板
       *对于这些类型，我们创建新的ABI感知访客
       **/

      template<typename M, typename Resolver, require_abi_t<M> = 1>
      static void extract( const variant& v, M& o, Resolver resolver, abi_traverse_context& ctx );

      /*
       *重载提取树中包含ABI信息的类型向量的模板
       *对于这些成员，我们调用：：extract以触发进一步的处理。
       **/

      template<typename M, typename Resolver, require_abi_t<M> = 1>
      static void extract( const variant& v, vector<M>& o, Resolver resolver, abi_traverse_context& ctx )
      {
         auto h = ctx.enter_scope();
         const variants& array = v.get_array();
         o.clear();
         o.reserve( array.size() );
         for( auto itr = array.begin(); itr != array.end(); ++itr ) {
            M o_iter;
            extract(*itr, o_iter, resolver, ctx);
            o.emplace_back(std::move(o_iter));
         }
      }

      /*
       *重载提取共享资源类型的模板，这些类型在其树中包含ABI信息
       *对于这些成员，我们调用：：extract以触发进一步的处理。
       **/

      template<typename M, typename Resolver, require_abi_t<M> = 1>
      static void extract( const variant& v, std::shared_ptr<M>& o, Resolver resolver, abi_traverse_context& ctx )
      {
         auto h = ctx.enter_scope();
         const variant_object& vo = v.get_object();
         M obj;
         extract(vo, obj, resolver, ctx);
         o = std::make_shared<M>(obj);
      }

      /*
       *对操作结构具有优先级的非模板化重载
       *此类型具有必须由ABI直接翻译的成员，因此
       *明确分解和处理
       **/

      template<typename Resolver>
      static void extract( const variant& v, action& act, Resolver resolver, abi_traverse_context& ctx )
      {
         auto h = ctx.enter_scope();
         const variant_object& vo = v.get_object();
         EOS_ASSERT(vo.contains("account"), packed_transaction_type_exception, "Missing account");
         EOS_ASSERT(vo.contains("name"), packed_transaction_type_exception, "Missing name");
         from_variant(vo["account"], act.account);
         from_variant(vo["name"], act.name);

         if (vo.contains("authorization")) {
            from_variant(vo["authorization"], act.authorization);
         }

         bool valid_empty_data = false;
         if( vo.contains( "data" ) ) {
            const auto& data = vo["data"];
            if( data.is_string() ) {
               from_variant(data, act.data);
               valid_empty_data = act.data.empty();
            } else if ( data.is_object() ) {
               auto abi = resolver(act.account);
               if (abi.valid()) {
                  auto type = abi->get_action_type(act.name);
                  if (!type.empty()) {
                     variant_to_binary_context _ctx(*abi, ctx, type);
_ctx.short_path = true; //为了安全起见，同时避免在整个地方执行重写布尔值的复杂性
                     act.data = std::move( abi->_variant_to_binary( type, data, _ctx ));
                     valid_empty_data = act.data.empty();
                  }
               }
            }
         }

         if( !valid_empty_data && act.data.empty() ) {
            if( vo.contains( "hex_data" ) ) {
               const auto& data = vo["hex_data"];
               if( data.is_string() ) {
                  from_variant(data, act.data);
               }
            }
         }

         EOS_ASSERT(valid_empty_data || !act.data.empty(), packed_transaction_type_exception,
                    "Failed to deserialize data for ${account}:${name}", ("account", act.account)("name", act.name));
      }

      template<typename Resolver>
      static void extract( const variant& v, packed_transaction& ptrx, Resolver resolver, abi_traverse_context& ctx )
      {
         auto h = ctx.enter_scope();
         const variant_object& vo = v.get_object();
         EOS_ASSERT(vo.contains("signatures"), packed_transaction_type_exception, "Missing signatures");
         EOS_ASSERT(vo.contains("compression"), packed_transaction_type_exception, "Missing compression");
         std::vector<signature_type> signatures;
         packed_transaction::compression_type compression;
         from_variant(vo["signatures"], signatures);
         from_variant(vo["compression"], compression);

         bytes packed_cfd;
         std::vector<bytes> cfd;
         bool use_packed_cfd = false;
         if( vo.contains("packed_context_free_data") && vo["packed_context_free_data"].is_string() && !vo["packed_context_free_data"].as_string().empty() ) {
            from_variant(vo["packed_context_free_data"], packed_cfd );
            use_packed_cfd = true;
         } else if( vo.contains("context_free_data") ) {
            from_variant(vo["context_free_data"], cfd);
         }

         if( vo.contains("packed_trx") && vo["packed_trx"].is_string() && !vo["packed_trx"].as_string().empty() ) {
            bytes packed_trx;
            from_variant(vo["packed_trx"], packed_trx);
            if( use_packed_cfd ) {
               ptrx = packed_transaction( std::move( packed_trx ), std::move( signatures ), std::move( packed_cfd ), compression );
            } else {
               ptrx = packed_transaction( std::move( packed_trx ), std::move( signatures ), std::move( cfd ), compression );
            }
         } else {
            EOS_ASSERT(vo.contains("transaction"), packed_transaction_type_exception, "Missing transaction");
            if( use_packed_cfd ) {
               transaction trx;
               extract( vo["transaction"], trx, resolver, ctx );
               ptrx = packed_transaction( std::move(trx), std::move(signatures), std::move(packed_cfd), compression );
            } else {
               signed_transaction trx;
               extract( vo["transaction"], trx, resolver, ctx );
               trx.signatures = std::move( signatures );
               trx.context_free_data = std::move(cfd);
               ptrx = packed_transaction( std::move( trx ), compression );
            }
         }
      }
   };

   /*
    *使用冲突解决程序为嵌套类型解析ABI的反射访问者
    *一旦类型不再包含
    *ABI相关信息
    *
    *@tparam resvorer-可通过签名调用（const name&code_account）->可选<abi_def>
    **/

   template<typename T, typename Resolver>
   class abi_from_variant_visitor : reflector_init_visitor<T>
   {
      public:
         abi_from_variant_visitor( const variant_object& _vo, T& v, Resolver _resolver, abi_traverse_context& _ctx )
         : reflector_init_visitor<T>(v)
         ,_vo(_vo)
         ,_resolver(_resolver)
         ,_ctx(_ctx)
         {}

         /*
          *访问单个成员并从variant对象中提取它
          *@tparam成员-要访问的成员
          *@tparam类-我们正在遍历的类
          *@tparam member-指向成员的指针
          *@param name-成员的名称
          **/

         template<typename Member, class Class, Member (Class::*member)>
         void operator()( const char* name )const
         {
            auto itr = _vo.find(name);
            if( itr != _vo.end() )
               abi_from_variant::extract( itr->value(), this->obj.*member, _resolver, _ctx );
         }

      private:
         const variant_object& _vo;
         Resolver _resolver;
         abi_traverse_context& _ctx;
   };

   template<typename M, typename Resolver, require_abi_t<M>>
   void abi_to_variant::add( mutable_variant_object &mvo, const char* name, const M& v, Resolver resolver, abi_traverse_context& ctx )
   {
      auto h = ctx.enter_scope();
      mutable_variant_object member_mvo;
      fc::reflector<M>::visit( impl::abi_to_variant_visitor<M, Resolver>( member_mvo, v, resolver, ctx) );
      mvo(name, std::move(member_mvo));
   }

   template<typename M, typename Resolver, require_abi_t<M>>
   void abi_from_variant::extract( const variant& v, M& o, Resolver resolver, abi_traverse_context& ctx )
   {
      auto h = ctx.enter_scope();
      const variant_object& vo = v.get_object();
      fc::reflector<M>::visit( abi_from_variant_visitor<M, decltype(resolver)>( vo, o, resolver, ctx ) );
   }
} ///namespace eosio:：chain:：impl

template<typename T, typename Resolver>
void abi_serializer::to_variant( const T& o, variant& vo, Resolver resolver, const fc::microseconds& max_serialization_time ) try {
   mutable_variant_object mvo;
   impl::abi_traverse_context ctx(max_serialization_time);
   impl::abi_to_variant::add(mvo, "_", o, resolver, ctx);
   vo = std::move(mvo["_"]);
} FC_RETHROW_EXCEPTIONS(error, "Failed to serialize type", ("object",o))

template<typename T, typename Resolver>
void abi_serializer::from_variant( const variant& v, T& o, Resolver resolver, const fc::microseconds& max_serialization_time ) try {
   impl::abi_traverse_context ctx(max_serialization_time);
   impl::abi_from_variant::extract(v, o, resolver, ctx);
} FC_RETHROW_EXCEPTIONS(error, "Failed to deserialize variant", ("variant",v))


} } //EOSIO：链
