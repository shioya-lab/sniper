#include <glib.h>

#ifdef __cplusplus
extern "C"
{
#endif

void pluginInit(void);
void pluginFini(void);
bool pluginBoolParse(const char* name, const char* val, bool* ret);
GArray* pluginGetRegisters(void);
void* pluginFindRegister(const GArray* registers, const char* name,
                         const char* feature);
int pluginReadRegister(void* reg, GByteArray* buf);

void* allocateTb(size_t size);
void* decode(void* tb, size_t index,
             const void* data, size_t size, uint64_t addr);
void handleSyscall(unsigned int threadid, int64_t num, uint64_t args[6]);
void sendInstruction(unsigned int thread, void* decoded);

void threadStart(unsigned int threadid);
void threadFinish(unsigned int threadid);

int start(const char* target_name, int argc, const char* argv[]);
void fini(void);

#ifdef __cplusplus
}
#endif
