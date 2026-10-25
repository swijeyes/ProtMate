/*********************************************************
 ProtMate core library.
 Developed by Sanjeeva Wijeyesakere.
*********************************************************/

#include "pmate_core.h"

#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <istream>
#include <ostream>
#include <sstream>

using namespace std;

namespace pmate {

const char* const VERSION = "1.2.0";
const char* const COPYRIGHT = "Copyright (C) 2024-2026 Sanjeeva Wijeyesakere";
const char* const HOMEPAGE = "https://github.com/swijeyes/ProtMate";

/* General helpers */

float round2(float x)
{
	double p;	/*Product (input number * 100)*/
	double f;	/*Result of Product - Whole*/
	double w;	/*Whole number*/

	p=100.0*x;
	if (!isfinite(p)) return x;
	w=trunc(p);
	f=p-w;
	if (2*f>=1) return ((w+1)/100.0);
	else return w / 100.0;
}

string fmtNum(double x)
{
	ostringstream s;
	s << x;
	return s.str();
}

bool isSafeFilename(const string& name)
{
	if (name.empty() || name[0] == '-') return 0;
	for (size_t i=0; i<name.size(); ++i)
	{
		unsigned char ch = name[i];
		if (!(isalnum(ch) || ch == '.' || ch == '_' || ch == '-' || ch == '/' || ch == '+')) return 0;
	}
	return 1;
}

bool isSafeConfigValue(const string& v)
{
	if (v.empty()) return 0;
	for (size_t i=0; i<v.size(); ++i)
	{
		unsigned char ch = v[i];
		if (ch < 0x20 || ch == 0x7f || ch == '#') return 0;
	}
	return 1;
}

bool parseFloat(const string& s, float& out)
{
	if (s.empty()) return 0;
	char* end = NULL;
	errno = 0;
	float v = strtof(s.c_str(), &end);
	if (errno != 0 || end == s.c_str() || *end != '\0' || !isfinite(v)) return 0;
	out = v;
	return 1;
}

char printable(char ch)
{
	return isprint(static_cast<unsigned char>(ch)) ? ch : '?';
}

bool readSequence(istream& in, vector<char>& seq)
{
	char r;
	while (in >> r)
	{
		if (seq.size() >= MAX_SEQ_LEN) return 0;
		seq.push_back(r);
	}
	return 1;
}

/* Weight to volume ratio */

float massForWV(float wv, float vol)
{
	return (wv/100)*vol;
}

float volumeForWV(float wv, float grams)
{
	return (grams/(wv/100));
}

/* Protein concentration from A280 */

float dilutionFactor(float cuvTotal, float cuvProt)
{
	return cuvTotal/cuvProt;
}

ProteinConc proteinConc(float a280, float cuvTotal, float cuvProt, float extCoef, float totalVol)
{
	ProteinConc r;
	r.dilution = dilutionFactor(cuvTotal, cuvProt);
	r.conc = round2((a280*r.dilution)/extCoef);
	r.amount = r.conc*totalVol;
	return r;
}

bool parseReadings(istream& in, vector<Reading>& readings)
{
	Reading rd;
	while (in >> rd.a280 >> rd.volume)
	{
		if ((int)readings.size() >= MAX_READINGS) return 1;
		readings.push_back(rd);
	}
	return 0;
}

vector<ProteinConc> proteinConcBatch(const vector<Reading>& readings, float cuvTotal, float cuvProt, float extCoef)
{
	vector<ProteinConc> results;
	for (size_t i=0; i<readings.size(); ++i)
		results.push_back(proteinConc(readings[i].a280, cuvTotal, cuvProt, extCoef, readings[i].volume));
	return results;
}

string formatReadingsReport(const vector<Reading>& readings, const vector<ProteinConc>& results)
{
	ostringstream s;
	s << "Sample #\tA280\tDil.\t[Protein] (mg/ml)\tYield (mg)\n";
	for (size_t r=0; r<readings.size() && r<results.size(); ++r)
		s << r+1 << '\t' << readings[r].a280 << '\t' << results[r].dilution << '\t' << results[r].conc << '\t' << results[r].amount << "\n";
	return s.str();
}

/* Extinction coefficient */

ExtResult extinction(float ntyr, float ntrp, float ncys, float mw)
{
	const int etyr = 1490;
	const int etrp = 5500;
	const int ecys = 125;

	ExtResult r;
	float ext = (double(ntyr) * etyr) + (double(ntrp) * etrp) + (double(ncys) * ecys);
	r.abs = ext / mw;
	r.ext = round2(ext);
	return r;
}

string formatExtReport(const ExtResult& r)
{
	ostringstream s;
	s << "\nExtinction coefficient of the native protein in water: " << r.ext << "\n"
	  << "Absorbance (at 280nm & 1 mg/ml) assuming path length of 1 cm: " << r.abs << "\n";
	return s.str();
}

/* Protein sequence analysis */

namespace {

struct Residue
{
	char code;
	const char* name;
	int c, n, o, h, s;	/* Atoms contributed by the residue */
};

/* Display order of the composition table; index NUM_RESIDUE_TYPES-1 is "Unk" */
const Residue RESIDUES[NUM_RESIDUE_TYPES-1] =
{
	{'y', "Tyr", 9, 1, 2,  9, 0},
	{'w', "Trp", 11, 2, 1, 10, 0},
	{'c', "Cys", 3, 1, 1,  5, 1},
	{'g', "Gly", 2, 1, 1,  3, 0},
	{'a', "Ala", 3, 1, 1,  5, 0},
	{'v', "Val", 5, 1, 1,  9, 0},
	{'l', "Leu", 6, 1, 1, 11, 0},
	{'i', "Ile", 6, 1, 1, 11, 0},
	{'s', "Ser", 3, 1, 2,  5, 0},
	{'t', "Thr", 4, 1, 2,  7, 0},
	{'m', "Met", 5, 1, 1,  9, 1},
	{'f', "Phe", 9, 1, 1,  9, 0},
	{'p', "Pro", 5, 1, 1,  7, 0},
	{'n', "Asn", 4, 2, 2,  6, 0},
	{'q', "Gln", 5, 2, 2,  8, 0},
	{'d', "Asp", 4, 1, 3,  5, 0},
	{'e', "Glu", 5, 1, 3,  7, 0},
	{'k', "Lys", 6, 2, 1, 12, 0},
	{'h', "His", 6, 3, 1,  7, 0},
	{'r', "Arg", 6, 4, 1, 12, 0}
};

const int TYR = 0, TRP = 1, CYS = 2, UNK = NUM_RESIDUE_TYPES-1;

int residueIndex(char code)
{
	char lc = tolower(static_cast<unsigned char>(code));
	for (int i=0; i<NUM_RESIDUE_TYPES-1; ++i)
		if (RESIDUES[i].code == lc) return i;
	return UNK;
}

}

const char* residueName(int index)
{
	if (index < 0 || index >= NUM_RESIDUE_TYPES) return "";
	if (index == UNK) return "Unk";
	return RESIDUES[index].name;
}

bool isKnownResidue(char code)
{
	return residueIndex(code) != UNK;
}

SeqAnalysis analyzeProtein(const vector<char>& seq, int start, int end)
{
	SeqAnalysis a;
	a.length = end - start + 1;
	for (int i=0; i<NUM_RESIDUE_TYPES; ++i) a.counts[i] = 0;
	a.carbon = a.hydrogen = a.oxygen = a.nitrogen = a.sulphur = 0;

	for (int tp = start; tp <= end; ++tp)
	{
		int i = residueIndex(seq[tp]);
		++a.counts[i];
		if (i == UNK) {a.unknownPositions.push_back(tp+1); continue;}
		a.carbon += RESIDUES[i].c;
		a.nitrogen += RESIDUES[i].n;
		a.oxygen += RESIDUES[i].o;
		a.hydrogen += RESIDUES[i].h;
		a.sulphur += RESIDUES[i].s;
	}

	/* Terminal water */
	a.hydrogen = a.hydrogen+2;
	a.oxygen = a.oxygen+1;

	float mw1 = (12.011*a.carbon) + (14.007*a.nitrogen) + (15.999*a.oxygen) + (32.066*a.sulphur) + (1.008*a.hydrogen);
	a.mw = round2(mw1);
	a.ext = extinction(a.counts[TYR], a.counts[TRP], a.counts[CYS], a.mw);
	return a;
}

string formatFormula(const SeqAnalysis& a)
{
	ostringstream s;
	s << "C("<<a.carbon<<")H("<<a.hydrogen<<")O("<<a.oxygen<<")N("<<a.nitrogen<<")S("<<a.sulphur<<")";
	return s.str();
}

string formatComposition(const SeqAnalysis& a)
{
	ostringstream s;
	s << "Your protein's Amino Acid composition is as follows:\n\n"
	  << "Res\tNum\t%\n"
	  << "======================\n";
	for (int i=0; i<NUM_RESIDUE_TYPES; ++i)
		s << residueName(i) << ":\t" << a.counts[i] << '\t' << round2(100*(a.counts[i]/a.length)) << "%" << "\n";
	return s.str();
}

string formatProteinReport(const SeqAnalysis& a, const string& name, const string& user)
{
	ostringstream s;
	s << "******************************************" << "\n"
	  << "|  PROTMATE REPORTER                     |\n"
	  << "******************************************\n\n";
	s << "Sequence analysis report for " << name << "." << "\n"
	  << "Operator: "<< user << "\n" << "\n";
	s << formatComposition(a);
	s << "\nFormula of protein/peptide: " << formatFormula(a) << "\n";
	s << "\nMolecular weight of peptide (da): " << a.mw << "\n";
	s << "\n"
	  << "Extinction coefficient of the native protein in water: "<< a.ext.ext << "\n";
	s << "Absorbance (at 280nm at 1 mg/ml) assuming path length of 1 cm: "
	  << a.ext.abs << "\n";
	return s.str();
}

/* Concentration of a nucleic acid from A260 */

float nucleicConc(NucleicType type, float od, int dilution)
{
	int factor = 50;
	if (type == NucleicType::RNA) factor = 40;
	else if (type == NucleicType::Oligo) factor = 20;

	od=od*dilution;
	return od*factor;
}

bool od260InRange(float od)
{
	return !(od < 0.1 || od > 10);
}

const char* nucleicUnit(NucleicType type)
{
	if (type == NucleicType::RNA) return "ug ssRNA / ml";
	if (type == NucleicType::Oligo) return "ug ss Oligonucleotide / ml";
	return "ug dsDNA / ml";
}

/* Primer melting point */

NucleotideCounts countNucleotides(const vector<char>& seq)
{
	NucleotideCounts n = {0, 0, 0, 0, 0, 0};
	for (size_t z=0; z<seq.size(); ++z)
	{
		char x = seq[z];

		if (x == 'a' || x == 'A') ++n.a;
		else if (x == 't' || x == 'T') ++n.t;
		else if (x == 'g' || x == 'G') ++n.g;
		else if (x == 'c' || x == 'C') ++n.c;
		else
		{
			n.badPosition = z+1;
			n.badBase = x;
			break;
		}
	}
	return n;
}

MeltResult meltPoint(float length, float gc, float salt)
{
	MeltResult r;
	float n = length;

	if (n < 14)
	{
		float at = n-gc;
		r.tm = (4*gc)+(2*at);
	}
	else
	{
		r.tm = 64.9+(41*((gc-16.4)/n));
	}

	r.pgc = (gc/n) *100;

	float div = (675/n);
	r.adtm = 81.5 + (16.6*log10(salt)) + (0.41*r.pgc) - div;
	return r;
}

string formatMeltReport(const MeltResult& r, const NucleotideCounts* counts)
{
	ostringstream s;
	if (counts != nullptr)
	{
		s << "Number of Adenines:\t" << counts->a << "\n";
		s << "Number of Thymines:\t" << counts->t << "\n";
		s << "Number of Guanines:\t" << counts->g << "\n";
		s << "Number of Cytosines:\t" << counts->c << "\n";
	}
	s << "\n% G-C content: " << r.pgc << "%\n";
	s << "\nMelting point of primer = " << round2(r.tm) << " C"
	  << "\nSalt-adjusted melting point of primer = " << round2(r.adtm) << " C" << "\n";
	return s.str();
}

/* Percent to molar concentration */

Molar molarFromWV(float per, float fw)
{
	Molar m;
	m.molar = (per/(0.1*fw));
	m.mmolar = 1000*m.molar;
	return m;
}

Molar molarFromVV(float per, float fw, float density)
{
	float weight = density*per;
	return molarFromWV(weight, fw);
}

/* Body mass index */

BmiResult bmi(float ft, float in, float lb)
{
	BmiResult r;
	r.m = ((30*ft) + (2.54 * in))/100;
	r.kg = (lb/2.2);
	r.bmi = r.kg / (r.m*r.m);

	r.lweight = (r.m*r.m) * 18.5;
	r.lweight = round2(r.lweight*2.2);
	r.hweight = (r.m*r.m) * 24.9;
	r.hweight = round2(r.hweight*2.2);

	if (r.bmi < 18.5) r.outcome = "Underweight";
	if (r.bmi >= 18.5 && r.bmi < 24.9) r.outcome = "Normal";
	if (r.bmi > 24.9 && r.bmi < 29.9) r.outcome = "Overweight";
	if (r.bmi > 29.9) r.outcome = "Obese";
	return r;
}

/* Factorial */

optional<uint64_t> factorial(int n)
{
	if (n < 0 || n > MAX_FACT) return nullopt;
	uint64_t r = 1;
	for (int i=2; i<=n; ++i) r *= i;
	return r;
}

/* Oscillation file renaming */

vector<pair<string, string> > renamePlan(const string& fstr, const string& dfstr, int start, int count)
{
	vector<pair<string, string> > plan;
	char pad[16];
	for (int n=0; n<count; ++n)
	{
		snprintf(pad, sizeof(pad), "%03d", start+n);
		plan.push_back(make_pair(fstr + "0" + pad, dfstr + pad + ".osc"));
	}
	return plan;
}

/* AutoDock Vina configuration file */

string vinaConfig(const VinaParams& p)
{
	ostringstream s;
	s << "receptor = " << p.receptor << "\n"
	  << "ligand = " << p.ligand << "\n" << "\n"
	  << "out = all.pdbqt\n\n"
	  << "center_x = " << p.x << "\n"
	  << "center_y = " << p.y << "\n"
	  << "center_z = " << p.z << "\n" << "\n"
	  << "size_x = " << p.grid << "\n"
	  << "size_y = " << p.grid << "\n"
	  << "size_z = " << p.grid << "\n" << "\n"
	  << "exhaustiveness = 8";
	return s.str();
}

/* PDBQT to PDB */

void pdbqtToPdb(istream& in, ostream& out)
{
	string l;
	while (getline(in, l))
	{
		string conv = l.substr(0, 60);
		if (l.size() > 69) conv += l.substr(69, 10);
		out << conv << '\n';
	}
}

/* X-PLOR code to solvate a protein */

namespace {

string xplorHeader(const string& pmtf, const string& ppdb, const string& wmtf, const string& wpdb)
{
	ostringstream s;
	s << "structure @ " << pmtf << " end\n"
	  << "coordinates @ " << ppdb << "\n"
	  << "structure @ " << wmtf << " end\n"
	  << "coordinates @ " << wpdb << "\n" << "\n";
	return s.str();
}

}

string xplorStage1(const string& pmtf, const string& ppdb, const string& wmtf, const string& wpdb)
{
	return xplorHeader(pmtf, ppdb, wmtf, wpdb)
		+ "write coordinates output=overlay1.pdb\n"
		  "  selection=\n"
		  "        ( byres\n"
		  "          ( segid \"W   \"\n"
		  "                and ( ( not segid \"W   \" and not (hydrogen) ) around 6.0)\n"
		  "        ))\n"
		  "end\nstop\n";
}

string xplorStage2(const string& pmtf, const string& ppdb, const string& wmtf, const string& wpdb)
{
	return xplorHeader(pmtf, ppdb, wmtf, wpdb)
		+ "delete\n"
		  "  selection=\n"
		  "        ( byres\n"
		  "          ( segid \"W   \"\n"
		  "                and ( ( not segid \"W   \" and not (hydrogen) ) around 2.3)\n"
		  "        ))\n"
		  "end\n"
		  "\nwrite coordinates output=overlay2.pdb end\n"
		  "stop\n";
}

}
