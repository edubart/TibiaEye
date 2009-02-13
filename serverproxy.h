#ifndef SERVERPROXY_H
#define SERVERPROXY_H

#include "abstractserverproxy.h"

class Protocol;
class TibiaSocket;

class ServerProxy : public AbstractServerProxy
{
	Q_OBJECT

public:
	ServerProxy(Protocol *protocol);
	virtual ~ServerProxy();

public slots:
	void connectToHost(const QString &host, uint16 port);
	void disconnect();
	void sendMessage(NetworkMessage &msg);

private:
	TibiaSocket *mServerSocket;
};

#endif // SERVERPROXY_H
