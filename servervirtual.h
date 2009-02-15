#ifndef SERVERVIRTUAL_H
#define SERVERVIRTUAL_H

#include "abstractserverproxy.h"
#include "networkmessage.h"

class ModeManager;
class Protocol;
struct Packet;

class ServerVirtual : public AbstractServerProxy
{
	Q_OBJECT

	enum eVirtualStates {
		VSTATE_IDLE,
		VSTATE_START,
		VSTATE_PLAYING,
		VSTATE_STOP,
		VSTATE_FINISHED
	};

public:
	ServerVirtual(Protocol *protocol);
	virtual ~ServerVirtual();

	void start();
	void disconnect();

private slots:
	void onTimer();

private:
	ModeManager *mModeManager;
	Protocol *mProtocol;

	eVirtualStates mState;
	NetworkMessage mReadMsg;
};

#endif // SERVERVIRTUAL_H
