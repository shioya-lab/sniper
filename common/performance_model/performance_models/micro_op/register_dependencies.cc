#include "register_dependencies.h"
#include "dynamic_micro_op.h"

RegisterDependencies::RegisterDependencies()
{
   clear();
}

void RegisterDependencies::setDependencies(DynamicMicroOp& microOp, uint64_t lowestValidSequenceNumber)
{
   // Create the dependencies for the microOp
   for(uint32_t i = 0; i < microOp.getMicroOp()->getSourceRegistersLength(); i++)
   {
      dl::Decoder::decoder_reg sourceRegister = microOp.getMicroOp()->getSourceRegister(i);
      uint32_t mappedRegister = Sim()->getDecoder()->map_register(sourceRegister);

      uint64_t producerSequenceNumber;
      LOG_ASSERT_ERROR(sourceRegister < Sim()->getDecoder()->last_reg(), "Source register src[%u]=%u is invalid", i, sourceRegister);
      if ((producerSequenceNumber = producers[sourceRegister]) != INVALID_SEQNR)
      {
         if (producerSequenceNumber >= lowestValidSequenceNumber)
         {
            for (uint64_t k = 0; k <= producerLength[sourceRegister]; k++) {
               if (producerSequenceNumber < microOp.getSequenceNumber() - microOp.getMicroOp()->getTypeOffset()) {
                  microOp.addDependency(producerSequenceNumber + k);
                  microOp.addRegDependency(producerSequenceNumber + k);
               }
            }
         }
         else
         {
            producers[mappedRegister] = INVALID_SEQNR;
         }
      }
   }

   // producerLength:
   // (0) vluxei v24,(a0),v24 // index-0
   // (1) vluxei v24,(a0),v24 // index-1
   // v24 is associated with (0) and (1)
   // producer[v24] = (0) and producerLength[v24] = 1;
   // --> consumer should depends on producer[v24] and producer[v24]+1
   if (microOp.getMicroOp()->isVector() &&
       !microOp.getMicroOp()->canVecSquash()) {
      for(uint32_t i = 0; i < microOp.getMicroOp()->getDestinationRegistersLength(); i++)
      {
         uint32_t destinationRegister = microOp.getMicroOp()->getDestinationRegister(i);
         LOG_ASSERT_ERROR(destinationRegister < Sim()->getDecoder()->last_reg(), "Destination register dst[%u] = %u is invalid", i, destinationRegister);
         if (microOp.isFirst()) {
            producerLength[destinationRegister] = 0;
         } else {
            producerLength[destinationRegister]++;
         }
      }
   } else {
      for(uint32_t i = 0; i < microOp.getMicroOp()->getDestinationRegistersLength(); i++)
      {
         uint32_t destinationRegister = microOp.getMicroOp()->getDestinationRegister(i);
         LOG_ASSERT_ERROR(destinationRegister < Sim()->getDecoder()->last_reg(), "Destination register dst[%u] = %u is invalid", i, destinationRegister);
         producerLength[destinationRegister] = 0;
      }
   }

   // Intermediate Vector Instruction doent' update producer register
   // VLUXEI case
   // vluxei v24,(a0),v24
   // -->
   // (0) vluxei v24,(a0),v24 // index-0
   // (1) vluxei v24,(a0),v24 // index-1
   // Then 1 and 0 are issued in same time, it may apper dependencies v24 between (0) and (1)
   // So, in this case (0) doesn't update producer denependency
   //
   if (microOp.getMicroOp()->isVector() &&
       !microOp.getMicroOp()->canVecSquash() &&
       !microOp.isFirst()) {
      // Not UnitStride, Gather/Scatter instructions are Issued in same time, then prevent Intermeditae Update
      return;
   }
   // Update the producers
   for(uint32_t i = 0; i < microOp.getMicroOp()->getDestinationRegistersLength(); i++)
   {
      uint32_t destinationRegister = microOp.getMicroOp()->getDestinationRegister(i);
      LOG_ASSERT_ERROR(destinationRegister < Sim()->getDecoder()->last_reg(), "Destination register dst[%u] = %u is invalid", i, destinationRegister);
      producers[destinationRegister] = microOp.getSequenceNumber();
   }

}

uint64_t RegisterDependencies::peekProducer(dl::Decoder::decoder_reg reg, uint64_t lowestValidSequenceNumber)
{
   if (reg == dl::Decoder::DL_REG_INVALID)
      return INVALID_SEQNR;

   uint64_t producerSequenceNumber = producers[reg];
   if (producerSequenceNumber == INVALID_SEQNR || producerSequenceNumber < lowestValidSequenceNumber)
      return INVALID_SEQNR;

   return producerSequenceNumber;
}

void RegisterDependencies::clear()
{
   for(uint32_t i = 0; i < Sim()->getDecoder()->last_reg(); i++)
   {
      producers[i] = INVALID_SEQNR;
      producerLength[i] = 0;
   }
}
