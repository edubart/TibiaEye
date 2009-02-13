#ifndef HEADERS_H
#define HEADERS_H

// Qt headers
#include <QtGui>
#include <QtNetwork>

// GMP headers
#include <gmp.h>

// OS specif headers
#ifdef WIN32
#include <windows.h>
#include <shlwapi.h>
#else
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <unistd.h>
#endif

// internal headers
#include "definitions.h"
#include "singleton.h"

#endif // HEADERS_H
