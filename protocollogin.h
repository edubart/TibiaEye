#ifndef PROTOCOLLOGIN_H
#define PROTOCOLLOGIN_H

#include "protocol.h"
#include "networkmessage.h"

class ModeManager;
class MovieFile;

class ProtocolLogin : public Protocol
{
	Q_OBJECT

public:
	ProtocolLogin(ClientProxy *client);
	virtual ~ProtocolLogin();

private slots:
	void parseFirstClientMessage(NetworkMessage& msg);
	
	void parseRecordServerConnect();
	void parseRecordServerMessage(NetworkMessage& msg);

private:
	void disconnectWithError(const QString &message);

	ModeManager *mModeManager;

	NetworkMessage sendMsg;
};

#endif // PROTOCOLLOGIN_H
