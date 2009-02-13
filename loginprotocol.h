#ifndef LOGINPROTOCOL_H
#define LOGINPROTOCOL_H

#include "protocol.h"
#include "networkmessage.h"

class ModeManager;
class MovieFile;

class LoginProtocol : public Protocol
{
	Q_OBJECT

public:
	LoginProtocol(ClientProxy *client);
	virtual ~LoginProtocol();

private slots:
	void parseFirstClientMessage(NetworkMessage& msg);
	
	void parseRecordServerConnect();
	void parseRecordServerMessage(NetworkMessage& msg);

private:
	void disconnectWithError(const QString &message);

	ModeManager *mModeManager;
	MovieFile *mMovieFile;

	NetworkMessage sendMsg;
};

#endif // LOGINPROTOCOL_H
