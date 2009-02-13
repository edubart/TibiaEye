#include "headers.h"
#include "watchmoviesview.h"
#include "ui_watchmoviesview.h"
#include "moviefile.h"
#include "modemanager.h"
#include "optionsview.h"

WatchMoviesView::WatchMoviesView(QWidget *parent) :
	QWidget(parent),
	mUi(new Ui::WatchMoviesView)
{
	mUi->setupUi(this);

	mUi->moviesTable->verticalHeader()->hide();
	mUi->moviesTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

	mMovieFile = NULL;

	connect(mUi->speedSlider, SIGNAL(valueChanged(int)), this, SLOT(speedChanged(int)));
	connect(mUi->refreshListButton, SIGNAL(clicked()), this, SLOT(updateMoviesList()));
	connect(mUi->refreshListButton, SIGNAL(clicked()), mUi->filterText, SLOT(clear()));
	connect(mUi->activateButton, SIGNAL(clicked()), this, SLOT(activatePlay()));
	connect(mUi->stopButton, SIGNAL(clicked()), this, SLOT(stopPlay()));
	connect(mUi->moviesTable, SIGNAL(itemSelectionChanged()), this, SLOT(movieChanged()));
	connect(mUi->filterText, SIGNAL(textChanged(QString)), this, SLOT(filterMovies(QString)));

	updateMoviesList();
}

WatchMoviesView::~WatchMoviesView()
{
	if(mMovieFile)
		delete mMovieFile;

	delete mUi;
}

void WatchMoviesView::updateMoviesList()
{
	mUi->moviesTable->clearContents();
	while(mUi->moviesTable->rowCount() != 0)
		mUi->moviesTable->removeRow(0);

	QDir directory = QDir(OptionsView::instance()->getMoviesDir());
	QStringList files = directory.entryList(QStringList("*.tem"), QDir::Files);

	MovieFile movieFile;

	for(int i = 0; i < files.size(); i++) {
		if(movieFile.loadMovie(OptionsView::instance()->getMoviesDir() + "/" + files[i], false)) {
			int row = mUi->moviesTable->rowCount();

			mUi->moviesTable->insertRow(row);
			mUi->moviesTable->setRowHeight(row, 16);

			mUi->moviesTable->setItem(row, 8, new QTableWidgetItem(files[i]));
			mUi->moviesTable->setItem(row, 7, new QTableWidgetItem(QString::number(movieFile.getTibiaVersion())));
			mUi->moviesTable->setItem(row, 6, new QTableWidgetItem(QTime().addMSecs(movieFile.getMovieDuration()).toString("hh:mm:ss")));
			mUi->moviesTable->setItem(row, 5, new QTableWidgetItem(QDateTime::fromTime_t(movieFile.getMovieDate()).toString()));
			mUi->moviesTable->setItem(row, 4, new QTableWidgetItem(QString::number(movieFile.getPlayerLevel())));
			mUi->moviesTable->setItem(row, 3, new QTableWidgetItem(movieFile.getPlayerWorld()));
			mUi->moviesTable->setItem(row, 2, new QTableWidgetItem(movieFile.getPlayerName()));
			mUi->moviesTable->setItem(row, 1, new QTableWidgetItem(movieFile.getMovieTypeName()));
			mUi->moviesTable->setItem(row, 0, new QTableWidgetItem(movieFile.getMovieTitle()));
		}
	}

	mUi->moviesTable->sortByColumn(0, Qt::AscendingOrder);
	mUi->moviesTable->resizeColumnsToContents();

	//if(mUi->moviesTable->rowCount() > 0)
	//	mUi->moviesTable->selectRow(0);
}

void WatchMoviesView::activatePlay()
{
	if(mUi->moviesTable->selectedItems().size() > 0 && mUi->moviesTable->item(mUi->moviesTable->currentRow(), 8)) {
		mMovieFile = new MovieFile(OptionsView::instance()->getMoviesDir() + "/" + mUi->moviesTable->item(mUi->moviesTable->currentRow(), 8)->text());
		if(mMovieFile->loadMovie(true)) {
			ModeManager *modeManager = ModeManager::instance();
			if(modeManager->startMode(MODE_PLAY)) {
				modeManager->setMovieFile(mMovieFile);
				mUi->stopButton->setEnabled(true);
				mUi->activateButton->setEnabled(false);
				mUi->movieFileLabelValue->setText(mUi->moviesTable->item(mUi->moviesTable->currentRow(), 8)->text());
				mUi->speedLabelValue->setText("1.00x");
				mUi->speedSlider->setValue(0);
				
				mUi->statusLabelValue->setText(tr("Waiting tibia connection..."));
				mUi->statusLabelValue->setStyleSheet("color: yellow; font-weight: bold");

				connect(mMovieFile, SIGNAL(movieStarted()), this, SLOT(onPlayStart()));
				connect(mMovieFile, SIGNAL(movieSaved()), this, SLOT(onPlayFinish()));
				connect(mMovieFile, SIGNAL(movieTime(uint32)), this, SLOT(onPlayTime(uint32)));
			}
		}
	} else
		qCritical() << qPrintable(tr("You must select a movie to play."));
}

//TODO: stopping movies before finish and playing again bugs tibia
void WatchMoviesView::stopPlay()
{
	onPlayFinish();
	mUi->movieFileLabelValue->setText(tr("none"));

	mUi->statusLabelValue->setText(tr("Idle"));
	mUi->statusLabelValue->setStyleSheet("color: red; font-weight: bold");

	ModeManager::instance()->stopMode();

	delete mMovieFile;
	mMovieFile = NULL;
}

void WatchMoviesView::speedChanged(int value)
{
	if(mMovieFile) {
		float newSpeed;

		if(value >= 1)
			newSpeed = 1.0f+(float)value;
		else
			newSpeed = (20.0f+value)/20.0f;

		mUi->speedLabelValue->setText(QString().sprintf("%.02fx",newSpeed));
		mMovieFile->setSpeed(newSpeed);
	}
}

void WatchMoviesView::movieChanged()
{
	//TODO: test movie switching without stopping
	QTableWidgetItem *item = mUi->moviesTable->item(mUi->moviesTable->currentRow(), 8);
	if(item) {
		MovieFile movieFile(OptionsView::instance()->getMoviesDir() + "/" + item->text());
		if(movieFile.loadMovie(false)) {
			mUi->titleLabelValue->setText(movieFile.getMovieTitle());
			mUi->typeLabelValue->setText(movieFile.getMovieTypeName());
			mUi->playerLabelValue->setText(movieFile.getPlayerName() + " (Level " + QString::number(movieFile.getPlayerLevel()) + ")");
			mUi->worldLabelValue->setText(movieFile.getPlayerWorld());
			mUi->dateLabelValue->setText(QDateTime::fromTime_t(movieFile.getMovieDate()).toString());
			mUi->durationLabelValue->setText(QTime().addMSecs(movieFile.getMovieDuration()).toString("hh:mm:ss"));
			mUi->versionLabelValue->setText(QString::number(movieFile.getTibiaVersion()));
			mUi->descriptionText->setText(movieFile.getMovieDescription());
		}
	}
}

void WatchMoviesView::filterMovies(const QString &filter)
{
	for(int x = 0; x < mUi->moviesTable->rowCount(); x++) {
		bool show = false;
		for(int y = 0; y < mUi->moviesTable->columnCount(); y++) {
			if(filter.isEmpty() || mUi->moviesTable->item(x, y)->text().contains(filter, Qt::CaseInsensitive)) {
				show = true;
				break;
			}
		}
		if(show)
			mUi->moviesTable->showRow(x);
		else
			mUi->moviesTable->hideRow(x);
	}
}

void WatchMoviesView::onPlayStart()
{
	mUi->speedSlider->setEnabled(true);

	mUi->statusLabelValue->setText(tr("Playing..."));
	mUi->statusLabelValue->setStyleSheet("color: blue; font-weight: bold");
}

void WatchMoviesView::onPlayFinish()
{
	mUi->speedSlider->setEnabled(false);
	mUi->stopButton->setEnabled(false);
	mUi->activateButton->setEnabled(true);

	mUi->statusLabelValue->setText(tr("Play finished."));
	mUi->statusLabelValue->setStyleSheet("color: green; font-weight: bold");
}

void WatchMoviesView::onPlayTime(uint32 mstime)
{
	mUi->timeLabelValue->setText(QTime().addMSecs(mstime).toString("hh:mm:ss"));
	mUi->playProgressBar->setValue(mMovieFile->getPlayedPercent());
}

