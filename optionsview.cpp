#include "headers.h"
#include "optionsview.h"
#include "ui_optionsview.h"
#include "mainwindow.h"
#include "rsa.h"
#include "consttibia.h"

OptionsView::OptionsView(MainWindow *mainWindow) :
	QWidget(mainWindow),
	mMainWindow(mainWindow),
	mUi(new Ui::OptionsView)
{
	mUi->setupUi(this);

	connect(mUi->setDefaultThemeButton, SIGNAL(clicked()), this, SLOT(defaultThemeButtonClicked()));
	connect(mUi->setDefaultLanguageButton, SIGNAL(clicked()), this, SLOT(defaultLanguageButtonClicked()));
	connect(mUi->changeFontButton, SIGNAL(clicked()), this, SLOT(changeFontButtonClicked()));
	connect(mUi->browseButton, SIGNAL(clicked()), this, SLOT(browseButtonClicked()));
	connect(mUi->tibiaServerRadioButton, SIGNAL(clicked()), this, SLOT(tibiaServerChanged()));
	connect(mUi->otservServerRadioButton, SIGNAL(clicked()), this, SLOT(tibiaServerChanged()));

	listThemes();
	listLanguages();
}

OptionsView::~OptionsView()
{
	delete mUi;
}

// settings keys
static const char * const settingsGroup = "Options";
static const char * const moviesDirKey = "MoviesDir";
static const char * const proxyListenPortKey = "ProxyListenPort";
static const char * const fontFamilyKey = "FontFamily";
static const char * const fontSizeKey = "FontSize";
static const char * const languageKey = "Language";
static const char * const customHostKey = "CustomHost";

void OptionsView::readSettings(QSettings *settings)
{
	settings->beginGroup(settingsGroup);
	const QVariant moviesDir = settings->value(moviesDirKey);
	if(moviesDir.isValid() && QDir(moviesDir.toString()).exists())
		mUi->moviesDirText->setText(settings->value(moviesDirKey).toString());
	else
		mUi->moviesDirText->setText(QCoreApplication::applicationDirPath() + "/movies");
	mUi->listenPortBox->setValue(settings->value(proxyListenPortKey, 8000).toInt());

	const QVariant fontFamily = settings->value(fontFamilyKey);
	if(fontFamily.isValid()) {
		int fontSize = settings->value(fontSizeKey, 9).toInt();
		QApplication::setFont(QFont(fontFamily.toString(), fontSize));
	}
	mUi->fontValueLabel->setText(QString("%1, %2").arg(QApplication::font().family()).arg(QApplication::font().pointSize()));

	mUi->customServerAddressText->setText(settings->value(customHostKey, "localhost:7171").toString());

	settings->endGroup();
}

void OptionsView::writeSettings(QSettings *settings)
{
	settings->beginGroup(settingsGroup);
	settings->setValue(moviesDirKey, mUi->moviesDirText->text());
	settings->setValue(proxyListenPortKey, mUi->listenPortBox->value());
	settings->setValue(fontFamilyKey, QApplication::font().family());
	settings->setValue(fontSizeKey, QApplication::font().pointSize());
	settings->setValue(customHostKey, mUi->customServerAddressText->text());
	settings->endGroup();

	if(!mLanguage.isEmpty())
		settings->setValue(languageKey, mLanguage);
}

void OptionsView::listThemes()
{
	QDir directory = QDir(QCoreApplication::applicationDirPath() + "/themes");
	QStringList files = directory.entryList(QDir::Dirs);

	for(int i = 0; i < files.size(); i++) {
		if(files[i] == ".." || files[i] == ".")
			continue;
		mUi->themesList->addItem(files[i]);
	}
}

void OptionsView::listLanguages()
{
	mUi->languagesList->addItem("en_US");

	QDir directory = QDir(QCoreApplication::applicationDirPath() + "/translations");
	QStringList files = directory.entryList(QStringList("*.qm"), QDir::Files);

	for(int i = 0; i < files.size(); i++)
		mUi->languagesList->addItem(files[i].replace(".qm",""));
}

void OptionsView::defaultThemeButtonClicked()
{
	if(mUi->themesList->currentItem())
		mMainWindow->setTheme(mUi->themesList->currentItem()->text());
}


void OptionsView::defaultLanguageButtonClicked()
{
	if(mUi->languagesList->currentItem()) {
		mLanguage = mUi->languagesList->currentItem()->text();
		QMessageBox::information(this, tr("Changing language"), tr("Restart is requeried to changes take effect."));
	}
}

void OptionsView::changeFontButtonClicked()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, QApplication::font());
	if(ok) {
		QApplication::setFont(font);
		mMainWindow->setTheme(mMainWindow->theme());
		mUi->fontValueLabel->setText(QString("%1, %2").arg(font.family()).arg(font.pointSize()));
	}
}

void OptionsView::browseButtonClicked()
{
	QString newDir = QFileDialog::getExistingDirectory(this, tr("Movies Directory"), QCoreApplication::applicationDirPath());
	if(!newDir.isEmpty() && QDir(newDir).exists())
		mUi->moviesDirText->setText(newDir);
}

void OptionsView::tibiaServerChanged()
{
	RSA *rsa = RSA::instance();
	if(rsa) {
		if(mUi->tibiaServerRadioButton->isChecked())
			rsa->setEncryptKey(ConstTibia::CipPublicRSA);
		else
			rsa->setEncryptKey(ConstTibia::OTServPublicRSA);
	}
}

uint16 OptionsView::getProxyListenPort() const
{
	return (uint16)mUi->listenPortBox->value();
}

uint16 OptionsView::getServerPort() const
{
	uint16 port = 7171;
	if(mUi->otservServerRadioButton->isChecked()) {
		QStringList stringList = mUi->customServerAddressText->text().split(':');
		if(stringList.size() == 2) {
			bool ok;
			port = stringList.at(1).toUShort(&ok);
			if(!ok)
				port = 7171;
		}
	}

	return port;
}

static const char cipsoftHostList[10][32] = {
	"tibia05.cipsoft.com",
	"tibia04.cipsoft.com",
	"tibia03.cipsoft.com",
	"tibia02.cipsoft.com",
	"tibia01.cipsoft.com",
	"login05.tibia.com",
	"login04.tibia.com",
	"login03.tibia.com",
	"login02.tibia.com",
	"login01.tibia.com"
};

QString OptionsView::getServerHost() const
{
	QString host = cipsoftHostList[qrand() % 10];

	if(mUi->otservServerRadioButton->isChecked()) {
		QStringList stringList = mUi->customServerAddressText->text().split(':');
		if(stringList.size() == 2)
			host = stringList.at(0);
	}

	return host;
}

QString OptionsView::getMoviesDir() const
{
	return mUi->moviesDirText->text();
}
