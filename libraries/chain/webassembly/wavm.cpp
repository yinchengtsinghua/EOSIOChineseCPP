
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <eosio/chain/webassembly/wavm.hpp>
#include <eosio/chain/wasm_eosio_constraints.hpp>
#include <eosio/chain/wasm_eosio_injection.hpp>
#include <eosio/chain/apply_context.hpp>
#include <eosio/chain/exceptions.hpp>

#include "IR/Module.h"
#include "Platform/Platform.h"
#include "WAST/WAST.h"
#include "IR/Operators.h"
#include "IR/Validate.h"
#include "Runtime/Linker.h"
#include "Runtime/Intrinsics.h"

#include <mutex>

using namespace IR;
using namespace Runtime;

namespace eosio { namespace chain { namespace webassembly { namespace wavm {

running_instance_context the_running_instance_context;

class wavm_instantiated_module : public wasm_instantiated_module_interface {
   public:
      wavm_instantiated_module(ModuleInstance* instance, std::unique_ptr<Module> module, std::vector<uint8_t> initial_mem) :
         _initial_memory(initial_mem),
         _instance(instance),
         _module(std::move(module))
      {}

      void apply(apply_context& context) override {
         vector<Value> args = {Value(uint64_t(context.receiver)),
	                       Value(uint64_t(context.act.account)),
                               Value(uint64_t(context.act.name))};

         call("apply", args, context);
      }

   private:
      void call(const string &entry_point, const vector <Value> &args, apply_context &context) {
         try {
            FunctionInstance* call = asFunctionNullable(getInstanceExport(_instance,entry_point));
            if( !call )
               return;

            EOS_ASSERT( getFunctionType(call)->parameters.size() == args.size(), wasm_exception, "" );

//内存实例在所有wavm_实例化的_模块中重复使用，但对于WASM实例除外。
//如果没有声明“memory”，则getDefaultMemory（）将看不到它。
            MemoryInstance* default_mem = getDefaultMemory(_instance);
            if(default_mem) {
//重置内存将沙盒内存大小调整为模块的初始内存大小，然后
//（有效地）memzero它全部
               resetMemory(default_mem, _module->memories.defs[0].type);

               char* memstart = &memoryRef<char>(getDefaultMemory(_instance), 0);
               memcpy(memstart, _initial_memory.data(), _initial_memory.size());
            }

            the_running_instance_context.memory = default_mem;
            the_running_instance_context.apply_ctx = &context;

            resetGlobalInstances(_instance);
            runInstanceStartFunc(_instance);
            Runtime::invokeFunction(call,args);
         } catch( const wasm_exit& e ) {
         } catch( const Runtime::Exception& e ) {
             FC_THROW_EXCEPTION(wasm_execution_error,
                         "cause: ${cause}\n${callstack}",
                         ("cause", string(describeExceptionCause(e.cause)))
                         ("callstack", e.callStack));
         } FC_CAPTURE_AND_RETHROW()
      }


      std::vector<uint8_t>     _initial_memory;
//裸指针，因为moduleInstance是不透明的
//删除wavm时，通过wavm的对象垃圾收集删除实例
      ModuleInstance*          _instance;
      std::unique_ptr<Module>  _module;
};


wavm_runtime::runtime_guard::runtime_guard() {
//要清理这个
//检查_wasm_opcode_dispositions（）；
   Runtime::init();
}

wavm_runtime::runtime_guard::~runtime_guard() {
   Runtime::freeUnreferencedObjects({});
}

static weak_ptr<wavm_runtime::runtime_guard> __runtime_guard_ptr;
static std::mutex __runtime_guard_lock;

wavm_runtime::wavm_runtime() {
   std::lock_guard<std::mutex> l(__runtime_guard_lock);
   if (__runtime_guard_ptr.use_count() == 0) {
      _runtime_guard = std::make_shared<runtime_guard>();
      __runtime_guard_ptr = _runtime_guard;
   } else {
      _runtime_guard = __runtime_guard_ptr.lock();
   }
}

wavm_runtime::~wavm_runtime() {
}

std::unique_ptr<wasm_instantiated_module_interface> wavm_runtime::instantiate_module(const char* code_bytes, size_t code_size, std::vector<uint8_t> initial_memory) {
   std::unique_ptr<Module> module = std::make_unique<Module>();
   try {
      Serialization::MemoryInputStream stream((const U8*)code_bytes, code_size);
      WASM::serialize(stream, *module);
   } catch(const Serialization::FatalSerializationException& e) {
      EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
   } catch(const IR::ValidationException& e) {
      EOS_ASSERT(false, wasm_serialization_error, e.message.c_str());
   }

   eosio::chain::webassembly::common::root_resolver resolver;
   LinkResult link_result = linkModule(*module, resolver);
   ModuleInstance *instance = instantiateModule(*module, std::move(link_result.resolvedImports));
   EOS_ASSERT(instance != nullptr, wasm_exception, "Fail to Instantiate WAVM Module");

   return std::make_unique<wavm_instantiated_module>(instance, std::move(module), initial_memory);
}

void wavm_runtime::immediately_exit_currently_running_module() {
#ifdef _WIN32
   throw wasm_exit();
#else
   Platform::immediately_exit();
#endif
}

}}}}
