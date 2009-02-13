#ifndef ABSTRACTSERVERPROXY_H
#define ABSTRACTSERVERPROXY_H

class NetworkMessage;

class AbstractServerProxy : public QObject
{
	Q_OBJECT

public:
	AbstractServerProxy(QObject *parent = 0) : QObject(parent) { }

public slots:
	virtual void disconnect() = 0;
	virtual void sendMessage(NetworkMessage&) { }
};

#endif // ABSTRACTSERVERPROXY_H
