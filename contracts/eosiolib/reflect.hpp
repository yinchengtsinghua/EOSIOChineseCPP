
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/stringize.hpp>

namespace eosio {
  template<typename T>
  struct reflector {
     typedef std::false_type is_reflected;
     typedef std::false_type is_enum;
  };

} ///EOSIO



#define EOSLIB_REFLECT_VISIT_BASE(r, visitor, base) \
  eosio::reflector<base>::visit( visitor );

#define EOSLIB_REFLECT_VISIT2_BASE(r, visitor, base) \
  eosio::reflector<base>::visit( t, forward<Visitor>(visitor) );


#define EOSLIB_REFLECT_VISIT_MEMBER( r, visitor, elem ) \
{ typedef decltype((static_cast<type*>(nullptr))->elem) member_type;  \
   visitor( &type::elem ); \
}

#define EOSLIB_REFLECT_VISIT2_MEMBER( r, visitor, elem ) \
{ typedef decltype((static_cast<type*>(nullptr))->elem) member_type;  \
   visitor( t.elem ); \
}


#define EOSLIB_REFLECT_BASE_MEMBER_COUNT( r, OP, elem ) \
  OP eosio::reflector<elem>::total_member_count

#define EOSLIB_REFLECT_MEMBER_COUNT( r, OP, elem ) \
  OP 1

#define EOSLIB_REFLECT_DERIVED_IMPL_INLINE( TYPE, INHERITS, MEMBERS ) \
template<typename Visitor>\
static inline void visit( Visitor&& v ) { \
    BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_VISIT_BASE, v, INHERITS ) \
    BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_VISIT_MEMBER, v, MEMBERS ) \
} \
template<typename Visitor>\
static inline void visit( const type& t, Visitor&& v ) { \
    BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_VISIT2_BASE, v, INHERITS ) \
    BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_VISIT2_MEMBER, v, MEMBERS ) \
} \
template<typename Visitor>\
static inline void visit( type& t, Visitor&& v ) { \
    BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_VISIT2_BASE, v, INHERITS ) \
    BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_VISIT2_MEMBER, v, MEMBERS ) \
}

#define EOSLIB_REFLECT_DERIVED_IMPL_EXT( TYPE, INHERITS, MEMBERS ) \
template<typename Visitor>\
void eosio::reflector<TYPE>::visit( Visitor&& v ) { \
    BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_VISIT_BASE, v, INHERITS ) \
    BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_VISIT_MEMBER, v, MEMBERS ) \
}


/*
 *@addtogroup序列化cpp
 *@
 **/


/*
 *执行类反射
 *
 *@brief专门为类型提供eosio:：reflector
 *@param type-要反映的类模板
 *@param members-成员名称的序列。（字段1）（字段2）（字段3）
 *
 *@见eoslib_reflect_derived
 **/

#define EOSLIB_REFLECT( TYPE, MEMBERS ) \
    EOSLIB_REFLECT_DERIVED( TYPE, BOOST_PP_SEQ_NIL, MEMBERS )

/*
 *执行类模板反射
 *
 *@brief执行类模板反射
 *@param template_args-模板参数的序列。（args1）（args2）（args3）
 *@param type-要反映的类模板
 *@param members-成员名称的序列。（字段1）（字段2）（字段3）
 **/

#define EOSLIB_REFLECT_TEMPLATE( TEMPLATE_ARGS, TYPE, MEMBERS ) \
    EOSLIB_REFLECT_DERIVED_TEMPLATE( TEMPLATE_ARGS, TYPE, BOOST_PP_SEQ_NIL, MEMBERS )

/*
 *对空类执行类反射
 *
 *@brief对空类执行类反射
 *@param type-要反映的类
 **/

#define EOSLIB_REFLECT_EMPTY( TYPE ) \
    EOSLIB_REFLECT_DERIVED( TYPE, BOOST_PP_SEQ_NIL, BOOST_PP_SEQ_NIL )

/*
 *执行类反射的正向声明
 *
 *@brief执行类反射的前向声明
 *@param type-要反映的类
 **/

#define EOSLIB_REFLECT_FWD( TYPE ) \
namespace eosio { \
  template<> struct reflector<TYPE> {\
       typedef TYPE type; \
       typedef eosio::true_type is_reflected; \
       enum  member_count_enum {  \
         local_member_count = BOOST_PP_SEQ_SIZE(MEMBERS), \
         total_member_count = local_member_count BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_BASE_MEMBER_COUNT, +, INHERITS )\
       }; \
       template<typename Visitor> static void visit( Visitor&& v ); \
       template<typename Visitor> static void visit( const type& t, Visitor&& v ); \
       template<typename Visitor> static void visit( type& t, Visitor&& v ); \
  }; }

///@

#define EOSLIB_REFLECT_DERIVED_IMPL( TYPE, MEMBERS ) \
    EOSLIB_REFLECT_IMPL_DERIVED_EXT( TYPE, BOOST_PP_SEQ_NIL, MEMBERS )

#define EOSLIB_REFLECT_IMPL( TYPE, MEMBERS ) \
    EOSLIB_REFLECT_DERIVED_IMPL_EXT( TYPE, BOOST_PP_SEQ_NIL, MEMBERS )


/*
 *@addtogroup序列化cpp
 *@
 **/


/*
 *在类型继承其他反射类的情况下执行类反射
 *
 *@brief专门为eosio:：reflector提供
 *类型继承其他反射类
 *
 *@param type-要反映的类
 *@param inherits-一个基类名序列（base a）（baseb）（basec）
 *@param members-成员名称的序列。（字段1）（字段2）（字段3）
 **/

#define EOSLIB_REFLECT_DERIVED( TYPE, INHERITS, MEMBERS ) \
namespace eosio {  \
template<> struct reflector<TYPE> {\
    typedef TYPE type; \
    typedef eosio::true_type  is_reflected; \
    typedef eosio::false_type is_enum; \
    enum  member_count_enum {  \
      local_member_count = 0  BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_MEMBER_COUNT, +, MEMBERS ),\
      total_member_count = local_member_count BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_BASE_MEMBER_COUNT, +, INHERITS )\
    }; \
    EOSLIB_REFLECT_DERIVED_IMPL_INLINE( TYPE, INHERITS, MEMBERS ) \
}; }

/*
 *在类型继承其他反射类的情况下执行类模板反射
 *
 *@brief执行类模板反射，其中类型继承其他反射类
 *
 *@param template_args-模板参数的序列。（args1）（args2）（args3）
 *@param type-要反映的类
 *@param inherits-一个基类名序列（base a）（baseb）（basec）
 *@param members-成员名称的序列。（字段1）（字段2）（字段3）
 **/

#define EOSLIB_REFLECT_DERIVED_TEMPLATE( TEMPLATE_ARGS, TYPE, INHERITS, MEMBERS ) \
namespace eosio {  \
template<BOOST_PP_SEQ_ENUM(TEMPLATE_ARGS)> struct reflector<TYPE> {\
    typedef TYPE type; \
    typedef eosio::true_type  is_defined; \
    typedef eosio::false_type is_enum; \
    enum  member_count_enum {  \
      local_member_count = 0  BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_MEMBER_COUNT, +, MEMBERS ),\
      total_member_count = local_member_count BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_BASE_MEMBER_COUNT, +, INHERITS )\
    }; \
    EOSLIB_REFLECT_DERIVED_IMPL_INLINE( TYPE, INHERITS, MEMBERS ) \
}; }


///@
