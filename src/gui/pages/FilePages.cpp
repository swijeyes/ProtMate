/* File utility pages: menu items 12-15. */

#include "Page.h"
#include "Pages.h"

#include "pmate_core.h"

#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QScrollBar>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <sstream>
#include <vector>

namespace {

QLabel* errorLabel()
{
	QLabel* l = ui::noteLabel();
	l->setStyleSheet(QStringLiteral("color: #c0392b;"));
	l->hide();
	return l;
}

void showError(QLabel* l, const QString& message)
{
	l->setText(message);
	l->setVisible(!message.isEmpty());
}

/* ---- Rename oscillation files (menu item 12) ---- */

class RenamePage : public Page
{
public:
	RenamePage()
		: Page(tr("Rename Oscillation Files"),
			   tr("Rename sequential oscillation images (e.g. 1.0001, 1.0002 …) to a name format Mosflm can read "
				  "(e.g. a_001.osc). Existing files are never overwritten."))
	{
		QFormLayout* f = new QFormLayout;
		f->addRow(tr("Directory:"), ui::filePicker(dir, tr("Choose the directory with your images"), QString(), true));
		count = ui::intSpin(1, pmate::MAX_READINGS, 100);
		first = ui::intSpin(1, 1000000, 1);
		from = new QLineEdit(QStringLiteral("1."));
		to = new QLineEdit(QStringLiteral("a_"));
		f->addRow(tr("Number of files:"), count);
		f->addRow(tr("Initial frame number:"), first);
		f->addRow(tr("Current file string (for 1.####, enter \"1.\"):"), from);
		f->addRow(tr("New file string (for a_###.osc, enter \"a_\"):"), to);

		error = errorLabel();

		table = new QTableWidget(0, 3);
		table->setHorizontalHeaderLabels({tr("Current name"), tr("New name"), tr("Status")});
		table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		table->setEditTriggers(QAbstractItemView::NoEditTriggers);
		table->setMinimumHeight(220);

		QHBoxLayout* h = new QHBoxLayout;
		summary = ui::noteLabel();
		rename = new QPushButton(tr("Rename…"));
		h->addWidget(summary, 1);
		h->addWidget(rename);

		body->addLayout(f);
		body->addWidget(error);
		body->addWidget(table, 1);
		body->addLayout(h);

		timer = new QTimer(this);
		timer->setSingleShot(true);
		timer->setInterval(150);
		connect(timer, &QTimer::timeout, this, [this]() { refresh(); });
		for (QLineEdit* e : {dir, from, to}) connect(e, &QLineEdit::textChanged, timer, qOverload<>(&QTimer::start));
		for (QSpinBox* s : {count, first}) connect(s, &QSpinBox::valueChanged, timer, qOverload<>(&QTimer::start));
		connect(rename, &QPushButton::clicked, this, [this]() { doRename(); });

		refresh();
	}

private:
	enum Status { Ready, Missing, Exists };

	/* Returns an error message, or "" if the inputs are usable */
	QString validate() const
	{
		std::string a = from->text().toStdString();
		std::string b = to->text().toStdString();
		if (dir->text().isEmpty()) return tr("Choose the directory that holds your images.");
		if (!QFileInfo(dir->text()).isDir()) return tr("%1 is not a directory.").arg(dir->text());
		if (!pmate::isSafeFilename(a) || a.find('/') != std::string::npos)
			return tr("The current file string may only contain letters, digits and . _ - +, and must not start with '-'.");
		if (!pmate::isSafeFilename(b) || b.find('/') != std::string::npos)
			return tr("The new file string may only contain letters, digits and . _ - +, and must not start with '-'.");
		return QString();
	}

	static bool present(const QString& path)
	{
		QFileInfo fi(path);
		return fi.exists() || fi.isSymLink();
	}

	void refresh()
	{
		plan.clear();
		status.clear();
		table->setRowCount(0);

		QString message = validate();
		showError(error, message);
		if (!message.isEmpty()) {summary->clear(); rename->setEnabled(false); return;}

		QDir d(dir->text());
		plan = pmate::renamePlan(from->text().toStdString(), to->text().toStdString(), first->value(), count->value());

		int ready = 0, missing = 0, exists = 0;
		table->setUpdatesEnabled(false);
		table->setRowCount(int(plan.size()));
		for (size_t i=0; i<plan.size(); ++i)
		{
			QString src = QString::fromStdString(plan[i].first);
			QString dst = QString::fromStdString(plan[i].second);
			Status s = !present(d.filePath(src)) ? Missing : present(d.filePath(dst)) ? Exists : Ready;
			status.push_back(s);

			QString text = s == Ready ? tr("Ready") : s == Missing ? tr("Not found") : tr("New name already exists");
			if (s == Ready) ++ready; else if (s == Missing) ++missing; else ++exists;

			table->setItem(int(i), 0, new QTableWidgetItem(src));
			table->setItem(int(i), 1, new QTableWidgetItem(dst));
			table->setItem(int(i), 2, new QTableWidgetItem(text));
		}
		table->setUpdatesEnabled(true);

		summary->setText(tr("%1 ready, %2 not found, %3 blocked by an existing file.").arg(ready).arg(missing).arg(exists));
		rename->setEnabled(ready > 0);
	}

	void doRename()
	{
		refresh();
		int ready = int(std::count(status.begin(), status.end(), Ready));
		if (ready == 0) return;
		if (QMessageBox::question(this, tr("Rename files"), tr("Rename %1 file(s) in %2?").arg(ready).arg(dir->text()))
			!= QMessageBox::Yes) return;

		QDir d(dir->text());
		int done = 0;
		QStringList failed;
		for (size_t i=0; i<plan.size(); ++i)
		{
			if (status[i] != Ready) continue;
			QString src = QString::fromStdString(plan[i].first);
			QString dst = QString::fromStdString(plan[i].second);

			/* QDir::rename is a plain rename that refuses to replace an existing file
			   (unlike QFile::rename, it never falls back to copy-and-delete) */
			if (!present(d.filePath(dst)) && d.rename(src, dst)) ++done;
			else failed << src;
		}

		refresh();
		if (failed.isEmpty())
			QMessageBox::information(this, tr("Rename files"), tr("Renamed %1 file(s).").arg(done));
		else
			QMessageBox::warning(this, tr("Rename files"), tr("Renamed %1 file(s). Could not rename:\n%2")
								 .arg(done).arg(failed.mid(0, 20).join(QStringLiteral("\n"))));
	}

	QLineEdit *dir, *from, *to;
	QSpinBox *count, *first;
	QLabel *error, *summary;
	QTableWidget* table;
	QPushButton* rename;
	QTimer* timer;
	std::vector<std::pair<std::string, std::string> > plan;
	std::vector<Status> status;
};

/* ---- AutoDock Vina configuration (menu item 13) ---- */

class VinaPage : public Page
{
public:
	VinaPage()
		: Page(tr("AutoDock Vina Configuration"),
			   tr("Write a configuration file for AutoDock Vina with a cubic search grid, and optionally run Vina with it. "
				  "Relative file names are resolved from the folder the configuration is saved in.\n"
				  "O. Trott, A. J. Olson (2010). J. Comput. Chem. 31, 455-461. http://vina.scripps.edu"))
	{
		vinaPath = QStandardPaths::findExecutable(QStringLiteral("vina"));

		QFormLayout* f = new QFormLayout;
		QString filter = tr("PDBQT files (*.pdbqt);;All files (*)");
		f->addRow(tr("Receptor (PDBQT):"), ui::filePicker(receptor, tr("Choose the receptor"), filter));
		f->addRow(tr("Ligand (PDBQT):"), ui::filePicker(ligand, tr("Choose the ligand"), filter));
		x = ui::doubleSpin(-100000, 100000, 0, 3, tr(" Å"));
		y = ui::doubleSpin(-100000, 100000, 0, 3, tr(" Å"));
		z = ui::doubleSpin(-100000, 100000, 0, 3, tr(" Å"));
		grid = ui::doubleSpin(0.001, 100000, 20, 3, tr(" Å"));
		f->addRow(tr("Grid centre X:"), x);
		f->addRow(tr("Grid centre Y:"), y);
		f->addRow(tr("Grid centre Z:"), z);
		f->addRow(tr("Length of one side of the grid:"), grid);

		error = errorLabel();
		preview = ui::textView();
		preview->setMaximumHeight(200);

		QHBoxLayout* h = new QHBoxLayout;
		QPushButton* save = new QPushButton(tr("Save configuration…"));
		run = new QPushButton(tr("Run Vina"));
		stop = new QPushButton(tr("Stop"));
		saveBtn = save;
		h->addWidget(save);
		h->addStretch();
		h->addWidget(run);
		h->addWidget(stop);

		vinaNote = ui::noteLabel();
		log = ui::textView();
		log->setMinimumHeight(140);

		body->addLayout(f);
		body->addWidget(error);
		body->addWidget(new QLabel(tr("Configuration preview:")));
		body->addWidget(preview);
		body->addLayout(h);
		body->addWidget(vinaNote);
		body->addWidget(log, 1);

		process = new QProcess(this);
		process->setProcessChannelMode(QProcess::MergedChannels);
		connect(process, &QProcess::readyReadStandardOutput, this, [this]() {
			appendLog(QString::fromLocal8Bit(process->readAllStandardOutput()));
		});
		connect(process, &QProcess::finished, this, [this](int code, QProcess::ExitStatus st) {
			if (st == QProcess::CrashExit) appendLog(tr("\nVina stopped.\n"));
			else appendLog(tr("\nVina finished with exit code %1. Docked conformations: %2\n")
						   .arg(code).arg(QDir(QFileInfo(configPath).absolutePath()).filePath(QStringLiteral("all.pdbqt"))));
			update();
		});
		connect(process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError e) {
			if (e == QProcess::FailedToStart) {appendLog(tr("Unable to start Vina: %1\n").arg(process->errorString())); update();}
		});

		for (QLineEdit* e : {receptor, ligand}) connect(e, &QLineEdit::textChanged, this, [this]() { update(); });
		for (QDoubleSpinBox* s : {x, y, z, grid}) connect(s, &QDoubleSpinBox::valueChanged, this, [this]() { update(); });
		connect(save, &QPushButton::clicked, this, [this]() { saveConfig(); });
		connect(run, &QPushButton::clicked, this, [this]() { runVina(); });
		connect(stop, &QPushButton::clicked, process, &QProcess::kill);

		update();
	}

	~VinaPage() override
	{
		/* Stop a running docking job without calling back into half-destroyed widgets */
		process->disconnect(this);
		if (process->state() != QProcess::NotRunning)
		{
			process->kill();
			process->waitForFinished(3000);
		}
	}

private:
	pmate::VinaParams params() const
	{
		pmate::VinaParams p;
		p.receptor = receptor->text().trimmed().toStdString();
		p.ligand = ligand->text().trimmed().toStdString();
		p.x = x->value();
		p.y = y->value();
		p.z = z->value();
		p.grid = grid->value();
		return p;
	}

	bool valid() const
	{
		pmate::VinaParams p = params();
		return pmate::isSafeConfigValue(p.receptor) && pmate::isSafeConfigValue(p.ligand);
	}

	void update()
	{
		pmate::VinaParams p = params();
		bool ok = valid();
		bool partly = !p.receptor.empty() || !p.ligand.empty();
		showError(error, ok || !partly ? QString()
			: tr("Choose a receptor and a ligand. File names must not contain '#' or control characters."));
		preview->setPlainText(QString::fromStdString(pmate::vinaConfig(p)));

		bool running = process->state() != QProcess::NotRunning;
		bool saved = !configPath.isEmpty() && savedConfig == pmate::vinaConfig(p);
		saveBtn->setEnabled(ok && !running);
		run->setEnabled(ok && saved && !vinaPath.isEmpty() && !running);
		stop->setEnabled(running);

		if (vinaPath.isEmpty())
			vinaNote->setText(tr("AutoDock Vina was not found on your PATH. You can still write the configuration file."));
		else if (!saved)
			vinaNote->setText(tr("Vina: %1 — save the configuration to run it.").arg(vinaPath));
		else
			vinaNote->setText(tr("Vina: %1 — configuration: %2").arg(vinaPath, configPath));
	}

	void saveConfig()
	{
		if (!valid()) return;
		QString suggested = QStringLiteral("conf.txt");
		QFileInfo r(receptor->text().trimmed());
		if (r.isAbsolute()) suggested = QDir(r.absolutePath()).filePath(suggested);

		std::string conf = pmate::vinaConfig(params());
		QString path = ui::saveText(this, tr("Save Vina configuration"), suggested, conf, tr("Text files (*.txt);;All files (*)"));
		if (path.isEmpty()) return;
		configPath = path;
		savedConfig = conf;
		update();
	}

	void runVina()
	{
		if (!run->isEnabled()) return;
		QString wd = QFileInfo(configPath).absolutePath();
		QStringList args = {QStringLiteral("--config"), configPath,
							QStringLiteral("--log"), QDir(wd).filePath(QStringLiteral("vina_log.txt"))};
		log->clear();
		appendLog(QStringLiteral("$ %1 %2\n\n").arg(vinaPath, args.join(QLatin1Char(' '))));

		/* Started directly with an argument list, never through a shell */
		process->setWorkingDirectory(wd);
		process->start(vinaPath, args);
		update();
	}

	void appendLog(const QString& text)
	{
		log->moveCursor(QTextCursor::End);
		log->insertPlainText(text);
		log->verticalScrollBar()->setValue(log->verticalScrollBar()->maximum());
	}

	QLineEdit *receptor, *ligand;
	QDoubleSpinBox *x, *y, *z, *grid;
	QLabel *error, *vinaNote;
	QPlainTextEdit *preview, *log;
	QPushButton *saveBtn, *run, *stop;
	QProcess* process;
	QString vinaPath, configPath;
	std::string savedConfig;
};

/* ---- PDBQT to PDB (menu item 14) ---- */

class PdbqtPage : public Page
{
public:
	PdbqtPage()
		: Page(tr("PDBQT to PDB"),
			   tr("Convert a PDBQT file to a standard PDB file, with the B-factors replaced by the partial atom charges."))
	{
		QFormLayout* f = new QFormLayout;
		f->addRow(tr("PDBQT file:"), ui::filePicker(input, tr("Choose a PDBQT file"), tr("PDBQT files (*.pdbqt);;All files (*)")));

		QHBoxLayout* h = new QHBoxLayout;
		convert = new QPushButton(tr("Convert and save…"));
		h->addStretch();
		h->addWidget(convert);
		status = ui::noteLabel();

		body->addLayout(f);
		body->addLayout(h);
		body->addWidget(status);
		body->addStretch();

		connect(input, &QLineEdit::textChanged, this, [this]() {
			convert->setEnabled(QFileInfo(input->text()).isFile());
			status->clear();
		});
		connect(convert, &QPushButton::clicked, this, [this]() { doConvert(); });
		convert->setEnabled(false);
	}

private:
	void doConvert()
	{
		QString in = input->text();
		std::string data;
		if (!ui::readFileBytes(this, in, data, 1024 * 1024 * 1024)) return;

		std::istringstream src(data);
		std::ostringstream pdb;
		pmate::pdbqtToPdb(src, pdb);

		QFileInfo fi(in);
		QString suggested = QDir(fi.absolutePath()).filePath(fi.completeBaseName() + QStringLiteral(".pdb"));
		QString path = ui::saveText(this, tr("Save PDB file"), suggested, pdb.str(), tr("PDB files (*.pdb);;All files (*)"));
		if (!path.isEmpty()) status->setText(tr("Converted %1 to %2").arg(in, path));
	}

	QLineEdit* input;
	QPushButton* convert;
	QLabel* status;
};

/* ---- X-PLOR code to solvate a protein (menu item 15) ---- */

class XplorPage : public Page
{
public:
	XplorPage()
		: Page(tr("Solvate a Protein (X-PLOR/CNS)"),
			   tr("Generate X-PLOR code to overlay a protein onto a solvent shell. Stage 1 keeps solvent within 6 Å "
				  "of the protein; stage 2 deletes solvent that clashes with it (within 2.3 Å). File names may only "
				  "contain letters, digits and . _ - / +."))
	{
		stage = new QComboBox;
		stage->addItem(tr("Stage 1"));
		stage->addItem(tr("Stage 2"));
		stage->addItem(tr("Both stages"));
		stage->setCurrentIndex(2);

		QString pdb = tr("PDB files (*.pdb);;All files (*)");
		QString mtf = tr("MTF files (*.mtf *.psf);;All files (*)");
		QFormLayout* f = new QFormLayout;
		f->addRow(tr("Generate:"), stage);
		f->addRow(tr("Protein PDB coordinate file:"), ui::filePicker(ppdb, tr("Protein coordinates"), pdb));
		f->addRow(tr("Protein MTF structure file:"), ui::filePicker(pmtf, tr("Protein structure"), mtf));
		wpdbLabel = new QLabel;
		wmtfLabel = new QLabel;
		f->addRow(wpdbLabel, ui::filePicker(wpdb, tr("Solvent coordinates"), pdb));
		f->addRow(wmtfLabel, ui::filePicker(wmtf, tr("Solvent structure"), mtf));

		stage2Box = new QGroupBox(tr("Stage 2 input (solvent files produced by stage 1)"));
		QFormLayout* f2 = new QFormLayout(stage2Box);
		f2->addRow(tr("Solvent's new PDB coordinate file:"), ui::filePicker(npdb, tr("Solvent coordinates from stage 1"), pdb));
		f2->addRow(tr("Solvent's new MTF structure file:"), ui::filePicker(nmtf, tr("Solvent structure from stage 1"), mtf));
		npdb->setPlaceholderText(QStringLiteral("overlay1.pdb"));

		error = errorLabel();
		tabs = new QTabWidget;
		script1 = ui::textView();
		script2 = ui::textView();
		tabs->addTab(script1, tr("Stage 1 script"));
		tabs->addTab(script2, tr("Stage 2 script"));
		tabs->setMinimumHeight(220);

		QHBoxLayout* h = new QHBoxLayout;
		save1 = new QPushButton(tr("Save stage 1…"));
		save2 = new QPushButton(tr("Save stage 2…"));
		status = ui::noteLabel();
		h->addWidget(status, 1);
		h->addWidget(save1);
		h->addWidget(save2);

		body->addLayout(f);
		body->addWidget(stage2Box);
		body->addWidget(error);
		body->addWidget(tabs, 1);
		body->addLayout(h);

		connect(stage, &QComboBox::currentIndexChanged, this, [this]() { update(); });
		for (QLineEdit* e : {ppdb, pmtf, wpdb, wmtf, npdb, nmtf}) connect(e, &QLineEdit::textChanged, this, [this]() { update(); });
		connect(save1, &QPushButton::clicked, this, [this]() { save(script1, QStringLiteral("stage1.inp")); });
		connect(save2, &QPushButton::clicked, this, [this]() { save(script2, QStringLiteral("stage2.inp")); });

		update();
	}

private:
	static std::string field(QLineEdit* e)
	{
		return e->text().trimmed().toStdString();
	}

	/* True if every given field is a usable X-PLOR file name; collects the bad ones */
	static bool check(std::initializer_list<QLineEdit*> edits, bool& anyFilled)
	{
		bool ok = true;
		for (QLineEdit* e : edits)
		{
			if (!e->text().trimmed().isEmpty()) anyFilled = true;
			if (!pmate::isSafeFilename(field(e))) ok = false;
		}
		return ok;
	}

	void update()
	{
		int mode = stage->currentIndex();
		bool both = (mode == 2);
		stage2Box->setVisible(both);
		wpdbLabel->setText(mode == 1 ? tr("Solvent PDB file (from stage 1):") : tr("Solvent PDB coordinate file:"));
		wmtfLabel->setText(mode == 1 ? tr("Solvent MTF file (from stage 1):") : tr("Solvent MTF structure file:"));

		bool anyFilled = false;
		bool ok = check({ppdb, pmtf, wpdb, wmtf}, anyFilled);
		if (both) ok = check({npdb, nmtf}, anyFilled) && ok;

		bool want1 = (mode != 1), want2 = (mode != 0);
		tabs->setTabEnabled(0, want1);
		tabs->setTabEnabled(1, want2);
		tabs->setCurrentIndex(want1 ? 0 : 1);

		QString waiting = tr("Fill in every file name to see the script.");
		script1->setPlainText(waiting);
		script2->setPlainText(waiting);
		save1->setEnabled(false);
		save2->setEnabled(false);
		showError(error, ok || !anyFilled ? QString()
			: tr("Each file name is required and may only contain letters, digits and . _ - / + (no spaces, and not starting with '-')."));
		if (!ok) return;

		if (want1)
		{
			script1->setPlainText(QString::fromStdString(pmate::xplorStage1(field(pmtf), field(ppdb), field(wmtf), field(wpdb))));
			save1->setEnabled(true);
		}
		if (want2)
		{
			std::string s2 = both
				? pmate::xplorStage2(field(pmtf), field(ppdb), field(nmtf), field(npdb))
				: pmate::xplorStage2(field(pmtf), field(ppdb), field(wmtf), field(wpdb));
			script2->setPlainText(QString::fromStdString(s2));
			save2->setEnabled(true);
		}
	}

	void save(QPlainTextEdit* script, const QString& suggested)
	{
		QByteArray bytes = script->toPlainText().toUtf8();
		QString path = ui::saveText(this, tr("Save X-PLOR script"), suggested,
									std::string(bytes.constData(), size_t(bytes.size())), tr("X-PLOR input (*.inp);;All files (*)"));
		if (!path.isEmpty()) status->setText(tr("Saved to %1").arg(path));
	}

	QComboBox* stage;
	QLineEdit *ppdb, *pmtf, *wpdb, *wmtf, *npdb, *nmtf;
	QLabel *wpdbLabel, *wmtfLabel, *error, *status;
	QGroupBox* stage2Box;
	QTabWidget* tabs;
	QPlainTextEdit *script1, *script2;
	QPushButton *save1, *save2;
};

}

Page* createRenamePage() { return new RenamePage; }
Page* createVinaPage() { return new VinaPage; }
Page* createPdbqtPage() { return new PdbqtPage; }
Page* createXplorPage() { return new XplorPage; }
