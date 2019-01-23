
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <chainbase/chainbase.hpp>
#include <fc/filesystem.hpp>

#include <memory>

namespace eosio { namespace testing {

/*
 *使用raii创建和删除临时chainbase:：数据库的实用程序类
 *
 *@tparam max_size-chainbase:：database的最大大小
 **/

template<uint64_t MAX_SIZE>
struct chainbase_fixture {
   chainbase_fixture()
   : _tempdir()
   , _db(std::make_unique<chainbase::database>(_tempdir.path(), chainbase::database::read_write, MAX_SIZE))
   {
   }

   ~chainbase_fixture()
   {
      _db.reset();
      _tempdir.remove();
   }

   fc::temp_directory                    _tempdir;
   std::unique_ptr<chainbase::database>  _db;
};

} }  //EOSIO：测试
