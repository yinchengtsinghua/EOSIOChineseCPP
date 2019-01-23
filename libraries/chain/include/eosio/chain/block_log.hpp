
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
#include <fc/filesystem.hpp>
#include <eosio/chain/block.hpp>
#include <eosio/chain/genesis_state.hpp>

namespace eosio { namespace chain {

   namespace detail { class block_log_impl; }

   /*块日志是带有标题的块的外部仅附加日志。只应阻止
    *在日志不可更改后写入日志，因为该日志只是追加的。日志是双重的
    *块的链接列表。只有块位置的辅助索引文件可以启用
    *o（1）按块号随机访问查找。
    *
    *+——————+————————+——————+——————+————+————+————+————+————+——+——+——+——+——+——+——+。
    *1区1位置2区2位置…头块头块位置
    *+——————+————————+——————+——————+————+————+————+————+————+——+——+——+——+——+——+——+。
    *
    *+————————+————————+————+——+——+——+————+——————+——+————+——+——+。
    *1区位置2区位置……头块位置
    *+————————+————————+————+——+——+——+————+——————+——+————+——+——+。
    *
    *可以通过反序列化块、跳过8个字节、反序列化
    *阻止，重复…通过查找包含的位置可以找到文件的头块。
    *在最后8个字节中，文件。块日志可以通过向后跳8个字节来读取，如下所示
    *位置、读取块、回跳8字节等。
    *
    *可以通过索引文件通过块号随机访问块。搜索到8*（块编号-1）
    *在主文件中查找块的位置。
    *
    *主文件是唯一需要保留的文件。索引文件可以在
    *主文件的线性扫描。
    **/


   class block_log {
      public:
         block_log(const fc::path& data_dir);
         block_log(block_log&& other);
         ~block_log();

         uint64_t append(const signed_block_ptr& b);
         void flush();
         void reset( const genesis_state& gs, const signed_block_ptr& genesis_block, uint32_t first_block_num = 1 );

         std::pair<signed_block_ptr, uint64_t> read_block(uint64_t file_pos)const;
         signed_block_ptr read_block_by_num(uint32_t block_num)const;
         signed_block_ptr read_block_by_id(const block_id_type& id)const {
            return read_block_by_num(block_header::num_from_id(id));
         }

         /*
          *返回文件中块的偏移量，如果块不存在，则返回块日志：：npos。
          **/

         uint64_t get_block_pos(uint32_t block_num) const;
         signed_block_ptr        read_head()const;
         const signed_block_ptr& head()const;
         uint32_t                first_block_num() const;

         static const uint64_t npos = std::numeric_limits<uint64_t>::max();

         static const uint32_t min_supported_version;
         static const uint32_t max_supported_version;

         static fc::path repair_log( const fc::path& data_dir, uint32_t truncate_at_block = 0 );

         static genesis_state extract_genesis_state( const fc::path& data_dir );

      private:
         void open(const fc::path& data_dir);
         void construct_index();

         std::unique_ptr<detail::block_log_impl> my;
   };

} }
