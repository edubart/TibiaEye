#include "headers.h"
#include "util.h"

ePlatformType getPlataformType()
{
#if defined(Q_WS_X11)
	return WS_X11;
#elif defined(Q_WS_MAC)
	return WS_MAC;
#elif defined(Q_WS_QWS)
	return WS_QWS;
#elif defined(Q_WS_WIN)
	return WS_WIN;
#else
	return WS_UNK;
#endif
}

#ifndef WIN32
/* find_pid_by_name()
 *
 *  This finds the pid of the specified process.
 *  Currently, it's implemented by rummaging through
 *  the proc filesystem.
 *
 *  Returns a list of all matching PIDs
 */
PidList findPidsByName(const char* pidName)
{
	DIR *dir;
	struct dirent *next;
	PidList pidList;

	dir = opendir("/proc");
	if(!dir)
		return pidList;

	while ((next = readdir(dir)) != NULL) {
		FILE *status;
		char filename[128];
		char buffer[8192];
		char name[128];

		/* Must skip ".." since that is outside /proc */
		if (strcmp(next->d_name, "..") == 0)
			continue;

		/* If it isn't a number, we don't want it */
		if (!isdigit(*next->d_name))
			continue;

		sprintf(filename, "/proc/%s/status", next->d_name);
		if (! (status = fopen(filename, "r")) ) {
			continue;
		}
		if (fgets(buffer, 8192-1, status) == NULL) {
			fclose(status);
			continue;
		}
		fclose(status);

		/* Buffer should contain a string like "Name:   binary_name" */
		sscanf(buffer, "%*s %s", name);
		if(strcmp(name, pidName) == 0) {
			pidList.push_back(strtol(next->d_name, NULL, 0));
		}
	}
	closedir(dir);
	return pidList;
}
#endif

#define MOD_ADLER 65521
uint32 adlerChecksum(uint8 *data, int32 len)
{
	if(len < 0)
		return 0;

	uint32 a = 1, b = 0;
	while (len > 0)
	{
		size_t tlen = len > 5552 ? 5552 : len;
		len -= tlen;
		do
		{
			a += *data++;
			b += a;
		} while (--tlen);

		a %= MOD_ADLER;
		b %= MOD_ADLER;
	}

	return (b << 16) | a;
}
