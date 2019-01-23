
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
/*
 *@文件datastream.hpp
 *@eos/license中定义的版权
 **/

#pragma once
#include <eosiolib/system.h>
#include <eosiolib/memory.h>
#include <eosiolib/vector.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <eosiolib/varint.hpp>
#include <array>
#include <set>
#include <map>
#include <string>

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>

#include <boost/pfr.hpp>


namespace eosio {

/*
 *@defgroup数据流数据流
 *@brief以字节的形式定义用于读写数据的数据流
 *@ingroup系列
 *@
 **/


/*
 *%a以字节形式读取和写入数据的数据流
 *
 *@brief%以字节形式读取和写入数据的数据流。
 *@tparam t-数据流缓冲区的类型
 **/

template<typename T>
class datastream {
   public:
      /*
       *根据缓冲区的大小和缓冲区的起始位置构造一个新的数据流对象。
       *
       *@brief构造一个新的数据流对象
       *@param start-缓冲区的起始位置
       *@param s-缓冲区的大小
       **/

      datastream( T start, size_t s )
      :_start(start),_pos(start),_end(start+s){}

     /*
      *跳过此流中的指定字节数
      *
      *@brief跳过此流中的特定字节数
      *@param s-要跳过的字节数
      **/

      inline void skip( size_t s ){ _pos += s; }

     /*
      *将流中指定数量的字节读取到缓冲区中
      *
      *@brief将此流中指定数量的字节读取到缓冲区中
      *@param d-指向目标缓冲区的指针
      *@param s-要读取的字节数
      *@返回真
      **/

      inline bool read( char* d, size_t s ) {
        eosio_assert( size_t(_end - _pos) >= (size_t)s, "read" );
        memcpy( d, _pos, s );
        _pos += s;
        return true;
      }

     /*
      *从缓冲区向流中写入指定数量的字节
      *
      *@brief从缓冲区向流中写入指定数量的字节
      *@param d-指向源缓冲区的指针
      *@param s-要写入的字节数
      *@返回真
      **/

      inline bool write( const char* d, size_t s ) {
        eosio_assert( _end - _pos >= (int32_t)s, "write" );
        memcpy( (void*)_pos, d, s );
        _pos += s;
        return true;
      }

     /*
      *将一个字节写入流中
      *
      *@brief向流中写入一个字节
      *@param c要写入的字节
      *@返回真
      **/

      inline bool put(char c) {
        eosio_assert( _pos < _end, "put" );
        *_pos = c;
        ++_pos;
        return true;
      }

     /*
      *从流中读取一个字节
      *
      *@brief从流中读取一个字节
      *@param c-对目标字节的引用
      *@返回真
      **/

      inline bool get( unsigned char& c ) { return get( *(char*)&c ); }

     /*
      *从流中读取一个字节
      *
      *@brief从流中读取一个字节
      *@param c-对目标字节的引用
      *@返回真
      **/

      inline bool get( char& c )
      {
        eosio_assert( _pos < _end, "get" );
        c = *_pos;
        ++_pos;
        return true;
      }

     /*
      *检索流的当前位置
      *
      *@brief检索流的当前位置
      *@返回t-流的当前位置
      **/

      T pos()const { return _pos; }
      inline bool valid()const { return _pos <= _end && _pos >= _start;  }

     /*
      *设置当前流中的位置
      *
      *@brief设置当前流中的位置
      *@param p-相对于原点的偏移量
      *@如果p在范围内，则返回true
      *@如果p不在RAWNGE中，则返回FALSE
      **/

      inline bool seekp(size_t p) { _pos = _start + p; return _pos <= _end; }

     /*
      *获取当前流中的位置
      *
      *@brief获取当前流中的位置
      *@返回p-当前流中的位置
      **/

      inline size_t tellp()const      { return size_t(_pos - _start); }

     /*
      *返回可读取/跳过的剩余字节数。
      *
      *@brief返回可以读取/跳过的剩余字节数
      *@返回大小\u t-剩余字节数
      **/

      inline size_t remaining()const  { return _end - _pos; }
    private:
      /*
       *缓冲器的起始位置
       *
       *@简述缓冲器的起始位置
       **/

      T _start;
      /*
       *缓冲器的当前位置
       *
       *@简述缓冲器的当前位置
       **/

      T _pos;
      /*
       *缓冲器的末端位置
       *
       *@简述缓冲区的结束位置
       **/

      T _end;
};

/*
 *@brief专用化数据流，用于帮助确定序列化值的最终大小。
 *数据流的专门化，用于帮助确定序列化值的最终大小
 **/

template<>
class datastream<size_t> {
   public:
      /*
       *在给定初始大小的情况下构造新的专用数据流对象
       *
       *@brief构造新的专用数据流对象
       *@param init_size-初始大小
       **/

     datastream( size_t init_size = 0):_size(init_size){}

     /*
      *将大小增加s。这与write（const char*，size_t s）的行为相同。
      *
      *@brief将大小增加s
      *@param s-要增加的大小
      *@返回真
      **/

     inline bool     skip( size_t s )                 { _size += s; return true;  }

     /*
      *将大小增加s。这与skip（大小t s）的行为相同。
      *
      *@brief将大小增加s
      *@param s-要增加的大小
      *@返回真
      **/

     inline bool     write( const char* ,size_t s )  { _size += s; return true;  }

     /*
      *尺寸增加1
      *
      *@brief将大小增加一个
      *@返回真
      **/

     inline bool     put(char )                      { ++_size; return  true;    }

     /*
      *检查有效性。它总是有效的
      *
      *@简短检查有效性
      *@返回真
      **/

     inline bool     valid()const                     { return true;              }

     /*
      *设置新的大小
      *
      *@brief设置新大小
      *@param p-新尺寸
      *@返回真
      **/

     inline bool     seekp(size_t p)                  { _size = p;  return true;  }

     /*
      *获取大小
      *
      *@brief获取大小
      *@返回大小\u t-大小
      **/

     inline size_t   tellp()const                     { return _size;             }

     /*
      *始终返回0
      *
      *@brief始终返回0
      *@返回大小\u t-0
      **/

     inline size_t   remaining()const                 { return 0;                 }
  private:
     /*
      *用于确定序列化值的最终大小的大小。
      *
      *@brief用于确定序列化值的最终大小的大小。
      **/

     size_t _size;
};

/*
 *将公钥序列化到流中
 *
 *@brief序列化一个公钥
 *@param ds-要写入的流
 *@param pubkey-要序列化的值
 *@tparam stream-数据流缓冲区的类型
 *@return datastream<stream>&-对数据流的引用
 **/

template<typename Stream>
inline datastream<Stream>& operator<<(datastream<Stream>& ds, const public_key pubkey) {
  ds.write( (const char*)&pubkey, sizeof(pubkey));
  return ds;
}

/*
 *从流中反序列化公钥
 *
 *@brief反序列化公钥
 *@param ds-要读取的流
 *@param pubkey-反序列化值的目标
 *@tparam stream-数据流缓冲区的类型
 *@return datastream<stream>&-对数据流的引用
 **/

template<typename Stream>
inline datastream<Stream>& operator>>(datastream<Stream>& ds, public_key& pubkey) {
  ds.read((char*)&pubkey, sizeof(pubkey));
  return ds;
}

/*
 *将key256序列化为流
 *
 *@brief序列化键256
 *@param ds-要写入的流
 *@param d-要序列化的值
 *@tparam stream-数据流缓冲区的类型
 *@return datastream<stream>&-对数据流的引用
 **/

template<typename Stream>
inline datastream<Stream>& operator<<(datastream<Stream>& ds, const key256& d) {
  ds.write( (const char*)d.data(), d.size() );
  return ds;
}

/*
 *从流中反序列化key256
 *
 *@brief反序列化键256
 *@param ds-要读取的流
 *@param d-反序列化值的目标
 *@tparam stream-数据流缓冲区的类型
 *@return datastream<stream>&-对数据流的引用
 **/

template<typename Stream>
inline datastream<Stream>& operator>>(datastream<Stream>& ds, key256& d) {
  ds.read((char*)d.data(), d.size() );
  return ds;
}

/*
 *将bool序列化为流
 *
 *@brief将bool序列化为流
 *@param ds-要读取的流
 *@param d-要序列化的值
 *@tparam stream-数据流缓冲区的类型
 *@return datastream<stream>&-对数据流的引用
 **/

template<typename Stream>
inline datastream<Stream>& operator<<(datastream<Stream>& ds, const bool& d) {
  return ds << uint8_t(d);
}

/*
 *从流中反序列化bool
 *
 *@brief反序列化bool
 *@param ds-要读取的流
 *@param d-反序列化值的目标
 *@tparam stream-数据流缓冲区的类型
 *@return datastream<stream>&-对数据流的引用
 **/

template<typename Stream>
inline datastream<Stream>& operator>>(datastream<Stream>& ds, bool& d) {
  uint8_t t;
  ds >> t;
  d = t;
  return ds;
}

/*
 *将校验和256序列化到流中
 *
 *@brief序列化校验和256
 *@param ds-要写入的流
 *@param d-要序列化的值
 *@tparam stream-数据流缓冲区的类型
 *@return datastream<stream>&-对数据流的引用
 **/

template<typename Stream>
inline datastream<Stream>& operator<<(datastream<Stream>& ds, const checksum256& d) {
   ds.write( (const char*)&d.hash[0], sizeof(d.hash) );
   return ds;
}

/*
 *从流中反序列化校验和256
 *
 *@brief反序列化校验和256
 *@param ds-要读取的流
 *@param d-反序列化值的目标
 *@tparam stream-数据流缓冲区的类型
 *@return datastream<stream>&-对数据流的引用
 **/

template<typename Stream>
inline datastream<Stream>& operator>>(datastream<Stream>& ds, checksum256& d) {
   ds.read((char*)&d.hash[0], sizeof(d.hash) );
   return ds;
}

/*
 *将字符串序列化为流
 *
 *@brief序列化字符串
 *@param ds-要写入的流
 *@param v-要序列化的值
 *@tparam datastream-数据流类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream>
DataStream& operator << ( DataStream& ds, const std::string& v ) {
   ds << unsigned_int( v.size() );
   if (v.size())
      ds.write(v.data(), v.size());
   return ds;
}

/*
 *从流中反序列化字符串
 *
 *@brief反序列化字符串
 *@param ds-要读取的流
 *@param v-反序列化值的目标
 *@tparam datastream-数据流类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream>
DataStream& operator >> ( DataStream& ds, std::string& v ) {
   std::vector<char> tmp;
   ds >> tmp;
   if( tmp.size() )
      v = std::string(tmp.data(),tmp.data()+tmp.size());
   else
      v = std::string();
   return ds;
}

/*
 *将固定大小的数组序列化为流
 *
 *@brief序列化固定大小的数组
 *@param ds-要写入的流
 *@param v-要序列化的值
 *@tparam datastream-数据流类型
 *@tparam t-数组中包含的对象的类型
 *@tparam n-数组的大小
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename T, std::size_t N>
DataStream& operator << ( DataStream& ds, const std::array<T,N>& v ) {
   for( const auto& i : v )
      ds << i;
   return ds;
}


/*
 *从流中反序列化固定大小的数组
 *
 *@brief反序列化固定大小的数组
 *@param ds-要读取的流
 *@param v-反序列化值的目标
 *@tparam datastream-数据流类型
 *@tparam t-数组中包含的对象的类型
 *@tparam n-数组的大小
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename T, std::size_t N>
DataStream& operator >> ( DataStream& ds, std::array<T,N>& v ) {
   for( auto& i : v )
      ds >> i;
   return ds;
}

namespace _datastream_detail {
   /*
    *检查T型是否为指针
    *
    *@brief检查t类型是否为指针
    *@tparam t-要检查的类型
    如果t是指针，则返回true
    *@否则返回false
    **/

   template<typename T>
   constexpr bool is_pointer() {
      return std::is_pointer<T>::value ||
             std::is_null_pointer<T>::value ||
             std::is_member_pointer<T>::value;
   }

   /*
    *检查类型T是否为基元类型
    *
    *@brief检查类型t是否为基元类型
    *@tparam t-要检查的类型
    *@如果t是基元类型，则返回true
    *@否则返回false
    **/

   template<typename T>
   constexpr bool is_primitive() {
      return std::is_arithmetic<T>::value ||
             std::is_enum<T>::value;
   }
}

/*
 *指针不应序列化，因此此函数将始终引发错误
 *
 *@brief反序列化指针
 *@param ds-要读取的流
 *@tparam datastream-数据流类型
 *@tparam t-指针类型
 *@返回数据流&-对数据流的引用
 *@post如果是指针，则引发异常
 **/

template<typename DataStream, typename T, std::enable_if_t<_datastream_detail::is_pointer<T>()>* = nullptr>
DataStream& operator >> ( DataStream& ds, T ) {
   static_assert(!_datastream_detail::is_pointer<T>(), "Pointers should not be serialized" );
   return ds;
}

/*
 *序列化非基元和非指针类型的固定大小数组
 *
 *@brief序列化非基元和非指针类型的固定大小数组
 *@param ds-要写入的流
 *@param v-要序列化的值
 *@tparam datastream-数据流类型
 *@tparam t-指针类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename T, std::size_t N,
         std::enable_if_t<!_datastream_detail::is_primitive<T>() &&
                          !_datastream_detail::is_pointer<T>()>* = nullptr>
DataStream& operator << ( DataStream& ds, const T (&v)[N] ) {
   ds << unsigned_int( N );
   for( uint32_t i = 0; i < N; ++i )
      ds << v[i];
   return ds;
}

/*
 *序列化非基元类型的固定大小数组
 *
 *@brief序列化非基元类型的固定大小数组
 *@param ds-要写入的流
 *@param v-要序列化的值
 *@tparam datastream-数据流类型
 *@tparam t-指针类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename T, std::size_t N,
         std::enable_if_t<_datastream_detail::is_primitive<T>()>* = nullptr>
DataStream& operator << ( DataStream& ds, const T (&v)[N] ) {
   ds << unsigned_int( N );
   ds.write((char*)&v[0], sizeof(v));
   return ds;
}

/*
 *反序列化非基元和非指针类型的固定大小数组
 *
 *@brief反序列化非基元和非指针类型的固定大小数组
 *@param ds-要读取的流
 *@param v-反序列化值的目标
 *@tparam t-数组中包含的对象的类型
 *@tparam n-数组的大小
 *@tparam datastream-数据流类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename T, std::size_t N,
         std::enable_if_t<!_datastream_detail::is_primitive<T>() &&
                          !_datastream_detail::is_pointer<T>()>* = nullptr>
DataStream& operator >> ( DataStream& ds, T (&v)[N] ) {
   unsigned_int s;
   ds >> s;
   eosio_assert( N == s.value, "T[] size and unpacked size don't match");
   for( uint32_t i = 0; i < N; ++i )
      ds >> v[i];
   return ds;
}

/*
 *反序列化非基元类型的固定大小数组
 *
 *@brief反序列化非基元类型的固定大小数组
 *@param ds-要读取的流
 *@param v-反序列化值的目标
 *@tparam t-数组中包含的对象的类型
 *@tparam n-数组的大小
 *@tparam datastream-数据流类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename T, std::size_t N,
         std::enable_if_t<_datastream_detail::is_primitive<T>()>* = nullptr>
DataStream& operator >> ( DataStream& ds, T (&v)[N] ) {
   unsigned_int s;
   ds >> s;
   eosio_assert( N == s.value, "T[] size and unpacked size don't match");
   ds.read((char*)&v[0], sizeof(v));
   return ds;
}

/*
 *序列化char的向量
 *
 *@brief序列化char的向量
 *@param ds-要写入的流
 *@param v-要序列化的值
 *@tparam datastream-数据流类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream>
DataStream& operator << ( DataStream& ds, const vector<char>& v ) {
   ds << unsigned_int( v.size() );
   ds.write( v.data(), v.size() );
   return ds;
}

/*
 *序列化向量
 *
 *@brief序列化一个向量
 *@param ds-要写入的流
 *@param v-要序列化的值
 *@tparam datastream-数据流类型
 *@tparam t-向量中包含的对象的类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename T>
DataStream& operator << ( DataStream& ds, const vector<T>& v ) {
   ds << unsigned_int( v.size() );
   for( const auto& i : v )
      ds << i;
   return ds;
}

/*
 *反序列化char的向量
 *
 *@brief反序列化char的向量
 *@param ds-要读取的流
 *@param v-反序列化值的目标
 *@tparam datastream-数据流类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream>
DataStream& operator >> ( DataStream& ds, vector<char>& v ) {
   unsigned_int s;
   ds >> s;
   v.resize( s.value );
   ds.read( v.data(), v.size() );
   return ds;
}

/*
 *反序列化向量
 *
 *@brief反序列化向量
 *@param ds-要读取的流
 *@param v-反序列化值的目标
 *@tparam datastream-数据流类型
 *@tparam t-向量中包含的对象的类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename T>
DataStream& operator >> ( DataStream& ds, vector<T>& v ) {
   unsigned_int s;
   ds >> s;
   v.resize(s.value);
   for( auto& i : v )
      ds >> i;
   return ds;
}

template<typename DataStream, typename T>
DataStream& operator << ( DataStream& ds, const std::set<T>& s ) {
   ds << unsigned_int( s.size() );
   for( const auto& i : s ) {
      ds << i;
   }
   return ds;
}

template<typename DataStream, typename T>
DataStream& operator >> ( DataStream& ds, std::set<T>& s ) {
   s.clear();
   unsigned_int sz; ds >> sz;

   for( uint32_t i = 0; i < sz.value; ++i ) {
      T v;
      ds >> v;
      s.emplace( std::move(v) );
   }
   return ds;
}

/*
 *序列化映射
 *
 *@brief序列化地图
 *@param ds-要写入的流
 *@param m-要序列化的值
 *@tparam datastream-数据流类型
 *@tparam k-映射中包含的键的类型
 *@tparam v-映射中包含的值的类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename K, typename V>
DataStream& operator << ( DataStream& ds, const std::map<K,V>& m ) {
   ds << unsigned_int( m.size() );
   for( const auto& i : m ) {
      ds << i.first << i.second;
   }
   return ds;
}

/*
 *反序列化映射
 *
 *@brief反序列化映射
 *@param ds-要读取的流
 *@param m-反序列化值的目标
 *@tparam datastream-数据流类型
 *@tparam k-映射中包含的键的类型
 *@tparam v-映射中包含的值的类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename K, typename V>
DataStream& operator >> ( DataStream& ds, std::map<K,V>& m ) {
   m.clear();
   unsigned_int s; ds >> s;

   for (uint32_t i = 0; i < s.value; ++i) {
      K k; V v;
      ds >> k >> v;
      m.emplace( std::move(k), std::move(v) );
   }
   return ds;
}

template<typename DataStream, typename T>
DataStream& operator << ( DataStream& ds, const boost::container::flat_set<T>& s ) {
   ds << unsigned_int( s.size() );
   for( const auto& i : s ) {
      ds << i;
   }
   return ds;
}

template<typename DataStream, typename T>
DataStream& operator >> ( DataStream& ds, boost::container::flat_set<T>& s ) {
   s.clear();
   unsigned_int sz; ds >> sz;

   for( uint32_t i = 0; i < sz.value; ++i ) {
      T v;
      ds >> v;
      s.emplace( std::move(v) );
   }
   return ds;
}


/*
 *序列化平面图
 *
 *@brief序列化平面图
 *@param ds-要写入的流
 *@param m-要序列化的值
 *@tparam datastream-数据流类型
 *@tparam k-平面图中包含的键的类型
 *@tparam v-平面图中包含的值的类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename K, typename V>
DataStream& operator<<( DataStream& ds, const boost::container::flat_map<K,V>& m ) {
   ds << unsigned_int( m.size() );
   for( const auto& i : m )
      ds << i.first << i.second;
   return ds;
}

/*
 *反序列化平面图
 *
 *@brief反序列化平面图
 *@param ds-要读取的流
 *@param m-反序列化值的目标
 *@tparam datastream-数据流类型
 *@tparam k-平面图中包含的键的类型
 *@tparam v-平面图中包含的值的类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename K, typename V>
DataStream& operator>>( DataStream& ds, boost::container::flat_map<K,V>& m ) {
   m.clear();
   unsigned_int s; ds >> s;

   for( uint32_t i = 0; i < s.value; ++i ) {
      K k; V v;
      ds >> k >> v;
      m.emplace( std::move(k), std::move(v) );
   }
   return ds;
}

/*
 *序列化元组
 *
 *@brief序列化元组
 *@param ds-要写入的流
 *@param t-要序列化的值
 *@tparam datastream-数据流类型
 *@tparam args-元组中包含的对象的类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename... Args>
DataStream& operator<<( DataStream& ds, const std::tuple<Args...>& t ) {
   boost::fusion::for_each( t, [&]( const auto& i ) {
       ds << i;
   });
   return ds;
}

/*
 *反序列化元组
 *
 *@brief反序列化元组
 *@param ds-要读取的流
 *@param t-反序列化值的目标
 *@tparam datastream-数据流类型
 *@tparam args-元组中包含的对象的类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename... Args>
DataStream& operator>>( DataStream& ds, std::tuple<Args...>& t ) {
   boost::fusion::for_each( t, [&]( auto& i ) {
       ds >> i;
   });
   return ds;
}

/*
 *序列化类
 *
 *@brief序列化类
 *@param ds-要写入的流
 *@param v-要序列化的值
 *@tparam datastream-数据流类型
 *@tparam t-类的类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename T, std::enable_if_t<std::is_class<T>::value>* = nullptr>
DataStream& operator<<( DataStream& ds, const T& v ) {
   boost::pfr::for_each_field(v, [&](const auto& field) {
      ds << field;
   });
   return ds;
}

/*
 *反序列化类
 *
 *@brief反序列化类
 *@param ds-要读取的流
 *@param v-反序列化值的目标
 *@tparam datastream-数据流类型
 *@tparam t-类的类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename T, std::enable_if_t<std::is_class<T>::value>* = nullptr>
DataStream& operator>>( DataStream& ds, T& v ) {
   boost::pfr::for_each_field(v, [&](auto& field) {
      ds >> field;
   });
   return ds;
}

/*
 *序列化基元类型
 *
 *@brief序列化基元类型
 *@param ds-要写入的流
 *@param v-要序列化的值
 *@tparam datastream-数据流类型
 *@tparam t-基元类型的类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename T, std::enable_if_t<_datastream_detail::is_primitive<T>()>* = nullptr>
DataStream& operator<<( DataStream& ds, const T& v ) {
   ds.write( (const char*)&v, sizeof(T) );
   return ds;
}

/*
 *反序列化基元类型
 *
 *@brief反序列化基元类型
 *@param ds-要读取的流
 *@param v-反序列化值的目标
 *@tparam datastream-数据流类型
 *@tparam t-基元类型的类型
 *@返回数据流&-对数据流的引用
 **/

template<typename DataStream, typename T, std::enable_if_t<_datastream_detail::is_primitive<T>()>* = nullptr>
DataStream& operator>>( DataStream& ds, T& v ) {
   ds.read( (char*)&v, sizeof(T) );
   return ds;
}

/*
 *将固定大小缓冲区内的数据解包为t
 *
 *@brief将固定大小缓冲区内的数据解包为t
 *@tparam t-解包数据的类型
 *@param buffer-指向缓冲区的指针
 *@param len-缓冲区长度
 *@return t-解包数据
 **/

template<typename T>
T unpack( const char* buffer, size_t len ) {
   T result;
   datastream<const char*> ds(buffer,len);
   ds >> result;
   return result;
}

/*
 *将可变大小缓冲区内的数据解包为t
 *
 *@brief将可变大小缓冲区内的数据解包为t
 *@tparam t-解包数据的类型
 *@param bytes-缓冲区
 *@return t-解包数据
 **/

template<typename T>
T unpack( const vector<char>& bytes ) {
   return unpack<T>( bytes.data(), bytes.size() );
}

/*
 *获取打包数据的大小
 *
 *@brief获取打包数据的大小
 *@tparam t-要打包的数据类型
 *@param value-要打包的数据
 *@返回大小\u t-打包数据的大小
 **/

template<typename T>
size_t pack_size( const T& value ) {
  datastream<size_t> ps;
  ps << value;
  return ps.tellp();
}

/*
 *获取打包数据
 *
 *@brief获取压缩数据
 *@tparam t-要打包的数据类型
 *@param value-要打包的数据
 *@返回字节-压缩数据
 **/

template<typename T>
bytes pack( const T& value ) {
  bytes result;
  result.resize(pack_size(value));

  datastream<char*> ds( result.data(), result.size() );
  ds << value;
  return result;
}

/*
 *序列化校验和160类型
 *
 *@brief序列化校验和160类型
 *@param ds-要写入的流
 *@param cs-要序列化的值
 *@tparam stream-数据流缓冲区的类型
 *@return datastream<stream>&-对数据流的引用
 **/

template<typename Stream>
inline datastream<Stream>& operator<<(datastream<Stream>& ds, const checksum160& cs) {
   ds.write((const char*)&cs.hash[0], sizeof(cs.hash));
   return ds;
}

/*
 *反序列化校验和160类型
 *
 *@brief反序列化校验和160类型
 *@param ds-要读取的流
 *@param cs-反序列化值的目标
 *@tparam stream-数据流缓冲区的类型
 *@return datastream<stream>&-对数据流的引用
 **/

template<typename Stream>
inline datastream<Stream>& operator>>(datastream<Stream>& ds, checksum160& cs) {
   ds.read((char*)&cs.hash[0], sizeof(cs.hash));
   return ds;
}

/*
 *序列化校验和512类型
 *
 *@brief序列化校验和512类型
 *@param ds-要写入的流
 *@param cs-要序列化的值
 *@tparam stream-数据流缓冲区的类型
 *@return datastream<stream>&-对数据流的引用
 **/

template<typename Stream>
inline datastream<Stream>& operator<<(datastream<Stream>& ds, const checksum512& cs) {
   ds.write((const char*)&cs.hash[0], sizeof(cs.hash));
   return ds;
}

/*
 *反序列化校验和512类型
 *
 *@brief反序列化校验和512类型
 *@param ds-要读取的流
 *@param cs-反序列化值的目标
 *@tparam stream-数据流缓冲区的类型
 *@return datastream<stream>&-对数据流的引用
 **/

template<typename Stream>
inline datastream<Stream>& operator>>(datastream<Stream>& ds, checksum512& cs) {
   ds.read((char*)&cs.hash[0], sizeof(cs.hash));
   return ds;
}
///@数据流
}
