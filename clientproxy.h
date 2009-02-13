#ifndef CLIENTPROXY_H
#define CLIENTPROXY_H

#include "networkmessage.h"

class TibiaSocket;

class ClientProxy : public QTcpServer
{
	Q_OBJECT

public:
	ClientProxy();
	virtual ~ClientProxy();

	bool startListen();
	void stopListen();

	void setListenPort(uint16 port) { mListenPort = port; }
	uint16 listenPort() const { return mListenPort; }

public slots:
	void disconnect();
	void sendMessage(NetworkMessage &msg);

protected:
	void incomingConnection(int socketDescriptor);

private slots:
	void onClientMessage(NetworkMessage &msg);
	void onClientDisconnect();

private:
	TibiaSocket *mClientSocket;

	uint16 mListenPort;
};

#endif // CLIENTPROXY_H
