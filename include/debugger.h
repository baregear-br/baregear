#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stddef.h>
#include <stdint.h>
#include <dynvar.h>

#include <lldb/API/SBDebugger.h>
#include <lldb/API/SBTarget.h>
#include <lldb/API/SBProcess.h>
#include <lldb/API/SBListener.h>
#include <lldb/API/SBBreakpoint.h>
#include <lldb/API/SBBreakpointLocation.h>
#include <lldb/API/SBThread.h>
#include <lldb/API/SBFrame.h>
#include <lldb/API/SBValue.h>

typedef struct {
    char* variableName;
    dynvar value;
    int pid;
} VariableInfo;

typedef struct {
    int lineNumber;
    dynvar sourcePath;
} SymInfo;

typedef struct {
    uintptr_t function;
    vector* params;
    dynvar value;
} VRecord;

/* typedef struct {
    dynvar name;
    uintptr_t location;
} VMap; */

#ifdef __cplusplus
extern "C" {
#endif
extern vector* VRecords;
extern lldb::SBTarget g_target;
extern lldb::SBProcess g_process;
// extern vector* VMaps;

extern void startDebugger(dynvar* host, unsigned short port, const void* buffer, size_t bufrSize);
extern VariableInfo getValueOfVariable(dynvar* variableName);
extern VariableInfo getReturnValue();
extern SymInfo getLineNumber(uint64_t address);
extern void stopDebugger();

#ifdef __cplusplus
}

extern lldb::SBBreakpoint addBreakpoint(int line, dynvar* source, void* callback);
extern void deleteBreakpoint(lldb::SBBreakpoint bp);
extern uintptr_t recordVar();
#endif

#endif // DEBUGGER_H