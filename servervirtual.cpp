#include "headers.h"
#include "servervirtual.h"
#include "protocol.h"
#include "modemanager.h"
#include "moviefile.h"

ServerVirtual::ServerVirtual(Protocol *protocol)
	: AbstractServerProxy(protocol)
{
	qDebug("ServerVirtual::ServerVirtual");

	mModeManager = ModeManager::instance();
	mProtocol = protocol;
	mState = VSTATE_IDLE;
}

ServerVirtual::~ServerVirtual()
{
	qDebug("ServerVirtual::~ServerVirtual");
}

void ServerVirtual::start()
{
	qDebug("ServerVirtual::start");

	mModeManager->getMovieFile()->rewind();

	mState = VSTATE_START;
	QTimer::singleShot(0, this, SLOT(onTimer()));
}

void ServerVirtual::disconnect()
{
	qDebug("ServerVirtual::disconnect");

	mState = VSTATE_STOP;
}

//TODO: shot timer when switch movie
void ServerVirtual::onTimer()
{
	qDebug("ServerVirtual::onTimer");

	if(mState == VSTATE_STOP) {
		mProtocol->onServerDisconnect();
		mState = VSTATE_FINISHED;
		return;
	}

	int shotTime = -1;

	if(mModeManager->getMovieFile()->getSpeed() > 0) {
		if(mState == VSTATE_START) {
			mProtocol->onServerConnect();
			mState = VSTATE_PLAYING;
			shotTime = 0;
		} else {
			shotTime = mModeManager->getMovieFile()->playMessage(mReadMsg);
			if(shotTime >= 0) {
				mProtocol->onRecvServerMessage(mReadMsg, false);
			} else {
				mProtocol->onServerDisconnect();
				mState = VSTATE_FINISHED;
			}
		}
	} else {
		shotTime = 300;
	}

	if(shotTime >= 0)
		QTimer::singleShot(shotTime, this, SLOT(onTimer()));
}
