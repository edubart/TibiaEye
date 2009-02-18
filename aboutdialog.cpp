#include "headers.h"
#include "aboutdialog.h"
#include "constants.h"

using namespace Constants;

AboutDialog::AboutDialog(QWidget *parent) :
	QDialog(parent)
{
	QLabel *imageLabel = new QLabel;
	imageLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	imageLabel->setScaledContents(true);
	imageLabel->setPixmap(QPixmap(":/images/tibiaeye_logo_128x128.png"));

	QVBoxLayout *imageLayout = new QVBoxLayout;
	imageLayout->addWidget(imageLabel);
	imageLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	QLabel *appLabel = new QLabel;
	appLabel->setText(QString("%1 %2").arg(APP_NAME, APP_VERSION_LONG));
	appLabel->setStyleSheet("font-weight: bold; font-size: 14px");

	QLabel *aboutLabel = new QLabel;
	aboutLabel->setText(tr("Built on %1\n"
						   "Author: %2\n"
						   "Contact: %3\n"
						   "Website: %4\n"
						   "\n"
						   "Copyright %5 %6. All rights reserved.\n"
						   "\n"
						   "The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING\n"
						   "THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A\n"
						   "PARTICULAR PURPOSE.")
							.arg(APP_BUILD_LONG, APP_AUTHOR,
								 APP_CONTACT, APP_WEBSITE, APP_YEAR, APP_COMPANY));

	QVBoxLayout *aboutLabelsLayout = new QVBoxLayout;
	aboutLabelsLayout->addWidget(appLabel);
	aboutLabelsLayout->addWidget(aboutLabel);

	QHBoxLayout *aboutLayout = new QHBoxLayout;
	aboutLayout->addLayout(imageLayout);
	aboutLayout->addLayout(aboutLabelsLayout);

	QPushButton *checkUpdatesButton = new QPushButton(tr("Check Updates"));
	checkUpdatesButton->setEnabled(false);

	QPushButton *websiteButton = new QPushButton(tr("Go to Website"));
	checkUpdatesButton->setEnabled(false);

	QPushButton *closeButton = new QPushButton(tr("Close"));
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

	QHBoxLayout *buttonsLayout = new QHBoxLayout;
	buttonsLayout->addWidget(checkUpdatesButton);
	buttonsLayout->addWidget(websiteButton);
	buttonsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
	buttonsLayout->addWidget(closeButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(aboutLayout);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);

	closeButton->setFocus();
	setWindowTitle(tr("About Tibia Eye"));
}
