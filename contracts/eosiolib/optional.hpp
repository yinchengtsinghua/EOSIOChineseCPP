
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include <utility>

namespace eosio {
  /*
   *@defgroup optional type可选类型
   *@brief定义了与boost：：可选类似的otional类型
   *@ingroup类型
   *@
   **/


  /*
   *提供基于堆栈的可空值，类似于boost：：可选
   *
   *@brief提供基于堆栈的可空值，类似于boost：：可选
   **/

   template<typename T>
   class optional {
      public:
         typedef T value_type;
         typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type storage_type;
         
         /*
          *默认构造函数
          *
          *@brief构造新的可选对象
          **/

         optional():_valid(false){}

         /*
          *析构函数
          *
          *@brief销毁可选对象
          **/

         ~optional(){ reset(); }

         /*
          *从另一个可选对象构造新的可选对象
          *
          *@brief构造新的可选对象
          **/

         optional( optional& o )
            :_valid(false)
         {
            if( o._valid ) new (ptr()) T( *o );
            _valid = o._valid;
         }

         /*
          *复制构造函数
          *
          *@brief构造新的可选对象
          **/

         optional( const optional& o )
            :_valid(false)
         {
            if( o._valid ) new (ptr()) T( *o );
            _valid = o._valid;
         }

         /*
          *移动构造函数
          *
          *@brief构造新的可选对象
          **/

         optional( optional&& o )
            :_valid(false)
         {
            if( o._valid ) new (ptr()) T( std::move(*o) );
            _valid = o._valid;
            o.reset();
         }

         /*
          *从其他类型的可选对象构造新的可选对象
          *
          *@brief从其他类型的可选对象构造新的可选对象
          **/

         template<typename U>
         optional( const optional<U>& o )
            :_valid(false)
         {
            if( o._valid ) new (ptr()) T( *o );
            _valid = o._valid;
         }

         /*
          *从其他类型的可选对象构造新的可选对象
          *
          *@brief从其他类型的可选对象构造新的可选对象
          **/

         template<typename U>
         optional( optional<U>& o )
            :_valid(false)
         {
            if( o._valid )
               {
                  new (ptr()) T( *o );
               }
            _valid = o._valid;
         }

         /*
          *从其他类型的可选对象构造新的可选对象
          *
          *@brief从其他类型的可选对象构造新的可选对象
          **/

         template<typename U>
         optional( optional<U>&& o )
            :_valid(false)
         {
            if( o._valid ) new (ptr()) T( std::move(*o) );
            _valid = o._valid;
            o.reset();
         }

         /*
          *从另一个对象构造新的可选对象
          *
          *@brief从另一个对象构造一个新的可选对象
          **/

         template<typename U>
         optional( U&& u )
            :_valid(true)
         {
            new ((char*)ptr()) T( std::forward<U>(u) );
         }

         /*
          *从另一个对象构造新的可选对象
          *
          *@brief从另一个对象构造一个新的可选对象
          **/

         template<typename U>
         optional& operator=( U&& u )
         {
            reset();
            new (ptr()) T( std::forward<U>(u) );
            _valid = true;
            return *this;
         }

        /*
         *就地构造包含的值
         *
         *@brief就地构造包含的值
         *@tparam args-包含值的类型
         *@param args-要指定为包含值的值
         **/

         template<typename ...Args>
         void emplace(Args&& ... args) {
            if (_valid) {
               reset();
            }

            new ((char*)ptr()) T( std::forward<Args>(args)... );
            _valid = true;
         }

        /*
         *分配运算符
         *
         *@brief赋值运算符
         *@tparam u-要从中分配的可选对象的包含值的类型
         *@param o-要从中分配的其他可选对象
         *@return可选&-对该对象的引用
         **/

         template<typename U>
         optional& operator=( optional<U>& o ) {
            if (this != &o) {
               if( _valid && o._valid ) {
                  ref() = *o;
               } else if( !_valid && o._valid ) {
                  new (ptr()) T( *o );
                  _valid = true;
               } else if (_valid) {
                  reset();
               }
            }
            return *this;
         }

        /*
         *分配运算符
         *
         *@brief赋值运算符
         *@tparam u-要从中分配的可选对象的包含值的类型
         *@param o-要从中分配的其他可选对象
         *@return可选&-对该对象的引用
         **/

         template<typename U>
         optional& operator=( const optional<U>& o ) {
            if (this != &o) {
               if( _valid && o._valid ) {
                  ref() = *o;
               } else if( !_valid && o._valid ) {
                  new (ptr()) T( *o );
                  _valid = true;
               } else if (_valid) {
                  reset();
               }
            }
            return *this;
         }

        /*
         *分配运算符
         *
         *@brief赋值运算符
         *@param o-要从中分配的其他可选对象
         *@return可选&-对该对象的引用
         **/

         optional& operator=( optional& o ) {
            if (this != &o) {
               if( _valid && o._valid ) {
                  ref() = *o;
               } else if( !_valid && o._valid ) {
                  new (ptr()) T( *o );
                  _valid = true;
               } else if (_valid) {
                  reset();
               }
            }
            return *this;
         }

        /*
         *分配运算符
         *
         *@brief赋值运算符
         *@param o-要从中分配的其他可选对象
         *@return可选&-对该对象的引用
         **/

         optional& operator=( const optional& o ) {
            if (this != &o) {
               if( _valid && o._valid ) {
                  ref() = *o;
               } else if( !_valid && o._valid ) {
                  new (ptr()) T( *o );
                  _valid = true;
               } else if (_valid) {
                  reset();
               }
            }
            return *this;
         }

        /*
         *分配运算符
         *
         *@brief赋值运算符
         *@tparam u-要从中分配的可选对象的包含值的类型
         *@param o-要从中分配的其他可选对象
         *@return可选&-对该对象的引用
         **/

         template<typename U>
         optional& operator=( optional<U>&& o )
         {
            if (this != &o)
               {
                  if( _valid && o._valid )
                     {
                        ref() = std::move(*o);
                        o.reset();
                     } else if ( !_valid && o._valid ) {
                     *this = std::move(*o);
                  } else if (_valid) {
                     reset();
                  }
               }
            return *this;
         }

        /*
         *分配运算符
         *
         *@brief赋值运算符
         *@param o-要从中分配的其他可选对象
         *@return可选&-对该对象的引用
         **/

         optional& operator=( optional&& o )
         {
            if (this != &o)
               {
                  if( _valid && o._valid )
                     {
                        ref() = std::move(*o);
                        o.reset();
                     } else if ( !_valid && o._valid ) {
                     *this = std::move(*o);
                  } else if (_valid) {
                     reset();
                  }
               }
            return *this;
         }

         /*
          *检查此选项是否具有有效的包含值
          *
          *@brief检查此选项是否具有有效的包含值
          *@返回真-如果此选项包含有效值
          *@返回false-否则
          **/

         bool valid()const     { return _valid;  }

         /*
          *逻辑否定运算符
          *
          *@brief逻辑否定运算符
          *@return true-如果此选项包含的值无效
          *@返回false-否则
          **/

         bool operator!()const { return !_valid; }

         /*
          *类似于valid（）。但是，此操作不安全，可能导致
          *强制转换和比较，请使用valid（）或！！
          *
          *@brief检查此选项是否具有有效的包含值
          *@返回真-如果此选项包含有效值
          *@返回false-否则
          **/

         explicit operator bool()const  { return _valid;  }

         /*
          *获取此选项的包含值
          *
          *@brief指针取消引用运算符
          *@返回包含值
          **/

         T&       operator*()      { eosio_assert(_valid, "dereference of empty optional"); return ref(); }

         /*
          *获取此选项的包含值
          *
          *@brief指针取消引用运算符
          *@返回包含值
          **/

         const T& operator*()const { eosio_assert(_valid, "dereference of empty optional"); return ref(); }

         /*
          *获取指向包含值的指针
          *
          *@brief成员通过指针运算符访问
          *@返回t&-指向包含值的指针
          **/

         T*       operator->()
         {
            eosio_assert(_valid, "dereference of empty optional");
            return ptr();
         }

         /*
          *获取指向包含值的指针
          *
          *@brief成员通过指针运算符访问
          *@返回t&-指向包含值的指针
          **/         

         const T* operator->()const
         {
            eosio_assert(_valid, "dereference of empty optional");
            return ptr();
         }

        /*
         *带nullptr的赋值运算符
         *
         带nullptr的*@brief赋值运算符
         *@return可选&-对该对象的引用
         **/

         optional& operator=(std::nullptr_t)
         {
            reset();
            return *this;
         }

         /*
          *调用包含值的析构函数，并将此可选项标记为有效
          *
          *@brief重置可选对象
          **/

         void reset()
         {
            if( _valid ) {
ref().~T(); //卡尔析构函数
            }
            _valid = false;
         }

         /*
          *检查包含值是否小于包含值B
          *
          *@brief小于operator
          *@param a-第一个要比较的对象
          *@param b-要比较的第二个对象
          *@返回true-如果包含的值小于包含的值b
          *@返回false-否则
          **/

         friend bool operator < ( const optional a, optional b )
         {
            if( a.valid() && b.valid() ) return *a < *b;
            return a.valid() < b.valid();
         }

         /*
          *检查包含值是否等于包含值
          *
          *@brief相等运算符
          *@param a-第一个要比较的对象
          *@param b-要比较的第二个对象
          *@返回真-如果包含值等于b包含值
          *@返回false-否则
          **/

         friend bool operator == ( const optional a, optional b )
         {
            if( a.valid() && b.valid() ) return *a == *b;
            return a.valid() == b.valid();
         }

        /*
         *序列化可选对象
         *
         *@brief序列化可选对象
         *@param ds-要写入的流
         *@param op-要序列化的值
         *@tparam stream-数据流类型
         *@return eosio:：datastream<stream>&-对datastream的引用
         **/

         template<typename Stream>
         friend inline eosio::datastream<Stream>& operator>> (eosio::datastream<Stream>& ds, optional& op)
         {
            char valid = 0;
            ds >> valid;
            if (valid) {
               op._valid = true;
               ds >> *op;
            }
            return ds;
         }

        /*
         *反序列化可选对象
         *
         *@brief反序列化可选对象
         *@param ds-要读取的流
         *@param op-反序列化值的目标
         *@tparam stream-数据流类型
         *@return eosio:：datastream<stream>&-对datastream的引用
         **/

         template<typename Stream>
         friend inline eosio::datastream<Stream>& operator<< (eosio::datastream<Stream>& ds, const optional& op)
         {
            char valid = op._valid;
            ds << valid;
            if (valid) ds << *op;
            return ds;
         }

      private:
         template<typename U> friend class optional;
         T&       ref()      { return *ptr(); }
         const T& ref()const { return *ptr(); }
         T*       ptr()      { return reinterpret_cast<T*>(&_value); }
         const T* ptr()const { return reinterpret_cast<const T*>(&_value); }

         bool         _valid;
         storage_type _value;

   };

   /*
    *检查共享同一包含值类型的两个可选对象之间的相等性
    *
    *@brief相等运算符
    *@tparam t-可选对象的包含值类型
    *@param left-要比较的第一个对象
    *@param right-要比较的第二个对象
    *@返回真-如果两个可选对象相等
    *@返回假
    **/

   template<typename T>
   bool operator == ( const optional<T>& left, const optional<T>& right ) {
      return (!left == !right) || (!!left && *left == *right);
   }

    /*
    *检查可选对象与其他对象之间的相等性
    *
    *@brief相等运算符
    *@tparam t-可选对象的包含值类型
    *@tparam u-要比较的其他对象的类型
    *@param left-要比较的第一个对象
    *@param u-要比较的第二个对象
    *@返回true-如果可选对象包含的值与比较对象相等
    *@返回假
    **/

   template<typename T, typename U>
   bool operator == ( const optional<T>& left, const U& u ) {
      return !!left && *left == u;
   }

   /*
    *检查共享同一包含值类型的两个可选对象之间的不等式
    *
    *@brief inquality运算符
    *@tparam t-可选对象的包含值类型
    *@param left-要比较的第一个对象
    *@param right-要比较的第二个对象
    *@返回true-如果两个可选对象都不相等
    *@返回假
    **/

   template<typename T>
   bool operator != ( const optional<T>& left, const optional<T>& right ) {
      return (!left != !right) || (!!left && *left != *right);
   }

   /*
    *检查可选对象与另一个对象之间的不等式
    *
    *@brief inqquality运算符
    *@tparam t-可选对象的包含值类型
    *@tparam u-要比较的其他对象的类型
    *@param left-要比较的第一个对象
    *@param u-要比较的第二个对象
    *@返回true-如果可选对象包含的值与比较对象不相等
    *@返回假
    **/

   template<typename T, typename U>
   bool operator != ( const optional<T>& left, const U& u ) {
      return !left || *left != u;
   }
///@可选
} //命名空间EOSIO
