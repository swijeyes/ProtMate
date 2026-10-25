/* Calculator pages: menu items 1-6, 8, 10, 11 and the percent-to-molar converter. */

#include "Page.h"
#include "Pages.h"

#include "pmate_core.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <functional>
#include <optional>
#include <sstream>
#include <vector>

namespace {

/* Calls `f` whenever any of the given inputs changes. */
template <typename F>
void watch(QObject* ctx, std::initializer_list<QDoubleSpinBox*> spins, F f)
{
	for (QDoubleSpinBox* s : spins) QObject::connect(s, &QDoubleSpinBox::valueChanged, ctx, f);
}

template <typename F>
void watch(QObject* ctx, std::initializer_list<QSpinBox*> spins, F f)
{
	for (QSpinBox* s : spins) QObject::connect(s, &QSpinBox::valueChanged, ctx, f);
}

/* ---- Weight to volume ratio (menu items 1 and 2) ---- */

class WeightVolumePage : public Page
{
public:
	WeightVolumePage()
		: Page(tr("Weight to Volume Ratio"),
			   tr("Work out the mass of a compound needed to make a % (w/v) solution, "
				  "or the volume of solvent needed for a given mass."))
	{
		QGroupBox* g1 = new QGroupBox(tr("Calculate weight"));
		QFormLayout* f1 = new QFormLayout(g1);
		wv1 = ui::doubleSpin(0, 1e6, 5, 3, tr(" % (w/v)"));
		vol = ui::doubleSpin(0, 1e9, 100, 3, tr(" ml"));
		mass = ui::resultLabel();
		f1->addRow(tr("Weight to volume percent:"), wv1);
		f1->addRow(tr("Desired volume:"), vol);
		f1->addRow(tr("Mass of sample required:"), mass);

		QGroupBox* g2 = new QGroupBox(tr("Calculate volume"));
		QFormLayout* f2 = new QFormLayout(g2);
		wv2 = ui::doubleSpin(0.001, 1e6, 5, 3, tr(" % (w/v)"));
		grams = ui::doubleSpin(0, 1e9, 5, 3, tr(" g"));
		volume = ui::resultLabel();
		f2->addRow(tr("Weight to volume percent:"), wv2);
		f2->addRow(tr("Sample weight:"), grams);
		f2->addRow(tr("Volume of liquid required:"), volume);

		body->addWidget(g1);
		body->addWidget(g2);
		body->addStretch();

		watch(this, {wv1, vol, wv2, grams}, [this]() { update(); });
		update();
	}

private:
	void update()
	{
		mass->setText(ui::num2(pmate::massForWV(wv1->value(), vol->value())) + tr(" g"));
		volume->setText(ui::num2(pmate::volumeForWV(wv2->value(), grams->value())) + tr(" ml"));
	}

	QDoubleSpinBox *wv1, *vol, *wv2, *grams;
	QLabel *mass, *volume;
};

/* ---- Protein concentration, single reading (menu item 3) ---- */

class ProteinConcPage : public Page
{
public:
	ProteinConcPage()
		: Page(tr("Protein Concentration (A280)"),
			   tr("Calculate a protein's concentration and total yield from a single A280 reading."))
	{
		QFormLayout* f = new QFormLayout;
		v = ui::doubleSpin(0, 1e6, 1, 3, tr(" ml"));
		a = ui::doubleSpin(0, 1000, 0.5, 3);
		t = ui::doubleSpin(0.001, 1e6, 1000, 3, tr(" µl"));
		s = ui::doubleSpin(0.001, 1e6, 100, 3, tr(" µl"));
		x = ui::doubleSpin(0.0001, 1e7, 1, 4);
		f->addRow(tr("Total volume of protein:"), v);
		f->addRow(tr("A280 of protein:"), a);
		f->addRow(tr("Total liquid in cuvette (protein + buffer):"), t);
		f->addRow(tr("Protein in cuvette:"), s);
		f->addRow(tr("Extinction coefficient:"), x);

		QGroupBox* g = new QGroupBox(tr("Results"));
		QFormLayout* r = new QFormLayout(g);
		dil = ui::resultLabel();
		conc = ui::resultLabel();
		amount = ui::resultLabel();
		r->addRow(tr("Dilution factor:"), dil);
		r->addRow(tr("Protein concentration:"), conc);
		r->addRow(tr("Total amount of protein:"), amount);

		body->addLayout(f);
		body->addWidget(g);
		body->addStretch();

		watch(this, {v, a, t, s, x}, [this]() { update(); });
		update();
	}

private:
	void update()
	{
		pmate::ProteinConc p = pmate::proteinConc(a->value(), t->value(), s->value(), x->value(), v->value());
		dil->setText(ui::num(p.dilution));
		conc->setText(ui::num(p.conc) + tr(" mg/ml"));
		amount->setText(ui::num(p.amount) + tr(" mg"));
	}

	QDoubleSpinBox *v, *a, *t, *s, *x;
	QLabel *dil, *conc, *amount;
};

/* ---- Protein concentration, multiple readings (menu items 4 and 5) ---- */

class MultiReadingPage : public Page
{
public:
	MultiReadingPage()
		: Page(tr("Protein Concentration – Multiple Readings"),
			   tr("Enter several A280 readings and yield volumes, or import them from a text file "
				  "of whitespace-separated \"A280 volume\" pairs."))
	{
		QFormLayout* f = new QFormLayout;
		t = ui::doubleSpin(0.001, 1e6, 1000, 3, tr(" µl"));
		s = ui::doubleSpin(0.001, 1e6, 100, 3, tr(" µl"));
		x = ui::doubleSpin(0.0001, 1e7, 1, 4);
		dil = ui::resultLabel();
		f->addRow(tr("Total liquid in cuvette (protein + buffer):"), t);
		f->addRow(tr("Protein in cuvette:"), s);
		f->addRow(tr("Extinction coefficient:"), x);
		f->addRow(tr("Dilution factor:"), dil);

		table = new QTableWidget(0, 4);
		table->setHorizontalHeaderLabels({tr("A280"), tr("Yield volume (ml)"), tr("[Protein] (mg/ml)"), tr("Yield (mg)")});
		table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		table->setMinimumHeight(220);

		QHBoxLayout* buttons = new QHBoxLayout;
		QPushButton* add = new QPushButton(tr("Add reading"));
		QPushButton* remove = new QPushButton(tr("Remove selected"));
		QPushButton* import = new QPushButton(tr("Import…"));
		QPushButton* clear = new QPushButton(tr("Clear"));
		QPushButton* save = new QPushButton(tr("Save report…"));
		buttons->addWidget(add);
		buttons->addWidget(remove);
		buttons->addWidget(import);
		buttons->addWidget(clear);
		buttons->addStretch();
		buttons->addWidget(save);

		status = ui::noteLabel();

		body->addLayout(f);
		body->addWidget(table, 1);
		body->addLayout(buttons);
		body->addWidget(status);

		watch(this, {t, s, x}, [this]() { update(); });
		connect(table, &QTableWidget::itemChanged, this, [this]() { update(); });
		connect(add, &QPushButton::clicked, this, [this]() {
			addRow(QString(), QString());
			table->setCurrentCell(table->rowCount()-1, 0);
			table->editItem(table->item(table->rowCount()-1, 0));
		});
		connect(remove, &QPushButton::clicked, this, [this]() { removeSelected(); });
		connect(clear, &QPushButton::clicked, this, [this]() { table->setRowCount(0); update(); });
		connect(import, &QPushButton::clicked, this, [this]() { importFile(); });
		connect(save, &QPushButton::clicked, this, [this]() { saveReport(); });

		for (int i=0; i<3; ++i) addRow(QString(), QString());
	}

private:
	/* Appends a row without recalculating */
	void insertRowQuiet(const QString& a280, const QString& volume)
	{
		table->blockSignals(true);
		int r = table->rowCount();
		table->insertRow(r);
		table->setItem(r, 0, new QTableWidgetItem(a280));
		table->setItem(r, 1, new QTableWidgetItem(volume));
		for (int c=2; c<4; ++c)
		{
			QTableWidgetItem* it = new QTableWidgetItem;
			it->setFlags(it->flags() & ~Qt::ItemIsEditable);
			table->setItem(r, c, it);
		}
		table->blockSignals(false);
	}

	void addRow(const QString& a280, const QString& volume)
	{
		insertRowQuiet(a280, volume);
		update();
	}

	void removeSelected()
	{
		QList<int> rows;
		for (QTableWidgetItem* it : table->selectedItems())
			if (!rows.contains(it->row())) rows.append(it->row());
		std::sort(rows.begin(), rows.end(), std::greater<int>());
		for (int r : rows) table->removeRow(r);
		update();
	}

	/* Collects complete rows. Returns false if any row is partly filled in or not a number. */
	bool collect(std::vector<pmate::Reading>& readings, std::vector<int>* rowOf = nullptr)
	{
		bool ok = true;
		for (int r=0; r<table->rowCount(); ++r)
		{
			QString at = table->item(r, 0)->text().trimmed();
			QString vt = table->item(r, 1)->text().trimmed();
			if (at.isEmpty() && vt.isEmpty()) continue;

			pmate::Reading rd;
			if (!pmate::parseFloat(at.toStdString(), rd.a280) || !pmate::parseFloat(vt.toStdString(), rd.volume))
			{
				ok = false;
				continue;
			}
			readings.push_back(rd);
			if (rowOf) rowOf->push_back(r);
		}
		return ok;
	}

	void update()
	{
		dil->setText(ui::num(pmate::dilutionFactor(t->value(), s->value())));

		std::vector<pmate::Reading> readings;
		std::vector<int> rowOf;
		bool ok = collect(readings, &rowOf);
		std::vector<pmate::ProteinConc> res = pmate::proteinConcBatch(readings, t->value(), s->value(), x->value());

		table->blockSignals(true);
		for (int r=0; r<table->rowCount(); ++r)
		{
			table->item(r, 2)->setText(QString());
			table->item(r, 3)->setText(QString());
		}
		for (size_t i=0; i<res.size(); ++i)
		{
			table->item(rowOf[i], 2)->setText(ui::num(res[i].conc));
			table->item(rowOf[i], 3)->setText(ui::num(res[i].amount));
		}
		table->blockSignals(false);

		status->setText(ok ? tr("%n complete reading(s).", nullptr, int(readings.size()))
						   : tr("Some rows are incomplete or contain something other than a number."));
	}

	void importFile()
	{
		QString path = QFileDialog::getOpenFileName(this, tr("Import A280 readings"), ui::lastDir(),
			tr("Text files (*.txt *.dat *.csv);;All files (*)"));
		if (path.isEmpty()) return;
		ui::rememberDir(path);

		std::string data;
		if (!ui::readFileBytes(this, path, data, 64 * 1024 * 1024)) return;
		std::istringstream in(data);
		std::vector<pmate::Reading> readings;
		bool truncated = pmate::parseReadings(in, readings);

		table->setRowCount(0);
		table->setUpdatesEnabled(false);
		for (const pmate::Reading& rd : readings) insertRowQuiet(ui::num(rd.a280), ui::num(rd.volume));
		table->setUpdatesEnabled(true);
		update();

		if (truncated)
			QMessageBox::warning(this, tr("Import A280 readings"), tr("Only the first %1 readings were used.").arg(pmate::MAX_READINGS));
		else if (readings.empty())
			QMessageBox::warning(this, tr("Import A280 readings"), tr("No \"A280 volume\" pairs were found in %1.").arg(path));
	}

	void saveReport()
	{
		std::vector<pmate::Reading> readings;
		if (!collect(readings) || readings.empty())
		{
			QMessageBox::warning(this, tr("Save report"), tr("Fill in at least one complete reading, and fix any incomplete rows, first."));
			return;
		}
		std::vector<pmate::ProteinConc> res = pmate::proteinConcBatch(readings, t->value(), s->value(), x->value());
		QString path = ui::saveText(this, tr("Save report"), QStringLiteral("protein_yield.txt"), pmate::formatReadingsReport(readings, res));
		if (!path.isEmpty()) status->setText(tr("Saved to %1").arg(path));
	}

	QDoubleSpinBox *t, *s, *x;
	QLabel *dil, *status;
	QTableWidget* table;
};

/* ---- Extinction coefficient (menu item 6) ---- */

class ExtinctionPage : public Page
{
public:
	ExtinctionPage()
		: Page(tr("Extinction Coefficient"),
			   tr("Calculate a native protein's extinction coefficient in water from its Tyr, Trp and Cys content. "
				  "To work these out from a sequence, use Sequence Analysis."))
	{
		QFormLayout* f = new QFormLayout;
		tyr = ui::intSpin(0, 1000000, 0);
		trp = ui::intSpin(0, 1000000, 0);
		cys = ui::intSpin(0, 1000000, 0);
		mw = ui::doubleSpin(1, 1e9, 10000, 2, tr(" Da"));
		f->addRow(tr("Tyrosines:"), tyr);
		f->addRow(tr("Tryptophans:"), trp);
		f->addRow(tr("Cysteines:"), cys);
		f->addRow(tr("Molecular weight:"), mw);

		QGroupBox* g = new QGroupBox(tr("Results"));
		QFormLayout* r = new QFormLayout(g);
		ext = ui::resultLabel();
		abs = ui::resultLabel();
		r->addRow(tr("Extinction coefficient:"), ext);
		r->addRow(tr("Absorbance (280 nm, 1 mg/ml, 1 cm):"), abs);

		QPushButton* save = new QPushButton(tr("Save report…"));
		status = ui::noteLabel();
		QHBoxLayout* h = new QHBoxLayout;
		h->addStretch();
		h->addWidget(save);

		body->addLayout(f);
		body->addWidget(g);
		body->addLayout(h);
		body->addWidget(status);
		body->addStretch();

		watch(this, {tyr, trp, cys}, [this]() { update(); });
		watch(this, {mw}, [this]() { update(); });
		connect(save, &QPushButton::clicked, this, [this]() {
			QString path = ui::saveText(this, tr("Save report"), QStringLiteral("extinction.txt"), pmate::formatExtReport(result()));
			if (!path.isEmpty()) status->setText(tr("Saved to %1").arg(path));
		});
		update();
	}

private:
	pmate::ExtResult result() const
	{
		return pmate::extinction(tyr->value(), trp->value(), cys->value(), mw->value());
	}

	void update()
	{
		pmate::ExtResult r = result();
		ext->setText(ui::num(r.ext));
		abs->setText(ui::num(r.abs));
	}

	QSpinBox *tyr, *trp, *cys;
	QDoubleSpinBox* mw;
	QLabel *ext, *abs, *status;
};

/* ---- Nucleic acid concentration from A260 (menu item 8) ---- */

class NucleicPage : public Page
{
public:
	NucleicPage()
		: Page(tr("Nucleic Acid Concentration (A260)"),
			   tr("Calculate the concentration of dsDNA, RNA or an oligonucleotide from its absorbance at 260 nm."))
	{
		QFormLayout* f = new QFormLayout;
		type = new QComboBox;
		type->addItem(tr("dsDNA"), int(pmate::NucleicType::DsDNA));
		type->addItem(tr("RNA"), int(pmate::NucleicType::RNA));
		type->addItem(tr("Oligonucleotide"), int(pmate::NucleicType::Oligo));
		od = ui::doubleSpin(0, 1000, 0.5, 3);
		dil = ui::intSpin(1, 1000000, 1);
		result = ui::resultLabel();
		warning = ui::noteLabel(tr("Results may not be accurate in this absorbance range (reliable range 0.1 – 10)."));
		warning->setStyleSheet(QStringLiteral("color: #b35900;"));
		f->addRow(tr("Nucleic acid:"), type);
		f->addRow(tr("OD 260:"), od);
		f->addRow(tr("Dilution factor:"), dil);
		f->addRow(tr("Concentration:"), result);

		body->addLayout(f);
		body->addWidget(warning);
		body->addStretch();

		connect(type, &QComboBox::currentIndexChanged, this, [this]() { update(); });
		watch(this, {od}, [this]() { update(); });
		watch(this, {dil}, [this]() { update(); });
		update();
	}

private:
	void update()
	{
		pmate::NucleicType t = pmate::NucleicType(type->currentData().toInt());
		result->setText(ui::num(pmate::nucleicConc(t, od->value(), dil->value())) + QLatin1Char(' ') + QString::fromLatin1(pmate::nucleicUnit(t)));
		warning->setVisible(!pmate::od260InRange(od->value()));
	}

	QComboBox* type;
	QDoubleSpinBox* od;
	QSpinBox* dil;
	QLabel *result, *warning;
};

/* ---- Percent to molar concentration ---- */

class PercentMolarPage : public Page
{
public:
	PercentMolarPage()
		: Page(tr("Mass/Volume Percent to Molar Concentration"),
			   tr("Convert a mass concentration (% w/v) or a volume percent (% v/v) to a molar concentration."))
	{
		wv = new QRadioButton(tr("Mass concentration (% w/v)"));
		vv = new QRadioButton(tr("Volume percent (% v/v)"));
		wv->setChecked(true);
		QButtonGroup* group = new QButtonGroup(this);
		group->addButton(wv);
		group->addButton(vv);

		QFormLayout* f = new QFormLayout;
		per = ui::doubleSpin(0, 1e6, 5, 3, tr(" %"));
		fw = ui::doubleSpin(0.0001, 1e7, 58.44, 4, tr(" g/mol"));
		density = ui::doubleSpin(0.0001, 100, 1, 4, tr(" g/ml"));
		result = ui::resultLabel();
		f->addRow(QString(), wv);
		f->addRow(QString(), vv);
		f->addRow(tr("Percentage:"), per);
		f->addRow(tr("Formula weight:"), fw);
		f->addRow(tr("Density:"), density);
		f->addRow(tr("Molar concentration:"), result);

		body->addLayout(f);
		body->addStretch();

		connect(group, &QButtonGroup::buttonToggled, this, [this]() { update(); });
		watch(this, {per, fw, density}, [this]() { update(); });
		update();
	}

private:
	void update()
	{
		density->setEnabled(vv->isChecked());
		pmate::Molar m = vv->isChecked()
			? pmate::molarFromVV(per->value(), fw->value(), density->value())
			: pmate::molarFromWV(per->value(), fw->value());
		result->setText(ui::num2(m.molar) + tr(" M = ") + ui::num2(m.mmolar) + tr(" mM"));
	}

	QRadioButton *wv, *vv;
	QDoubleSpinBox *per, *fw, *density;
	QLabel* result;
};

/* ---- Body mass index (menu item 10) ---- */

class BmiPage : public Page
{
public:
	BmiPage()
		: Page(tr("Body Mass Index"),
			   tr("Reference values from the CDC (http://www.cdc.gov/nccdphp/dnpa/bmi):\n"
				  "BMI < 18.5: Underweight   ·   18.5 – 24.9: Normal   ·   25.0 – 29.9: Overweight   ·   30 and above: Obese"))
	{
		QFormLayout* f = new QFormLayout;
		ft = ui::doubleSpin(0, 10, 5, 0, tr(" ft"));
		in = ui::doubleSpin(0, 11.99, 9, 1, tr(" in"));
		lb = ui::doubleSpin(1, 2000, 150, 1, tr(" lb"));
		f->addRow(tr("Height:"), ft);
		f->addRow(QString(), in);
		f->addRow(tr("Weight:"), lb);

		QGroupBox* g = new QGroupBox(tr("Results"));
		QFormLayout* r = new QFormLayout(g);
		metres = ui::resultLabel();
		kg = ui::resultLabel();
		bmi = ui::resultLabel();
		ideal = ui::resultLabel();
		r->addRow(tr("Height:"), metres);
		r->addRow(tr("Weight:"), kg);
		r->addRow(tr("Body mass index:"), bmi);
		r->addRow(tr("Ideal weight range:"), ideal);

		body->addLayout(f);
		body->addWidget(g);
		body->addStretch();

		watch(this, {ft, in, lb}, [this]() { update(); });
		update();
	}

private:
	void update()
	{
		pmate::BmiResult b = pmate::bmi(ft->value(), in->value(), lb->value());
		metres->setText(ui::num2(b.m) + tr(" m"));
		kg->setText(ui::num2(b.kg) + tr(" kg"));
		QString outcome = QString::fromStdString(b.outcome);
		bmi->setText(ui::num2(b.bmi) + tr(" kg/m²") + (outcome.isEmpty() ? QString() : QStringLiteral("   [ %1 ]").arg(outcome)));
		ideal->setText(ui::num(b.lweight) + tr(" lb to ") + ui::num(b.hweight) + tr(" lb"));
	}

	QDoubleSpinBox *ft, *in, *lb;
	QLabel *metres, *kg, *bmi, *ideal;
};

/* ---- Factorial (menu item 11) ---- */

class FactorialPage : public Page
{
public:
	FactorialPage()
		: Page(tr("Factorial"), tr("Calculate n! for an integer from 0 to %1.").arg(pmate::MAX_FACT))
	{
		QFormLayout* f = new QFormLayout;
		n = ui::intSpin(0, pmate::MAX_FACT, 5);
		result = ui::resultLabel();
		f->addRow(tr("n:"), n);
		f->addRow(tr("n! ="), result);

		body->addLayout(f);
		body->addStretch();

		watch(this, {n}, [this]() { update(); });
		update();
	}

private:
	void update()
	{
		std::optional<uint64_t> r = pmate::factorial(n->value());
		result->setText(r ? QString::number(qulonglong(*r)) : QStringLiteral("—"));
	}

	QSpinBox* n;
	QLabel* result;
};

}

Page* createWeightVolumePage() { return new WeightVolumePage; }
Page* createProteinConcPage() { return new ProteinConcPage; }
Page* createMultiReadingPage() { return new MultiReadingPage; }
Page* createExtinctionPage() { return new ExtinctionPage; }
Page* createNucleicPage() { return new NucleicPage; }
Page* createPercentMolarPage() { return new PercentMolarPage; }
Page* createBmiPage() { return new BmiPage; }
Page* createFactorialPage() { return new FactorialPage; }
