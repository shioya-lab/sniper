#include <riscv_decoder.h>
#include "qemu_frontend.h"

namespace frontend
{

class Memory
{
   public:
   virtual ~Memory()
   {
   }

   virtual uint32_t handle(unsigned int threadid, void* regs[],
                           dl::DecodedInst* inst) = 0;
};

struct RegisterBuffer final
{
   RegisterBuffer() : array(g_byte_array_new())
   {
   }

   ~RegisterBuffer()
   {
      g_byte_array_unref(array);
   }

   void read(void* reg)
   {
      g_byte_array_set_size(array, 0);
      pluginReadRegister(reg, array);
   }

   uint64_t le64()
   {
      return GUINT64_FROM_LE(*reinterpret_cast<uint64_t*>(array->data));
   }

   GByteArray * const array;
};

class RiscvVector final
{
   public:
   RiscvVector(void* regs[], RegisterBuffer& buffer, uint64_t id)
      : m_regs(regs)
      , m_buffer(buffer)
      , m_id(id + 64)
   {
   }

   uint64_t read(uint8_t width)
   {
      uint64_t value = 0;

      for (uint8_t e = 0; e < width; e++)
      {
         if (m_index >= m_buffer.array->len)
         {
            m_buffer.read(m_regs[m_id]);
            m_id++;
            m_index = 0;
         }

         value |= (m_buffer.array->data[m_index] << (e * 8));
         m_index++;
      }

      return value;
   }

   private:
   void* const* m_regs;
   RegisterBuffer& m_buffer;
   uint64_t m_id;
   uint64_t m_index = UINT64_MAX;
};

template<typename T>
class RiscvMemory final : public Memory
{
   public:
   virtual uint32_t handle(unsigned int threadid, void* regs[],
                           dl::DecodedInst* inst) override
   {
      auto riscv_inst = static_cast<dl::RISCVDecodedInst*>(inst);
      auto& dec = *riscv_inst->get_rv8_dec();

      if (rv_inst_format[dec.op] == rv_fmt_rd_offset_rs1 ||
          rv_inst_format[dec.op] == rv_fmt_frd_offset_rs1 ||
          rv_inst_format[dec.op] == rv_fmt_rs2_offset_rs1 ||
          rv_inst_format[dec.op] == rv_fmt_frs2_offset_rs1)
      {
         m_buffer.read(regs[dec.rs1]);
         T::handleMemory(threadid, dec.imm + m_buffer.le64());
         return 1;
      }

      if (rv_inst_format[dec.op] == rv_fmt_aqrl_rd_rs2_rs1)
      {
         m_buffer.read(regs[dec.rs1]);
         T::handleMemory(threadid, m_buffer.le64());
         return 1;
      }

      return 0;
   }

   private:
   RegisterBuffer m_buffer;
};

}
