
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

#include <eosio/template_plugin/template_plugin.hpp>

namespace eosio {
   static appbase::abstract_plugin& _template_plugin = app().register_plugin<template_plugin>();

class template_plugin_impl {
   public:
};

template_plugin::template_plugin():my(new template_plugin_impl()){}
template_plugin::~template_plugin(){}

void template_plugin::set_program_options(options_description&, options_description& cfg) {
   cfg.add_options()
         ("option-name", bpo::value<string>()->default_value("default value"),
          "Option Description")
         ;
}

void template_plugin::plugin_initialize(const variables_map& options) {
   try {
      if( options.count( "option-name" )) {
//处理选项
      }
   }
   FC_LOG_AND_RETHROW()
}

void template_plugin::plugin_startup() {
//让魔法发生
}

void template_plugin::plugin_shutdown() {
//好吧，这足够神奇了
}

}
