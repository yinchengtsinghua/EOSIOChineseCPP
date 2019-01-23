
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


/*
 *@defgroup varint variable length integer类型
 *@brief定义了长度可变的整数类型，提供了更有效的序列化
 *@ingroup类型
 *@
 **/


/*
 *可变长度无符号整数。这样可以更有效地序列化32位无符号int。
 *它以尽可能少的字节对一个32位无符号整数进行串行运算。
 *`varuint32`无符号，使用[vlq或base-128编码]（https://en.wikipedia.org/wiki/variable-length_-quantity）
 *
 *@brief变量长度无符号整数
 **/

struct unsigned_int {
    /*
     *构造一个新的无符号int对象
     *
     *@brief构造一个新的无符号int对象
     *@param v-源
     **/

    unsigned_int( uint32_t v = 0 ):value(v){}

    /*
     *从可转换为uint32的类型构造新的无符号int对象
     *
     *@brief构造一个新的无符号int对象
     *@tparam t-源的类型
     *@param v-源
     *@pre t必须可转换为uint32
     **/

    template<typename T>
    unsigned_int( T v ):value(v){}

//运算符uint32_t（）const返回值；
//运算符uint64_t（）const返回值；

    /*
     *将无符号int转换为t
     *@brief转换运算符
     *@tparam t-转换的目标类型
     *@返回T-转换目标
     **/

    template<typename T>
    operator T()const { return static_cast<T>(value); }

    /*
     *分配32位无符号整数
     *
     *@brief赋值运算符
     *@参数V-SORUCE
     *@返回对该对象的无符号int&-引用
     **/

    unsigned_int& operator=( uint32_t v ) { value = v; return *this; }

    /*
     *包含值
     *
     *@brief包含值
     **/

    uint32_t value;

    /*
     *检查无符号整数和32位无符号整数之间的相等性
     *
     *@brief相等运算符
     *@param i-要比较的无符号\u int对象
     *@param v-要比较的32位无符号整数
     *@返回真-如果等于
     *@返回false-否则
     **/

    friend bool operator==( const unsigned_int& i, const uint32_t& v )     { return i.value == v; }

    /*
     *检查32位无符号整数与无符号整数对象之间的相等性
     *
     *@brief相等运算符
     *@param i-要比较的32位无符号整数
     *@param v-要比较的无符号\u int对象
     *@返回真-如果等于
     *@返回false-否则
     **/

    friend bool operator==( const uint32_t& i, const unsigned_int& v )     { return i       == v.value; }

    /*
     *检查两个无符号对象之间的相等性
     *
     *@brief相等运算符
     *@param i-要比较的第一个无符号\u int对象
     *@param v-要比较的第二个无符号int对象
     *@返回真-如果等于
     *@返回false-否则
     **/

    friend bool operator==( const unsigned_int& i, const unsigned_int& v ) { return i.value == v.value; }

    /*
     *检查无符号整型对象与32位无符号整型之间的不等式
     *
     *@brief不等式运算符
     *@param i-要比较的无符号\u int对象
     *@param v-要比较的32位无符号整数
     *@返回真-如果不相等
     *@返回false-否则
     **/

    friend bool operator!=( const unsigned_int& i, const uint32_t& v )     { return i.value != v; }

    /*
     *检查32位无符号整数与无符号整数对象之间的不等式
     *
     *@brief相等运算符
     *@param i-要比较的32位无符号整数
     *@param v-要比较的无符号\u int对象
     *@返回真-如果不相等
     *@返回false-否则
     **/

    friend bool operator!=( const uint32_t& i, const unsigned_int& v )     { return i       != v.value; }

    /*
     *检查两个无符号整数对象之间的不等式
     *
     *@brief不等式运算符
     *@param i-要比较的第一个无符号\u int对象
     *@param v-要比较的第二个无符号int对象
     *@返回真-如果不相等
     *@返回false-否则
     **/

    friend bool operator!=( const unsigned_int& i, const unsigned_int& v ) { return i.value != v.value; }

    /*
     *检查给定的无符号整数对象是否小于给定的32位无符号整数。
     *
     *@brief小于operator
     *@param i-要比较的无符号\u int对象
     *@param v-要比较的32位无符号整数
     *@返回真-如果我小于V
     *@返回false-否则
     **/

    friend bool operator<( const unsigned_int& i, const uint32_t& v )      { return i.value < v; }

    /*
     *检查给定的32位无符号整数是否小于给定的无符号整数对象
     *
     *@brief小于operator
     *@param i-要比较的32位无符号整数
     *@param v-要比较的无符号\u int对象
     *@返回真-如果我小于V
     *@返回false-否则
     **/

    friend bool operator<( const uint32_t& i, const unsigned_int& v )      { return i       < v.value; }

    /*
     *检查第一个给定的无符号int是否小于第二个给定的无符号int对象
     *
     *@brief小于operator
     *@param i-要比较的第一个无符号\u int对象
     *@param v-要比较的第二个无符号int对象
     *@返回真-如果我小于V
     *@返回false-否则
     **/

    friend bool operator<( const unsigned_int& i, const unsigned_int& v )  { return i.value < v.value; }

    /*
     *检查给定的无符号整数对象是否大于或等于给定的32位无符号整数。
     *
     *@brief大于或等于operator
     *@param i-要比较的无符号\u int对象
     *@param v-要比较的32位无符号整数
     *@返回真-如果我大于或等于v
     *@返回false-否则
     **/

    friend bool operator>=( const unsigned_int& i, const uint32_t& v )     { return i.value >= v; }

    /*
     *检查给定的32位无符号整数是否大于或等于给定的无符号整数对象
     *
     *@brief大于或等于operator
     *@param i-要比较的32位无符号整数
     *@param v-要比较的无符号\u int对象
     *@返回真-如果我大于或等于v
     *@返回false-否则
     **/

    friend bool operator>=( const uint32_t& i, const unsigned_int& v )     { return i       >= v.value; }

    /*
     *检查第一个给定的无符号int是否大于或等于第二个给定的无符号int对象
     *
     *@brief大于或等于operator
     *@param i-要比较的第一个无符号\u int对象
     *@param v-要比较的第二个无符号int对象
     *@返回真-如果我大于或等于v
     *@返回false-否则
     **/

    friend bool operator>=( const unsigned_int& i, const unsigned_int& v ) { return i.value >= v.value; }

    /*
     *用尽可能少的字节序列化无符号的int对象
     *
     *@brief用尽可能少的字节序列化一个无符号的int对象
     *@param ds-要写入的流
     *@param v-要序列化的值
     *@tparam datastream-数据流类型
     *@返回数据流&-对数据流的引用
     **/

    template<typename DataStream>
    friend DataStream& operator << ( DataStream& ds, const unsigned_int& v ){
       uint64_t val = v.value;
       do {
          uint8_t b = uint8_t(val) & 0x7f;
          val >>= 7;
          b |= ((val > 0) << 7);
ds.write((char*)&b,1);//放（b）；
       } while( val );
       return ds;
    }

    /*
     *反序列化无符号的\u int对象
     *
     *@brief反序列化一个无符号的\u int对象
     *@param ds-要读取的流
     *@param vi-反序列化值的目标
     *@tparam datastream-数据流类型
     *@返回数据流&-对数据流的引用
     **/

    template<typename DataStream>
    friend DataStream& operator >> ( DataStream& ds, unsigned_int& vi ){
      uint64_t v = 0; char b = 0; uint8_t by = 0;
      do {
         ds.get(b);
         v |= uint32_t(uint8_t(b) & 0x7f) << by;
         by += 7;
      } while( uint8_t(b) & 0x80 );
      vi.value = static_cast<uint32_t>(v);
      return ds;
    }
};

/*
 *可变长度有符号整数。这样可以更有效地序列化32位有符号int。
 *它以尽可能少的字节序列化一个32位有符号整数。
 *`varint32'已签名并使用[zig-zag编码]（https://developers.google.com/protocol buffers/docs/encoding signed integers）
 *
 *@brief变量长度有符号整数
 **/

struct signed_int {
    /*
     *构造一个新的有符号int对象
     *
     *@brief构造一个新的有符号int对象
     *@param v-源
     **/

    signed_int( int32_t v = 0 ):value(v){}

    /*
     *将有符号整数转换为基元32位有符号整数
     *@brief转换运算符
     *
     *@return int32_t-转换后的结果
     **/

    operator int32_t()const { return value; }


    /*
     *分配一个可转换为Int32的对象
     *
     *@brief赋值运算符
     *@tparam t-分配对象的类型
     *@param v-源
     *@返回对该对象的无符号int&-引用
     **/

    template<typename T>
    signed_int& operator=( const T& v ) { value = v; return *this; }

    /*
     *递增运算符
     *
     *@brief递增运算符
     *@return signed_int-新的有符号_int，值从当前对象的值递增
     **/

    signed_int operator++(int) { return value++; }

    /*
     *递增运算符
     *
     *@brief递增运算符
     *@return signed_int-对当前对象的引用
     **/

    signed_int& operator++(){ ++value; return *this; }

    /*
     *包含值
     *
     *@brief包含值
     **/

    int32_t value;

    /*
     *检查有符号整数对象和32位整数之间的相等性
     *
     *@brief相等运算符
     *@param i-要比较的有符号对象
     *@param v-要比较的32位整数
     *@返回真-如果等于
     *@返回false-否则
     **/

    friend bool operator==( const signed_int& i, const int32_t& v )    { return i.value == v; }

    /*
     *检查32位整数与有符号整数对象之间的相等性
     *
     *@brief相等运算符
     *@param i-要比较的32位整数
     *@param v-要比较的有符号对象
     *@返回真-如果等于
     *@返回false-否则
     **/

    friend bool operator==( const int32_t& i, const signed_int& v )    { return i       == v.value; }

    /*
     *检查两个有符号对象之间的相等性
     *
     *@brief相等运算符
     *@param i-要比较的第一个有符号对象
     *@param v-要比较的第二个有符号对象
     *@返回真-如果等于
     *@返回false-否则
     **/

    friend bool operator==( const signed_int& i, const signed_int& v ) { return i.value == v.value; }


    /*
     *检查有符号整数对象和32位整数之间的不等式
     *
     *@brief不等式运算符
     *@param i-要比较的有符号对象
     *@param v-要比较的32位整数
     *@返回真-如果不相等
     *@返回false-否则
     **/

    friend bool operator!=( const signed_int& i, const int32_t& v )    { return i.value != v; }

    /*
     *检查32位整数与有符号整数对象之间的不等式
     *
     *@brief相等运算符
     *@param i-要比较的32位整数
     *@param v-要比较的有符号对象
     *@返回真-如果不相等
     *@返回false-否则
     **/

    friend bool operator!=( const int32_t& i, const signed_int& v )    { return i       != v.value; }

    /*
     *检查两个有符号整数对象之间的不等式
     *
     *@brief不等式运算符
     *@param i-要比较的第一个有符号对象
     *@param v-要比较的第二个有符号对象
     *@返回真-如果不相等
     *@返回false-否则
     **/

    friend bool operator!=( const signed_int& i, const signed_int& v ) { return i.value != v.value; }

    /*
     *检查给定的有符号整数对象是否小于给定的32位整数。
     *
     *@brief小于operator
     *@param i-要比较的有符号对象
     *@param v-要比较的32位整数
     *@返回真-如果我小于V
     *@返回false-否则
     **/

    friend bool operator<( const signed_int& i, const int32_t& v )     { return i.value < v; }

    /*
     *检查给定的32位整数是否小于给定的有符号整数对象
     *
     *@brief小于operator
     *@param i-要比较的32位整数
     *@param v-要比较的有符号对象
     *@返回真-如果我小于V
     *@返回false-否则
     **/

    friend bool operator<( const int32_t& i, const signed_int& v )     { return i       < v.value; }

    /*
     *检查第一个给定的有符号整数是否小于第二个给定的有符号整数对象
     *
     *@brief小于operator
     *@param i-要比较的第一个有符号对象
     *@param v-要比较的第二个有符号对象
     *@返回真-如果我小于V
     *@返回false-否则
     **/

    friend bool operator<( const signed_int& i, const signed_int& v )  { return i.value < v.value; }


    /*
     *检查给定的有符号整数对象是否大于或等于给定的32位整数。
     *
     *@brief大于或等于operator
     *@param i-要比较的有符号对象
     *@param v-要比较的32位整数
     *@返回真-如果我大于或等于v
     *@返回false-否则
     **/

    friend bool operator>=( const signed_int& i, const int32_t& v )    { return i.value >= v; }

    /*
     *检查给定的32位整数是否大于或等于给定的有符号整数对象
     *
     *@brief大于或等于operator
     *@param i-要比较的32位整数
     *@param v-要比较的有符号对象
     *@返回真-如果我大于或等于v
     *@返回false-否则
     **/

    friend bool operator>=( const int32_t& i, const signed_int& v )    { return i       >= v.value; }

    /*
     *检查第一个给定的有符号整数是否大于或等于第二个给定的有符号整数对象
     *
     *@brief大于或等于operator
     *@param i-要比较的第一个有符号对象
     *@param v-要比较的第二个有符号对象
     *@返回真-如果我大于或等于v
     *@返回false-否则
     **/

    friend bool operator>=( const signed_int& i, const signed_int& v ) { return i.value >= v.value; }


    /*
     *用尽可能少的字节序列化有符号的int对象
     *
     *@brief用尽可能少的字节序列化有符号的int对象
     *@param ds-要写入的流
     *@param v-要序列化的值
     *@tparam datastream-数据流类型
     *@返回数据流&-对数据流的引用
     **/

    template<typename DataStream>
    friend DataStream& operator << ( DataStream& ds, const signed_int& v ){
      uint32_t val = uint32_t((v.value<<1) ^ (v.value>>31));
      do {
         uint8_t b = uint8_t(val) & 0x7f;
         val >>= 7;
         b |= ((val > 0) << 7);
ds.write((char*)&b,1);//放（b）；
      } while( val );
       return ds;
    }

    /*
     *反序列化已签名的对象
     *
     *@brief反序列化签名的\u int对象
     *@param ds-要读取的流
     *@param vi-反序列化值的目标
     *@tparam datastream-数据流类型
     *@返回数据流&-对数据流的引用
     **/

    template<typename DataStream>
    friend DataStream& operator >> ( DataStream& ds, signed_int& vi ){
      uint32_t v = 0; char b = 0; int by = 0;
      do {
         ds.get(b);
         v |= uint32_t(uint8_t(b) & 0x7f) << by;
         by += 7;
      } while( uint8_t(b) & 0x80 );
      vi.value = ((v>>1) ^ (v>>31)) + (v&0x01);
      vi.value = v&0x01 ? vi.value : -vi.value;
      vi.value = -vi.value;
      return ds;
    }
};

///@
