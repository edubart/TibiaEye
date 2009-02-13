/**
 * This source file is used to print out a stack-trace when your program
 * segfaults.  It is relatively reliable and spot-on accurate.
 *
 * This code is in the public domain.  Use it as you see fit, some credit
 * would be appreciated, but is not a prerequisite for usage.  Feedback
 * on it's use would encourage further development and maintenance.
 *
 * Just include this file in your project and make sure that you
 * have addr2line (it comes with binutils package). Also is necessary to
 * link libdl.
 *
 * Remember to always compile your program with -rdynamic flag to
 * generate useful backtraces. And optionally with -ggdb or -g to get
 * line backtraces.
 *
 * Author:  Eduardo Bart <bart_barthe@hotmail.com>
 *
 * Copyright (C) 2009 Eduardo Bart
 */

#ifndef __BACKTRACE_H_
#define __BACKTRACE_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <ucontext.h>
#include <dlfcn.h>
#include <execinfo.h>

#define MAX_BACKTRACE 128

void backtrace_printf(const char *message, ...)
{
	static char buffer[4096];
	va_list argptr;

	va_start(argptr, message);
	vsnprintf(buffer, sizeof (buffer) - 1, message, argptr);
	va_end(argptr);

	fprintf(stderr, "%s", buffer);
}

void backtrace_addr2line(int number, void *address, char *symbol)
{
	int rc;
	Dl_info info;
	const void *addr;
	FILE *output;
	char tmpbuf[1024];
	char printout[1024];
	char *pos;
	struct stat fileInfo;
	size_t i;

	// we only want to read symbol begin
	for(i = 1; i < strlen(symbol); i++) {
		if(symbol[i] == ' ') {
			symbol[i] = 0x00;
			break;
		}
	}

	snprintf(printout, sizeof(printout), "@ [0x%08x] frame %03d %s", (unsigned int)address, number, symbol);

	// try to get exe/lib info via dl
	rc = dladdr(address, &info);
	if(rc != 0 && info.dli_fname != NULL && stat(info.dli_fname, &fileInfo) == 0) {
		// convert addr to use in addr2line
		addr = address;
		if (info.dli_fbase >= (const void *) 0x40000000)
			addr = (void *)((unsigned long)((const char *) addr) - (unsigned long) info.dli_fbase);

		// try to get address line
		snprintf(tmpbuf, sizeof(tmpbuf), "addr2line --functions --demangle -e %s %p", info.dli_fname, addr);
		output = popen(tmpbuf, "r");
		if(output) {
			// read addr2line output
			int func = 1;
			while(!feof(output)) {
				fgets(tmpbuf, sizeof(tmpbuf) - 1, output);
				if(tmpbuf[0] == '\0')
					continue;

				pos = strchr(tmpbuf, '\n');
				if(pos > 0)
					pos[0] = '\0';

				pos = strchr(tmpbuf, ':');
				if(pos > 0 && pos[1] != ':')
					func = 0;

				strncat(printout, " ", sizeof(printout));
				if(func)
					strncat(printout, "[function ", sizeof(printout));
				strncat(printout, tmpbuf, sizeof(printout));
				if(func)
					strncat(printout, "]", sizeof(printout));

				func = 1;
				tmpbuf[0] = '\0';
			}
			pclose (output);
		}
	}

	strncat(printout, "\n", sizeof(printout));
	backtrace_printf(printout);
}

void crash_handler(int signum, siginfo_t *info, void *context)
{
	size_t i;
	struct ucontext *uc = (ucontext_t*)context;
	void *trace[MAX_BACKTRACE];
	size_t trace_size;
	char **symbols;
	void *bugaddr = (void *)uc->uc_mcontext.gregs[REG_EIP];
	char tmpstr[128];

	// print crash info
	backtrace_printf("\n======= program crashed =======\n");

	const char *signame;
	const char *sigerror = NULL;
	switch(signum) {
		case SIGSEGV:
			signame = "Segmentation fault";
			switch(info->si_code) {
				case SEGV_MAPERR:
					sigerror = "Address not mapped to object";
					break;
				case SEGV_ACCERR:
					sigerror = "Invalid permissions for mapped object";
					break;
			}
			break;
		case SIGILL:
			signame = "Illegal instruction";
			switch(info->si_code) {
				case ILL_ILLOPC:
					sigerror = "Illegal opcode";
					break;
				case ILL_ILLOPN:
					sigerror = "Illegal operand";
					break;
				case ILL_ILLADR:
					sigerror = "Iillegal addressing mode";
					break;
				case ILL_ILLTRP:
					sigerror = "Illegal trap";
					break;
				case ILL_PRVOPC :
					sigerror = "Privileged opcode";
					break;
				case ILL_PRVREG:
					sigerror = "Privileged register";
					break;
				case ILL_COPROC:
					sigerror = "Coprocessor error";
					break;
				case ILL_BADSTK:
					sigerror = "Internal stack error";
					break;
			}
			break;
		case SIGFPE:
			signame = "Floating point exception";
			switch(info->si_code) {
				case FPE_INTDIV:
					sigerror = "Integer divide by zero";
					break;

				case FPE_INTOVF:
					sigerror = "Integer overflow";
					break;

				case FPE_FLTDIV:
					sigerror = "Floating point divide by zero";
					break;

				case FPE_FLTOVF:
					sigerror = "Floating point overflow";
					break;

				case FPE_FLTUND:
					sigerror = "Floating point underflow";
					break;

				case FPE_FLTRES:
					sigerror = "Floating point inexact result";
					break;

				case FPE_FLTINV:
					sigerror = "Floating point invalid operation";
					break;

				case FPE_FLTSUB:
					sigerror = "Subscript out of range";
					break;
			}
			break;
		case SIGBUS:
			signame = "Bus error";
			switch(info->si_code) {
				case BUS_ADRALN:
					sigerror = "Invalid address alignment";
					break;

				case BUS_ADRERR:
					sigerror = "Nonexistent physical address";
					break;
				case BUS_OBJERR:
					sigerror = "Object-specific hardware error";
					break;
			}
			break;
		case SIGABRT: signame = "Abort signal"; break;
		default: signame = "Unexpected signal"; break;
	}
	backtrace_printf("%s at eip 0x%08x\n", signame, bugaddr);
	if(sigerror)
		backtrace_printf("Error description: %s at memory address 0x%08x\n", sigerror, info->si_addr);

	backtrace_printf("Compilation date: %s %s\n", __DATE__, __TIME__);

	time_t clock;
	time (&clock);
	strftime(tmpstr, 128, "%Y/%m/%d at %H:%M:%S", localtime(&clock));
	backtrace_printf("Crash time: %s\n", tmpstr);

	// rint registers
	backtrace_printf("======= registers information =======\n");
	backtrace_printf("eax 0x%08x ebx 0x%08x ecx 0x%08x edx 0x%08x esi 0x%08x edi 0x%08x\n",
					 uc->uc_mcontext.gregs[REG_EAX], uc->uc_mcontext.gregs[REG_EBX],
					 uc->uc_mcontext.gregs[REG_ECX], uc->uc_mcontext.gregs[REG_EDX],
					 uc->uc_mcontext.gregs[REG_ESI], uc->uc_mcontext.gregs[REG_EDI]);
	backtrace_printf("ebp 0x%08x esp 0x%08x eip 0x%08x efl 0x%08x err 0x%08x trp 0x%08x\n",
					 uc->uc_mcontext.gregs[REG_EBP], uc->uc_mcontext.gregs[REG_ESP],
					 uc->uc_mcontext.gregs[REG_EIP], uc->uc_mcontext.gregs[REG_EFL],
					 uc->uc_mcontext.gregs[REG_ERR], uc->uc_mcontext.gregs[REG_TRAPNO]);

	// print backtrace
	backtrace_printf("======= backtrace information =======\n");
	trace_size = backtrace(trace, MAX_BACKTRACE);
	symbols = backtrace_symbols(trace, trace_size);

	if(symbols != NULL && trace_size > 0) {
		for(i = 0; i < trace_size-2; i++) {
			backtrace_addr2line(i + 1, trace[i+2], symbols[i+2]);
		}
	}

	backtrace_printf("\n");

	// forces core dump
	signal(signum, SIG_DFL);
}

void setup_signals() {
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = crash_handler;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &sa, NULL); // segmentation fault
	sigaction(SIGILL, &sa, NULL);  // illegal instruction
	sigaction(SIGFPE, &sa, NULL);  // floating point exception
	sigaction(SIGBUS, &sa, NULL);  // bus error (bad memory access)
	sigaction(SIGABRT, &sa, NULL);  // abort signal
}


static void __attribute((constructor)) init(void) {
	setup_signals();
}

#endif // __BACKTRACE_H_
