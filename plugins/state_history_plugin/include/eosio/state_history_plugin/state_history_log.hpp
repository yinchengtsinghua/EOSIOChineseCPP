
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

#include <boost/filesystem.hpp>
#include <fstream>
#include <stdint.h>

#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/types.hpp>
#include <fc/log/logger.hpp>

namespace eosio {

/*
 **日志：
 ＊＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋
 *条目I条目I位置条目I+1条目I+1位置…入口Z入口Z位置
 ＊＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋
 *
 **指数：
 *+——————+————————+——————+————+——————+
 *摘要I摘要I+1…总结Z
 *+——————+————————+——————+————+——————+
 *
 *每个条目：
 *uint32_t块编号
 *block_id_类型block_id
 *UInt64负载大小
 *uint8_t版本
 *有效载荷
 *
 *每个摘要：
 *uint64*日志中的条目位置
 *
 *状态有效载荷：
 *uint32三角洲的大小
 *字符[]增量
 **/


//TODO:寻找将此转换为序列化而不是memcpy的方法
//TODO:考虑改写版本控制
//TODO:考虑删除块编号，因为它包含在块ID中
//TODO:当前只检查第一条记录的版本。需要恢复块
struct state_history_log_header {
   uint32_t             block_num = 0;
   chain::block_id_type block_id;
   uint64_t             payload_size = 0;
   uint8_t              version      = 0;
};

struct state_history_summary {
   uint64_t pos = 0;
};

class state_history_log {
 private:
   const char* const    name = "";
   std::string          log_filename;
   std::string          index_filename;
   std::fstream         log;
   std::fstream         index;
   uint32_t             _begin_block = 0;
   uint32_t             _end_block   = 0;
   chain::block_id_type last_block_id;

 public:
   state_history_log(const char* const name, std::string log_filename, std::string index_filename)
       : name(name)
       , log_filename(std::move(log_filename))
       , index_filename(std::move(index_filename)) {
      open_log();
      open_index();
   }

   uint32_t begin_block() const { return _begin_block; }
   uint32_t end_block() const { return _end_block; }

   template <typename F>
   void write_entry(const state_history_log_header& header, const chain::block_id_type& prev_id, F write_payload) {
      EOS_ASSERT(_begin_block == _end_block || header.block_num <= _end_block, chain::plugin_exception,
                 "missed a block in ${name}.log", ("name", name));

      if (_begin_block != _end_block && header.block_num > _begin_block) {
         if (header.block_num == _end_block) {
            EOS_ASSERT(prev_id == last_block_id, chain::plugin_exception, "missed a fork change in ${name}.log",
                       ("name", name));
         } else {
            state_history_log_header prev;
            get_entry(header.block_num - 1, prev);
            EOS_ASSERT(prev_id == prev.block_id, chain::plugin_exception, "missed a fork change in ${name}.log",
                       ("name", name));
         }
      }

      if (header.block_num < _end_block)
         truncate(header.block_num);
      log.seekg(0, std::ios_base::end);
      uint64_t pos = log.tellg();
      log.write((char*)&header, sizeof(header));
      write_payload(log);
      uint64_t end = log.tellg();
      EOS_ASSERT(end == pos + sizeof(header) + header.payload_size, chain::plugin_exception,
                 "wrote payload with incorrect size to ${name}.log", ("name", name));
      log.write((char*)&pos, sizeof(pos));

      index.seekg(0, std::ios_base::end);
      state_history_summary summary{.pos = pos};
      index.write((char*)&summary, sizeof(summary));
      if (_begin_block == _end_block)
         _begin_block = header.block_num;
      _end_block    = header.block_num + 1;
      last_block_id = header.block_id;
   }

//返回位于有效负载的流
   std::fstream& get_entry(uint32_t block_num, state_history_log_header& header) {
      EOS_ASSERT(block_num >= _begin_block && block_num < _end_block, chain::plugin_exception,
                 "read non-existing block in ${name}.log", ("name", name));
      log.seekg(get_pos(block_num));
      log.read((char*)&header, sizeof(header));
      return log;
   }

   chain::block_id_type get_block_id(uint32_t block_num) {
      state_history_log_header header;
      get_entry(block_num, header);
      return header.block_id;
   }

 private:
   bool get_last_block(uint64_t size) {
      state_history_log_header header;
      uint64_t                 suffix;
      log.seekg(size - sizeof(suffix));
      log.read((char*)&suffix, sizeof(suffix));
      if (suffix > size || suffix + sizeof(header) > size) {
         elog("corrupt ${name}.log (2)", ("name", name));
         return false;
      }
      log.seekg(suffix);
      log.read((char*)&header, sizeof(header));
      if (suffix + sizeof(header) + header.payload_size + sizeof(suffix) != size) {
         elog("corrupt ${name}.log (3)", ("name", name));
         return false;
      }
      _end_block    = header.block_num + 1;
      last_block_id = header.block_id;
      if (_begin_block >= _end_block) {
         elog("corrupt ${name}.log (4)", ("name", name));
         return false;
      }
      return true;
   }

   void recover_blocks(uint64_t size) {
      ilog("recover ${name}.log", ("name", name));
      uint64_t pos       = 0;
      uint32_t num_found = 0;
      while (true) {
         state_history_log_header header;
         if (pos + sizeof(header) > size)
            break;
         log.seekg(pos);
         log.read((char*)&header, sizeof(header));
         uint64_t suffix;
         if (header.payload_size > size || pos + sizeof(header) + header.payload_size + sizeof(suffix) > size)
            break;
         log.seekg(pos + sizeof(header) + header.payload_size);
         log.read((char*)&suffix, sizeof(suffix));
         if (suffix != pos)
            break;
         pos = pos + sizeof(header) + header.payload_size + sizeof(suffix);
         if (!(++num_found % 10000)) {
            printf("%10u blocks found, log pos=%12llu\r", (unsigned)num_found, (unsigned long long)pos);
            fflush(stdout);
         }
      }
      log.flush();
      boost::filesystem::resize_file(log_filename, pos);
      log.sync();
      EOS_ASSERT(get_last_block(pos), chain::plugin_exception, "recover ${name}.log failed", ("name", name));
   }

   void open_log() {
      log.open(log_filename, std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::app);
      log.seekg(0, std::ios_base::end);
      uint64_t size = log.tellg();
      if (size >= sizeof(state_history_log_header)) {
         state_history_log_header header;
         log.seekg(0);
         log.read((char*)&header, sizeof(header));
         EOS_ASSERT(header.version == 0 && sizeof(header) + header.payload_size + sizeof(uint64_t) <= size,
                    chain::plugin_exception, "corrupt ${name}.log (1)", ("name", name));
         _begin_block  = header.block_num;
         last_block_id = header.block_id;
         if (!get_last_block(size))
            recover_blocks(size);
         ilog("${name}.log has blocks ${b}-${e}", ("name", name)("b", _begin_block)("e", _end_block - 1));
      } else {
         EOS_ASSERT(!size, chain::plugin_exception, "corrupt ${name}.log (5)", ("name", name));
         ilog("${name}.log is empty", ("name", name));
      }
   }

   void open_index() {
      index.open(index_filename, std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::app);
      index.seekg(0, std::ios_base::end);
      if (index.tellg() == (_end_block - _begin_block) * sizeof(state_history_summary))
         return;
      ilog("Regenerate ${name}.index", ("name", name));
      index.close();
      index.open(index_filename, std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

      log.seekg(0, std::ios_base::end);
      uint64_t size      = log.tellg();
      uint64_t pos       = 0;
      uint32_t num_found = 0;
      while (pos < size) {
         state_history_log_header header;
         EOS_ASSERT(pos + sizeof(header) <= size, chain::plugin_exception, "corrupt ${name}.log (6)", ("name", name));
         log.seekg(pos);
         log.read((char*)&header, sizeof(header));
         uint64_t suffix_pos = pos + sizeof(header) + header.payload_size;
         uint64_t suffix;
         EOS_ASSERT(suffix_pos + sizeof(suffix) <= size, chain::plugin_exception, "corrupt ${name}.log (7)",
                    ("name", name));
         log.seekg(suffix_pos);
         log.read((char*)&suffix, sizeof(suffix));
//ilog（“block$b at$pos-$end suffix=$suffix file_size=$fs”，
//（b，header.block_num）（“pos”，pos）（“end”，后缀_pos+sizeof（suffix））（后缀，后缀）（“fs”，大小））；
         EOS_ASSERT(suffix == pos, chain::plugin_exception, "corrupt ${name}.log (8)", ("name", name));

         state_history_summary summary{.pos = pos};
         index.write((char*)&summary, sizeof(summary));
         pos = suffix_pos + sizeof(suffix);
         if (!(++num_found % 10000)) {
            printf("%10u blocks found, log pos=%12llu\r", (unsigned)num_found, (unsigned long long)pos);
            fflush(stdout);
         }
      }
   }

   uint64_t get_pos(uint32_t block_num) {
      state_history_summary summary;
      index.seekg((block_num - _begin_block) * sizeof(summary));
      index.read((char*)&summary, sizeof(summary));
      return summary.pos;
   }

   void truncate(uint32_t block_num) {
      log.flush();
      index.flush();
      uint64_t num_removed = 0;
      if (block_num <= _begin_block) {
         num_removed = _end_block - _begin_block;
         log.seekg(0);
         index.seekg(0);
         boost::filesystem::resize_file(log_filename, 0);
         boost::filesystem::resize_file(index_filename, 0);
         _begin_block = _end_block = 0;
      } else {
         num_removed  = _end_block - block_num;
         uint64_t pos = get_pos(block_num);
         log.seekg(0);
         index.seekg(0);
         boost::filesystem::resize_file(log_filename, pos);
         boost::filesystem::resize_file(index_filename, (block_num - _begin_block) * sizeof(state_history_summary));
         _end_block = block_num;
      }
      log.sync();
      index.sync();
      ilog("fork or replay: removed ${n} blocks from ${name}.log", ("n", num_removed)("name", name));
   }
}; //状态历史记录

} //命名空间EOSIO
