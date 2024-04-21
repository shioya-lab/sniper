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
   RiscvMemory(const GArray* regs)
      : m_vlenb(readVlenb(regs, m_buffers[0]))
      , m_vl(pluginFindRegister(regs, "vl", "org.gnu.gdb.riscv.csr"))
      , m_vstart(pluginFindRegister(regs, "vstart", "org.gnu.gdb.riscv.csr"))
      , m_vtype(pluginFindRegister(regs, "vtype", "org.gnu.gdb.riscv.csr"))
   {
   }

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
         m_buffers[0].read(regs[dec.rs1]);
         T::handleMemory(threadid, dec.imm + m_buffers[0].le64());
         return 1;
      }

      if (rv_inst_format[dec.op] == rv_fmt_aqrl_rd_rs2_rs1)
      {
         m_buffers[0].read(regs[dec.rs1]);
         T::handleMemory(threadid, m_buffers[0].le64());
         return 1;
      }

      if (m_vtype)
      {
         switch (dec.op)
         {
            case rv_op_vle8_v:
            case rv_op_vse8_v:
            case rv_op_vle8ff_v:
               return handleV(threadid, regs, dec, 1, 1, 1);

            case rv_op_vlse8_v:
            case rv_op_vsse8_v:
               m_buffers[0].read(regs[dec.rs2]);
               return handleV(threadid, regs, dec, 1, 1, m_buffers[0].le64());

            case rv_op_vluxei8_v:
            case rv_op_vloxei8_v:
            case rv_op_vsuxei8_v:
            case rv_op_vsoxei8_v:
               return handleVIndex(threadid, regs, dec, 1, 1);

            case rv_op_vl1re8_v:
            case rv_op_vs1re8_v:
               return handleVWhole(threadid, regs, dec, 1, 1);

            case rv_op_vl2re8_v:
            case rv_op_vs2re8_v:
               return handleVWhole(threadid, regs, dec, 2, 1);

            case rv_op_vl4re8_v:
            case rv_op_vs4re8_v:
               return handleVWhole(threadid, regs, dec, 4, 1);

            case rv_op_vl8re8_v:
            case rv_op_vs8re8_v:
               return handleVWhole(threadid, regs, dec, 8, 1);

            case rv_op_vle16_v:
            case rv_op_vse16_v:
            case rv_op_vle16ff_v:
               return handleV(threadid, regs, dec, 1, 1, 2);

            case rv_op_vlse16_v:
            case rv_op_vsse16_v:
               m_buffers[0].read(regs[dec.rs2]);
               return handleV(threadid, regs, dec, 1, 1, m_buffers[0].le64());

            case rv_op_vluxei16_v:
            case rv_op_vloxei16_v:
            case rv_op_vsuxei16_v:
            case rv_op_vsoxei16_v:
               return handleVIndex(threadid, regs, dec, 1, 2);

            case rv_op_vl1re16_v:
            case rv_op_vs1re16_v:
               return handleVWhole(threadid, regs, dec, 1, 2);

            case rv_op_vl2re16_v:
            case rv_op_vs2re16_v:
               return handleVWhole(threadid, regs, dec, 2, 2);

            case rv_op_vl4re16_v:
            case rv_op_vs4re16_v:
               return handleVWhole(threadid, regs, dec, 4, 2);

            case rv_op_vl8re16_v:
            case rv_op_vs8re16_v:
               return handleVWhole(threadid, regs, dec, 8, 2);

            case rv_op_vle32_v:
            case rv_op_vse32_v:
            case rv_op_vle32ff_v:
               return handleV(threadid, regs, dec, 1, 4, 4);

            case rv_op_vlse32_v:
            case rv_op_vsse32_v:
               m_buffers[0].read(regs[dec.rs2]);
               return handleV(threadid, regs, dec, 1, 4, m_buffers[0].le64());

            case rv_op_vluxei32_v:
            case rv_op_vloxei32_v:
            case rv_op_vsuxei32_v:
            case rv_op_vsoxei32_v:
               return handleVIndex(threadid, regs, dec, 1, 4);

            case rv_op_vl1re32_v:
            case rv_op_vs1re32_v:
               return handleVWhole(threadid, regs, dec, 1, 4);

            case rv_op_vl2re32_v:
            case rv_op_vs2re32_v:
               return handleVWhole(threadid, regs, dec, 2, 4);

            case rv_op_vl4re32_v:
            case rv_op_vs4re32_v:
               return handleVWhole(threadid, regs, dec, 4, 4);

            case rv_op_vl8re32_v:
            case rv_op_vs8re32_v:
               return handleVWhole(threadid, regs, dec, 8, 4);

            case rv_op_vlseg4e32_v:
            case rv_op_vsseg4e32_v:
               return handleV(threadid, regs, dec, 4, 4, 4);

            case rv_op_vlseg8e32_v:
            case rv_op_vsseg8e32_v:
               return handleV(threadid, regs, dec, 8, 4, 8);

            case rv_op_vle64_v:
            case rv_op_vse64_v:
            case rv_op_vle64ff_v:
               return handleV(threadid, regs, dec, 1, 8, 8);

            case rv_op_vlse64_v:
            case rv_op_vsse64_v:
               m_buffers[0].read(regs[dec.rs2]);
               return handleV(threadid, regs, dec, 1, 8, m_buffers[0].le64());

            case rv_op_vluxei64_v:
            case rv_op_vloxei64_v:
            case rv_op_vsuxei64_v:
            case rv_op_vsoxei64_v:
               return handleVIndex(threadid, regs, dec, 1, 8);

            case rv_op_vl1re64_v:
            case rv_op_vs1re64_v:
               return handleVWhole(threadid, regs, dec, 1, 8);

            case rv_op_vl2re64_v:
            case rv_op_vs2re64_v:
               return handleVWhole(threadid, regs, dec, 2, 8);

            case rv_op_vl4re64_v:
            case rv_op_vs4re64_v:
               return handleVWhole(threadid, regs, dec, 4, 8);

            case rv_op_vl8re64_v:
            case rv_op_vs8re64_v:
               return handleVWhole(threadid, regs, dec, 8, 8);
         }
      }

      return 0;
   }

   private:
   static uint64_t readVlenb(const GArray* regs, RegisterBuffer& buffer)
   {
      auto reg = pluginFindRegister(regs, "vlenb", "org.gnu.gdb.riscv.csr");
      if (!reg)
      {
         return 0;
      }

      buffer.read(reg);
      return buffer.le64();
   }

   uint32_t handleV(unsigned int threadid, void* regs[],
                    const riscv::decode& dec, uint64_t nf,
                    uint64_t width, uint64_t stride)
   {
      m_buffers[0].read(regs[dec.rs1]);
      auto base = m_buffers[0].le64();
      m_buffers[0].read(m_vl);
      auto vl = m_buffers[0].le64();
      RiscvVector maskv(regs, m_buffers[0], 0);
      uint32_t num_addresses = 0;
      uint8_t mask = 0;

      for (uint64_t e = 0; e < vl; e++)
      {
         if (!dec.vm)
         {
            if (!(e % 8))
            {
               mask = maskv.read(0);
            }

            if ((mask >> (e % 8)) & 1)
            {
               continue;
            }
         }

         for (uint64_t f = 0; f < nf; f++)
         {
            T::handleMemory(threadid, base + e * stride + f * width);
            num_addresses++;
         }
      }

      return num_addresses;
   }

   uint32_t handleVIndex(unsigned int threadid, void* regs[],
                         const riscv::decode& dec, uint64_t nf,
                         uint64_t width)
   {
      m_buffers[0].read(regs[dec.rs1]);
      auto base = m_buffers[0].le64();
      m_buffers[0].read(m_vtype);
      auto vsew = (m_buffers[0].array->data[0] >> 3) & 7;
      m_buffers[0].read(m_vl);
      auto vl = m_buffers[0].le64();
      RiscvVector maskv(regs, m_buffers[0], 0);
      RiscvVector indexv(regs, m_buffers[1], dec.rs2);
      uint32_t num_addresses = 0;
      uint8_t mask = 0;

      for (uint64_t e = 0; e < vl; e++)
      {
         auto index = indexv.read(width);

         if (!dec.vm)
         {
            if (!(e % 8))
            {
               mask = maskv.read(0);
            }

            if ((mask >> (e % 8)) & 1)
            {
               continue;
            }
         }

         for (uint64_t f = 0; f < nf; f++)
         {
            T::handleMemory(threadid, base + index + (f << vsew));
            num_addresses++;
         }
      }

      return num_addresses;
   }

   uint32_t handleVWhole(unsigned int threadid, void* regs[],
                         const riscv::decode& dec, uint64_t nf,
                         uint64_t width)
   {
      m_buffers[0].read(regs[dec.rs1]);
      auto base = m_buffers[0].le64();
      m_buffers[0].read(m_vstart);
      auto vstart = m_buffers[0].le64();
      auto ne = m_vlenb / width * nf;
      auto num_addresses = ne - vstart;

      while (vstart < ne)
      {
         T::handleMemory(threadid, base + vstart * width);
         vstart++;
      }

      return num_addresses;
   }

   RegisterBuffer m_buffers[2];
   const uint64_t m_vlenb;
   void* const m_vl;
   void* const m_vstart;
   void* const m_vtype;
};

}
