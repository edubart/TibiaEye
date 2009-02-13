#include "headers.h"
#include "recordmoviesview.h"
#include "ui_recordmoviesview.h"
#include "modemanager.h"
#include "moviefile.h"
#include "optionsview.h"

RecordMoviesView::RecordMoviesView(QWidget *parent) :
	QWidget(parent),
	mUi(new Ui::RecordMoviesView)
{
	mUi->setupUi(this);

	mMovieFile = NULL;

	connect(mUi->browseButton, SIGNAL(clicked()), this, SLOT(browseMovie()));
	connect(mUi->startRecordingButton, SIGNAL(clicked()), this, SLOT(startRecord()));
	connect(mUi->stopRecordingButton, SIGNAL(clicked()), this, SLOT(stopRecord()));
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
	if(mMovieFile)
		return;

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

	if(ModeManager::instance()->startMode(MODE_RECORD)) {
		mMovieFile = new MovieFile(mUi->fileText->text());
		mMovieFile->setMovieTitle(mUi->titleText->text());
		mMovieFile->setMovieDescription(mUi->descriptionText->toHtml());
		mMovieFile->setMovieType(mUi->typeComboBox->currentIndex());

		ModeManager::instance()->setMovieFile(mMovieFile);

		mUi->startRecordingButton->setEnabled(false);
		mUi->stopRecordingButton->setEnabled(true);

		connect(mMovieFile, SIGNAL(movieStarted()), this, SLOT(onRecordStart()));
		connect(mMovieFile, SIGNAL(movieSaved()), this, SLOT(onRecordSave()));
		connect(mMovieFile, SIGNAL(movieTime(uint32)), this, SLOT(onRecordTime(uint32)));

		mUi->statusLabelValue->setText(tr("Waiting player login.."));
		mUi->statusLabelValue->setStyleSheet("color: #808000; font-weight: bold;");

		mUi->timeLabelValue->setText("00:00:00");
	}
}

//TODO: recheck how stop and memory will work
//TODO: not disconnect client on stop record
void RecordMoviesView::stopRecord()
{
	mMovieFile->lockRecord(true);
	mMovieFile->saveMovie();

	ModeManager::instance()->stopMode();

	delete mMovieFile;
	mMovieFile = NULL;

	mUi->startRecordingButton->setEnabled(true);
	mUi->stopRecordingButton->setEnabled(false);
}

void RecordMoviesView::onRecordStart()
{
	mUi->statusLabelValue->setText(tr("Recording.."));
	mUi->statusLabelValue->setStyleSheet("color: blue; font-weight: bold;");
}

void RecordMoviesView::onRecordTime(uint32 mstime)
{
	mUi->timeLabelValue->setText(QTime().addMSecs(mstime).toString("hh:mm:ss"));
}

//TODO: fix bug on multiple record connections, crashed, why?
void RecordMoviesView::onRecordSave()
{
	mUi->statusLabelValue->setText(tr("Recorded and saved."));
	mUi->statusLabelValue->setStyleSheet("color: green; font-weight: bold;");
/*
	ModeManager::instance()->stopMode();

	delete mMovieFile;
	mMovieFile = NULL;

	mUi->startRecordingButton->setEnabled(true);
	mUi->stopRecordingButton->setEnabled(false);
	*/
}

void RecordMoviesView::browseMovie()
{
	mUi->fileText->setText(QFileDialog::getSaveFileName(this, tr("Save Movie"), OptionsView::instance()->getMoviesDir(), tr("Tibia Eye Movies (*.tem)")));
}
