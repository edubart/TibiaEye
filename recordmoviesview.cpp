#include "headers.h"
#include "recordmoviesview.h"
#include "ui_recordmoviesview.h"
#include "modemanager.h"
#include "moviefile.h"
#include "optionsview.h"
#include "clientproxy.h"

RecordMoviesView::RecordMoviesView(QWidget *parent) :
	QWidget(parent),
	mUi(new Ui::RecordMoviesView)
{
	mUi->setupUi(this);

	mMovieFile = NULL;

	connect(mUi->browseButton, SIGNAL(clicked()), this, SLOT(browseMovie()));
	connect(mUi->startRecordingButton, SIGNAL(clicked()), this, SLOT(startRecord()));
	connect(mUi->stopRecordingButton, SIGNAL(clicked()), this, SLOT(stopRecord()));
	connect(mUi->fileText, SIGNAL(textChanged(QString)), this, SLOT(movieFileChanged(QString)));
}

RecordMoviesView::~RecordMoviesView()
{
	if(mMovieFile) {
		mMovieFile->saveMovie();
		delete mMovieFile;
	}

	delete mUi;
}

void RecordMoviesView::startRecord()
{
	if(mMovieFile) {
		qCritical() << qPrintable(tr("You are already recoding or your record finished, you should logout from tibia before recording again."));
		return;
	}

	if(mUi->fileText->text().isEmpty()) {
		qCritical() << qPrintable(tr("Please, enter the movie file path."));
		return;
	}

	if(mUi->titleText->text().isEmpty()) {
		qCritical() << qPrintable(tr("Please, enter the movie title."));
		return;
	}

	if(mUi->typeComboBox->currentIndex() == 0) {
		qCritical() << qPrintable(tr("Please, enter the movie type."));
		return;
	}

	ModeManager *modeManager = ModeManager::instance();
	if(modeManager->startMode(MODE_RECORD)) {
		mMovieFile = new MovieFile(mUi->fileText->text());

		if(QFile(mUi->fileText->text()).exists()) {
			QMessageBox messageBox(this);

			messageBox.setText(tr("You selected a file movie that already exists, "
						"you are able to overwrite it or continue a multisession recording.\n"
						"What do you want to do?"));

			QAbstractButton *multisessionButton = messageBox.addButton(tr("Continue a Multisession Record"), QMessageBox::ActionRole);
			QAbstractButton *overwriteButton = messageBox.addButton(tr("Overwrite"), QMessageBox::ActionRole);
			messageBox.addButton(tr("Cancel"), QMessageBox::ActionRole);

			messageBox.exec();

			if(messageBox.clickedButton() == multisessionButton) {
				mMovieFile->loadMovie();
			} else if(messageBox.clickedButton() == overwriteButton) {

			} else {
				delete mMovieFile;
				mMovieFile = NULL;
				modeManager->stopMode();
				return;
			}
		}

		mMovieFile->setMovieTitle(mUi->titleText->text());
		mMovieFile->setMovieDescription(mUi->descriptionText->toPlainText());
		mMovieFile->setMovieType(mUi->typeComboBox->currentIndex());

		modeManager->setMovieFile(mMovieFile);

		mUi->startRecordingButton->setEnabled(false);
		mUi->stopRecordingButton->setEnabled(true);

		connect(mMovieFile, SIGNAL(movieStarted()), this, SLOT(onRecordStart()));
		connect(mMovieFile, SIGNAL(movieSaved()), this, SLOT(onRecordSave()));
		connect(mMovieFile, SIGNAL(movieTime(uint32)), this, SLOT(onRecordTime(uint32)));
		connect(modeManager->getClient(), SIGNAL(clientDisconnected()), this, SLOT(onClientDisconnect()));

		mUi->timeLabelValue->setText("00:00:00");

		mUi->statusLabelValue->setText(tr("Waiting player login.."));
		mUi->statusLabelValue->setStyleSheet("color: #808000; font-weight: bold;");
	}
}

void RecordMoviesView::stopRecord()
{
	if(ModeManager::instance()->getClient()->isConnected()) {
		mMovieFile->lockRecord(true);
		mMovieFile->saveMovie();

		mUi->statusLabelValue->setText(tr("Waiting client disconnect..."));
		mUi->statusLabelValue->setStyleSheet("color: green; font-weight: bold;");
	} else
		stopRecordMode();

	mUi->startRecordingButton->setEnabled(true);
	mUi->stopRecordingButton->setEnabled(false);
}

void RecordMoviesView::browseMovie()
{
	mUi->fileText->setText(QFileDialog::getSaveFileName(this, tr("Save Movie"),
						   OptionsView::instance()->getMoviesDir(),
						   tr("Tibia Eye Movies (*.tem)"),
						   0,
						   QFileDialog::DontConfirmOverwrite));

	if(QFile(mUi->fileText->text()).exists()) {
		MovieFile movieFile(mUi->fileText->text());
		if(movieFile.loadMovie(false)) {
			mUi->typeComboBox->setCurrentIndex(movieFile.getMovieType());
			mUi->titleText->setText(movieFile.getMovieTitle());
			mUi->descriptionText->setText(movieFile.getMovieDescription());
		}
	}
}


void RecordMoviesView::movieFileChanged(const QString &text)
{
	if(!text.isEmpty() && !text.endsWith(".tem"))
		mUi->fileText->setText(text + ".tem");
}

void RecordMoviesView::onRecordStart()
{
	mUi->statusLabelValue->setText(tr("Recording.."));
	mUi->statusLabelValue->setStyleSheet("color: blue; font-weight: bold;");
}

//TODO: display current time even when there no incomming packets too
void RecordMoviesView::onRecordTime(uint32 mstime)
{
	mUi->timeLabelValue->setText(QTime().addMSecs(mstime).toString("hh:mm:ss"));
}

void RecordMoviesView::onRecordSave()
{
	mUi->statusLabelValue->setText(tr("Record saved."));
	mUi->statusLabelValue->setStyleSheet("color: green; font-weight: bold;");
}


void RecordMoviesView::onClientDisconnect()
{
	if(mMovieFile && mMovieFile->isRecordLocked())
		stopRecordMode();
}

void RecordMoviesView::stopRecordMode()
{
	ModeManager *modeManager = ModeManager::instance();
	modeManager->stopMode();
	disconnect(modeManager->getClient(), 0, this, 0);

	modeManager->setMovieFile(NULL);
	delete mMovieFile;
	mMovieFile = NULL;

	mUi->statusLabelValue->setText(tr("Idle"));
	mUi->statusLabelValue->setStyleSheet("color: red; font-weight: bold;");
}
