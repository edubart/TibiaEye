#ifndef MEMORYINJECTION_H
#define MEMORYINJECTION_H

class MemoryInjection : public Singleton<MemoryInjection>
{
public:
	MemoryInjection();
	virtual ~MemoryInjection();

	bool injectTibia();
	void desinjectTibia();

	bool isProcessAlive();
	bool isPlayerOnline();
	bool setServerHost(const char *ip, uint32 port);

private:
	//tibia vars
	int mTibiaVersion;
	uint8 *mHostsBackup;

	//process handling methods
	bool findTibiaProcess();
	bool checkTibiaVersion();

	bool attachProcess();
	void detachProcess();

	bool readProcessMemory(uint32 address, uint32 size, void *buffer);
	bool writeProcessMemory(uint32 address, uint32 size, const void *buffer);

	//internal process vars
#ifdef WIN32
	DWORD mProcessId;
	HANDLE mProcess;
#else
	pid_t mProcessId;
	int mProcess;
#endif
};

#endif // MEMORYINJECTION_H
