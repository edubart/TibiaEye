#include "headers.h"
#include "optionsview.h"
#include "ui_optionsview.h"
#include "mainwindow.h"

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

void OptionsView::readSettings(QSettings *settings)
{
	settings->beginGroup(settingsGroup);
	const QVariant moviesDir = settings->value(moviesDirKey);
	if(moviesDir.isValid() && QDir(moviesDir.toString()).exists())
		mUi->moviesDirText->setText(settings->value(moviesDirKey).toString());
	else
		mUi->moviesDirText->setText(QDir::currentPath() + "/movies");
	mUi->listenPortBox->setValue(settings->value(proxyListenPortKey, 8000).toInt());

	// read font
	const QVariant fontFamily = settings->value(fontFamilyKey);
	if(fontFamily.isValid()) {
		int fontSize = settings->value(fontSizeKey, 9).toInt();
		QApplication::setFont(QFont(fontFamily.toString(), fontSize));
	}
	mUi->fontValueLabel->setText(QString("%1, %2").arg(QApplication::font().family()).arg(QApplication::font().pointSize()));

	settings->endGroup();
}

void OptionsView::writeSettings(QSettings *settings)
{
	settings->beginGroup(settingsGroup);
	settings->setValue(moviesDirKey, mUi->moviesDirText->text());
	settings->setValue(proxyListenPortKey, mUi->listenPortBox->value());
	settings->setValue(fontFamilyKey, QApplication::font().family());
	settings->setValue(fontSizeKey, QApplication::font().pointSize());
	settings->endGroup();

	if(!mLanguage.isEmpty())
		settings->setValue(languageKey, mLanguage);
}

void OptionsView::listThemes()
{
	QDir directory = QDir("themes");
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

	QDir directory = QDir("translations");
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
	QString newDir = QFileDialog::getExistingDirectory(this, tr("Movies Directory"), QDir::currentPath());
	if(!newDir.isEmpty() && QDir(newDir).exists())
		mUi->moviesDirText->setText(newDir);
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
