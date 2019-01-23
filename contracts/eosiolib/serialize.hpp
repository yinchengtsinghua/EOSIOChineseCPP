
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/stringize.hpp>

#define EOSLIB_REFLECT_MEMBER_OP( r, OP, elem ) \
  OP t.elem 

/*
 *@defgroup serialize序列化API
 *@brief定义要序列化和反序列化对象的函数
 *@ingroup contractdev公司
 **/


/*
 *@ DePixSerialIZEPP序列化C++ API
 *@简要定义C++ API以序列化和反序列化对象
 *@ingroup系列
 *@
 **/


/*
 *定义类的序列化和反序列化
 *
 *@brief定义类的序列化和反序列化
 *
 *@param type-定义其序列化和反序列化的类
 *@param members-成员名称的序列。（字段1）（字段2）（字段3）
 **/

#define EOSLIB_SERIALIZE( TYPE,  MEMBERS ) \
 template<typename DataStream> \
 friend DataStream& operator << ( DataStream& ds, const TYPE& t ){ \
    return ds BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_MEMBER_OP, <<, MEMBERS );\
 }\
 template<typename DataStream> \
 friend DataStream& operator >> ( DataStream& ds, TYPE& t ){ \
    return ds BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_MEMBER_OP, >>, MEMBERS );\
 } 

/*
 *为从其他类继承的类定义序列化和反序列化
 *定义其序列化和反序列化
 *
 *@brief为从其他类继承的类定义序列化和反序列化
 *定义其序列化和反序列化
 *
 *@param type-定义其序列化和反序列化的类
 *@param base-基类名的序列（base a）（baseb）（basec）
 *@param members-成员名称的序列。（字段1）（字段2）（字段3）
 **/

#define EOSLIB_SERIALIZE_DERIVED( TYPE, BASE, MEMBERS ) \
 template<typename DataStream> \
 friend DataStream& operator << ( DataStream& ds, const TYPE& t ){ \
    ds << static_cast<const BASE&>(t); \
    return ds BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_MEMBER_OP, <<, MEMBERS );\
 }\
 template<typename DataStream> \
 friend DataStream& operator >> ( DataStream& ds, TYPE& t ){ \
    ds >> static_cast<BASE&>(t); \
    return ds BOOST_PP_SEQ_FOR_EACH( EOSLIB_REFLECT_MEMBER_OP, >>, MEMBERS );\
 } 
///@序列化cpp
