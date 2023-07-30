#include <qemu-plugin.h>
#include "qemu_frontend.h"

static qemu_plugin_id_t static_id;

static void pluginVcpuInit(qemu_plugin_id_t id, unsigned int vcpu_index)
{
   threadStart(vcpu_index);
}

static void pluginVcpuExit(qemu_plugin_id_t id, unsigned int vcpu_index)
{
   threadFinish(vcpu_index);
}

static void pluginVcpuSyscall(qemu_plugin_id_t id, unsigned int vcpu_index,
                              int64_t num, uint64_t a1, uint64_t a2,
                              uint64_t a3, uint64_t a4, uint64_t a5,
                              uint64_t a6, uint64_t a7, uint64_t a8)
{
   uint64_t args[6];

   args[0] = a1;
   args[1] = a2;
   args[2] = a3;
   args[3] = a4;
   args[4] = a5;
   args[5] = a6;

   handleSyscall(vcpu_index, num, args);
}

static void pluginVcpuTbTrans(qemu_plugin_id_t id,
                              struct qemu_plugin_tb* plugin_tb)
{
   size_t n = qemu_plugin_tb_n_insns(plugin_tb);
   void* tb = allocateTb(n);

   for (size_t index = 0; index < n; index++) {
      struct qemu_plugin_insn* insn = qemu_plugin_tb_get_insn(plugin_tb, index);
      const void* insn_data = qemu_plugin_insn_data(insn);
      size_t insn_size = qemu_plugin_insn_size(insn);
      uint64_t insn_vaddr = qemu_plugin_insn_vaddr(insn);
      void* decoded = decode(tb, index, insn_data, insn_size, insn_vaddr);

      qemu_plugin_register_vcpu_insn_exec_cb(insn, sendInstruction,
                                             QEMU_PLUGIN_CB_R_REGS, decoded);
    }
}

static void pluginAtexit(qemu_plugin_id_t id, void *userdata)
{
   fini();
}

void pluginInit(void)
{
   qemu_plugin_register_vcpu_init_cb(static_id, pluginVcpuInit);
   qemu_plugin_register_vcpu_exit_cb(static_id, pluginVcpuExit);
   qemu_plugin_register_vcpu_syscall_cb(static_id, pluginVcpuSyscall);
   qemu_plugin_register_vcpu_tb_trans_cb(static_id, pluginVcpuTbTrans);
}

void pluginFini(void)
{
   qemu_plugin_register_atexit_cb(static_id, pluginAtexit, NULL);
}

bool pluginBoolParse(const char* name, const char* val, bool* ret)
{
   return qemu_plugin_bool_parse(name, val, ret);
}

GArray* pluginGetRegisters(void)
{
   return qemu_plugin_get_registers();
}

void* pluginFindRegister(const GArray* registers,
                         const char* name, const char* feature)
{
   for (guint i = 0; i < registers->len; i++)
   {
      qemu_plugin_reg_descriptor *descriptor =
         &g_array_index(registers, qemu_plugin_reg_descriptor, i);

      if (!strcmp(name, descriptor->name) &&
          !strcmp(feature, descriptor->feature))
      {
         return descriptor->handle;
      }
   }

   return NULL;
}

int pluginReadRegister(void* reg, GByteArray* buf)
{
   return qemu_plugin_read_register(reg, buf);
}

QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;

QEMU_PLUGIN_EXPORT int qemu_plugin_install(qemu_plugin_id_t id,
                                           const qemu_info_t* info, int argc,
                                           char** argv)
{
   static_id = id;
   return start(info->target_name, argc, (const char**)argv);
}
