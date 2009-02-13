#ifndef TIBIASOCKET_H
#define TIBIASOCKET_H

#include "networkmessage.h"

class TibiaSocket : public QTcpSocket
{
	Q_OBJECT

public:
	TibiaSocket(QObject *parent = 0);
	virtual ~TibiaSocket();

signals:
	void messageRecived(NetworkMessage &msg);

public slots:
	void sendMessage(NetworkMessage &msg);
	void disconnectLater();
	void disconnect();

private slots:
	void handleError(QAbstractSocket::SocketError error);
	void internalRead();

private:
	static NetworkMessage mMsg;
};

#endif // TIBIASOCKET_H
