#include "MainWindow.h"

#include "pages/Page.h"
#include "pages/Pages.h"
#include "pmate_core.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QScrollArea>
#include <QSplitter>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QSysInfo>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	setWindowTitle(QStringLiteral("ProtMate"));
	resize(1040, 760);

	nav = new QListWidget;
	nav->setMinimumWidth(240);
	nav->setMaximumWidth(340);
	pages = new QStackedWidget;

	/* Same grouping as the terminal version's main menu */
	addSection(tr("Calculators"));
	addPage(tr("Weight to Volume Ratio"), createWeightVolumePage());
	addPage(tr("Protein Concentration (A280)"), createProteinConcPage());
	addPage(tr("Multiple A280 Readings"), createMultiReadingPage());
	addPage(tr("Extinction Coefficient"), createExtinctionPage());
	addPage(tr("Sequence Analysis"), createSequenceAnalysisPage());
	addPage(tr("Nucleic Acid Concentration (A260)"), createNucleicPage());
	addPage(tr("Primer Melting Point"), createMeltPage());
	addPage(tr("Mass/Volume Percent to Molar Conc."), createPercentMolarPage());
	addPage(tr("Body Mass Index"), createBmiPage());
	addPage(tr("Factorial"), createFactorialPage());
	addSection(tr("File Utilities"));
	addPage(tr("Rename Oscillation Files"), createRenamePage());
	addPage(tr("AutoDock Vina Configuration"), createVinaPage());
	addPage(tr("PDBQT to PDB"), createPdbqtPage());
	addPage(tr("Solvate a Protein (X-PLOR)"), createXplorPage());

	connect(nav, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* item) {
		if (item && item->data(Qt::UserRole).isValid()) pages->setCurrentIndex(item->data(Qt::UserRole).toInt());
	});
	nav->setCurrentRow(1);

	QSplitter* split = new QSplitter;
	split->addWidget(nav);
	split->addWidget(pages);
	split->setStretchFactor(1, 1);
	split->setChildrenCollapsible(false);
	setCentralWidget(split);

	createMenus();
}

void MainWindow::addSection(const QString& title)
{
	QListWidgetItem* item = new QListWidgetItem(title.toUpper(), nav);
	item->setFlags(Qt::NoItemFlags);
	QFont f = item->font();
	f.setBold(true);
	f.setPointSizeF(f.pointSizeF() * 0.85);
	item->setFont(f);
}

void MainWindow::addPage(const QString& name, Page* page)
{
	/* Pages scroll rather than squash when the window is small */
	QScrollArea* scroll = new QScrollArea;
	scroll->setWidget(page);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);

	QListWidgetItem* item = new QListWidgetItem(QStringLiteral("   ") + name, nav);
	item->setData(Qt::UserRole, pages->addWidget(scroll));
}

void MainWindow::createMenus()
{
	QMenu* file = menuBar()->addMenu(tr("&File"));

	rasmolPath = QStandardPaths::findExecutable(QStringLiteral("rasmol"));
	QAction* rasmol = file->addAction(tr("Open in RasMol…"), this, &MainWindow::openInRasmol);
	rasmol->setEnabled(!rasmolPath.isEmpty());
	if (rasmolPath.isEmpty()) rasmol->setText(tr("Open in RasMol… (RasMol not found)"));

	file->addSeparator();
	QAction* quit = file->addAction(tr("&Quit"), qApp, &QApplication::quit);
	quit->setShortcut(QKeySequence::Quit);

	QMenu* help = menuBar()->addMenu(tr("&Help"));
	help->addAction(tr("&About ProtMate"), this, &MainWindow::about)->setMenuRole(QAction::AboutRole);
	help->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt)->setMenuRole(QAction::AboutQtRole);
}

void MainWindow::openInRasmol()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Open in RasMol"), ui::lastDir(),
		tr("PDB files (*.pdb *.ent);;All files (*)"));
	if (path.isEmpty()) return;
	ui::rememberDir(path);

	/* Started directly with an argument list, never through a shell */
	if (!QProcess::startDetached(rasmolPath, {path}))
		QMessageBox::warning(this, tr("Open in RasMol"), tr("Unable to start %1.").arg(rasmolPath));
}

void MainWindow::about()
{
	QString home = QString::fromLatin1(pmate::HOMEPAGE);
	QMessageBox::about(this, tr("About ProtMate"),
		tr("<h3>ProtMate %1</h3>"
		   "<p>Laboratory formula calculator.<br>Developed by Sanjeeva Wijeyesakere.<br>%2</p>"
		   "<p>ProtMate is free software: you can redistribute it and/or modify it under the terms of "
		   "the <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">GNU General Public License "
		   "version 3</a>.</p>"
		   "<p>This program comes with ABSOLUTELY NO WARRANTY. The author assumes no liability for the "
		   "accuracy of calculations or for the functionality of the program.</p>"
		   "<p>Source code, releases and issues: <a href=\"%3\">%3</a></p>"
		   "<p><small>System: %4 (%5 %6, %7)<br>Qt %8</small></p>")
			.arg(QString::fromLatin1(pmate::VERSION), QString::fromLatin1(pmate::COPYRIGHT).replace(QStringLiteral("(C)"), QStringLiteral("\u00A9")).toHtmlEscaped(), home,
				 QSysInfo::prettyProductName(), QSysInfo::kernelType(), QSysInfo::kernelVersion(),
				 QSysInfo::currentCpuArchitecture(), QString::fromLatin1(qVersion())));
}
