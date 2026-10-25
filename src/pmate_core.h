/*********************************************************
 ProtMate core library.
 Developed by Sanjeeva Wijeyesakere.

 The calculations behind every ProtMate module, free of any
 user interface code so that the terminal and graphical front
 ends share exactly the same results and report formats.
 Standard C++17 only: no console I/O, no platform calls.
*********************************************************/

#ifndef PMATE_CORE_H
#define PMATE_CORE_H

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace pmate {

/* Constants */

extern const char* const VERSION;
extern const char* const COPYRIGHT;	/* "Copyright (C) <years> <author>" */
extern const char* const HOMEPAGE;

const int MAX_READINGS = 100000;	/* Upper bound on A280 readings held in memory */
const size_t MAX_SEQ_LEN = 1000000;	/* Upper bound on residues read from a .seq file */
const int MAX_FACT = 20;			/* 20! is the largest factorial that fits in 64 bits */

/* General helpers */

float round2(float x);							/* Rounds to 2 decimal places */
std::string fmtNum(double x);					/* Formats a number exactly as std::ostream does by default */
bool isSafeFilename(const std::string& name);	/* Conservative filename characters, no leading '-' */
bool isSafeConfigValue(const std::string& v);	/* No control characters or '#' (for config files) */
bool parseFloat(const std::string& s, float& out);
char printable(char ch);						/* Non-printable characters become '?' */

/* Reads every non-whitespace character. Returns false if MAX_SEQ_LEN is exceeded. */
bool readSequence(std::istream& in, std::vector<char>& seq);

/* Weight to volume ratio (menu items 1 and 2) */

float massForWV(float wv, float vol);		/* g = vol * ((w/v)/100) */
float volumeForWV(float wv, float grams);	/* vol = g / ((w/v)/100) */

/* Protein concentration from A280 (menu items 3, 4 and 5) */

struct ProteinConc
{
	float dilution;	/* Dilution factor in the cuvette */
	float conc;		/* mg/ml, rounded to 2 decimal places */
	float amount;	/* mg present in the given volume */
};

ProteinConc proteinConc(float a280, float cuvTotal, float cuvProt, float extCoef, float totalVol);

struct Reading
{
	float a280;
	float volume;
};

/* Reads whitespace-separated (A280, volume) pairs. Returns true if it stopped at MAX_READINGS. */
bool parseReadings(std::istream& in, std::vector<Reading>& readings);

float dilutionFactor(float cuvTotal, float cuvProt);
std::vector<ProteinConc> proteinConcBatch(const std::vector<Reading>& readings, float cuvTotal, float cuvProt, float extCoef);
std::string formatReadingsReport(const std::vector<Reading>& readings, const std::vector<ProteinConc>& results);

/* Extinction coefficient (menu item 6) */

struct ExtResult
{
	float ext;	/* Extinction coefficient, rounded to 2 decimal places */
	float abs;	/* Absorbance at 280nm, 1 mg/ml, 1 cm */
};

ExtResult extinction(float ntyr, float ntrp, float ncys, float mw);
std::string formatExtReport(const ExtResult& r);

/* Protein sequence analysis (menu item 7) */

const int NUM_RESIDUE_TYPES = 21;	/* 20 amino acids plus "Unk" */

struct SeqAnalysis
{
	int length;							/* Residues analysed (end - start + 1) */
	float counts[NUM_RESIDUE_TYPES];	/* In the order of residueName() */
	int carbon, hydrogen, oxygen, nitrogen, sulphur;
	float mw;							/* Daltons, rounded to 2 decimal places */
	ExtResult ext;
	std::vector<int> unknownPositions;	/* 1-based */
};

const char* residueName(int index);		/* "Tyr", "Trp", ... "Unk" */
bool isKnownResidue(char code);

/* Analyses seq[start..end] (0-based, inclusive). The caller must ensure 0 <= start <= end < seq.size(). */
SeqAnalysis analyzeProtein(const std::vector<char>& seq, int start, int end);

std::string formatFormula(const SeqAnalysis& a);		/* "C(..)H(..)O(..)N(..)S(..)" */
std::string formatComposition(const SeqAnalysis& a);	/* Heading plus the residue table */
std::string formatProteinReport(const SeqAnalysis& a, const std::string& name, const std::string& user);

/* Concentration of a nucleic acid from A260 (menu item 8) */

enum class NucleicType { DsDNA, RNA, Oligo };

float nucleicConc(NucleicType type, float od, int dilution);	/* ug/ml */
bool od260InRange(float od);									/* Reliable absorbance range */
const char* nucleicUnit(NucleicType type);						/* e.g. "ug dsDNA / ml" */

/* Primer melting point (menu item 9) */

struct NucleotideCounts
{
	int a, t, g, c;
	int badPosition;	/* 1-based position of the first unknown base, or 0 */
	char badBase;
};

struct MeltResult
{
	float pgc;	/* % G-C content */
	float tm;	/* Melting point, C */
	float adtm;	/* Salt-adjusted melting point, C */
};

NucleotideCounts countNucleotides(const std::vector<char>& seq);
MeltResult meltPoint(float length, float gc, float salt);	/* salt in M, must be > 0 */
std::string formatMeltReport(const MeltResult& r, const NucleotideCounts* counts = nullptr);

/* Percent to molar concentration */

struct Molar
{
	float molar;
	float mmolar;
};

Molar molarFromWV(float per, float fw);
Molar molarFromVV(float per, float fw, float density);

/* Body mass index (menu item 10) */

struct BmiResult
{
	float m;			/* Height, metres */
	float kg;			/* Weight, kilograms */
	float bmi;
	float lweight;		/* Lower bound of the ideal weight range, lb */
	float hweight;		/* Upper bound of the ideal weight range, lb */
	std::string outcome;
};

BmiResult bmi(float ft, float in, float lb);

/* Factorial (menu item 11) */

std::optional<uint64_t> factorial(int n);	/* Empty unless 0 <= n <= MAX_FACT */

/* Oscillation file renaming (menu item 12): (old name, new name) pairs */

std::vector<std::pair<std::string, std::string> > renamePlan(const std::string& fstr, const std::string& dfstr, int start, int count);

/* AutoDock Vina configuration file (menu item 13) */

struct VinaParams
{
	std::string receptor;
	std::string ligand;
	float x, y, z;
	float grid;
};

std::string vinaConfig(const VinaParams& p);

/* PDBQT to PDB (menu item 14): equivalent of `cut -c 1-60,70-79` */

void pdbqtToPdb(std::istream& in, std::ostream& out);

/* X-PLOR code to solvate a protein (menu item 15) */

std::string xplorStage1(const std::string& pmtf, const std::string& ppdb, const std::string& wmtf, const std::string& wpdb);
std::string xplorStage2(const std::string& pmtf, const std::string& ppdb, const std::string& wmtf, const std::string& wpdb);

}

#endif
