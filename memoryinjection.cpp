#include "headers.h"
#include "memoryinjection.h"
#include "consttibia.h"
#include "util.h"

MemoryInjection::MemoryInjection()
{
	mHostsBackup = NULL;
	mTibiaVersion = -1;
	mProcessId = 0;
	mProcess = 0;
}

MemoryInjection::~MemoryInjection()
{
	//desinjects any injected tibia
	desinjectTibia();
}

bool MemoryInjection::injectTibia()
{
	//finds the tibia window
	if(!findTibiaProcess())
		return false;

	//attach process id
	if(!attachProcess())
		return false;

	// checks tibia version
	//TODO: fix tibia version checks
	mTibiaVersion = 0;

	bool ret = true;
#ifdef WIN32
	char thisVersion[13];
	for(uint8 i = 0; i < ConstTibia::VersionsNum; i++) {
		if(!readProcessMemory(ConstTibia::VersionAddress[i], 13, &thisVersion)) {
			ret = false;
			break;
		}

		if(strncmp(ConstTibia::AvailableVersions[i], thisVersion, 13) == 0) {
			mTibiaVersion = i;
			ret = false;
			break;
		}
	}
#endif

	//detachs the process
	detachProcess();
	return ret;
}

void MemoryInjection::desinjectTibia()
{
	//first check if there is an injected tibia
	if(mTibiaVersion == -1 || mProcessId == 0)
		return;

	//restore backuped ips, ports and rsa
	if(attachProcess()) {
		if(mHostsBackup)
			writeProcessMemory(ConstTibia::HostAddress[mTibiaVersion][0],
							   ConstTibia::HostAddress[mTibiaVersion][2]*ConstTibia::HostAddress[mTibiaVersion][3],
							   mHostsBackup);

		writeProcessMemory(ConstTibia::RsaAddress[mTibiaVersion], strlen(ConstTibia::CipPublicRSA) + 1, (void*)ConstTibia::CipPublicRSA);
		detachProcess();
	}

	//delete backups
	if(mHostsBackup) {
		delete [] mHostsBackup;
		mHostsBackup = NULL;
	}

	//reset vars
	mTibiaVersion = - 1;
	mProcessId = 0;
}

bool MemoryInjection::findTibiaProcess()
{
#ifdef WIN32
	//find first tibia window
	HWND tibiaWnd = FindWindowA((LPCSTR)"TibiaClient",NULL);
	if(!tibiaWnd)
		return false;

	//get pid
	GetWindowThreadProcessId(tibiaWnd, &mProcessId);
#else
	//list pids named Tibia
	PidList pidList = findPidsByName("Tibia");
	if(pidList.size() == 0 || pidList.begin() == pidList.end())
		return false;

	//get pid
	mProcessId = (*pidList.begin());
#endif
	return true;
}

bool MemoryInjection::attachProcess()
{
#ifdef WIN32
	//open process handle
	mProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, mProcessId);
	if(!mProcess)
		return false;
#else
	//activate ptrace
	if(ptrace(PTRACE_ATTACH, mProcessId, NULL, NULL))
		return false;
	//wait the attachment
	wait(NULL);
	mProcess = 1;
#endif
	return true;
}

void MemoryInjection::detachProcess()
{
#ifdef WIN32
	if(!mProcess)
		return;
	//closes process handle
	CloseHandle(mProcess);
	mProcess = NULL;
#else
	if(!mProcess)
		return;
	//desactivate ptrace
	ptrace(PTRACE_DETACH, mProcessId, NULL, NULL);
#endif
}

bool MemoryInjection::isProcessAlive()
{
	//attach and detach to check if process is alive
	if(attachProcess()) {
		detachProcess();
		return true;
	}
	return false;
}

bool MemoryInjection::readProcessMemory(uint32 address, uint32 size, void *buffer)
{
	if(!mProcess)
		return false;
#ifdef WIN32
	if((bool)!ReadProcessMemory(mProcess, (LPVOID)address, buffer, size, NULL))
		return false;
#else
	//read memory every 4 bytes and puts in the buffer
	long val;
	const static int long_size = sizeof(long);
	for(unsigned int i=0;i < size/long_size + (size % long_size > 0) ? 1 : 0; i++) {
		val = ptrace(PTRACE_PEEKDATA, mProcessId, (void *)(address+i*long_size), NULL);
		memcpy((void*)(((char*)buffer)+i*long_size),&val, qMin(long_size, (int)(size-i*4)));
	}
#endif
	return true;
}

bool MemoryInjection::writeProcessMemory(uint32 address, uint32 size, const void *buffer)
{
	if(!mProcess)
		return false;
#ifdef WIN32
	if((bool)!WriteProcessMemory(mProcess, (LPVOID)address, buffer, size, NULL))
		return false;
#else
	//write the memory every 4 bytes from buffer
	//this may overwrite additional bytes if the size is not divisiable by 4, we should fix this issue if any problem occurs
	long val;
	const static int long_size = sizeof(long);
	for(unsigned int i=0;i < size/long_size + (size % long_size > 0) ? 1 : 0; i++) {
		memcpy(&val,(void*)(((char*)buffer)+i*long_size), qMin(long_size, (int)(size-i*4)));
		ptrace(PTRACE_POKEDATA, mProcessId, (void *)(address+i*long_size), val);
	}
#endif
	return true;
}

bool MemoryInjection::isPlayerOnline()
{
	//first check tibia version and process
	if(mTibiaVersion == -1 || mProcessId == 0)
		return false;

	//attach process
	if(!attachProcess())
		return false;

	//check the connection status
	uint32 connectionStatus;
	bool ret = false;
	if(readProcessMemory(ConstTibia::ConnectionAddress[mTibiaVersion], 4, &connectionStatus)) {
		if(connectionStatus == 8)
			ret = true;
	}

	//detach and return
	detachProcess();
	return ret;
}

bool MemoryInjection::setServerHost(const char *ip, uint32 port)
{
	//first check tibia version and process
	if(mTibiaVersion == -1 || mProcessId == 0)
		return false;

	//attach process
	if(!attachProcess())
		return false;

	// backup all hosts.
	if(!mHostsBackup) {
		mHostsBackup = new uint8[ConstTibia::HostAddress[mTibiaVersion][2]*ConstTibia::HostAddress[mTibiaVersion][3]];
		readProcessMemory(ConstTibia::HostAddress[mTibiaVersion][0],
						  ConstTibia::HostAddress[mTibiaVersion][2]*ConstTibia::HostAddress[mTibiaVersion][3],
						  mHostsBackup);
	}

	//overwrite all tibia ips and ports
	bool ret = true;
	for(uint8 x = 0; x < ConstTibia::HostAddress[mTibiaVersion][3]; x++)
	{
		if(!writeProcessMemory(ConstTibia::HostAddress[mTibiaVersion][0] + ConstTibia::HostAddress[mTibiaVersion][2] * x, strlen(ip) + 1, ip) ||
		   !writeProcessMemory(ConstTibia::HostAddress[mTibiaVersion][1] + ConstTibia::HostAddress[mTibiaVersion][2] * x, 4, &port))
			ret = false;
	}

	if(ret) {
#ifdef WIN32
		//desprotect memory in windows systems
		DWORD oldProtection;
				VirtualProtectEx(mProcess, (LPVOID)ConstTibia::RsaAddress[mTibiaVersion], strlen(ConstTibia::OTServPublicRSA) + 1, PAGE_EXECUTE_READWRITE, &oldProtection);
#endif
		//rewrite RSA key
		ret = writeProcessMemory(ConstTibia::RsaAddress[mTibiaVersion], strlen(ConstTibia::OTServPublicRSA) + 1, (void*)ConstTibia::OTServPublicRSA);
	}

	//detach and return
	detachProcess();
	return ret;
}
