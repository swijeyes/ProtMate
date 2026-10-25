/*********************************************************
 ProtMate (Laboratory formula calculator).
 Developed by Sanjeeva Wijeyesakere.

 This program is used to calculate values commonly used
 in the laboratory setting.

 Terminal front end. The calculations themselves live in
 the shared core library (core/pmate_core.h). Builds on
 POSIX systems and on Windows (MinGW or MSVC).
*********************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <cstdio>
#include <cerrno>
#include <climits>
#include <cmath>
#include <ctime>
#include <limits>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#endif
#include "pmate_core.h"

using namespace std;

/* Global Variables */

string rasmol1="";
bool rasmol=0;
string date = "";
bool dspmenu = 0;
bool cls = 0;
bool dsptle = 1;
string user="";
bool fmnu = 0;
bool privileged = 0;

/*Functions: */

void menu();
void select();
int waitKey();
void help();
void about();
void p1();
void p2();
void p3();
void p4();
void p5();
void ext();
void ext3();
void sys();
void open();
void overlay();
void od260();
void melt();
void rname();
void bmi();
void bmiinfo();
void vina();
void pdbqt();
int fact();
void percentmole();

/* Platform helpers: */

int runProgram(const vector<string>&);
string findInPath(const string&);
bool openOutFile(ofstream&, const string&);
bool readSequenceFile(const string&, vector<char>&);
void clearScreen();
void dropPrivileges();
bool pathExists(const string&);
string currentUser();
string currentDir();
string systemDescription();
#ifdef _WIN32
string systemProgram(const string&);
#endif

/* Runs an external program directly (no shell), so arguments are never
   interpreted as shell syntax. Returns the program's exit status, or -1. */
int runProgram(const vector<string>& args)
{
	if (args.empty()) return -1;

#ifdef _WIN32
	/* CreateProcess takes a single command line, so quote each argument the way the
	   C runtime splits it again. args[0] is always an absolute path, which stops
	   Windows from searching the current directory for the program. */
	string cmd;
	for (size_t i=0; i<args.size(); ++i)
	{
		const string& a = args[i];
		if (i) cmd += ' ';
		if (!a.empty() && a.find_first_of(" \t\n\v\"") == string::npos) {cmd += a; continue;}
		cmd += '"';
		for (size_t j=0; ; ++j)
		{
			size_t slashes = 0;
			while (j < a.size() && a[j] == '\\') {++j; ++slashes;}
			if (j == a.size()) {cmd.append(slashes * 2, '\\'); break;}
			if (a[j] == '"') cmd.append(slashes * 2 + 1, '\\');
			else cmd.append(slashes, '\\');
			cmd += a[j];
		}
		cmd += '"';
	}

	cout.flush();
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	vector<char> line(cmd.begin(), cmd.end());
	line.push_back('\0');
	if (!CreateProcessA(args[0].c_str(), &line[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return -1;
	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD code = 0;
	bool ok = GetExitCodeProcess(pi.hProcess, &code) != 0;
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return ok ? int(code) : -1;
#else
	vector<char*> argv;
	for (size_t i=0; i<args.size(); ++i) argv.push_back(const_cast<char*>(args[i].c_str()));
	argv.push_back(NULL);

	cout.flush();
	pid_t pid = fork();
	if (pid < 0) return -1;
	if (pid == 0)
	{
		execvp(argv[0], &argv[0]);
		_exit(127);
	}

	int status = 0;
	while (waitpid(pid, &status, 0) < 0)
	{
		if (errno != EINTR) return -1;
	}
	if (WIFEXITED(status)) return WEXITSTATUS(status);
	return -1;
#endif
}

/* Returns the absolute path of an executable found in an absolute PATH entry,
   or "" if it is not installed. Relative entries (incl. empty = cwd) are skipped. */
string findInPath(const string& name)
{
	const char* env = getenv("PATH");
	if (env == NULL) return "";

	stringstream path(env);
	string dir;
#ifdef _WIN32
	/* Only fully qualified entries (C:\\... or \\\\server\\...) are searched */
	while (getline(path, dir, ';'))
	{
		if (dir.size() >= 2 && dir[0] == '"' && dir[dir.size()-1] == '"') dir = dir.substr(1, dir.size()-2);
		bool drive = dir.size() >= 3 && isalpha((unsigned char)dir[0]) && dir[1] == ':' && (dir[2] == '\\' || dir[2] == '/');
		bool unc = dir.size() >= 2 && dir[0] == '\\' && dir[1] == '\\';
		if (!drive && !unc) continue;
		string full = dir + "\\" + name + ".exe";
		DWORD attr = GetFileAttributesA(full.c_str());
		if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) return full;
	}
#else
	while (getline(path, dir, ':'))
	{
		if (dir.empty() || dir[0] != '/') continue;
		string full = dir + "/" + name;
		struct stat st;
		if (stat(full.c_str(), &st) == 0 && S_ISREG(st.st_mode) && access(full.c_str(), X_OK) == 0) return full;
	}
#endif
	return "";
}

/* Opens an output file, refusing to write through a symbolic link and asking
   before an existing file is overwritten. */
bool openOutFile(ofstream& fout, const string& path)
{
#ifdef _WIN32
	DWORD attr = GetFileAttributesA(path.c_str());
	if (attr != INVALID_FILE_ATTRIBUTES)
	{
		if (attr & FILE_ATTRIBUTE_REPARSE_POINT) {cout << "ERR: " << path << " is a symbolic link - refusing to write to it.\n"; return 0;}
		if (attr & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE)) {cout << "ERR: " << path << " is not a regular file.\n"; return 0;}
#else
	struct stat st;
	if (lstat(path.c_str(), &st) == 0)
	{
		if (S_ISLNK(st.st_mode)) {cout << "ERR: " << path << " is a symbolic link - refusing to write to it.\n"; return 0;}
		if (!S_ISREG(st.st_mode)) {cout << "ERR: " << path << " is not a regular file.\n"; return 0;}
#endif

		string ow;
		cout << path << " already exists. Overwrite it (yes/no)? "; cin >> ow;
		if (ow != "yes" && ow != "YES") {cout << "Not overwriting " << path << ".\n"; return 0;}
	}

	/* Binary mode, so reports have the same LF line endings on every platform */
	fout.open(path.c_str(), ios::out | ios::binary);
	if (!fout) {cout << "ERR: Unable to open output file. Make sure you have read/write/execute permissions\n"
					 << "in this directory.\n"; return 0;}
	return 1;
}

/* Reads every non-whitespace character of a sequence file. */
bool readSequenceFile(const string& file, vector<char>& seq)
{
	ifstream in;
	in.open(file.c_str());
	if (!in) return 0;

	if (!pmate::readSequence(in, seq))
	{
		cout << "\nERR: Sequence exceeds " << pmate::MAX_SEQ_LEN << " residues.\n";
		return 0;
	}
	return 1;
}

void clearScreen()
{
#ifdef _WIN32
	/* Escape sequences only work in a console with virtual terminal processing on */
	DWORD mode = 0;
	if (GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &mode) && (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING))
		cout << "\033[H\033[2J" << flush;
#else
	if (isatty(STDOUT_FILENO)) cout << "\033[H\033[2J" << flush;
#endif
}

/* ProtMate never needs elevated privileges; if installed setuid/setgid, give them up. */
void dropPrivileges()
{
#ifdef _WIN32
	/* Privileges can't be dropped on Windows, but note an elevated (administrator) session */
	HANDLE token;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
	{
		TOKEN_ELEVATION elev;
		DWORD len = 0;
		if (GetTokenInformation(token, TokenElevation, &elev, sizeof(elev), &len)) privileged = (elev.TokenIsElevated != 0);
		CloseHandle(token);
	}
	/* Turn on escape sequence support for clearScreen() (Windows 10 and later) */
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD mode = 0;
	if (GetConsoleMode(out, &mode)) SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
	if (getegid() != getgid() && setgid(getgid()) != 0) {cerr << "Unable to drop group privileges\n"; exit(1);}
	if (geteuid() != getuid() && setuid(getuid()) != 0) {cerr << "Unable to drop user privileges\n"; exit(1);}
	if (geteuid() != getuid() || getegid() != getgid()) {cerr << "Unable to drop privileges\n"; exit(1);}
	privileged = (geteuid() == 0);
#endif
}

/* True if anything (including a dangling symbolic link) exists at path */
bool pathExists(const string& path)
{
#ifdef _WIN32
	return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
#else
	struct stat st;
	return lstat(path.c_str(), &st) == 0;
#endif
}

string currentUser()
{
#ifdef _WIN32
	char buf[257];
	DWORD len = sizeof(buf);
	if (GetUserNameA(buf, &len)) return buf;
#else
	struct passwd* pw = getpwuid(geteuid());
	if (pw != NULL && pw->pw_name != NULL) return pw->pw_name;
#endif
	return "";
}

string currentDir()
{
#ifdef _WIN32
	char buf[MAX_PATH * 4];
	DWORD n = GetCurrentDirectoryA(sizeof(buf), buf);
	if (n > 0 && n < sizeof(buf)) return buf;
#else
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd))) return cwd;
#endif
	return "";
}

string systemDescription()
{
#ifdef _WIN32
	/* RtlGetVersion reports the real version (GetVersionEx is capped by the manifest) */
	string sys = "Windows";
	typedef LONG (WINAPI *RtlGetVersionFn)(OSVERSIONINFOW*);
	HMODULE ntdll = GetModuleHandleA("ntdll.dll");
	RtlGetVersionFn getVersion = ntdll ? (RtlGetVersionFn)(void*)GetProcAddress(ntdll, "RtlGetVersion") : NULL;
	OSVERSIONINFOW vi;
	ZeroMemory(&vi, sizeof(vi)); vi.dwOSVersionInfoSize = sizeof(vi);
	char host[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD len = sizeof(host);
	if (GetComputerNameA(host, &len)) sys += string(" ") + host;
	if (getVersion && getVersion(&vi) == 0)
	{
		stringstream v;
		v << " " << vi.dwMajorVersion << "." << vi.dwMinorVersion << " (build " << vi.dwBuildNumber << ")";
		sys += v.str();
	}
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) sys += " x86_64";
	else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64) sys += " arm64";
	else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) sys += " x86";
	return sys;
#else
	struct utsname un;
	if (uname(&un) == 0) return string(un.sysname) + " " + un.nodename + " " + un.release + " " + un.version + " " + un.machine;
	return "";
#endif
}

#ifdef _WIN32
/* Absolute path of a program in the Windows system directory (never looked up via PATH) */
string systemProgram(const string& exe)
{
	char dir[MAX_PATH];
	UINT n = GetSystemDirectoryA(dir, MAX_PATH);
	if (n == 0 || n >= MAX_PATH) return exe;
	return string(dir) + "\\" + exe;
}
#endif

void percentmole()
{
    float per=0;
    float fw=0;
    float density=0;
    pmate::Molar m;

    string conversion = "";

    cout << endl << "Please select the conversion you want to perform:"
         << endl << " [1] convert a mass concentration (%w/v) to a molar concentration"
         << endl << " [2] convert a percent volume (%v/v) to a molar concentration" << endl << endl << "> ";

    cin >> conversion;

    if(conversion == "1")
    {
        cout << "\nConvert a mass concentration (%w/v) to molar concentration\n"
            << "==========================================================\n\n";
        cout << "Enter percentage solution (% (w/v)) for your compound: "; cin >> per;
        cout << "Enter formula weight of compound (g/mole): "; cin >> fw;
        m = pmate::molarFromWV(per, fw);
        cout << "\nThe molar concentration of your compound is: " << pmate::round2(m.molar) << " M = " << pmate::round2(m.mmolar) << " mM\n";
        waitKey();
        return;
    }

    else if(conversion == "2")
    {
        cout << "\nConvert a percent volume (%v/v to molar concentration\n"
             << "  =======================================================\n\n";
        cout << "Enter the percent volume (% (v/v)) of the solution: "; cin >> per;
        cout << "Enter formula weight of the compound (g/mole): "; cin >> fw;
        cout << "Enter formula density of your compound (g/mL): "; cin >> density;
        m = pmate::molarFromVV(per, fw, density);
        cout << "\nThe molar concentration of your compound is: " << pmate::round2(m.molar) << " M = " << pmate::round2(m.mmolar) << " mM\n";
        waitKey();
        return;
    }

    else {cout << "\nReturning to main menu...\n\n"; waitKey(); return;}

}

int fact()
{
	int n=0;
	float input=0;

	cout <<"\nFactorial Calculator"
		 <<"\n====================\n\n";
	cout << "Enter an integer: "; cin >> input;

	if(!isfinite(input) || input<0) {cout << "\n(ERR) Input value must be positive\n"; waitKey(); return 1;}
	if(input>pmate::MAX_FACT) {cout << "\n(ERR) Input value must not exceed " << pmate::MAX_FACT << "\n"; waitKey(); return 1;}

	n=int(input);

	cout << endl << n <<"! = " << *pmate::factorial(n) << endl << endl;

	waitKey();
	return 0;
}

void pdbqt()
{
	string file="";
	string outfile="";

	cout << "PDBQT to PDB File Converter\n"
		 << "===========================\n\n"
		 << "This module converts a PDBQT file to a standard PDB file\n"
		 << "with B-factors replaced by the partial atom charges.\n\n";
	cout << "Enter name of your PDBQT file leaving out its \nfile extension (.pdbqt) (Leave empty or enter '0' to exit module): "; getline(cin, file);

	if(file=="" || file=="0")
	{
		cout << "\n(ESC) Quitting module...\n";
		waitKey();
		return;
	}

	outfile = file+".pdb";
	file = file+".pdbqt";

	ifstream in;
	in.open(file.c_str());
	if (!in) {cout << "\nInvalid filename ...\n"; waitKey(); return;}

	ofstream fout;
	if (!openOutFile(fout, outfile)) {waitKey(); return;}

	cout << endl
		 << "Converting "<<file<<" to "<<outfile<<"...\n\n";

	pmate::pdbqtToPdb(in, fout);

	waitKey();

	return;
}

void vina()
{
	pmate::VinaParams p;
	p.x = p.y = p.z = p.grid = 0;
	string run="";

	string out="conf.txt";
	ofstream fout;

	cout << "Vina Configuration File Creator" << endl
		 << "===============================\n\n"
	     << "This module allows you to write a configuration file (conf.txt) for AutoDock\n"
		 << "Vina. The grid generated by this module is cubic, so you only need to provide\n"
		 << "the length of a single side. For more information on AutoDock Vina, please\n"
		 << "visit http://vina.scripps.edu.\n\n"
		 << "Reference:\nO. Trott, A. J. Olson. (2010). AutoDock Vina: improving the speed and\n"
		 << "accuracy of docking with a new scoring function, efficient optimization and\n"
		 << "multithreading. Journal of Computational Chemistry 31, 455-461.\n\n\n";

	cout << "Receptor & Ligand file information (enter 0 to quit module)\n"
		 << "-----------------------------------------------------------\n\n"
		 << "Receptor file (in PDBQT format, with file extension): "; cin >> p.receptor;
	if(p.receptor=="0") {cout << "\n(ESC) Quitting Module...\n"; waitKey(); return;}
	if(!pmate::isSafeFilename(p.receptor)) {cout << "\n(ERR) Invalid receptor file name\n"; waitKey(); return;}
	cout << "Ligand file (in PDBQT format with file extension): "; cin >> p.ligand;
	if(p.ligand=="0") {cout << "\n(ESC) Quitting Module...\n"; waitKey(); return;}
	if(!pmate::isSafeFilename(p.ligand)) {cout << "\n(ERR) Invalid ligand file name\n"; waitKey(); return;}
	cout << "\n\nDocking Parameters\n"
		 << "------------------\n\n";
	cout << "Enter the X co-ordinate for the grid center: "; cin >> p.x;
	cout << "Enter the Y co-ordinate for the grid center: "; cin >> p.y;
	cout << "Enter the Z co-ordinate for the grid center: "; cin >> p.z;
	cout << "Enter the desired size for one side of the grid (in angstroms): "; cin >> p.grid;

	cout << endl << "Opening output stream...\t\t";
	if (!openOutFile(fout, out)) {waitKey(); return;}
	else {cout << "[OK]\n\n";}

	cout << "Writing conf.txt...\t\t\t";

	fout << pmate::vinaConfig(p);
	fout.close();

	cout << "[OK]\n\n";

	cout << "Would you like to run AutoDock Vina with this configuration file (yes/no)? "; cin >> run;

	if(run=="yes" || run == "YES")
	{
		cout << "\nChecking if Vina is installed...\t";
		string vinapath = findInPath("vina");

		if(vinapath!="") {cout << "[OK]\n\n";}
		else {cout << "AutoDock Vina is not installed on this system.\n"; waitKey(); return;}

		cout << "\nRunning Vina...\t";
		vector<string> args;
		args.push_back(vinapath); args.push_back("--config"); args.push_back(out);
		args.push_back("--log"); args.push_back("vina_log.txt");
		runProgram(args);
		cout << "[Done]\n\n" << "The Vina run log is stored as vina_log.txt and the ligand conformations\n"
			 << "are stored in all.pdbqt. For more information on using AutoDock\n"
			 << "Vina, please visit http://vina.scripps.edu.\n\n";
	}

	waitKey();

	return;
}

void bmi()
{
	bool repeat = 1;

	cout << "Body Mass Index (BMI) Calculator\n"
		 << "================================\n"
		 << endl
		 << "Use <bmiiinfo> to access reference values from the CDC\n"
		 << endl;

	while(repeat)
	{
		string yn = "no";
		float lb=0, ft=0, in=0;
		cout << "Enter height in feet and inches (ft [space] inches): "; cin >> ft >> in;
		cout << "Enter weight in pounds (lb): "; cin >> lb;

		pmate::BmiResult r = pmate::bmi(ft, in, lb);

		cout << endl
			 << "Your height in meters (m): " << pmate::round2(r.m) << " metres\n"
			 << "Your weight in kilograms (KGs): " << pmate::round2(r.kg) << " kg\n\n"
			 << "You body mass index (BMI) is " << pmate::round2(r.bmi) << " kg/m^2" << '\t' << "[ " << r.outcome << " ]" << endl
			 << "The ideal weight range for your height of " << pmate::round2(r.m) << " metres is: " << r.lweight << " lb to " << r.hweight << " lb" << endl
			 << endl
			 << "Reference values from the CDC (http://www.cdc.gov/nccdphp/dnpa/bmi):" << endl<< endl
			 << "BMI < 18.5" << "\t\tUnderweight" << endl
			 << "18.5 <= BMI < 24.9" << "\tNormal" << endl
			 << "25.0 <= BMI < 29.9" << "\tOverweight" << endl
			 << "BMI >= 30" << "\t\tObese"
			 << endl;
		cout << endl
			 << "Do you wish to enter more data (yes/no)? "; cin >> yn;

		if(yn == "yes") {repeat=1; cout << endl;}
		else repeat=0;
	}
	waitKey();
	return;
}

void bmiinfo()
{
	cout << "Reference BMI values from the CDC (http://www.cdc.gov/nccdphp/dnpa/bmi):" << endl<< endl
		 << "BMI < 18.5" << "\t\tUnderweight" << endl
		 << "18.5 <= BMI < 24.9" << "\tNormal" << endl
		 << "25.0 <= BMI < 29.9" << "\tOverweight" << endl
		 << "BMI >= 30" << "\t\tObese"
		 << endl << endl;
	return;
}

void rname()
{
	int num=0;
	int snum=0;
	string ans;
	string fstr;
	string dfstr;
	string tdir;
	string dir;
	string dummy;

	cout << "Rename Osciallation Files\n"
		 << "=========================\n"
		 << "\nThis module will rename sequential oscillation files to \na name format that can be used by Mosflm.\n\n";

	cout << "Enter number of files: "; cin >> num;
	if (num == 0) {cout << "Returning to main menu...\n"; waitKey(); return;}
	if (num < 0 || num > pmate::MAX_READINGS) {cout << "(ERR) Number of files must be between 1 and " << pmate::MAX_READINGS << "\n"; waitKey(); return;}

	cout << "Enter initial frame number: "; cin >> snum;
	if (snum == 0) {cout << "Returning to main menu...\n"; waitKey(); return;}
	if (snum < 0 || snum > 1000000) {cout << "(ERR) Initial frame number must be between 1 and 1000000\n"; waitKey(); return;}

	cout << "Enter file string (e.g. for files 1.###.*, enter \"1.\"): "; cin >> fstr;
	if (fstr == "0") {cout << "Returning to main menu...\n"; waitKey(); return;}
	if (!pmate::isSafeFilename(fstr) || fstr.find('/') != string::npos) {cout << "(ERR) Invalid file string\n"; waitKey(); return;}

	cout << "Enter desired file string (e.g to rename files a_###.osc, enter \"a_\"): "; cin >> dfstr;
	if (dfstr == "0") {cout << "Returning to main menu...\n"; waitKey(); return;}
	if (!pmate::isSafeFilename(dfstr) || dfstr.find('/') != string::npos) {cout << "(ERR) Invalid file string\n"; waitKey(); return;}

	cout << "Working directory: "; getline(cin, dummy); getline(cin, tdir);
	if (tdir == "0") {cout << "Returning to main menu...\n"; waitKey(); return;}
	else if(tdir == "?")
	{
		cout << "\nHelp on option...\n\n"
			 << "Enter the name of the directory where your oscillation files are stored \n"
			 << "(without the terminal '/'). For example, if your files are stored in \n"
			 << "/xtal/xtal1/, enter '/xtal/xtal1' (without inverted commas). If your files\n"
			 << "are stored in the current directory, enter '.' (without inverted commas).\n"
			 << "To exit and return to the main menu, enter '0'\n\n";
		cout << "Working directory: "; getline(cin, tdir);
		if (tdir == "0") {cout << "Returning to main menu...\n"; waitKey(); return;}
	}

	struct stat st;
	if (tdir.empty() || stat(tdir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
	{
		cout << "(ERR) " << tdir << " is not a directory\n"; waitKey(); return;
	}

	dir = tdir;

	vector<pair<string, string> > plan = pmate::renamePlan(fstr, dfstr, snum, num);

	cout << dir << endl;
	cout << endl
		 << "Starting Frame No.: " << snum << '\t' << "Ending Frame No.: " << snum+num-1 << "\n\n"
		 << "Frame" << '\t' << "File\n";

	for(size_t n=0; n < plan.size(); ++n)
	{
		cout << n+1 << '\t' << plan[n].first << endl;
	}

	cout << "Do you wish to proceed (y/n)? "; getline(cin, ans);

	if (ans =="y")
	{
		for(size_t j=0; j < plan.size(); ++j)
		{
			const string& nme = plan[j].first;
			const string& nnme = plan[j].second;

			cout << "Reading " << nme << "...\n";

			string src = dir + "/" + nme;
			string dst = dir + "/" + nnme;

			cout << "Re-naming file " << nme << " to " << nnme << "...\n";

			/* rename(2) directly instead of `mv` via the shell; never clobber an existing file */
			if (pathExists(dst)) cout << "(ERR) " << nnme << " already exists - skipping\n";
			else if (rename(src.c_str(), dst.c_str()) != 0) cout << "(ERR) Unable to rename " << nme << ": " << strerror(errno) << "\n";
		}
	}

	waitKey();
	return;
}

void overlay()
{
	string ppdb;
	string pmtf;
	string wpdb;
	string wmtf;
	string out;
	ofstream fout;
	string inp="1";

	cout << "Hydrate Protein\n"
		 << "===============\n\n"
		 << "This module lets you generate X-PLOR code to overlay two protein\n"
		 << "databank (PDB) files (one of which must contain a solvent shell).\n"
		 << "There are 2 stages involved in this procedure and the X-PLOR code \n"
		 << "can be rendered simultaneously using option [3]. Enjoy...\n\n";

	cout << "Which file(s) do you want to generate? \n"
		 << "[1] First stage [2] Second stage [3] Both Stages [0] Return to main menu: [1] "; getline(cin,inp);

	if (inp == "") {inp="1";}

	if (inp == "1" || inp == "2" || inp == "3")
	{
		cout << "Enter name of protein's PDB coordinate file: "; cin >> ppdb;
		cout << "Enter name of protein's MTF structure file : "; cin >> pmtf;
		if (inp == "2")
		{
			cout << "Enter name of solvent's PDB coordinate file (from stage 1):  "; cin >> wpdb;
			cout << "Enter name of solvent's MTF structure file (from stage 1): "; cin >> wmtf;
		}
		else
		{
			cout << "Enter name of solvent's PDB coordinate file: "; cin >> wpdb;
			cout << "Enter name of solvent's MTF structure file : "; cin >> wmtf;
		}
		cout << endl;
		if (!pmate::isSafeFilename(ppdb) || !pmate::isSafeFilename(pmtf) || !pmate::isSafeFilename(wpdb) || !pmate::isSafeFilename(wmtf))
		{cout << "(ERR) File names may only contain letters, digits and . _ - / +\n"; waitKey(); return;}
		cout << "Enter name of output file (.inp): "; cin >> out;

		if (!openOutFile(fout, out)) {waitKey(); return;}

		if (inp == "3") cout << "\nWriting X-PLOR code for stage 1...\n";
		else cout << "\nWriting X-PLOR code...\n";

		if (inp == "2") fout << pmate::xplorStage2(pmtf, ppdb, wmtf, wpdb);
		else fout << pmate::xplorStage1(pmtf, ppdb, wmtf, wpdb);

		fout.close();

		if (inp == "3")
		{
			ofstream fout2;
			string out2;

			cout << endl;
			cout << "Enter name of solvent's new PDB coordinate file (from stage 1):  "; cin >> wpdb;
			cout << "Enter name of solvent's new MTF structure file (from stage 1): "; cin >> wmtf;
			cout << endl;
			if (!pmate::isSafeFilename(wpdb) || !pmate::isSafeFilename(wmtf))
			{cout << "(ERR) File names may only contain letters, digits and . _ - / +\n"; waitKey(); return;}
			cout << "Enter name of output file (.inp): "; cin >> out2;

			if (!openOutFile(fout2, out2)) {waitKey(); return;}

			cout << "\nWriting X-PLOR code for stage 2...\n";

			fout2 << pmate::xplorStage2(pmtf, ppdb, wmtf, wpdb);
			fout2.close();
		}

		waitKey();
		return;
	}

	else if (inp=="0")
	{
		cout << "\n(ESC) Quitting Module...\n";
		waitKey();
		return;
	}

	else
	{
		cout << "\nInvalid response..\n";
		waitKey();
	}

	return;
}

void melt()
{
	string inp="1";
	float salt=0;
	pmate::MeltResult r;
	cout << "Enter input technique\n[1] Manual [2] Automatic from .seq file [0] Return to main menu: [1] "; getline(cin,inp);

	if (inp == "") {inp="1";}

	if (inp=="0") {cout << "\n(ESC) Quitting Module...\n"; waitKey(); return;}

	if (inp == "1")
	{
		float gc=0;
		float n=0;
		string tmp="";

		cout << endl
			 << "Enter length of primer/oligo (total number of nucleotides in primer/oligonucleotide): "; cin >> tmp;
		if (!pmate::parseFloat(tmp, n) || n <= 0) {cout << "\n(ERR) Invalid primer length\n"; waitKey(); return;}
		tmp="";
		cout << "Enter number of G+Cs in primer: "; cin >> tmp;
		if (!pmate::parseFloat(tmp, gc) || gc < 0) {cout << "\n(ERR) Invalid number of G+Cs\n"; waitKey(); return;}
		tmp="";
		if (gc > n)
		{
			cout << "\n[ERROR] Error [code 1a65] : Number of nucleotides cannot exceed total length of primer *******\n";
			waitKey();
			return;
		}

		else if (gc == n)
		{
			string snum;
			cin.ignore();
			cout << endl
				 << "Confirmation: Is your primer composed entirely of guanines and cytosines (yes/no)? [YES] ";
			getline(cin,snum);

			if (snum == "no" || snum == "NO")
			{
				cout << "\nOperation aborted...\n";
				waitKey();
				return;
			}
		}

		cout << "Enter total salt concentration (in Molar) (Typical [salt] = 0.05 M): "; cin >> salt;

		if(salt<=0) {cout << "A [salt] of " << salt << " M is not valid. [Salt] will revert to 0.05 M\n"; salt=0.05;}

		if (n < 14) cout << "\nPrimer length = " << n<< endl;

		r = pmate::meltPoint(n, gc, salt);

		cout << endl << endl << "**** RESULTS ****"<<endl;
		cout << "\n% G-C content: " << r.pgc << "%\n";
		cout << "\nMelting point of primer = " << pmate::round2(r.tm) << " C"
			 << "\nSalt-adjusted melting point of primer = " << pmate::round2(r.adtm) << " C" << endl;

		string ans2;

		cout << "\n\nDo you want to save your data (yes/no)? "; cin >> ans2;

		if (ans2 == "yes")
		{
			ofstream fout;
			string ofile;
			cout << "Enter name of output file: "; cin >> ofile;

			if(ofile=="0") {cout << "\n(ESC) Quitting Module...\n"; waitKey(); return;}

			if (!openOutFile(fout, ofile)) {waitKey(); return;}

			fout << pmate::formatMeltReport(r);
			fout.close();
		}

		waitKey();
		return;
	}

	else if (inp=="2")
	{
		string file;
		string tmp="";

		cout << "\nEnter name of sequence file: "; getline(cin,file);
		cout << "\nEnter total salt concentration (in Molar) (Typical [salt] = 0.05 M): "; cin >> tmp;
		if (!pmate::parseFloat(tmp, salt)) salt = 0;
		tmp="";

		if(salt<=0) {cout << "A [salt] of " << salt << " M is not valid. [Salt] will revert to 0.05 M\n"; salt=0.05;}

		vector<char> seq;
		if(!readSequenceFile(file, seq))
		{
			cout << "\nInvalid filename ...\n";
			waitKey();
			return;
		}
		if(seq.empty()) {cout << "\n(ERR) Sequence file is empty\n"; waitKey(); return;}

		float y=seq.size();

		cout << endl << endl << "***** RESULTS *****" << endl;
		cout << endl << "Total number of nucleotides: " << y << endl;

		pmate::NucleotideCounts nc = pmate::countNucleotides(seq);
		if (nc.badPosition != 0)
		{
			cout <<"\n[ERROR] ERROR [code 1a66z"<<nc.badPosition<<"] : Unknown residue at position " << nc.badPosition << " [" << pmate::printable(nc.badBase) <<"]\n\n"
				 << "Please check the contents of the sequence file before re-analyzing it\n\n";
			waitKey();
			return;
		}

		cout << "Number of Adenines:\t" << nc.a << endl;
		cout << "Number of Thymines:\t" << nc.t << endl;
		cout << "Number of Guanines:\t" << nc.g << endl;
		cout << "Number of Cytosines:\t" << nc.c << endl;

		r = pmate::meltPoint(y, nc.g+nc.c, salt);

		cout << "\n% G-C content: " << r.pgc << "%\n";
		cout << "\nMelting point of primer = " << pmate::round2(r.tm) << " C"
			 << "\nSalt-adjusted melting point of primer = " << pmate::round2(r.adtm) << " C" << endl;

		string ans2;

		cout << "\n\nDo you want to save your data (yes/no)? "; cin >> ans2;

		if (ans2 == "yes")
		{
			ofstream fout;
			string ofile;
			cout << "Enter name of output file: "; cin >> ofile;

			if(ofile=="0") {cout << "\n(ESC) Quitting Module...\n"; waitKey(); return;}

			if (!openOutFile(fout, ofile)) {waitKey(); return;}

			fout << pmate::formatMeltReport(r, &nc);
			fout.close();
		}

		cout << endl;

		return;
	}

	else
	{
		cout << "\nInvalid response..\n";
		waitKey();
	}

	waitKey();
	return;
}

void od260()
{
	float od = 0;
	int dil = 1;
	string x = "";
	string df="";
	pmate::NucleicType type;
	const char* label;

	cout << "What type of Nucleic Acid do you have (dna, rna or oligo)? "; cin >> x;
	cout << endl;

	if (x == "dna") {type = pmate::NucleicType::DsDNA; label = "dsDNA";}
	else if (x == "rna") {type = pmate::NucleicType::RNA; label = "RNA";}
	else if (x == "oligo") {type = pmate::NucleicType::Oligo; label = "oligonucleotide";}
	else
	{
		cout << "Invalid Response" << endl;
		waitKey();
		return;
	}

	cout << "Enter OD 260 for " << label << ": "; cin >> od;
	cin.ignore();
	cout << "Is the dilution factor different from 1 (yes/no)? [NO] "; getline(cin,df);
	if (df == "yes") {cout << "Enter Dilution factor: "; cin >> dil;}
	if (!pmate::od260InRange(od)) cout << "\n\n****** Results may not be accurate in this absorbance range... ******\n\n";
	cout << "\nConcentration = " << pmate::nucleicConc(type, od, dil) << " " << pmate::nucleicUnit(type) << "\n\n";

	waitKey ();
	return;
}

void open()
{
	if (rasmol == 1)
	{
		string x;
		cout << endl << "Enter name of pdb file: ";
		cin >> x;
		if (!pmate::isSafeFilename(x)) {cout << "\n(ERR) Invalid file name\n"; return;}
		vector<string> args;
		args.push_back(rasmol1); args.push_back(x);
		runProgram(args);
	}
}

void sys()
{
#ifdef _WIN32
	if (privileged) {cout << "\n(ERR) System commands are disabled when ProtMate runs as administrator\n\n"; return;}
#else
	if (privileged) {cout << "\n(ERR) System commands are disabled when ProtMate runs as root\n\n"; return;}
#endif

	string x;
	cout << endl << "Enter a System Command" << endl << endl;
	cout << "> ";
	getline(cin,x);
	cout << endl;
	/* Deliberately runs the user's own command through the shell, with the user's own privileges */
	if (x != "") system(x.c_str());
	cout << endl;
}


int main()
{
	dropPrivileges();

	/* Rasmol detection, date and user name are obtained directly rather than via
	   shell commands writing to predictable temporary files in the current directory */
	rasmol1 = findInPath("rasmol");
	rasmol = (rasmol1 != "");

	time_t now = time(NULL);
	struct tm tmv;
	char buf[128];
#ifdef _WIN32
	if (localtime_s(&tmv, &now) == 0 && strftime(buf, sizeof(buf), "%a %b %e %H:%M:%S %Y", &tmv)) date = buf;
#else
	if (localtime_r(&now, &tmv) && strftime(buf, sizeof(buf), "%a %b %e %H:%M:%S %Z %Y", &tmv)) date = buf;
#endif

	user = currentUser();

	cls = 1;

	while(1)
	{
		if (dspmenu == 1 || fmnu == 1) {menu();}	/*Display menu constantly*/
		else {select();}
	}
	return 0;
}

void about()
{
	cout << "Loading program information...\t\t";
	string sys = systemDescription();
	cout << "[OK]\n";
	cout << "Displaying program information...\t";
	cout << "[OK]\n\n";
	cout << "About ProtMate (version " << pmate::VERSION << ")\n"
		 << pmate::COPYRIGHT << "\n"
		 << pmate::HOMEPAGE << "\n\n"
		 << "ProtMate comes with ABSOLUTELY NO WARRANTY. This is free software, and you are\n"
		 << "welcome to redistribute it under the terms of the GNU General Public License\n"
		 << "version 3 (see the LICENSE file). The author assumes no liability for the\n"
		 << "accuracy of calculations or for the functionality of the program.\n\n"
		 << "System: " << sys << endl << endl;
}

/* (Renamed from pause(), which clashed with the POSIX function of that name.) */
int waitKey()
{
	cout << "\nPress the Return key to continue....\n";
	if (cin.fail() && !cin.eof())
	{
		/* Recover from a bad numeric entry instead of leaving the stream unusable */
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
	}
	else cin.ignore();
	cin.get();
	return 0;
}

void ext()
{
        int ntyr = 0;
        int ntrp = 0;
        int ncys = 0;
        float mw = 1;

		string ans;

        cout << "Enter number of Tyrosines in protein: "; cin >> ntyr;
        cout << "Enter number of Tryptophans in protein: "; cin >> ntrp;
        cout << "Enter number of Cysteines in protein: "; cin >> ncys;
        cout << endl
                << "Enter molecular weight of protein (in daltons): "; cin >> mw;

        pmate::ExtResult r = pmate::extinction(ntyr, ntrp, ncys, mw);

        cout << endl << endl
		<< "Extinction coefficient of the native protein in water: "<< r.ext << endl << endl;
        cout << "Absorbance (at 280nm at 1 mg/ml) assuming path length of 1 cm: "
		<< r.abs << endl << endl;

		cout << "Do you want to save your data (yes/no)? "; cin >> ans;

		if (ans == "yes")
		{
			ofstream fout;
			string ofile;
			cout << "Enter name of output file: "; cin >> ofile;

			if (!openOutFile(fout, ofile)) {waitKey(); return;}

			fout << pmate::formatExtReport(r);
		}

		cout << endl;
}

void ext3()
{
	int start=0;
	int end=0;
	float newline = 1;

	string ans;
	int y=0;
	int nwarn=0;

	string file;

	cout << "Enter name of sequence file: "; cin >> file;

	vector<char> x;
	if(!readSequenceFile(file, x))
	{
		cout << "\nInvalid filename ...\n";
		waitKey();
		return;
	}

	y = x.size();

	cout << "\nNumber of residues in sequence: " << y << endl << endl;

	if (y == 0) {cout << "Sequence file is empty. Returning to main menu...\n"; waitKey(); return;}

	cout << "Enter number of the first residue (1 to start at first residue;  0 to exit): ";
	cin >> start;

	start = start-1;

	if((start+1) == 0)
	{
		cout << "Returning to main menu...\n";
		waitKey();
		return;
	}

	cout << "Enter number of the final residue (to end at the final residue, enter 0): ";
	cin >> end;
	end = end-1;
	if((end+1) == 0)
	{
		end = y-1;
	}

	/* Keep both indices inside the sequence buffer */
	if (start < 0 || start >= y || end < 0 || end >= y)
	{
		cout << "Error: Residue numbers must be between 1 and " << y << "\n\n";
		waitKey();
		return;
	}

	if (end < start)
	{
		cout << "Error: End value is smaller than starting residue number\n\n";
		waitKey();
		return;
	}

	cout << "\nSequence to be analyzed:\n";

	for (int tp = start; tp <= end; ++tp)
	{
		cout << pmate::printable(x[tp]);
		if ((newline/70) == int(newline/70))
		{
			cout << endl;
		}
		if (!pmate::isKnownResidue(x[tp])) {cout << "\nWarning: Unknown residue at pos "<< tp+1 << endl; ++nwarn;}
		++newline;
	}

	pmate::SeqAnalysis a = pmate::analyzeProtein(x, start, end);

	cout << endl<<endl;
	cout << "Warnings: " << nwarn << endl << endl;
	if(nwarn > 0)
	{
		cout << "*** Warning [0xf6976]***: One or more warnings were issued during the analysis of your\n"
			 << "protein sequence. Please verify your sequence before using the calculated results below:\n\n";
	}

	cout << pmate::formatComposition(a);

	cout << "\nFormula of protein/peptide: " << pmate::formatFormula(a) << endl;

		cout << "\nMolecular weight of protein/peptide (da): " << a.mw << endl;

        cout << "\nExtinction coefficient of the native protein in water: "<< a.ext.ext << endl << endl;
        cout << "Absorbance (at 280nm & 1 mg/ml) assuming a path length of 1 cm: "
		<< a.ext.abs << endl << endl;

		string dummy;
		getline(cin, dummy);
		cout << "Do you want to save your data (yes/no)? "; getline(cin,ans);

		if (ans == "yes")
		{
			string pnme;
			ofstream fout;
			string ofile;
			cout << "Enter name of output file: "; cin >> ofile;

			if (!openOutFile(fout, ofile)) {waitKey(); return;}

			cout << "Enter a name or unique identifier for your protein/peptide: "; cin >> pnme;

			if(fout)
			{
				fout << pmate::formatProteinReport(a, pnme, user);
				cout << endl << "Your report was generated successfully." << endl;
			}
		}

		return;
}


void p1()
{
	float wv=0;
	float vol=0;
	cout << "Enter weight to volume percent ( %(w/v)): ";
	cin >> wv;
	cout << "Enter desired volume (in ml): ";
	cin >> vol;
	cout << "\nMass of sample required = " << pmate::round2(pmate::massForWV(wv, vol)) <<" g\n";
        waitKey();
}

void p2()
{
	float w=0;
	float wv=0;
	cout << "Enter weight to volume percent ( %(w/v)): ";
	cin >> wv;
	cout << "Enter sample weight (in g): ";
	cin >> w;
	cout << "\nVolume of liquid required = "<< pmate::round2(pmate::volumeForWV(wv, w)) << " ml\n";
	waitKey();
}

void p3()
{
	float a=0;	/* A280 */
	float x=0;	/* Extinction coeff */
	float t=0;	/* Total volume in cuvette */
	float s=0;	/* Volume of protein in cuvette */
	float v=0;	/* Total volume of protien present */

	cout << "Enter total volume of protein (in ml): "; cin >> v;
	cout << "Enter A280 of protein: "; cin >> a;
	cout << "Enter total volume of liquid in cuvette (protien + buffer) (in ul): "; cin >> t;
	cout << "Enter volume of protein in cuvette (ul): "; cin >> s;
	cout << "Enter protein's extinction coefficient: "; cin >> x;

	pmate::ProteinConc r = pmate::proteinConc(a, t, s, x, v);

	cout << "\nDilution factor: " << r.dilution << endl;
	cout << endl;
	cout << "Protein concentration: " << r.conc << " mg/ml"<<endl;
	cout << "Total amount of protein present in " << v << " ml: " << r.amount << " mg\n";
        waitKey();
}

/* Prints the results table and offers to save it (shared by p4 and p5) */
static void showReadings(const vector<pmate::Reading>& rd, const vector<pmate::ProteinConc>& res)
{
	cout << "Sample #\tA280\tDil.\t[Protein] (mg/ml)\tYield (mg)\n";
	for (size_t r=0; r<rd.size(); ++r)
	{
		cout << r+1 << '\t' << '\t' << rd[r].a280 << '\t' << res[r].dilution << '\t' << res[r].conc << '\t' << '\t' << '\t' << res[r].amount << endl;
	}
	cout << endl;

	string ans;
	cout << "Do you want to save your data (yes/no)? "; cin >> ans;

	if (ans == "yes")
	{
		ofstream fout;
		string ofile;
		cout << "Enter name of output file: "; cin >> ofile;

		if (openOutFile(fout, ofile)) fout << pmate::formatReadingsReport(rd, res);
	}
}

void p4()
{
	float x=0;	/* Extinction coeff */
	float t=0;	/* Total vol in cuvette */
	float s=0;	/* Vol of protein in cuvette */
	int n=0;	/* Number of readings */

	cout << "Enter total volume of liquid in cuvette (protien + buffer) (in ul): "; cin >> t;
	cout << "Enter volume of protein in cuvette (ul): "; cin >> s;
	cout << "Enter protein's extinction coefficient: "; cin >> x;
	cout << "Enter number of A280 readings: "; cin >> n;

	if (n <= 0 || n > pmate::MAX_READINGS)
	{
		cout << "\n(ERR) Number of readings must be between 1 and " << pmate::MAX_READINGS << "\n";
		waitKey();
		return;
	}

	vector<pmate::Reading> rd(n);

	cout << endl;

	for (int q=0; q<n; ++q)
	{
		rd[q].a280 = 0; rd[q].volume = 0;
		cout << "Enter A280 for sample " << q+1 << ": "; cin >> rd[q].a280;
		cout << "Enter yield volume for your sample " << q+1 << " (mL): "; cin >> rd[q].volume;
		cout << endl;
	}

	cout << endl;

	showReadings(rd, pmate::proteinConcBatch(rd, t, s, x));

	waitKey();
    return;
}

void p5()
{
	float x=0;	/* Extinction coeff */
	float t=0;	/* Total vol in cuvette */
	float s=0;	/* Vol of protein in cuvette */
	string file;

	cout << "Enter total volume of liquid in cuvette (protien + buffer) (in ul): "; cin >> t;
	cout << "Enter volume of protein in cuvette (ul): "; cin >> s;
	cout << "Enter protein's extinction coefficient: "; cin >> x;
	cout << endl
		<< "Enter name of input file: "; cin >> file;

	ifstream in;
	in.open(file.c_str());

	if(!in) {cout << "Invalid file name ... \n\n"; waitKey(); return;}

	cout << endl;

	vector<pmate::Reading> rd;
	if (pmate::parseReadings(in, rd)) cout << "(WARN) Only the first " << pmate::MAX_READINGS << " readings were used\n";

	cout << endl;

	showReadings(rd, pmate::proteinConcBatch(rd, t, s, x));

	waitKey();
}


void help()
{
	cout << "\n======================"
         << "\n ProtMate Help Center\n"
		 << "======================\n"
		 << "\nCommand line options:\n"
         <<"\nfact:	   Calculate the factorial of an integer"
         <<"\ndna:       Calculate a nucleic acid's concentration from its A260nm"
         <<"\next:       Calculate a protein's extinction coefficient from a sequence file"
		 <<"\nmelt:      Calculate a primer / oligonucleotide's melting point"
         <<"\npdb:	   Convert a PDBQT file to a standard PDB format"
         <<"\nmprot:     Calculate protein concentration and yield based on multiple\n"
		 << "           A280 readings and an extinction coefficient"
         <<"\noverlay:   Generate CNS/XPLOR code to solvate a protein structure"
          <<"\nprot:      Calculate protein concentration and yield based on its\n"
         << "           A280 and extinction coefficient"
         <<"\npmol:      Convert a mass concentration (% (w/v)) to a molar concentration"
		 <<"\nmprot_auto: Calculate protein concentration and yield based on multiple\n"
		 <<"           A280 readings obtained from an input file and an\n"
		 <<"           extinction coefficient"
         <<"\nrname:	   Re-name oscillation files to a Mosflm-readable format"
         <<"\nvina:	   Generate configuration file for AutoDock Vina (R)"
         << "\nwv-weight: Calculate weight of a compound required to reach a desired\n"
         << "           weight to volume ratio, given a certain volume of solvent"
         <<"\nwv-vol:    Calculate volume of solvent needed to reach a desired\n"
		 << "           weight to volume ratio, given a certain weight of sample"
		 <<"\nxplor:	   Generate CNS/XPLOR code to solvate a protein structure"
         <<"\nsys:       Issue a system command to the operating system"
		 <<"\nmnu:	   Displays ProtMate's main menu"
		 <<"\nbmi:	   Calculate your body mass index"
		 <<"\nabout:     Displays information about ProtMate"
		 <<"\nabt:       Displays information about ProtMate"
		 <<"\nclear:     Clears the display and returns to the main menu"
		 <<"\nwhoami:    Displays information about the current user"
		 <<"\nps:        Display currently running proceses"
         <<"\nquit:      Quit ProtMate"
		 <<"\n\nIn general, selecting '0' within a module allows you to quit and return to\nthe selection prompt (This does not work across all modules as this system\ncannot be implemented in those modules that require numerical input)."
		 << endl<< endl;
}

void menu()
{
	clearScreen();
	cout << "ProtMate\n"
  		 << "Developed by Sanjeeva Wijeyesakere\nVer " << pmate::VERSION << '\t'
		 << "Session started on " << date << endl << endl;
	cout 	<< "ProtMate Main Menu\n"
			<< "==================\n\n"
		<< "Calculators:\n"
		<< " 1.  Weight to Volume Ratio - Calculate weight\n"
		<< " 2.  Weight to Volume Ratio - Calculate volume\n"
		<< " 3.  Calculate Protein Concentration Using A280 (Single Reading)\n"
		<< " 4.  Calculate Protein Concentration Using A280 (Multiple Readings)\n"
		<< " 5.  Calculate Protein Concentration Using A280 (Multiple Readings, auto input)\n"
		<< " 6.  Calculate Protein Extinction Coefficient\n"
		<< " 7.  Calculate Protein Extinction Coefficient From a Sequence (*.seq) File\n"
		<< " 8.  Calculate Concentration of a Nucleic Acid from A260\n"
		<< " 9.  Calculate Melting Point of a Primer/Oligonucleotide\n"
		<< " 10. Calculate your Body Mass Index (BMI)\n"
		<< " 11. Factorial Calculator\n\n"
		<< "External Interfaces & File Utilities:\n"
		<< " 12. Rename Oscillation Files\n"
		<< " 13. Generate Configuration/Input File to Run AutoDock Vina\n"
		<< " 14. Convert a PDBQT file to a PDB file\n"
		<< " 15. Generate XPLOR code to solvate a protein structure\n\n"
		<< "Other Functions:\n"
		<< " ?   - Help (Command line commands)\n"
		<< " ls  - List Files in the Current Directory\n"
		<< " pwd - Display the name of the Current Directory\n"
		<< " sys - Issue a System Command\n"
		<< " abt - About Protmate\n"
		<< " 0   - Quit"
		<< endl << endl
        << "*** For a complete list of avaiable functions, consult the help menu" << endl << "*** (enter \"help\" or \"?\" at the prompt)" << endl << endl;
		select();
		return;
}

void select()
{
	bool repeat = 1;

	while (repeat == 1)
	{
		if(dspmenu == 0 && dsptle == 1)
		{
			if (cls==1)
			{
				clearScreen();
			}
			cout << "ProtMate\n"
			 	 << "Developed by Sanjeeva Wijeyesakere\nVer " << pmate::VERSION << '\t'
				 << "Session started on " << date << endl << endl;
		}

		cout << user << "% > ";
		string x;
		if (!getline(cin,x))
		{
			/* Exit at end of input instead of spinning forever on a dead stream */
			if (cin.eof()) {cout << endl; exit(0);}
			cin.clear();
			continue;
		}
		if (x=="1" || x=="wv-weight")	{repeat = 0; cls=1; dspmenu = 0; dsptle = 1; cout << endl; p1();}
		else if (x=="2" || x=="wv-vol")	{repeat = 0; cls=1; dspmenu = 0; dsptle = 1; cout << endl; p2();}
		else if (x=="3" || x=="prot")	{repeat = 0; cls=1; dspmenu = 0; dsptle = 1; cout << endl; p3();}
		else if (x=="4" || x=="mprot")	{repeat = 0; cls=1; dspmenu = 0; dsptle = 1; cout << endl; p4();}
		else if (x=="5" || x=="mprot_auto")	{repeat = 0; cls=1; dspmenu = 0; dsptle = 1; cout << endl; p5();}
		else if (x=="6")				{repeat = 0; cls=1; dspmenu = 0; dsptle = 1; cout << endl; ext();}
		else if (x=="7" || x=="ext")    {repeat = 0; cls=1; dspmenu = 0; dsptle = 1; cout << endl; ext3();}
		else if (x=="8" || x=="dna")	{repeat = 0; cls=1; dspmenu = 0; dsptle = 1; cout << endl; od260();}
		else if (x=="9" || x=="melt")	{repeat = 0; cls=1; dspmenu = 0; dsptle = 1; cout << endl; melt();}
		else if (x=="12" || x=="rname")	{repeat = 0; cls=1; dspmenu = 0; dsptle = 1; cout << endl; rname();}
		else if (x=="15" || x=="xplor" || x=="overlay") {repeat = 0; cls=1; dspmenu = 0; dsptle = 1; cout << endl; overlay();}
		else if (x=="10" || x=="bmi")	{repeat = 0; cls=1; dspmenu = 0; dsptle = 1; cout << endl; bmi();}
		else if (x=="13" || x=="vina" || x=="VINA") {repeat = 0; cls=1; dspmenu=0; dsptle=1; cout << endl; vina();}
		else if (x=="14" || x =="pdb" || x=="pdbqt" || x=="PDB" || x=="PDBQT") {repeat = 0; cls=1; dspmenu=0; dsptle=1; cout << endl; pdbqt();}
		else if (x=="bmiinfo")			{repeat = 0; cls=0; dspmenu = 0; dsptle = 0; cout << endl; bmiinfo();}
		else if (x=="about" || x=="abt" || x=="ABT") {repeat = 0; cls=0; dspmenu = 0; dsptle = 0; cout << endl; about();}
		else if (x=="0" || x =="quit" || x=="stop" || x=="exit" || x == "bye")	{clearScreen(); cout << "ProtMate \nDeveloped by Sanjeeva Wijeyesakere\nVer " << pmate::VERSION << '\n' << "Session Started: " << date << endl << endl << "Thank you for using ProtMate.\n\nToodle-oo...\n\n"; exit(0);}
		else if (x=="?" || x=="help")	{repeat = 1; cls=0; dspmenu = 0; dsptle = 0; help();}
		else if (x=="ls")				{repeat = 0; cls=0; dspmenu = 0; dsptle = 0; vector<string> args;
#ifdef _WIN32
										  args.push_back(systemProgram("cmd.exe")); args.push_back("/d"); args.push_back("/c"); args.push_back("dir");
#else
										  args.push_back("/bin/ls"); args.push_back("-alp");
#endif
										  runProgram(args);}
		else if (x=="sys" || x=="$")	{repeat = 0; cls=0; dspmenu = 0; dsptle = 0; sys();}
		else if (x=="")					{repeat = 1; cls=0; dspmenu= 0; dsptle = 0;  }
		else if (x=="clear" || x=="clr"){clearScreen(); dspmenu = 0; dsptle = 1;  return;}
		else if (x=="menu" || x=="mnu")	{clearScreen(); dspmenu=1; dsptle = 1; return;}
		else if (x=="whoami")			{repeat = 0; cls=0; dspmenu=0; dsptle=0; cout << user << endl; return;}
		else if (x=="who")				{repeat = 0; cls=0; dspmenu=0; dsptle=0; vector<string> args;
#ifdef _WIN32
										  args.push_back(systemProgram("query.exe")); args.push_back("user");
										  if (!pathExists(args[0])) {cout << user << endl; return;}
#else
										  args.push_back("/usr/bin/who");
#endif
										  runProgram(args); return;}
		else if (x=="pwd")				{repeat = 0; cls=0; dspmenu=0; dsptle=0; cout << "You are running ProtMate as " << user << " in" << endl; string cwd = currentDir(); if (cwd != "") cout << cwd << endl; cout << endl; return;}
		else if (x=="ps")				{repeat = 0; cls=0; dspmenu=0; dsptle=0; vector<string> args;
#ifdef _WIN32
										  args.push_back(systemProgram("tasklist.exe"));
#else
										  args.push_back("/bin/ps"); args.push_back("-a");
#endif
										  runProgram(args); return;}
		else if (x=="11" || x=="fact")	{repeat = 0; cls=1; dspmenu=0; dsptle=1; fact(); return;}
		else if (x=="open" || x=="rasmol")	 {repeat = 0; cls = 0; dspmenu = 0; dsptle = 0; if (rasmol == 1) open(); else cout << "\n[ERR] Rasmol not available /n000x3q1429\n\n";}
		else if (x=="pmol" || x=="pmole") {repeat = 0; cls=1; dspmenu = 0; dsptle = 1; cout << endl; percentmole();}
        else							{cout << "Invalid Response ...\n"; repeat = 1; cls=0; dspmenu = 0; dsptle = 0;}

	}
	return;
}
