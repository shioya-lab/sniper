#include <glib.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct Inst {
   void (* send)(unsigned int, void*);
   void *data;
};

void pluginInit(void);
void pluginFini(void);
int pluginFindRegisterFile(unsigned int vcpu_index, const char *name);
int pluginFindRegister(unsigned int vcpu_index, int file, const char *name);
int pluginReadRegister(GByteArray *buf, int reg);

void* allocateTb(size_t size);
struct Inst decode(void* tb, size_t index, const void* data, size_t size,
                   uint64_t addr);
void handleSyscall(unsigned int threadid, int64_t num, uint64_t args[6]);

void threadStart(unsigned int threadid);
void threadFinish(unsigned int threadid);

int start(const char* target_name, int argc, const char* argv[]);
void fini(void);

#ifdef __cplusplus
}
#endif
