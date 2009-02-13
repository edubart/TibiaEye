#ifndef SERVERVIRTUAL_H
#define SERVERVIRTUAL_H

#include "abstractserverproxy.h"
#include "networkmessage.h"

class Protocol;
class MovieFile;
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
	ServerVirtual(Protocol *protocol, MovieFile *movieFile);
	virtual ~ServerVirtual();

	void start();
	void disconnect();

private slots:
	void onTimer();

private:
	void onServerConnect();
	void onServerRecive();
	void onServerDisconnect();

	Packet *mCurrentPacket;
	Packet *mNextPacket;

	MovieFile *mMovieFile;
	Protocol *mProtocol;

	eVirtualStates mState;
	NetworkMessage mReadMsg;
};

#endif // SERVERVIRTUAL_H
