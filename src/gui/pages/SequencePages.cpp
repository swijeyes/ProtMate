/* Sequence pages: protein sequence analysis (menu item 7) and primer melting point (menu item 9). */

#include "Page.h"
#include "Pages.h"

#include "pmate_core.h"

#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <optional>
#include <sstream>
#include <vector>

namespace {

/* Parses the box's text with the same rules as a .seq file. Returns false if it is too long. */
bool parseSequence(QPlainTextEdit* edit, std::vector<char>& seq)
{
	QByteArray bytes = edit->toPlainText().toUtf8();
	std::istringstream in(std::string(bytes.constData(), size_t(bytes.size())));
	seq.clear();
	return pmate::readSequence(in, seq);
}

/* Restarts `timer` whenever the text changes, so large pastes are parsed once. */
QTimer* debounce(QObject* parent, QPlainTextEdit* edit)
{
	QTimer* timer = new QTimer(parent);
	timer->setSingleShot(true);
	timer->setInterval(200);
	QObject::connect(edit, &QPlainTextEdit::textChanged, timer, qOverload<>(&QTimer::start));
	return timer;
}

QString tooLongMessage()
{
	return QObject::tr("The sequence exceeds %1 residues.").arg(qulonglong(pmate::MAX_SEQ_LEN));
}

/* ---- Protein sequence analysis (menu item 7) ---- */

class SequenceAnalysisPage : public Page
{
public:
	SequenceAnalysisPage()
		: Page(tr("Sequence Analysis"),
			   tr("Amino acid composition, chemical formula, molecular weight and extinction coefficient "
				  "of a protein or peptide. Paste a sequence in one-letter codes or open a .seq file."))
	{
		QHBoxLayout* top = new QHBoxLayout;
		QPushButton* open = new QPushButton(tr("Open .seq file…"));
		source = ui::noteLabel();
		top->addWidget(open);
		top->addWidget(source, 1);

		seqEdit = ui::sequenceEdit();
		seqEdit->setPlaceholderText(tr("Paste a protein sequence here, e.g. MSFIDPYQHIIVEHQYSHKF…"));

		QFormLayout* f = new QFormLayout;
		total = ui::resultLabel();
		start = ui::intSpin(1, 1, 1);
		end = ui::intSpin(1, 1, 1);
		f->addRow(tr("Residues in sequence:"), total);
		f->addRow(tr("First residue:"), start);
		f->addRow(tr("Final residue:"), end);

		results = ui::textView();
		results->setMinimumHeight(260);

		QHBoxLayout* rep = new QHBoxLayout;
		name = new QLineEdit;
		name->setPlaceholderText(tr("Name or identifier for the report"));
		save = new QPushButton(tr("Save report…"));
		rep->addWidget(name, 1);
		rep->addWidget(save);
		status = ui::noteLabel();

		body->addLayout(top);
		body->addWidget(seqEdit);
		body->addLayout(f);
		body->addWidget(results, 1);
		body->addLayout(rep);
		body->addWidget(status);

		timer = debounce(this, seqEdit);
		connect(timer, &QTimer::timeout, this, [this]() { reparse(); });
		connect(start, &QSpinBox::valueChanged, this, [this]() { analyze(); });
		connect(end, &QSpinBox::valueChanged, this, [this]() { analyze(); });
		connect(open, &QPushButton::clicked, this, [this]() {
			QString path = ui::loadSequenceInto(this, seqEdit);
			if (path.isEmpty()) return;

			/* A new file starts with the whole sequence selected */
			timer->stop();
			source->setText(path);
			start->blockSignals(true);
			start->setValue(1);
			start->blockSignals(false);
			end->blockSignals(true);
			end->setValue(end->maximum());
			end->blockSignals(false);
			reparse();
			if (name->text().isEmpty()) name->setText(QFileInfo(path).completeBaseName());
		});
		connect(save, &QPushButton::clicked, this, [this]() { saveReport(); });

		reparse();
	}

private:
	void reparse()
	{
		bool ok = parseSequence(seqEdit, seq);
		int n = int(seq.size());
		total->setText(ok ? QString::number(n) : tr("too long"));

		/* Keep the final residue pinned to the end of the sequence if it was there */
		bool followEnd = (end->value() == end->maximum());
		start->blockSignals(true);
		end->blockSignals(true);
		start->setRange(1, qMax(1, n));
		end->setRange(1, qMax(1, n));
		if (followEnd) end->setValue(end->maximum());
		start->blockSignals(false);
		end->blockSignals(false);

		if (!ok) {seq.clear(); analysis.reset(); results->setPlainText(tooLongMessage()); save->setEnabled(false); return;}
		analyze();
	}

	void analyze()
	{
		analysis.reset();
		save->setEnabled(false);

		int n = int(seq.size());
		if (n == 0) {results->setPlainText(tr("Paste a sequence or open a .seq file to begin.")); return;}

		int s = start->value() - 1;
		int e = end->value() - 1;
		if (e < s) {results->setPlainText(tr("The final residue must not come before the first.")); return;}

		analysis = pmate::analyzeProtein(seq, s, e);
		const pmate::SeqAnalysis& a = *analysis;

		QString text = tr("Residues analysed: %1 (%2 to %3 of %4)\n\n").arg(a.length).arg(s+1).arg(e+1).arg(n);
		if (!a.unknownPositions.empty())
		{
			QStringList pos;
			for (size_t i=0; i<a.unknownPositions.size() && i<50; ++i) pos << QString::number(a.unknownPositions[i]);
			if (a.unknownPositions.size() > 50) pos << tr("… and %1 more").arg(a.unknownPositions.size() - 50);
			text += tr("Warning: %n unknown residue(s) at position(s) ", nullptr, int(a.unknownPositions.size()))
				 + pos.join(QStringLiteral(", "))
				 + tr(".\nPlease verify your sequence before using the results below.\n\n");
		}
		text += QString::fromStdString(pmate::formatComposition(a));
		text += tr("\nFormula of protein/peptide: ") + QString::fromStdString(pmate::formatFormula(a)) + QLatin1Char('\n');
		text += tr("\nMolecular weight of protein/peptide (Da): ") + ui::num(a.mw) + QLatin1Char('\n');
		text += tr("\nExtinction coefficient of the native protein in water: ") + ui::num(a.ext.ext) + QLatin1Char('\n');
		text += tr("Absorbance (at 280 nm & 1 mg/ml) assuming a path length of 1 cm: ") + ui::num(a.ext.abs) + QLatin1Char('\n');
		results->setPlainText(text);
		save->setEnabled(true);
	}

	void saveReport()
	{
		if (!analysis) return;
		QString id = name->text().trimmed();
		if (id.isEmpty()) id = tr("unnamed protein");
		std::string report = pmate::formatProteinReport(*analysis, id.toStdString(), ui::userName().toStdString());
		QString path = ui::saveText(this, tr("Save report"), id + QStringLiteral("_report.txt"), report);
		if (!path.isEmpty()) status->setText(tr("Saved to %1").arg(path));
	}

	QPlainTextEdit *seqEdit, *results;
	QLabel *source, *total, *status;
	QSpinBox *start, *end;
	QLineEdit* name;
	QPushButton* save;
	QTimer* timer;
	std::vector<char> seq;
	std::optional<pmate::SeqAnalysis> analysis;
};

/* ---- Primer melting point (menu item 9) ---- */

class MeltPage : public Page
{
public:
	MeltPage()
		: Page(tr("Primer Melting Point"),
			   tr("Melting point of a primer or oligonucleotide: 2(A+T) + 4(G+C) below 14 nucleotides, otherwise "
				  "64.9 + 41(G+C − 16.4)/N, plus a salt-adjusted value."))
	{
		manual = new QRadioButton(tr("Enter length and G+C count"));
		fromSeq = new QRadioButton(tr("From a sequence"));
		manual->setChecked(true);
		QButtonGroup* group = new QButtonGroup(this);
		group->addButton(manual);
		group->addButton(fromSeq);
		QHBoxLayout* modes = new QHBoxLayout;
		modes->addWidget(manual);
		modes->addWidget(fromSeq);
		modes->addStretch();

		inputs = new QStackedWidget;

		QWidget* manualPage = new QWidget;
		QFormLayout* mf = new QFormLayout(manualPage);
		mf->setContentsMargins(0, 0, 0, 0);
		length = ui::intSpin(1, 100000, 20);
		gc = ui::intSpin(0, 20, 10);
		mf->addRow(tr("Length of primer (nucleotides):"), length);
		mf->addRow(tr("Number of G+Cs:"), gc);
		inputs->addWidget(manualPage);

		QWidget* seqPage = new QWidget;
		QVBoxLayout* sv = new QVBoxLayout(seqPage);
		sv->setContentsMargins(0, 0, 0, 0);
		QHBoxLayout* top = new QHBoxLayout;
		QPushButton* open = new QPushButton(tr("Open .seq file…"));
		source = ui::noteLabel();
		top->addWidget(open);
		top->addWidget(source, 1);
		seqEdit = ui::sequenceEdit();
		seqEdit->setPlaceholderText(tr("Paste a DNA sequence here, e.g. ACGTTGCA…"));
		sv->addLayout(top);
		sv->addWidget(seqEdit);
		inputs->addWidget(seqPage);

		QFormLayout* sf = new QFormLayout;
		salt = ui::doubleSpin(0.0001, 10, 0.05, 4, tr(" M"));
		sf->addRow(tr("Total salt concentration:"), salt);

		QGroupBox* g = new QGroupBox(tr("Results"));
		QFormLayout* r = new QFormLayout(g);
		counts = ui::resultLabel();
		pgc = ui::resultLabel();
		tm = ui::resultLabel();
		adtm = ui::resultLabel();
		countsLabel = new QLabel(tr("Base counts:"));
		r->addRow(countsLabel, counts);
		r->addRow(tr("G-C content:"), pgc);
		r->addRow(tr("Melting point:"), tm);
		r->addRow(tr("Salt-adjusted melting point:"), adtm);
		error = ui::noteLabel();
		error->setStyleSheet(QStringLiteral("color: #c0392b;"));

		QHBoxLayout* h = new QHBoxLayout;
		save = new QPushButton(tr("Save report…"));
		h->addStretch();
		h->addWidget(save);
		status = ui::noteLabel();

		body->addLayout(modes);
		body->addWidget(inputs);
		body->addLayout(sf);
		body->addWidget(error);
		body->addWidget(g);
		body->addLayout(h);
		body->addWidget(status);
		body->addStretch();

		connect(group, &QButtonGroup::buttonToggled, this, [this]() {
			inputs->setCurrentIndex(fromSeq->isChecked() ? 1 : 0);
			update();
		});
		connect(length, &QSpinBox::valueChanged, this, [this](int n) { gc->setMaximum(n); update(); });
		connect(gc, &QSpinBox::valueChanged, this, [this]() { update(); });
		connect(salt, &QDoubleSpinBox::valueChanged, this, [this]() { update(); });
		QTimer* timer = debounce(this, seqEdit);
		connect(timer, &QTimer::timeout, this, [this]() { update(); });
		connect(open, &QPushButton::clicked, this, [this]() {
			QString path = ui::loadSequenceInto(this, seqEdit);
			if (!path.isEmpty()) source->setText(path);
		});
		connect(save, &QPushButton::clicked, this, [this]() { saveReport(); });

		update();
	}

private:
	void clearResults(const QString& message)
	{
		result.reset();
		for (QLabel* l : {counts, pgc, tm, adtm}) l->setText(QStringLiteral("—"));
		error->setText(message);
		error->setVisible(!message.isEmpty());
		save->setEnabled(false);
	}

	void update()
	{
		bool seqMode = fromSeq->isChecked();
		countsLabel->setVisible(seqMode);
		counts->setVisible(seqMode);
		nc.reset();

		float n, g;
		if (!seqMode)
		{
			n = length->value();
			g = gc->value();
		}
		else
		{
			std::vector<char> seq;
			if (!parseSequence(seqEdit, seq)) {clearResults(tooLongMessage()); return;}
			if (seq.empty()) {clearResults(QString()); return;}

			pmate::NucleotideCounts c = pmate::countNucleotides(seq);
			if (c.badPosition != 0)
			{
				clearResults(tr("Unknown base '%1' at position %2. Only A, C, G and T are allowed.")
							 .arg(QChar::fromLatin1(pmate::printable(c.badBase))).arg(c.badPosition));
				return;
			}
			nc = c;
			n = seq.size();
			g = c.g + c.c;
			counts->setText(tr("A %1   C %2   G %3   T %4   (%5 nt)").arg(c.a).arg(c.c).arg(c.g).arg(c.t).arg(seq.size()));
		}

		result = pmate::meltPoint(n, g, salt->value());
		error->setVisible(false);
		pgc->setText(ui::num(result->pgc) + QLatin1Char('%'));
		tm->setText(ui::num2(result->tm) + tr(" °C"));
		adtm->setText(ui::num2(result->adtm) + tr(" °C"));
		save->setEnabled(true);
	}

	void saveReport()
	{
		if (!result) return;
		std::string report = pmate::formatMeltReport(*result, nc ? &*nc : nullptr);
		QString path = ui::saveText(this, tr("Save report"), QStringLiteral("melting_point.txt"), report);
		if (!path.isEmpty()) status->setText(tr("Saved to %1").arg(path));
	}

	QRadioButton *manual, *fromSeq;
	QStackedWidget* inputs;
	QSpinBox *length, *gc;
	QPlainTextEdit* seqEdit;
	QDoubleSpinBox* salt;
	QLabel *source, *countsLabel, *counts, *pgc, *tm, *adtm, *error, *status;
	QPushButton* save;
	std::optional<pmate::MeltResult> result;
	std::optional<pmate::NucleotideCounts> nc;
};

}

Page* createSequenceAnalysisPage() { return new SequenceAnalysisPage; }
Page* createMeltPage() { return new MeltPage; }
