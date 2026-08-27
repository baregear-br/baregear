/*
 * baregear - A programming language compiler
 * Copyright (C) 2026 First Person
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sys/mman.h>
#include <unistd.h>
#include <lldb/API/SBDebugger.h>
#include <lldb/API/SBTarget.h>
#include <lldb/API/SBProcess.h>
#include <lldb/API/SBListener.h>
#include <lldb/API/SBBreakpoint.h>
#include <lldb/API/SBThread.h>
#include <lldb/API/SBFrame.h>
#include <lldb/API/SBValue.h>

#include <debugger.h>
#include <definations.h>

static lldb::SBDebugger g_debugger;
static lldb::SBTarget g_target;
static lldb::SBProcess g_process;
static bool g_debuggerInitialized = false;

void startDebugger(const char* host, unsigned short port, const void* buffer, size_t bufrSize) {
    if (g_debuggerInitialized)
        return;

    g_debugger = lldb::SBDebugger::Create();
    if (!g_debugger.IsValid())
        return;

    g_debugger.SetAsync(false);

    char connectionUrl[256];
    snprintf(connectionUrl, sizeof(connectionUrl), "connect://%s:%u", host, port);

    // 3. Create an empty target for remote debugging
    lldb::SBError error;
    lldb::SBTarget target = g_debugger.CreateTarget("", "", "", true, error);

    if (!target.IsValid()) {
        lldb::SBDebugger::Destroy(g_debugger);
        return;
    }

    g_target = target;

    lldb::SBListener listener = g_debugger.GetListener();
    lldb::SBProcess process = g_target.ConnectRemote(listener, connectionUrl, nullptr, error);

    if (!process.IsValid() || error.Fail()) {
        lldb::SBDebugger::Destroy(g_debugger);
        return;
    }

    g_process = process;

    if (buffer && bufrSize > 0) {
#ifdef __linux__
        // Create an anonymous RAM file descriptor
        int mem_fd = memfd_create("ram_symbol_module", MFD_CLOEXEC);
        if (mem_fd != -1) {
            // Write the raw binary/symbol payload from RAM into the file descriptor
            ssize_t written = write(mem_fd, buffer, bufrSize);
            if (written > 0) {
                char path_buf[64];
                snprintf(path_buf, sizeof(path_buf), "/proc/self/fd/%d", mem_fd);

                // Add the module to LLDB using the virtual memory path
                lldb::SBModule module = g_target.AddModule(path_buf, "x86_64-pc-linux", nullptr);
                if (module.IsValid()) {
                    // Successfully loaded symbols into the debugger from RAM
                    // Note: mem_fd can be closed or left open; LLDB maintains its reference via the descriptor path
                }
            }
        }
#endif
    }

    g_debuggerInitialized = true;
}

void stopDebugger() {
    if (g_process.IsValid()) {
        g_process.Destroy();
        g_process = lldb::SBProcess();
    }
    
    if (g_target.IsValid())
        g_target = lldb::SBTarget();
    
    if (g_debugger.IsValid()) {
        lldb::SBDebugger::Destroy(g_debugger);
        g_debugger = lldb::SBDebugger();
    }
    
    g_debuggerInitialized = false;
}

VariableInfo getValueOfVariable(const char* variableName) {
    VariableInfo info;
    info.variableName = nullptr;
    info.value = nullptr;
    info.pid = -1;

    if (!g_debuggerInitialized) {
        bugDetected("Debugger not initialized. Call startDebugger() first.");
        return info;
    }

    if (!g_process.IsValid()) {
        bugDetected("No active debugging process.");
        return info;
    }

    // Get the PID from the process
    info.pid = g_process.GetProcessID();

    // Get the current thread
    lldb::SBThread thread = g_process.GetSelectedThread();
    if (!thread.IsValid()) {
        bugDetected("No valid thread found.");
        return info;
    }

    // Get the current frame
    lldb::SBFrame frame = thread.GetSelectedFrame();
    if (!frame.IsValid()) {
        bugDetected("No valid frame found.");
        return info;
    }

    // Find the specific variable
    lldb::SBValueList variables = frame.GetVariables(true, true, false, true);
    size_t numVariables = variables.GetSize();

    for (size_t i = 0; i < numVariables; ++i) {
        lldb::SBValue variable = variables.GetValueAtIndex(i);
        if (variable.IsValid()) {
            const char* name = variable.GetName();
            if (name && strcmp(name, variableName) == 0) {
                // Found the variable
                const char* valueStr = variable.GetValue();
                
                // Copy variable name
                info.variableName = strdup(name);
                
                // Copy value
                if (valueStr)
                    info.value = strdup(valueStr);
                else
                    info.value = strdup("<no value>");
                
                return info;
            }
        }
    }

    // Variable not found
    std::string errorMsg = "Variable '" + std::string(variableName) + "' not found in current frame.";
    bugDetected(errorMsg.c_str());
    return info;
}