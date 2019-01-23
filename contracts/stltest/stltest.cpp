
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

//包括整个libc
#include <alloca.h>
#include <assert.h>
#include <byteswap.h>
#include <crypt.h>
#include <ctype.h>
#include <endian.h>
#include <errno.h>
#include <features.h>
#include <inttypes.h>
#include <iso646.h>
#include <limits.h>
#include <locale.h>
#include <malloc.h>
#include <math.h>
#include <memory.h>
#include <monetary.h>
#include <search.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdc-predef.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <strings.h>
#include <uchar.h>
#include <unistd.h>
#include <values.h>
#include <wchar.h>
#include <wctype.h>

//包括整个libstdc++
#include<algorithm>
#include<any>
#include<array>
#include<bitset>
#include<cassert>
//包括<ccomplex>
#include<cctype>
#include<cerrno>
//包括<CFEnv>
#include<cfloat>
//包括<时间>
#include<cinttypes>
#include<ciso646>
#include<climits>
#include<clocale>
#include<cmath>
#include<codecvt>
//包括<complex>
#include<condition_variable>
//包括<csetjmp>
//包括<csignal>
#include<cstdarg>
#include<cstdbool>
#include<cstddef>
#include<cstdint>
#include<cstdio>
#include<cstdlib>
#include<cstring>
//包括<ctgmath>
#include<ctime>
#include<cwchar>
#include<cwctype>
#include<deque>
#include<exception>
#include<forward_list>
//包括<fstream>
#include<functional>
//包括<未来>
#include<initializer_list>
#include<iomanip>
#include<ios>
#include<iosfwd>
#include<iostream>
#include<istream>
#include<iterator>
#include<limits>
#include<list>
#include<locale>
#include<map>
#include<memory>
#include<mutex>
#include<new>
#include<numeric>
#include<optional>
#include<ostream>
#include<queue>
//包含<随机>
#include<ratio>
#include<regex>
#include<scoped_allocator>
#include<set>
//包括<shared\u mutex>
#include<sstream>
#include<stack>
#include<stdexcept>
#include<streambuf>
#include<string>
#include<string_view>
#include<strstream>
#include<system_error>
//包含<线程>
#include<tuple>
#include<type_traits>
#include<typeindex>
#include<typeinfo>
#include<unordered_map>
#include<unordered_set>
#include<utility>
#include<valarray>
#include<variant>
#include<vector>


/*
include<array>
include<vector>
include<stack>
include<queue>
包括<deque>
包括“列表”
其中包括<MAP>
包含“SET>”
包括<unordered_map>
include<unordered_set>
include<string>
include<stdExcept>
**/

//包括<eosiolib/eos.hpp>
#include <eosiolib/dispatcher.hpp>

using namespace eosio;
/*
命名空间STD{
   外部iOS启动标准流；
}
**/

namespace stltest {

   struct MSTR {
      MSTR() : x(7891) {
         prints("ATTENTION! S::S() called\n");
      }
      int x;
      ~MSTR() {
         prints("~MSTR");
      }
   };
    
    class contract {
    public:
        static const uint64_t sent_table_name = N(sent);
        static const uint64_t received_table_name = N(received);

        struct message {
            account_name from;
            account_name to;
//字符串MSG；

            static uint64_t get_account() { return N(stltest); }
            static uint64_t get_name()  { return N(message); }

            template<typename DataStream>
            friend DataStream& operator << ( DataStream& ds, const message& m ){
return ds << m.from << m.to;//<M.MSG；
            }
            template<typename DataStream>
            friend DataStream& operator >> ( DataStream& ds, message& m ){
return ds >> m.from >> m.to;//> M.MSG；
            }
        };

       static void f(const char* __restrict, ...) {
          prints("f() called\n");
       }

        static void on( const message& ) {
           /*全局变量的手动初始化
           新建（&std:：uu start_std_streams）std:：ios_base:：init；
           **/

           /*
           标准：OStringstream OSM；
           osm<“abcdef”；
           std:：string s=osm.str（）；
           打印_l（s.data（），s.size（））；
           **/

           /*
           prints（“标准字符串：”）；prints（s.c_str（））；
           prints（“\neos string:“）；prints_l（s2.get_data（），s2.get_size（））；
           **/

           prints("STL test start\n");
           /*不适用于wasm:：serializewithinjection
           printf（“stdout输出\n”，0）；
           fprintf（stderr，“stderr输出\n”，0）；
           **/

           void* ptr = malloc(10);
           free(ptr);
           f("abc", 10, 20);

//auto mptr=new mstr（）；
//删除MPTR；

           std::array<uint32_t, 10> arr;
           arr.fill(3);
           arr[0] = 0;

           std::vector<uint32_t> v;
           v.push_back(0);

           std::stack<char> stack;
           stack.push('J');
           stack.pop();

           std::queue<unsigned int> q;
           q.push(0);

           std::deque<long> dq;
           dq.push_front(0.0f);

           std::list<uint32_t> l;
           l.push_back(0);

           std::string s = "abcdef";
           s.append(1, 'g');

           std::map<int, long> m;
           m.emplace(0, 1);
           m.lower_bound(2);

           std::set<long> st;
           st.insert(0);
           st.erase(st.begin());
           st.count(0);

//std:：unordered_map<int，string>hm；
//HM〔0〕＝“ABC”；
//std:：unordered_set<int>hs；
//插入物（0）；

           sort(dq.begin(), dq.end());
           find_if(l.begin(), l.end(), [](uint32_t f) { return f < 10; });
           prints("STL test done.\n");
//std:：cout<“stl test done.”<<std:：endl；
        }

        static void apply( account_name c, action_name act) {
            eosio::dispatch<stltest::contract, message>(c,act);
        }
    };

} ///命名空间eosio


extern "C" {
///apply方法实现将事件分派到此协定
void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
    (void)receiver;
    stltest::contract::apply( code, action );
}
}
