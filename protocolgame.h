#ifndef PROTOCOLGAME_H
#define PROTOCOLGAME_H

#include "protocol.h"
#include "networkmessage.h"

class ModeManager;
class MovieFile;

class ProtocolGame : public Protocol
{
	Q_OBJECT

public:
	ProtocolGame(ClientProxy *client);
	virtual ~ProtocolGame();

private slots:
	// play parses
	void parsePlayFirstClientMessage(NetworkMessage& msg);

	void parsePlayServerConnect();
	void parsePlayServerDisconnect();

	void parsePlayServerMessage(NetworkMessage&);
	void parsePlayClientMessage(NetworkMessage& msg);

	void parsePlayClientLogout(NetworkMessage&);
	void parsePlayClientWalk(NetworkMessage& msg);
	//void parsePlayClientSay(NetworkMessage& msg);

	// record parses
	void parseRecordFirstClientMessage(NetworkMessage& msg);
	void parseRecordServerConnect();
	void parseRecordServerDisconnect();

	void parseRecordServerMessage(NetworkMessage& msg);
	void parseRecordClientMessage(NetworkMessage& msg);

	void parseRecordClientPrivateMessage(NetworkMessage& msg);

	void parseRecordPlayerStats(NetworkMessage& msg);

private:
	void disconnectWithError(const QString &message);

	ModeManager *mModeManager;
	NetworkMessage mMsg;
};

#endif // PROTOCOLGAME_H
