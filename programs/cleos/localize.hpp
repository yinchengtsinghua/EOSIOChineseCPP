
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

#include <libintl.h>
#include <fc/variant.hpp>

namespace eosio { namespace client { namespace localize {
   #if !defined(_)
   #define _(str) str
   #endif

   #define localized(str, ...) localized_with_variant((str), fc::mutable_variant_object() __VA_ARGS__ )

   inline auto localized_with_variant( const char* raw_fmt, const fc::variant_object& args) {
      if (raw_fmt != nullptr) {
         try {
            return fc::format_string(::gettext(raw_fmt), args);
         } catch (...) {
         }
         return std::string(raw_fmt);
      }
      return std::string();
   }
}}}
