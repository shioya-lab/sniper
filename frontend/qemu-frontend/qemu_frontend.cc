#include <charconv>
#include <forward_list>
#include <optional>
#include <decoder.h>
#include "frontend.h"
#include "qemu_frontend.h"

namespace frontend
{

class QemuFrontend final : public Frontend<QemuFrontend>
{
   public:
   struct Inst final
   {
      addr_t addr;
      uint32_t size;
      bool is_branch;
      bool is_pause;
   };

   Inst& allocateTb(size_t size)
   {
      auto& ptr = m_tbs.emplace_front();
      auto tb = new Inst[size];
      ptr.reset(tb);
      return *tb;
   }

   void decode(Inst& inst, const void* data, size_t size, uint64_t addr)
   {
      std::unique_ptr<dl::DecodedInst> decoded(
         m_decoder_factory.CreateInstruction(
            m_decoder.get(), static_cast<const uint8_t*>(data), size, addr));

      m_decoder->decode(decoded.get());

      inst.addr = addr;
      inst.size = size;
      inst.is_branch = decoded->is_conditional_branch();
      inst.is_pause = m_decoder->is_pause_opcode(decoded->inst_num_id());
   }

   inline void init();

   void executeMemoryAccess(unsigned int threadid, uint64_t address)
   {
      if (!m_executions[threadid].address)
      {
         m_executions[threadid].address = address;

         if (m_control->get_any_thread_in_detail())
         {
            QemuFrontend::handleMemory(threadid, address);
         }
      }
      else
      {
         // QEMU calls back when a memory access happens while
         // QemuFrontend::handleMemory expects it will be called in the order
         // of operands in instruction encoding. Since currently there is no
         // way to close the gap, instructions with several memory operands are
         // not supported.
         // However, it is still possible to have several memory accesses for
         // the sole memory operand; a typical example is an atomic instruction
         // that performs a load and a store in order. In such a case,
         // the memory accesses should operate on the same address.
         assert(m_executions[threadid].address == address);
      }
   }

   void sendInstruction(unsigned int threadid, const Inst& inst)
   {
      if (m_executions[threadid].inst)
      {
         if (m_control->get_any_thread_in_detail())
         {
            auto addr = m_executions[threadid].inst->addr;
            auto size = m_executions[threadid].inst->size;
            auto is_branch = m_executions[threadid].inst->is_branch;
            auto is_pause = m_executions[threadid].inst->is_pause;
            auto num_addresses = m_executions[threadid].address.has_value();
            auto taken = addr + size != inst.addr;

            FrontendCallbacks<QemuFrontend>::sendInstruction(
               threadid, addr, size, num_addresses, is_branch, taken,
               false, true, false, is_pause);
         }
         else
         {
            FrontendCallbacks<QemuFrontend>::countInsns(threadid, 1);
         }
      }

      m_executions[threadid].inst = &inst;
      m_executions[threadid].address.reset();
   }

   void threadStart(unsigned int threadid)
   {
      assert(threadid < MAX_NUM_THREADS);
   }

   void threadFinish(unsigned int threadid)
   {
      auto& last_inst = *m_executions[threadid].inst;
      Inst inst;
      inst.addr = last_inst.addr + last_inst.size;
      sendInstruction(threadid, inst);
   }

   private:
   struct Execution final
   {
      const Inst* inst;
      std::optional<uint64_t> address;
   };

   dl::DecoderFactory m_decoder_factory;
   std::unique_ptr<dl::Decoder> m_decoder;
   std::forward_list<std::unique_ptr<Inst[]>> m_tbs;
   Execution m_executions[MAX_NUM_THREADS];
};

template <>
class FrontendOptions<QemuFrontend> : public OptionsBase<QemuFrontend>
{
   public:
   FrontendOptions(int argc, const char *argv[])
   {
      parsing_error = false;
      current_mode = Sift::ModeIcount;
      verbose = false;
      use_roi = false;
      mpi_implicit_roi = false;
      fast_forward_target = 0;
      detailed_target = 0;
      blocksize = 0;
      emulate_syscalls = false; // Not implemented.
      response_files = false;
      send_physical_address = false;
      stop_address = 0;
      app_id = 0;
      flow_control = 1000;
      flow_control_ff = 100000;
      ssh = false;

      for (int index = 0; index < argc; index++)
      {
         if (parse(verbose, "verbose", argv[index]) ||
             parse(use_roi, "use_roi", argv[index]) ||
             parse(fast_forward_target, "fast_forward_target", argv[index]) ||
             parse(detailed_target, "detailed_target", argv[index]) ||
             parse(blocksize, "blocksize", argv[index]) ||
             parse(output_file, "output_file", argv[index]) ||
             parse(response_files, "response_files", argv[index]) ||
             parse(stop_address, "stop_address", argv[index]) ||
             parse(app_id, "app_id", argv[index]) ||
             parse(flow_control, "flow_control", argv[index]) ||
             parse(flow_control_ff, "flow_control_ff", argv[index]) ||
             parse(ssh, "ssh", argv[index]))
         {
            continue;
         }

         std::cerr << "unknown key\n";
         parsing_error = true;
      }
   }

   std::string cmd_summary()
   {
      return "Options:\n"
             "  verbose=<bool (on/off)> - activate verbose output (default = off)\n"
             "  use_roi=<bool (on/off)> - use ROI markers (default = off)\n"
             "  fast_forward_target=<num> - instructions to fast forward (default = 0)\n"
             "  detailed_target=<num> - instructions to trace in detail (default = 0 = all)\n"
             "  blocksize=<num> - create several traces, each with <num> instructions (default = 0 = disabled)\n"
             "  output_file=<str> - name of the output file to save the trace\n"
             "  response_files=<bool> - use response files (default = off)\n"
             "  stop_address=<num> - stop address (default = 0 = disabled)\n"
             "  app_id=<num> - sift app id (default = 0)\n"
             "  flow_control=<num> - instructions before sync (default = 1000)\n"
             "  flow_control_ff=<num> - instructions before sync in fast-forward mode (default = 100000)\n"
             "  ssh=<bool (on/off)> - backend and frontend communicate over the network\n";
   }

   FrontendISA get_theISA()
   {
      return s_isa;
   }

   static bool set_theISA(const char* name)
   {
      // ISAs with predicates are not supported because we lack code
      // to detect them.
      static const Target s_targets[] = {
#if SNIPER_ARM
         { "aarch64", ARM_AARCH64 },
#endif
#if SNIPER_RISCV
         { "riscv64", RISCV },
#endif
      };

      for (auto& target : s_targets)
      {
         if (!strcmp(target.name, name))
         {
            s_isa = target.isa;
            return true;
         }
      }

      return false;
   }

   private:
   struct Target final
   {
      const char* name;
      FrontendISA isa;
   };

   static FrontendISA s_isa;

   bool parse(const char*& result, std::string key, const char* input)
   {
      if (memcmp(input, key.data(), key.size()) || input[key.size()] != '=')
      {
         return false;
      }

      result = input + key.size() + 1;
      return true;
   }

   template <typename T>
   bool parse(T& result, const std::string& key, const char* input)
   {
      const char* value;

      if (!parse(value, key, input))
      {
         return false;
      }

      if (!parseValue(result, key, value))
      {
         parsing_error = true;
      }

      return true;
   }

   bool parseValue(bool& result, const std::string& key, const char *value)
   {
      return pluginBoolParse(key.c_str(), value, &result);
   }

   bool parseValue(std::string& result, const std::string& key,
                   const char* value)
   {
      result = value;
      return true;
   }

   template <typename T>
   bool parseValue(T& result, const std::string& key, const char* value)
   {
      auto [ptr, ec] = std::from_chars(value, value + strlen(value), result);
      if (ptr)
      {
         return true;
      }

      switch (ec)
      {
         case std::errc::invalid_argument:
            std::cerr << "Parameter '" << key << "' expects a number\n";
            return false;

         case std::errc::result_out_of_range:
            std::cerr << "Parameter '" << key << "' is out of range\n";
            return false;

         default:
            abort();
      }
   }
};

template <> class FrontendSyscallModel<QemuFrontend>
   : public FrontendSyscallModelBase<QemuFrontend>
{
   using FrontendSyscallModelBase<QemuFrontend>::FrontendSyscallModelBase;

   public:
   static void handleSyscall(threadid_t threadid, addr_t num,
                             syscall_args_t args)
   {
      FrontendSyscallModel<QemuFrontend>::setTID(threadid);
      FrontendSyscallModel<QemuFrontend>::doSyscall(threadid, num, args);
      assert(!m_thread_data[threadid].last_syscall_emulated);
   }
};

FrontendISA FrontendOptions<QemuFrontend>::s_isa;
static QemuFrontend* s_frontend;

void QemuFrontend::init()
{
   dl::dl_arch arch;

   switch (m_options->get_theISA())
   {
      case ARM_AARCH64:
         arch = dl::DL_ARCH_ARMv8;
         break;

      case RISCV:
         arch = dl::DL_ARCH_RISCV;
         break;

      default:
         abort();
   }

   m_decoder.reset(
      m_decoder_factory.CreateDecoder(
         arch, dl::DL_MODE_64, dl::DL_SYNTAX_DEFAULT, 0));

   m_threads->initThreads();
}

template <>
void FrontendThreads<QemuFrontend>::callFinishHelper(threadid_t threadid)
{
  threadFinishHelper(reinterpret_cast<void*>(threadid));
}

template <> void ExecFrontend<QemuFrontend>::handle_frontend_init()
{
   m_frontend->init();
   s_frontend = m_frontend;
   pluginInit();
}

template <> void ExecFrontend<QemuFrontend>::handle_frontend_start()
{
}

template <> void ExecFrontend<QemuFrontend>::handle_frontend_fini()
{
   pluginFini();
}

extern "C" void* allocateTb(size_t size)
{
   try
   {
      return &s_frontend->allocateTb(size);
   }
   catch (...)
   {
      abort();
   }
}

extern "C" void* decode(void* tb, size_t index,
                        const void* data, size_t size, uint64_t addr)
{
   try
   {
      auto& inst = static_cast<QemuFrontend::Inst*>(tb)[index];
      s_frontend->decode(inst, data, size, addr);
      return &inst;
   }
   catch (...)
   {
      abort();
   }
}

extern "C" void handleSyscall(unsigned int threadid,
                               int64_t num, uint64_t args[6])
{
   try
   {
      FrontendSyscallModel<QemuFrontend>::handleSyscall(threadid, num, args);
   }
   catch (...)
   {
      abort();
   }
}

extern "C" void executeMemoryAccess(unsigned int threadid, uint64_t address)
{
   try
   {
      s_frontend->executeMemoryAccess(threadid, address);
   }
   catch (...)
   {
      abort();
   }
}

extern "C" void sendInstruction(unsigned int threadid, void* decoded)
{
   try
   {
      s_frontend->sendInstruction(
         threadid,
         *static_cast<const frontend::QemuFrontend::Inst*>(decoded));
   }
   catch (...)
   {
      abort();
   }
}

extern "C" void threadStart(unsigned int threadid)
{
   try
   {
      s_frontend->threadStart(threadid);
      FrontendThreads<QemuFrontend>::threadStart(threadid);
   }
   catch (...)
   {
      abort();
   }
}

extern "C" void threadFinish(unsigned int threadid)
{
   try
   {
      s_frontend->threadFinish(threadid);
      FrontendThreads<QemuFrontend>::threadFinish(threadid, 0);
   }
   catch (...)
   {
      abort();
   }
}

extern "C" int start(const char* target_name, int argc, const char* argv[])
{
   try
   {
      if (!FrontendOptions<QemuFrontend>::set_theISA(target_name))
      {
         std::cerr << "unsupported target\n";
         return 1;
      }

      frontend::ExecFrontend<QemuFrontend>(argc, argv).start();
      return 0;
   }
   catch (...)
   {
      return 1;
   }
}

extern "C" void fini(void)
{
   try
   {
      s_frontend->get_control()->Fini();
   }
   catch (...)
   {
      abort();
   }
}

};
