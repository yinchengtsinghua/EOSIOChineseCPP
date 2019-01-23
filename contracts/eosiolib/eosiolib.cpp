
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "datastream.hpp"
#include "memory.hpp"
#include "privileged.hpp"

void* sbrk(size_t num_bytes) {
      constexpr uint32_t NBPPL2  = 16U;
      constexpr uint32_t NBBP    = 65536U;

      static bool initialized;
      static uint32_t sbrk_bytes;
      if(!initialized) {
         sbrk_bytes = __builtin_wasm_current_memory() * NBBP;
         initialized = true;
      }

      if(num_bytes > INT32_MAX)
         return reinterpret_cast<void*>(-1);

//uint32_t num_bytes=（uint32_t）num_bytesi；
      const uint32_t prev_num_bytes = sbrk_bytes;
      const uint32_t current_pages = __builtin_wasm_current_memory();

//将num_字节的绝对值舍入到对齐边界
      num_bytes = (num_bytes + 7U) & ~7U;

//更新分配的字节数，并计算所需的页数
      const uint32_t num_desired_pages = (sbrk_bytes + num_bytes + NBBP - 1) >> NBPPL2;

      if(num_desired_pages > current_pages) {
//不幸的是，clang4没有提供grow_内存的返回代码，这就是为什么需要
//要返回并再次检查当前内存，以确保它已实际增长！
         __builtin_wasm_grow_memory(num_desired_pages - current_pages);
         if(num_desired_pages != __builtin_wasm_current_memory())
            return reinterpret_cast<void*>(-1);
      }

      sbrk_bytes += num_bytes;
      return reinterpret_cast<void*>(prev_num_bytes);
}

namespace eosio {

   void set_blockchain_parameters(const eosio::blockchain_parameters& params) {
      char buf[sizeof(eosio::blockchain_parameters)];
      eosio::datastream<char *> ds( buf, sizeof(buf) );
      ds << params;
      set_blockchain_parameters_packed( buf, ds.tellp() );
   }

   void get_blockchain_parameters(eosio::blockchain_parameters& params) {
      char buf[sizeof(eosio::blockchain_parameters)];
      size_t size = get_blockchain_parameters_packed( buf, sizeof(buf) );
      eosio_assert( size <= sizeof(buf), "buffer is too small" );
      eosio::datastream<const char*> ds( buf, size_t(size) );
      ds >> params;
   }

   using ::memset;
   using ::memcpy;



class memory_manager  //注意：不应该分配内存管理器的另一个实例
   {
   friend void* ::malloc(size_t size);
   friend void* ::calloc(size_t count, size_t size);
   friend void* ::realloc(void* ptr, size_t size);
   friend void ::free(void* ptr);
   public:
      memory_manager()
//注意：如果对象是全局分配的，那么WASM似乎与初始化列表有问题。
//似乎只是将成员初始化为0
      : _heaps_actual_size(0)
      , _active_heap(0)
      , _active_free_heap(0)
      {
      }

   private:
      class memory;

      memory* next_active_heap()
      {
         constexpr uint32_t wasm_page_size = 64*1024;
         memory* const current_memory = _available_heaps + _active_heap;

         const uint32_t current_memory_size = reinterpret_cast<uint32_t>(sbrk(0));
         if(static_cast<int32_t>(current_memory_size) < 0)
            return nullptr;

//抓取到当前WASM内存页的末尾，前提是它还有1KIB，否则
//增长到下一页末尾
         uint32_t heap_adj;
         if(current_memory_size % wasm_page_size <= wasm_page_size-1024)
            heap_adj = (current_memory_size + wasm_page_size) - (current_memory_size % wasm_page_size) - current_memory_size;
         else
            heap_adj = (current_memory_size + wasm_page_size*2) - (current_memory_size % (wasm_page_size*2)) - current_memory_size;
         char* new_memory_start = reinterpret_cast<char*>(sbrk(heap_adj));
         if(reinterpret_cast<int32_t>(new_memory_start) == -1) {
//确保清除所有剩余的未分配内存
            current_memory->cleanup_remaining();
            ++_active_heap;
            _heaps_actual_size = _active_heap;
            return nullptr;
         }

//如果我们可以扩展当前内存，继续使用它
         if (current_memory->expand_memory(new_memory_start, heap_adj))
            return current_memory;

//确保清除所有剩余的未分配内存
         current_memory->cleanup_remaining();

         ++_active_heap;
         memory* const next = _available_heaps + _active_heap;
         next->init(new_memory_start, heap_adj);

         return next;
      }

      void* malloc(uint32_t size)
      {
         if (size == 0)
            return nullptr;

//请参阅有关ctor的注释
         if (_heaps_actual_size == 0)
            _heaps_actual_size = _heaps_size;

         adjust_to_mem_block(size);

//第一次循环从不需要初始化可用堆中的槽
         char* buffer = nullptr;
         memory* current = nullptr;
//需要确保
         if (_active_heap < _heaps_actual_size)
         {
            memory* const start_heap = &_available_heaps[_active_heap];
//只有堆0不会被初始化
            if(_active_heap == 0 && !start_heap->is_init())
            {
               start_heap->init(_initial_heap, _initial_heap_size);
            }

            current = start_heap;
         }

         while (current != nullptr)
         {
            buffer = current->malloc(size);
//如果我们有缓冲区就完成了
            if (buffer != nullptr)
               break;

            current = next_active_heap();
         }

         if (buffer == nullptr)
         {
            const uint32_t end_free_heap = _active_free_heap;

            do
            {
               buffer = _available_heaps[_active_free_heap].malloc_from_freed(size);

               if (buffer != nullptr)
                  break;

               if (++_active_free_heap == _heaps_actual_size)
                  _active_free_heap = 0;

            } while (_active_free_heap != end_free_heap);
         }

         return buffer;
      }

      void* realloc(void* ptr, uint32_t size)
      {
         if (size == 0)
         {
            free(ptr);
            return nullptr;
         }

         adjust_to_mem_block(size);

         char* realloc_ptr = nullptr;
         uint32_t orig_ptr_size = 0;
         if (ptr != nullptr)
         {
            char* const char_ptr = static_cast<char*>(ptr);
            for (memory* realloc_heap = _available_heaps; realloc_heap < _available_heaps + _heaps_actual_size && realloc_heap->is_init(); ++realloc_heap)
            {
               if (realloc_heap->is_in_heap(char_ptr))
               {
                  realloc_ptr = realloc_heap->realloc_in_place(char_ptr, size, &orig_ptr_size);

                  if (realloc_ptr != nullptr)
                     return realloc_ptr;
                  else
                     break;
               }
            }
         }

         char* new_alloc = static_cast<char*>(malloc(size));
         if (new_alloc == nullptr)
            return nullptr;

         const uint32_t copy_size = (size < orig_ptr_size) ? size : orig_ptr_size;
         if (copy_size > 0)
         {
            memcpy(new_alloc, ptr, copy_size);
            free (ptr);
         }

         return new_alloc;
      }

      void free(void* ptr)
      {
         if (ptr == nullptr)
            return;

         char* const char_ptr = static_cast<char*>(ptr);
         for (memory* free_heap = _available_heaps; free_heap < _available_heaps + _heaps_actual_size && free_heap->is_init(); ++free_heap)
         {
            if (free_heap->is_in_heap(char_ptr))
            {
               free_heap->free(char_ptr);
               break;
            }
         }
      }

      void adjust_to_mem_block(uint32_t& size)
      {
         const uint32_t remainder = (size + _size_marker) & _rem_mem_block_mask;
         if (remainder > 0)
         {
            size += _mem_block - remainder;
         }
      }

      class memory
      {
      public:
         memory()
         : _heap_size(0)
         , _heap(nullptr)
         , _offset(0)
         {
         }

         void init(char* const mem_heap, uint32_t size)
         {
            _heap_size = size;
            _heap = mem_heap;
         }

         uint32_t is_init() const
         {
            return _heap != nullptr;
         }

         uint32_t is_in_heap(const char* const ptr) const
         {
            const char* const end_of_buffer = _heap + _heap_size;
            const char* const first_ptr_of_buffer = _heap + _size_marker;
            return ptr >= first_ptr_of_buffer && ptr < end_of_buffer;
         }

         uint32_t is_capacity_remaining() const
         {
            return _offset + _size_marker < _heap_size;
         }

         char* malloc(uint32_t size)
         {
            uint32_t used_up_size = _offset + size + _size_marker;
            if (used_up_size > _heap_size)
            {
               return nullptr;
            }

            buffer_ptr new_buff(&_heap[_offset + _size_marker], size, _heap + _heap_size);
            _offset += size + _size_marker;
            new_buff.mark_alloc();
            return new_buff.ptr();
         }

         char* malloc_from_freed(uint32_t size)
         {
            eosio_assert(_offset == _heap_size, "malloc_from_freed was designed to only be called after _heap was completely allocated");

            char* current = _heap + _size_marker;
            while (current != nullptr)
            {
               buffer_ptr current_buffer(current, _heap + _heap_size);
               if (!current_buffer.is_alloc())
               {
//如果有足够的连续内存就完成了
                  if (current_buffer.merge_contiguous(size))
                  {
                     current_buffer.mark_alloc();
                     return current;
                  }
               }

               current = current_buffer.next_ptr();
            }

//找不到任何可用内存
            return nullptr;
         }

         char* realloc_in_place(char* const ptr, uint32_t size, uint32_t* orig_ptr_size)
         {
            const char* const end_of_buffer = _heap + _heap_size;

            buffer_ptr orig_buffer(ptr, end_of_buffer);
            *orig_ptr_size = orig_buffer.size();
//传入的指针有效吗
            char* const orig_buffer_end = orig_buffer.end();
            if (orig_buffer_end > end_of_buffer)
            {
               *orig_ptr_size = 0;
               return nullptr;
            }

            if (ptr > end_of_buffer - size)
            {
//无法就地调整大小
               return nullptr;
            }

            if( *orig_ptr_size > size )
            {
//使用缓冲区指针分配内存以释放
               char* const new_ptr = ptr + size + _size_marker;
               buffer_ptr excess_to_free(new_ptr, *orig_ptr_size - size, _heap + _heap_size);
               excess_to_free.mark_free();

               return ptr;
            }
//如果ptr是最后分配的缓冲区，我们可以扩展
            else if (orig_buffer_end == &_heap[_offset])
            {
               orig_buffer.size(size);
               _offset += size - *orig_ptr_size;

               return ptr;
            }
            if (size == *orig_ptr_size )
               return ptr;

            if (!orig_buffer.merge_contiguous_if_available(size))
//无法就地调整大小
               return nullptr;

            orig_buffer.mark_alloc();
            return ptr;
         }

         void free(char* ptr)
         {
            buffer_ptr to_free(ptr, _heap + _heap_size);
            to_free.mark_free();
         }

         void cleanup_remaining()
         {
            if (_offset == _heap_size)
               return;

//把剩余的内存拿走，像分配的一样
            const uint32_t size = _heap_size - _offset - _size_marker;
            buffer_ptr new_buff(&_heap[_offset + _size_marker], size, _heap + _heap_size);
            _offset = _heap_size;
            new_buff.mark_free();
         }

         bool expand_memory(char* exp_mem, uint32_t size)
         {
            if (_heap + _heap_size != exp_mem)
               return false;

            _heap_size += size;

            return true;
         }

      private:
         class buffer_ptr
         {
         public:
            buffer_ptr(void* ptr, const char* const heap_end)
            : _ptr(static_cast<char*>(ptr))
            , _size(*reinterpret_cast<uint32_t*>(static_cast<char*>(ptr) - _size_marker) & ~_alloc_memory_mask)
            , _heap_end(heap_end)
            {
            }

            buffer_ptr(void* ptr, uint32_t buff_size, const char* const heap_end)
            : _ptr(static_cast<char*>(ptr))
            , _heap_end(heap_end)
            {
               size(buff_size);
            }

            uint32_t size() const
            {
               return _size;
            }

            char* next_ptr() const
            {
               char* const next = end() + _size_marker;
               if (next >= _heap_end)
                  return nullptr;

               return next;
            }

            void size(uint32_t val)
            {
//保持以前设置的状态（已分配或可用）
               const uint32_t memory_state = *reinterpret_cast<uint32_t*>(_ptr - _size_marker) & _alloc_memory_mask;
               *reinterpret_cast<uint32_t*>(_ptr - _size_marker) = val | memory_state;
               _size = val;
            }

            char* end() const
            {
               return _ptr + _size;
            }

            char* ptr() const
            {
               return _ptr;
            }

            void mark_alloc()
            {
               *reinterpret_cast<uint32_t*>(_ptr - _size_marker) |= _alloc_memory_mask;
            }

            void mark_free()
            {
               *reinterpret_cast<uint32_t*>(_ptr - _size_marker) &= ~_alloc_memory_mask;
            }

            bool is_alloc() const
            {
               return *reinterpret_cast<const uint32_t*>(_ptr - _size_marker) & _alloc_memory_mask;
            }

            bool merge_contiguous_if_available(uint32_t needed_size)
            {
               return merge_contiguous(needed_size, true);
            }

            bool merge_contiguous(uint32_t needed_size)
            {
               return merge_contiguous(needed_size, false);
            }
         private:
            bool merge_contiguous(uint32_t needed_size, bool all_or_nothing)
            {
//如果没有可分配的连续空间，则不必麻烦
               if( all_or_nothing && uint32_t(_heap_end - _ptr) < needed_size )
                  return false;

               uint32_t possible_size = _size;
               while (possible_size < needed_size  && (_ptr + possible_size < _heap_end))
               {
                  const uint32_t next_mem_flag_size = *reinterpret_cast<const uint32_t*>(_ptr + possible_size);
//如果已分配，则使用连续的空闲内存完成
                  if (next_mem_flag_size & _alloc_memory_mask)
                     break;

                  possible_size += (next_mem_flag_size & ~_alloc_memory_mask) + _size_marker;
               }

               if (all_or_nothing && possible_size < needed_size)
                  return false;

//结合
               const uint32_t new_size = possible_size < needed_size ? possible_size : needed_size;
               size(new_size);

               if (possible_size > needed_size)
               {
                  const uint32_t freed_size = possible_size - needed_size - _size_marker;
                  buffer_ptr freed_remainder(_ptr + needed_size + _size_marker, freed_size, _heap_end);
                  freed_remainder.mark_free();
               }

               return new_size == needed_size;
            }

            char* _ptr;
            uint32_t _size;
            const char* const _heap_end;
         };

         uint32_t _heap_size;
         char* _heap;
         uint32_t _offset;
      };

      static const uint32_t _size_marker = sizeof(uint32_t);
//以8个字符块分配内存
      static const uint32_t _mem_block = 8;
      static const uint32_t _rem_mem_block_mask = _mem_block - 1;
static const uint32_t _initial_heap_size = 8192;//32768；
//如果在这个文件之外没有调用sbrk，那么这是我们可以调用它的最大次数
      static const uint32_t _heaps_size = 16;
      char _initial_heap[_initial_heap_size];
      memory _available_heaps[_heaps_size];
      uint32_t _heaps_actual_size;
      uint32_t _active_heap;
      uint32_t _active_free_heap;
      static const uint32_t _alloc_memory_mask = uint32_t(1) << 31;
   };

   memory_manager memory_heap;

} ///命名空间eosio

extern "C" {

void* __dso_handle = 0;

void* malloc(size_t size)
{
   return eosio::memory_heap.malloc(size);
}

void* calloc(size_t count, size_t size)
{
   void* ptr = eosio::memory_heap.malloc(count*size);
   memset(ptr, 0, count*size);
   return ptr;
}

void* realloc(void* ptr, size_t size)
{
   return eosio::memory_heap.realloc(ptr, size);
}

void free(void* ptr)
{
   return eosio::memory_heap.free(ptr);
}

}
