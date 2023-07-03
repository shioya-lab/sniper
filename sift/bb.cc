#include <iostream>
#include <unordered_map>
#include "sift_reader.h"

static constexpr unsigned long defaultInterval = 100000000;

static void printUsage(char *argv[])
{
   std::cerr << "Usage: " << (argv[0] ? argv[0] : "bb")
             << " <file.sift> [interval=" << defaultInterval << "]\n";
}

int main(int argc, char* argv[])
{
   std::unordered_map<uint64_t, unsigned long> map;
   Sift::Instruction inst;
   uint64_t bb;
   unsigned long interval;
   unsigned long count = 0;
   bool ended = true;

   if (argc < 2)
   {
      printUsage(argv);
      return EXIT_FAILURE;
   }

   if (argc < 3)
   {
      interval = defaultInterval;
   }
   else
   {
      interval = std::strtoul(argv[2], NULL, 0);
      if (!interval)
      {
         printUsage(argv);
         return EXIT_FAILURE;
      }
   }

   Sift::Reader reader(argv[1]);

   while (reader.Read(inst))
   {
      if (ended)
      {
         bb = inst.sinst->addr;
      }

      ended = inst.is_branch;
      if (ended)
      {
         auto [iterator, inserted] = map.insert({ bb, 1 });
         if (!inserted)
         {
            auto k = map[bb];
            iterator->second++;
            assert(k + 1 == map[bb]);
         }
      }

      count++;
      if (count >= interval)
      {
         std::cout << "T";

         for (auto [key, value] : map)
         {
            std::cout << ":0x" << key << std::hex << ":" << std::dec << value
                      << "   ";
         }

         std::cout << "\n";
         map.clear();
         count = 0;
      }
   }
}
