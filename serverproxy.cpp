#include "headers.h"
#include "serverproxy.h"
#include "protocol.h"
#include "tibiasocket.h"

ServerProxy::ServerProxy(Protocol *protocol) :
	AbstractServerProxy(protocol)
{
	qDebug("ServerProxy::ServerProxy");

	mServerSocket = new TibiaSocket(this);
	connect(mServerSocket, SIGNAL(connected()), protocol, SLOT(onServerConnect()));
	connect(mServerSocket, SIGNAL(messageRecived(NetworkMessage&)), protocol, SLOT(onRecvServerMessage(NetworkMessage&)));
	connect(mServerSocket, SIGNAL(disconnected()), protocol, SLOT(onServerDisconnect()));
}

ServerProxy::~ServerProxy()
{
	qDebug("ServerProxy::~ServerProxy");
}

void ServerProxy::connectToHost(const QString &host, uint16 port)
{
	qDebug("ServerProxy::connectToHost");

	mServerSocket->connectToHost(host, port);
}

void ServerProxy::disconnect()
{
	qDebug("ServerProxy::disconnect");

	mServerSocket->disconnectLater();
}

void ServerProxy::sendMessage(NetworkMessage &msg)
{
	qDebug("ServerProxy::sendMessage");

	mServerSocket->sendMessage(msg);
}
