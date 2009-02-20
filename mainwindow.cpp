#include "headers.h"
#include "mainwindow.h"
#include "fancytabwidget.h"
#include "watchmoviesview.h"
#include "recordmoviesview.h"
#include "optionsview.h"
#include "aboutdialog.h"
#include "modemanager.h"
#include "memoryinjection.h"
#include "rsa.h"
#include "consttibia.h"
#include "clientproxy.h"

MainWindow::MainWindow(QSettings *settings, QWidget *parent) :
	QMainWindow(parent),
	mSettings(settings)
{
	setWindowTitle("Tibia Eye");

	setupSearchPaths();

	mFancyTab = new FancyTabWidget(this);
	setCentralWidget(mFancyTab);

	mOptions = new OptionsView(this);
	readSettings();

	createDefaultViews();

	setMinimumWidth(780);
	setMinimumHeight(515);

	mModeManager = new ModeManager();
	mMemoryInjection = new MemoryInjection();

	mRSA = new RSA();
	mRSA->setDecryptKey(ConstTibia::OTServPrivateRSA_P,
						ConstTibia::OTServPrivateRSA_Q,
						ConstTibia::OTServPrivateRSA_D);
	mRSA->setEncryptKey(ConstTibia::CipPublicRSA);
}

MainWindow::~MainWindow()
{
	hide();

	delete mMemoryInjection;
	delete mModeManager;
	delete mRSA;
}

void MainWindow::setupSearchPaths()
{
	QDir::setSearchPaths("images", QStringList() << ":/images/buttons/" << ":/images/");
}

void MainWindow::createDefaultViews()
{
	//mFancyTab->setSplitIndex(VIEW_DOWNLOAD_MOVIES);
	mFancyTab->setSplitIndex(VIEW_RECORD_MOVIES);

	mFancyTab->insertTab(VIEW_WATCH_MOVIES, new WatchMoviesView(this), tr("Watch Movies"));
	mFancyTab->insertTab(VIEW_RECORD_MOVIES, new RecordMoviesView(this), tr("Record Movies"));
	/*
	mFancyTab->insertTab(VIEW_WATCH_STREAMS, new QWidget(this), tr("Watch Streams"));
	mFancyTab->insertTab(VIEW_STREAM_MOVIES, new QWidget(this), tr("Stream Movies"));
	mFancyTab->insertTab(VIEW_DOWNLOAD_MOVIES, new QWidget(this), tr("Download Movies"));
	mFancyTab->insertTab(VIEW_TIBIAEYE_ACCOUNT, new QWidget(this), tr("Tibia Eye Account"));
	*/
	mFancyTab->insertTab(VIEW_OPTIONS, mOptions, tr("Options"));

	QPushButton *button = mFancyTab->insertButton(VIEW_ABOUT, tr("About"));
	connect(button, SIGNAL(clicked()), this, SLOT(about()));

	/*
	mFancyTab->setLocked(VIEW_WATCH_STREAMS, true);
	mFancyTab->setLocked(VIEW_STREAM_MOVIES, true);
	mFancyTab->setLocked(VIEW_DOWNLOAD_MOVIES, true);
	mFancyTab->setLocked(VIEW_TIBIAEYE_ACCOUNT, true);
	*/
}

// settings keys
static const char * const settingsGroup = "MainWindow";
static const char * const geometryKey = "Geometry";
static const char * const maximizeKey = "Maxmized";
static const char * const themeKey = "DefaultTheme";

void MainWindow::readSettings()
{
	mSettings->beginGroup(settingsGroup);
	const QVariant geom = mSettings->value(geometryKey);
	if(geom.isValid())
		setGeometry(geom.toRect());
	if (mSettings->value(maximizeKey, false).toBool())
		setWindowState(Qt::WindowMaximized);
	const QVariant theme = mSettings->value(themeKey);
	mSettings->endGroup();

	mOptions->readSettings(mSettings);

	if(theme.isValid())
		setTheme(theme.toString());
	else
		setTheme("clean");
}

void MainWindow::writeSettings()
{
	mSettings->beginGroup(settingsGroup);
	const QString maxSettingsKey = maximizeKey;
	if(windowState() & Qt::WindowMaximized) {
		mSettings->setValue(maxSettingsKey, true);
	} else {
		mSettings->setValue(maxSettingsKey, false);
		mSettings->setValue(geometryKey, geometry());
	}
	if(!mTheme.isEmpty())
		mSettings->setValue(themeKey, mTheme);
	mSettings->endGroup();

	mOptions->writeSettings(mSettings);
}

bool MainWindow::setTheme(const QString &theme)
{
	QString themeFolder(QDir::currentPath() + "/themes/" + theme);
	QDir::setSearchPaths("images", QStringList() << themeFolder << ":/images/buttons/" << ":/images/");

	QFile file(themeFolder + "/tibiaeye.qss");
	if(file.open(QFile::ReadOnly)) {
		mTheme = theme;
		QString styleSheet = QLatin1String(file.readAll());
		file.close();

		setStyleSheet(styleSheet);
		return true;
	}

	qCritical() << qPrintable(tr("Failed opening theme \"%1\".").arg(theme));
	return false;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	bool closeIt = true;

	if(mModeManager->getClient() && mModeManager->getClient()->isConnected()) {
		QMessageBox::StandardButton res = QMessageBox::question(this,
							  tr("Closing Tibia Eye"),
							  tr("You are trying to close Tibia Eye while a Tibia client is connected, "
								 "doing this will disconnect Tibia and stop any action that it is running on it.\n"
								 "Are you sure that you want to continue?"),
							  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
		if(res == QMessageBox::No)
			closeIt = false;
	}

	if(closeIt) {
		writeSettings();
		event->accept();
	} else
		event->ignore();
}

void MainWindow::about()
{
	AboutDialog aboutDialog;
	aboutDialog.exec();
}

void MainWindow::exit()
{
	QTimer::singleShot(0, this,  SLOT(close()));
}
