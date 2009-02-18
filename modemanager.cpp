#include "headers.h"
#include "modemanager.h"
#include "optionsview.h"
#include "memoryinjection.h"
#include "clientproxy.h"
#include "moviefile.h"

ModeManager::ModeManager()
{
	qDebug("ModeManager::ModeManager");

	mClientProxy = NULL;
	mMovieFile = NULL;
	mMode = MODE_NONE;
}

ModeManager::~ModeManager()
{
	qDebug("ModeManager::~ModeManager");

	if(mClientProxy)
		delete mClientProxy;

	qDeleteAll(mCharList);
}

bool ModeManager::startMode(eModes mode)
{
	qDebug("ModeManager::startMode");

	if(mClientProxy && mClientProxy->isConnected()) {
		qCritical() << qPrintable(tr("A Tibia client is connected playing or recording, you should logout it first."));
		return false;
	}

	if(isModeRunning() || mode == MODE_NONE) {
		qCritical() << qPrintable(tr("Tibia Eye is currently in record or play action, you need to stop it first."));
		return false;
	}

	uint16 port = OptionsView::instance()->getProxyListenPort();

	//TODO: check if tibia is running every X time and if is closed stop the current mode
	MemoryInjection *memoryInjection = MemoryInjection::instance();
	if(!memoryInjection->injectTibia() || !memoryInjection->setServerHost("127.0.0.1", port)) {
		qCritical() << qPrintable(tr("Unable to find a supported Tibia client running."));
		return false;
	}

	if(!mClientProxy)
		mClientProxy = new ClientProxy();
	mClientProxy->setListenPort(port);
	
	if(mClientProxy->startListen()) {
		mMode = mode;
		return true;
	}

	return false;
}

void ModeManager::stopMode()
{
	qDebug("ModeManager::stopMode");

	if(!isModeRunning())
		return;

	MemoryInjection::instance()->desinjectTibia();
	mClientProxy->disconnect();
	mClientProxy->stopListen();

	mMode = MODE_NONE;
}
