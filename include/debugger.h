#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char* variableName;
    char* value;
    int pid;
} VariableInfo;

extern void startDebugger(const char* host, unsigned short port, const void* buffer, size_t bufrSize);
extern VariableInfo getValueOfVariable(const char* variableName);
extern void stopDebugger();

#ifdef __cplusplus
}
#endif

#endif // DEBUGGER_H