#ifndef GAMEPROTOCOL_H
#define GAMEPROTOCOL_H

#include "protocol.h"
#include "networkmessage.h"

class ModeManager;
class MovieFile;

class GameProtocol : public Protocol
{
	Q_OBJECT

public:
	GameProtocol(ClientProxy *client);
	virtual ~GameProtocol();

private slots:
	void parseFirstClientMessage(NetworkMessage& msg);

	void parseRecordServerConnect();

private:
	void disconnectWithError(const QString &message);

	ModeManager *mModeManager;
	MovieFile *mMovieFile;

	NetworkMessage sendMsg;
};

#endif // GAMEPROTOCOL_H
