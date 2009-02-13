#ifndef UTIL_H
#define UTIL_H

enum ePlatformType {
	WS_WIN,
	WS_X11,
	WS_MAC,
	WS_QWS,
	WS_UNK = 0xFF
};

ePlatformType getPlataformType();

#ifndef WIN32
typedef QVector<pid_t> PidList;
PidList findPidsByName(const char* pidName);
#endif

uint32 adlerChecksum(uint8 *data, int32 len);

#endif // UTIL_H
