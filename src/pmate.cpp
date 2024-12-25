/*********************************************************
 ProtMate (Laboratory formula calculator).
 Developed by Sanjeeva Wijeyesakere.

 This program is used to calculate values commonly used
 in the laboratory setting.
*********************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <math.h>
#include "pmate.h"

using namespace std;

/* Constant Global Variables: */

const string ver = "1.1.5";

/* Global Variables */

string line="";
string rasmol1="";
bool rasmol=0;
string date = "";
bool dspmenu = 0;
bool cls = 0;
bool dsptle = 1;
string user="";
bool fmnu = 0;
string year = "2024";

/*Functions: */

void menu();
void select();
int getnum(string);
int pause();
float roundf(float);// noexcept;
// float roundf(float) noexcept;
void help();
void about();
void p1();
void p2();
void p3();
void p4();
void p5();
void ext();
void ext2();
void ext3();
void sys();
void open();
void overlay();
void od260();
void melt();
string itos(int);
void rname();
void bmi();
void bmiinfo();
void vina();
void pdbqt();
int fact();
unsigned long ffac(unsigned long);
void percentmole();

void percentmole()
{
    float per=0;
    float fw=0;
    float molar=0;
    float mmolar=0;
    float density=0;
    float weight=0;
    
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
        molar = (per/(0.1*fw));
        mmolar = 1000*molar;
        cout << "\nThe molar concentration of your compound is: " << roundf(molar) << " M = " << roundf(mmolar) << " mM\n";
        pause();
        return;
    }
    
    else if(conversion == "2")
    {
        cout << "\nConvert a percent volume (%v/v to molar concentration\n"
             << "  =======================================================\n\n";
        cout << "Enter the percent volume (% (v/v)) of the solution: "; cin >> per;
        cout << "Enter formula weight of the compound (g/mole): "; cin >> fw;
        cout << "Enter formula density of your compound (g/mL): "; cin >> density;
        weight=density*per;
        molar = (weight/(0.1*fw));
        mmolar = 1000*molar;
        cout << "\nThe molar concentration of your compound is: " << roundf(molar) << " M = " << roundf(mmolar) << " mM\n";
        pause();
        return;
    }
    
    else {cout << "\nReturning to main menu...\n\n"; pause(); return;}

}

string intostr(int istr)
{
	stringstream strg; 
	strg << istr; 
	return strg.str();
}

int fact()
{
	int n=0;
	float input=0;
	unsigned long factorial=0;
	
	cout <<"\nFactorial Calculator"
		 <<"\n====================\n\n";
	cout << "Enter an integer: "; cin >> input;
	
	n=int(input);
	
	if(n<0) {cout << "\n(ERR) Input value must be positive\n"; pause(); return 1;}
	
	factorial=ffac(n);
	
	cout << endl << n <<"! = " << factorial << endl << endl;
	
	pause();
	return 0;	
}

unsigned long ffac(unsigned long n)
{
	if(n>1) return n*ffac(n-1);
	else return 1;
}

float roundf(float x)
{
	float p;	/*Product (input number * 100)*/
	float a;	
	float f;	/*Result of Product - Whole*/
	int w;		/*Whole number*/
	
   	p=100*x ;
   	w=static_cast<int> (p);
   	f=p-w;
   	if (2*f>=1) return ((w+1)/100.0);
   	else return w / 100.0;
}

void pdbqt()
{
	string command="";
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
		pause();
		return;
	}
	
	outfile = file+".pdb";
	file = file+".pdbqt";
	
	cout << endl
		 << "Converting "<<file<<" to "<<outfile<<"...\n\n";
	
	command =  "cut -c 1-60,70-79 " + file + " > "+outfile;
	
	system(command.c_str());
	
	pause();
	
	return;
}

void vina()
{
	string receptor="";
	string ligand="";
	float x=0;
	float y=0;
	float z=0;
	float grid=0;
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
		 << "Receptor file (in PDBQT format, with file extension): "; cin >> receptor;
	if(receptor=="0") {cout << "\n(ESC) Quitting Module...\n"; pause(); return;}
	cout << "Ligand file (in PDBQT format with file extension): "; cin >> ligand;
	if(ligand=="0") {cout << "\n(ESC) Quitting Module...\n"; pause(); return;}
	cout << "\n\nDocking Parameters\n"
		 << "------------------\n\n";
	cout << "Enter the X co-ordinate for the grid center: "; cin >> x;
	cout << "Enter the Y co-ordinate for the grid center: "; cin >> y;
	cout << "Enter the Z co-ordinate for the grid center: "; cin >> z;
	cout << "Enter the desired size for one side of the grid (in angstroms): "; cin >> grid;
	
	cout << endl << "Opening output stream...\t\t";
	fout.open(out.c_str());
	if (!fout) {cout << "Unable to open output stream - Make sure you have read/write/execute permissions in this directory...\n\n"; pause(); return;}
	else {cout << "[OK]\n\n";}

	cout << "Writing conf.txt...\t\t\t";
	
	fout << "receptor = " << receptor << endl
		 << "ligand = " << ligand << endl << endl
		 << "out = all.pdbqt\n\n"
		 << "center_x = " << x << endl
		 << "center_y = " << y << endl
		 << "center_z = " << z << endl <<endl
		 << "size_x = " << grid << endl
		 << "size_y = " << grid << endl
		 << "size_z = " << grid << endl << endl
		 << "exhaustiveness = 8";
		
	cout << "[OK]\n\n";
	
	cout << "Would you like to run AutoDock Vina with this configuration file (yes/no)? "; cin >> run;
	
	if(run=="yes" || run == "YES")
	{
		cout << "\nChecking if Vina is installed...\t";
		bool vina;
		string test;
		ifstream fin;
		
		system("which vina | grep Command > .tmp");
		fin.open(".tmp");
		if(!fin) {cout << "(ERR) No input\n\n"; pause(); return;}
		getline(fin, test);
		fin.close();
		system ("rm .tmp");
		
		if(test=="") {cout << "[OK]\n\n";}
		else if(test != "") {cout << "AutoDock Vina is not installed on this system.\n"; pause(); return;}
		
		cout << "\nRunning Vina...\t";
		system("vina --config conf.txt --log vina_log.txt");
		cout << "[Done]\n\n" << "The Vina run log is stored as log.txt and the ligand conformations\n"
			 << "are stored in all.pdbqt. For more information on using AutoDock\n"
			 << "Vina, please visit http://vina.scripps.edu.\n\n";
	}
	
	pause();
	
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
		float lb, ft, in, m, kg;
		float bmi, bmir, lweight, hweight;
		string outcome = "";
		cout << "Enter height in feet and inches (ft [space] inches): "; cin >> ft >> in;
		cout << "Enter weight in pounds (lb): "; cin >> lb;
		
		m = ((30*ft) + (2.54 * in))/100;
		kg = (lb/2.2);
		bmi = kg / (m*m);
		bmir = roundf(bmi);
		
		lweight = (m*m) * 18.5;
		lweight = roundf(lweight*2.2);
		hweight = (m*m) * 24.9;
		hweight = roundf(hweight*2.2);
		
		if (bmi < 18.5) outcome = "Underweight";
		if (bmi >= 18.5 && bmi < 24.9) outcome = "Normal";
		if (bmi > 24.9 && bmi < 29.9) outcome = "Overweight";
		if (bmi > 29.9) outcome = "Obese";
		
		cout << endl
			 << "Your height in meters (m): " << roundf(m) << " metres\n"
			 << "Your weight in kilograms (KGs): " << roundf(kg) << " kg\n\n"
			 << "You body mass index (BMI) is " << bmir << " kg/m^2" << '\t' << "[ " << outcome << " ]" << endl 
			 << "The ideal weight range for your height of " << roundf(m) << " metres is: " << lweight << " lb to " << hweight << " lb" << endl
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
	pause();
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
	int num;
	int snum;
	int fnum;
	int tnum;
	string ans;
	string fds;
	string fstr;
	string fname;
	string nme;
	string ssnum;
	string dfstr;
	string tdir;
	string dir;
	string dummy;

	cout << "Rename Osciallation Files\n"
		 << "=========================\n"
		 << "\nThis module will rename sequential oscillation files to \na name format that can be used by Mosflm.\n\n";
	
	cout << "Enter number of files: "; cin >> num;
	if (num == 0) {cout << "Returning to main menu...\n"; pause(); return;}
	
	cout << "Enter initial frame number: "; cin >> snum;
	if (snum == 0) {cout << "Returning to main menu...\n"; pause(); return;}
	
	fnum = snum+num;
	tnum = snum;
	
	cout << "Enter file string (e.g. for files 1.###.*, enter \"1.\"): "; cin >> fstr; 
	if (fstr == "0") {cout << "Returning to main menu...\n"; pause(); return;}
	
	cout << "Enter desired file string (e.g to rename files a_###.osc, enter \"a_\"): "; cin >> dfstr;
	if (dfstr == "0") {cout << "Returning to main menu...\n"; pause(); return;}
	
	cout << "Working directory: "; getline(cin, dummy); getline(cin, tdir);
	if (tdir == "0") {cout << "Returning to main menu...\n"; pause(); return;}
	else if(tdir == "?")
	{
		cout << "\nHelp on option...\n\n"
			 << "Enter the name of the directory where your oscillation files are stored \n"
			 << "(without the terminal '/'). For example, if your files are stored in \n"
			 << "/xtal/xtal1/, enter '/xtal/xtal1' (without inverted commas). If your files\n"
			 << "are stored in the current directory, enter '.' (without inverted commas).\n"
			 << "To exit and return to the main menu, enter '0'\n\n";
		cout << "Working directory: "; getline(cin, tdir);
		if (tdir == "0") {cout << "Returning to main menu...\n"; pause(); return;}
	}
	
	
	dir = tdir;
	
	/* dir = "\""+tdir+"/"; */
	
	cout << dir << endl;
	cout << endl
		 << "Starting Frame No.: " << snum << '\t' << "Ending Frame No.: " << fnum-1 << "\n\n"
		 << "Frame" << '\t' << "File\n";
		 
	for(int n=0; n < num && snum < fnum ; ++n)
	{	
		ssnum = intostr(snum);
		if (snum < 10) fname = "00" + ssnum;
		else if (snum > 9 && snum < 100) fname = "0" + ssnum;
		else if (snum > 99 && snum < 1000) fname = ssnum;
 
		nme = fstr + "0" +fname;
		cout << n+1 << '\t' << nme << endl;
		++snum;
	}
	
	snum=tnum;
/*	cout << snum <<endl; */
	
	cout << "Do you wish to proceed (y/n)? "; getline(cin, ans);
	
	if (ans =="y")
	{	
		for(int j=0; j < num && snum < fnum; ++j)
		{
			ssnum = intostr(snum);
			if (snum < 10) fname = "00" + ssnum;
			else if (snum > 9 && snum < 100) fname = "0" + ssnum;
			else if (snum > 99 && snum < 1000) fname = ssnum;
 
			nme = fstr + "0" +fname;

			cout << "Reading " << nme << "...\n";
			
			string nnme = dfstr + fname + ".osc";
			
			string command = "mv " + dir + "/" + nme + " " + dir + "/" + nnme;
			
			cout << "Re-naming file " << nme << " to " << nnme << "...\n";
			/*
			cout << command << endl;
			*/
			system(command.c_str());
			
			++snum;
		}
	}
	
	pause();
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
	
	if (inp == "1")
	{
		cout << "Enter name of protein's PDB coordinate file: "; cin >> ppdb;
		cout << "Enter name of protein's MTF structure file : "; cin >> pmtf;
		cout << "Enter name of solvent's PDB coordinate file: "; cin >> wpdb;
		cout << "Enter name of solvent's MTF structure file : "; cin >> wmtf;
		cout << endl;
		cout << "Enter name of output file (.inp): "; cin >> out;
	
		fout.open(out.c_str());
	
		if (!fout) {cout << "(ERR) Unable to open output file. Make sure you have read/write/execute permissions\n"
						 << "in this directory.\n"; pause(); return;}
	
		cout << "\nWriting X-PLOR code...\n";
	
		fout << "structure @ " << pmtf << " end\n"
			 << "coordinates @ " << ppdb << endl
	    	 << "structure @ " << wmtf << " end\n"
			<< "coordinates @ " << wpdb << endl << endl
			<< "write coordinates output=overlay1.pdb\n"
			<< "  selection=\n"
			<< "        ( byres\n"
			<< "          ( segid \"W   \"" << endl
			<< "                and ( ( not segid \"W   \" and not (hydrogen) ) around 6.0)" << endl
			<< "        ))" << endl
			<< "end\nstop\n";
	
		fout.close();
	
		pause();
		return;
	}
	
	else if (inp=="2")
	{
		cout << "Enter name of protein's PDB coordinate file: "; cin >> ppdb;
		cout << "Enter name of protein's MTF structure file : "; cin >> pmtf;
		cout << "Enter name of solvent's PDB coordinate file (from stage 1):  "; cin >> wpdb;
		cout << "Enter name of solvent's MTF structure file (from stage 1): "; cin >> wmtf;
		cout << endl;
		cout << "Enter name of output file (.inp): "; cin >> out;
	
		fout.open(out.c_str());
	
		if (!fout) {cout << "ERR: Unable to open output file. Make sure you have read/write/execute permissions\n"
					     << "in this directory.\n"; pause(); return;}
	
		cout << "\nWriting X-PLOR code...\n";
	
		fout << "structure @ " << pmtf << " end\n"
			 << "coordinates @ " << ppdb << endl
	    	 << "structure @ " << wmtf << " end\n"
			<< "coordinates @ " << wpdb << endl << endl
			<< "delete" << endl
			<< "  selection=\n"
			<< "        ( byres\n"
			<< "          ( segid \"W   \"" << endl
			<< "                and ( ( not segid \"W   \" and not (hydrogen) ) around 2.3)" << endl
			<< "        ))" << endl
			<< "end\n"
			<<"\nwrite coordinates output=overlay2.pdb end" << endl
			<<"stop" << endl;
	
		fout.close();
	
		pause();
		return;
	}
	
	if (inp == "3")
	{
		cout << "Enter name of protein's PDB coordinate file: "; cin >> ppdb;
		cout << "Enter name of protein's MTF structure file : "; cin >> pmtf;
		cout << "Enter name of solvent's PDB coordinate file: "; cin >> wpdb;
		cout << "Enter name of solvent's MTF structure file : "; cin >> wmtf;
		cout << endl;
		cout << "Enter name of output file (.inp): "; cin >> out;
	
		fout.open(out.c_str());
	
		if (!fout) {cout << "ERR: Unable to open output file. Make sure you have read/write/execute permissions\n"
						 << "in this directory.\n"; pause(); return;}
	
		cout << "\nWriting X-PLOR code for stage 1...\n";
	
		fout << "structure @ " << pmtf << " end\n"
			 << "coordinates @ " << ppdb << endl
	    	 << "structure @ " << wmtf << " end\n"
	 	 	 << "coordinates @ " << wpdb << endl << endl
			 << "write coordinates output=overlay1.pdb\n"
			 << "  selection=\n"
			 << "        ( byres\n"
			 << "          ( segid \"W   \"" << endl
			 << "                and ( ( not segid \"W   \" and not (hydrogen) ) around 6.0)" << endl
			 << "        ))" << endl
			 << "end\nstop\n";
	
		fout.close();
		
		ofstream fout2;
		string out2;
		
		cout << endl;
		cout << "Enter name of solvent's new PDB coordinate file (from stage 1):  "; cin >> wpdb;
		cout << "Enter name of solvent's new MTF structure file (from stage 1): "; cin >> wmtf;
		cout << endl;
		cout << "Enter name of output file (.inp): "; cin >> out2;
	
		fout2.open(out2.c_str());
	
		if (!fout2) {cout << "Unable to open output file...\n\n"; pause(); return;}
	
		cout << "\nWriting X-PLOR code for stage 2...\n";
	
		fout2 << "structure @ " << pmtf << " end\n"
			 << "coordinates @ " << ppdb << endl
	    	 << "structure @ " << wmtf << " end\n"
			<< "coordinates @ " << wpdb << endl << endl
			<< "delete" << endl
			<< "  selection=\n"
			<< "        ( byres\n"
			<< "          ( segid \"W   \"" << endl
			<< "                and ( ( not segid \"W   \" and not (hydrogen) ) around 2.3)" << endl
			<< "        ))" << endl
			<< "end\n"
			<<"\nwrite coordinates output=overlay2.pdb end" << endl
			<<"stop" << endl;
	
		fout.close();
	
		pause();
		return;
	}

	
	else if (inp=="0")
	{
		cout << "\n(ESC) Quitting Module...\n"; 
		pause();
		return;
	}
	
	else
	{
		cout << "\nInvalid response..\n";
		pause();
	}
	
	return;
}

void melt()
{
	string inp="1";
	float pgc=0;
	float salt=0;
	float adtm=0;
	cout << "Enter input technique\n[1] Manual [2] Automatic from .seq file [0] Return to main menu: [1] "; getline(cin,inp);

	if (inp == "") {inp="1";}
	
	if (inp=="0") {cout << "\n(ESC) Quitting Module...\n"; pause(); return;}
	
	if (inp == "1")
	{
		float at=0;
		float gc=0;
		float n=0;
		float tm=0;
		string tmp="";
				
		cout << endl
			 << "Enter length of primer/oligo (total number of nucleotides in primer/oligonucleotide): "; cin >> tmp;
		n = stof(tmp); tmp="";
		cout << "Enter number of G+Cs in primer: "; cin >> tmp;
		gc = stof(tmp); tmp="";
		if (gc > n)
		{
			cout << "\n[ERROR] Error [code 1a65] : Number of nucleotides cannot exceed total length of primer *******\n";
			pause();
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
				pause();
				return;
			}
		}
		
		cout << "Enter total salt concentration (in Molar) (Typical [salt] = 0.05 M): "; cin >> salt;
		
		if(salt==0) {cout << "A [salt] of 0 M is not valid. [Salt] will revert to 0.05 M\n"; salt=0.05;}
		
		if (n < 14) 
		{
			cout << "\nPrimer length = " << n<< endl;
			at = n-gc;
			tm = (4*gc)+(2*at);
		}
	
		else 
		{
			tm = 64.9+(41*((gc-16.4)/n));
		}
		
		pgc = (gc/n) *100;
		cout << endl << endl << "**** RESULTS ****"<<endl;
		cout << "\n% G-C content: " << pgc << "%\n";

		float div = (675/n);
		adtm = 81.5 + (16.6*log10(salt)) + (0.41*pgc) - div;
		
		cout << "\nMelting point of primer = " << roundf(tm) << " C"
			 << "\nSalt-adjusted melting point of primer = " << roundf(adtm) << " C" << endl;

		string ans2;
		string ofile;
		
		cout << "\n\nDo you want to save your data (yes/no)? "; cin >> ans2;
		
		if (ans2 == "yes")
		{
			ofstream fout;
			string ofile;
			cout << "Enter name of output file: "; cin >> ofile;
			
			if(ofile=="0") {cout << "\n(ESC) Quitting Module...\n"; pause(); return;}
			
			fout.open(ofile.c_str());
			if (!fout) {cout << "ERR: Unable to open output file. Make sure you have read/write/execute permissions\n"
							 << "in this directory.\n"; pause(); return;}
		
			if(fout)
			{
				fout << "\n% G-C content: " << pgc << "%\n";
				fout << "\nMelting point of primer = " << roundf(tm) << " C"
					 << "\nSalt-adjusted melting point of primer = " << roundf(adtm) << " C" << endl;
			}
			
			fout.close();
		}
		
		pause();
		return;
	}

	else if (inp=="2")
	{
		char x;
		string file;
		ifstream inf;
		float pgc=0;
		float salt=0;
		float adtm=0;

		string tmp="";
		
		cout << "\nEnter name of sequence file: "; getline(cin,file);
		cout << "\nEnter total salt concentration (in Molar) (Typical [salt] = 0.05 M): "; cin >> tmp;
		salt = stof(tmp); tmp="";
		
		if(salt==0) {cout << "A [salt] of 0 M is not valid. [Salt] will revert to 0.05 M\n"; salt=0.05;}
		
		inf.open(file.c_str());
	
		if(!inf)
		{
			cout << "\nInvalid filename ...\n";
			pause();
			return;
		}
	
		int a = 0;
		int t = 0;
		int g = 0;
		int cyt = 0;

		string ans;	
		float y=-1;
		char r;

		while(inf)
		{
			inf >> r;
			++y;
		}

		cout << endl << endl << "***** RESULTS *****" << endl;
		cout << endl << "Total number of nucleotides: " << y << endl;
		inf.close();

		ifstream in;
		in.open(file.c_str());

		for(int z=0; z<y; ++z)
		{
			in >> x;

			if (x == 'a' || x == 'A') ++a;
			else if (x == 't' || x == 'T') ++t;
			else if (x == 'g' || x == 'G') ++g;
			else if (x == 'c' || x == 'C') ++cyt;
			else 
			{
				cout <<"\n[ERROR] ERROR [code 1a66z"<<z+1<<"] : Unknown residue at position " << z+1 << " [" << x <<"]\n\n"
					 << "Please check the contents of the sequence file before re-analyzing it\n\n"; 
				pause(); 
				return;
			}
		}

		cout << "Number of Adenines:\t" << a << endl;
		cout << "Number of Thymines:\t" << t << endl;
		cout << "Number of Guanines:\t" << g << endl;
		cout << "Number of Cytosines:\t" << cyt << endl;
	
		float n = y;
		int at = a+t;
		int gc = g+cyt;
		
		float tm=0;
	
		if (n < 14) 
			{
				tm = (4*gc)+(2*at);
			}
	
			else 
			{
				tm = 64.9+(41*((gc-16.4)/n));
			}
		
		pgc = (gc/n) *100;
		cout << "\n% G-C content: " << pgc << "%\n";

		float div = (675/n);
		adtm = 81.5 + (16.6*log10(salt)) + (0.41*pgc) - div;
		cout << "\nMelting point of primer = " << roundf(tm) << " C"
			 << "\nSalt-adjusted melting point of primer = " << roundf(adtm) << " C" << endl;
	
		string ans2;
		string ofile;
		
		cout << "\n\nDo you want to save your data (yes/no)? "; cin >> ans2;
		
		if (ans2 == "yes")
		{
			ofstream fout;
			string ofile;
			cout << "Enter name of output file: "; cin >> ofile;
			
			if(ofile=="0") {cout << "\n(ESC) Quitting Module...\n"; pause(); return;}
			
			fout.open(ofile.c_str());
			if (!fout) {cout << "ERR: Unable to open output file. Make sure you have read/write/execute permissions\n"
							 << "in this directory.\n"; pause(); return;}
		
			if(fout)
			{
				fout << "Number of Adenines:\t" << a << endl;
				fout << "Number of Thymines:\t" << t << endl;
				fout << "Number of Guanines:\t" << g << endl;
				fout << "Number of Cytosines:\t" << cyt << endl;	
				fout << "\n% G-C content: " << pgc << "%\n";
				fout << "\nMelting point of primer = " << roundf(tm) << " C"
					 << "\nSalt-adjusted melting point of primer = " << roundf(adtm) << " C" << endl;
			}
			
			fout.close();
		}
		
		cout << endl;
		
		return;
	}

	else
	{
		cout << "\nInvalid response..\n";
		pause();
	}

	pause();
	return;
}

void od260()
{
	float od = 0;
	int dil = 1;
	float result = 0;
	string x = "";
	string df="";

	cout << "What type of Nucleic Acid do you have (dna, rna or oligo)? "; cin >> x;
	cout << endl;
	
	if (x == "dna")
	{	
		cout << "Enter OD 260 for dsDNA: "; cin >> od;
		cin.ignore();
		cout << "Is the dilution factor different from 1 (yes/no)? [NO] "; getline(cin,df);
		if (df == "yes") {cout << "Enter Dilution factor: "; cin >> dil;}
		if (od < 0.1 || od > 10) cout << "\n\n****** Results may not be accurate in this absorbance range... ******\n\n";
		od=od*dil;
		result = od*50;
//		result = roundf(oddna*50);
		cout << "\nConcentration = " << result << " ug dsDNA / ml\n\n";
	}

	else if (x == "rna")
	{
           cout << "Enter OD 260 for RNA: "; cin >> od;
		   cin.ignore();
		   cout << "Is the dilution factor different from 1 (yes/no)? [NO] "; getline(cin,df);
		   if (df == "yes") {cout << "Enter Dilution factor: "; cin >> dil;}
		   if (od < 0.1 || od > 10) cout << "\n\n****** Results may not be accurate in this absorbance range... ******\n\n";
	       od=od*dil;
		   result = od*40;
//		   result = roundf(odrna*40);
           cout << "\nConcentration = " << result << " ug ssRNA / ml\n\n";
	}

	else if (x == "oligo")
	{
           cout << "Enter OD 260 for oligonucleotide: "; cin >> od;
		   cin.ignore();
	       cout << "Is the dilution factor different from 1 (yes/no)? [NO] "; getline(cin,df);
		   if (df == "yes") {cout << "Enter Dilution factor: "; cin >> dil;}
		   if (od < 0.1 || od > 10) cout << "\n\n****** Results may not be accurate in this absorbance range... ******\n\n";
	       od=od*dil;
		   result = od*20;
//		   result = roundf(odoligo*20);
           cout << "\nConcentration = " << result << " ug ss Oligonucleotide / ml\n\n";
	}

	else
	{
		cout << "Invalid Response" << endl;
	}

	pause ();
	return;
}

void open()
{
	if (rasmol == 1)
	{
		string x;
		cout << endl << "Enter name of pdb file: ";
		cin >> x;
		string y = "rasmol "+x;
		system(y.c_str());
	}
}

void sys()
{
	string x;
	cout << endl << "Enter a System Command" << endl << endl;
	cout << "> ";
	getline(cin,x);
	cout << endl;
	system(x.c_str());
	cout << endl;
//	pause();
}


int getnum(string file)
{
	ifstream in;
	in.open(file.c_str());
	string temp;
	
	if(!in) { cout << "Invalid input file\n"; return(0);}
	
	int n=0;
	while(getline(in,temp)) {++n;}
	in.close();
	return n;
}

int main()
{
	string x = "which rasmol | grep Command > .tml";
	string dte = "date > .date";
	system(x.c_str());
	system(dte.c_str());
	ifstream in;
	in.open(".tml");
	string xx;
	if(in)getline(in,xx);
	in.close();
	ifstream indate;
	indate.open(".date");
	if(indate) getline(indate,date);
	indate.close();
	string g = "rm .tml";
	string ddte = "rm .date";
	system(ddte.c_str());
	system(g.c_str());
	if(xx == "rasmol: Command not found.") {rasmol1 = "Rasmol not detected"; rasmol = 0;}
	else {rasmol1 = "Rasmol detected"; rasmol = 1;}
	cls = 1;
	
	system("whoami > .t");
	
	ifstream inusr;
	inusr.open(".t");
	
	if(inusr) inusr >> user;
	
	inusr.close();
	
	system("rm .t");
	
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
	string q = "uname -a > .tml";
	system(q.c_str());
	ifstream in;
	in.open(".tml");
	cout << "[OK]\n";
	cout << "Displaying program information...\t";
	string sys;
	getline(in,sys);
	in.close();
	string y = "rm .tml";
	system (y.c_str());
	cout << "[OK]\n\n";
	cout << "About ProtMate (version " << ver << ")\n"
		 << "System: " << sys << endl << endl;
	select();
}

int pause()
{
	cout << "\nPress the Return key to continue....\n";
	cin.ignore();
	cin.get();
	return 0;
}     

void ext()
{
        int ntyr = 0;
        int etyr = 1490;
        int ntrp = 0;
        int etrp = 5500;
        int ncys = 0;
        int ecys = 125;
        float mw = 1;
		
		string ans;

        cout << "Enter number of Tyrosines in protein: "; cin >> ntyr;
        cout << "Enter number of Tryptophans in protein: "; cin >> ntrp;
        cout << "Enter number of Cysteines in protein: "; cin >> ncys;
        cout << endl 
                << "Enter molecular weight of protein (in daltons): "; cin >> mw; 

        float ext = (ntyr * etyr) + (ntrp * etrp) + (ncys *ecys);
        float abs = ext / mw;

		ext = roundf(ext);

        cout << endl << endl
		<< "Extinction coefficient of the native protein in water: "<< ext << endl << endl;
        cout << "Absorbance (at 280nm at 1 mg/ml) assuming path length of 1 cm: "
		<< abs << endl << endl;
		
		cout << "Do you want to save your data (yes/no)? "; cin >> ans;
		
		if (ans == "yes")
		{
			ofstream fout;
			string ofile;
			cout << "Enter name of output file: "; cin >> ofile;
		
			fout.open(ofile.c_str());
			if (!fout) {cout << "ERR: Unable to open output file. Make sure you have read/write/execute permissions\n"
							 << "in this directory.\n"; pause(); return;}
		
			if(fout)
			{
				fout << endl 
					<< "Extinction coefficient of the native protein in water: "<< ext << endl;
				fout << "Absorbance (at 280nm & 1 mg/ml) assuming path length of 1 cm: "
					<< abs << endl;
			}
		}
		
		cout << endl;
		
//		pause();
}

void ext2()
{
	char* x;

	string file;

	ifstream inf;

	cout << "Enter name of sequence file: "; cin >> file;

	inf.open(file.c_str());
	
	if(!inf)
	{
		cout << "\nInvalid filename ...\n";
		pause();
		return;
	}
	
	int ntyr = 0;
    	int etyr = 1490;
    	int ntrp = 0;
    	int etrp = 5500;
    	int ncys = 0;
    	int ecys = 125;
    	float mw = 1;
	string ans;	
	int y=-1;
	char r;

	while(inf)
	{
		inf >> r;
		++y;
	}

	inf.close();

	ifstream in;
	in.open(file.c_str());

	x=new char[y];

	for(int t=0; t<y; ++t)
	{
		in >> x[t];

		if (x[t] == 'y' || x[t] == 'Y') ++ntyr;
		if (x[t] == 'w' || x[t] == 'W') ++ntrp;
		if (x[t] == 'c' || x[t] == 'C') ++ncys;
	}

	cout << "Number of Tyrosines:\t" << ntyr << endl;
	cout << "Number of Tryptophans:\t" << ntrp << endl;
	cout << "Number of Cysteines:\t" << ncys << endl;

	cout << endl
                << "Enter molecular weight of protein: "; cin >> mw; 

        float ext = (ntyr * etyr) + (ntrp * etrp) + (ncys *ecys);
        float abs = ext / mw;

		ext = roundf(ext);

        cout << endl << endl
		<< "Extinction coefficient of the native protein in water: "<< ext << endl << endl;
        cout << "Absorbance (at 280nm & 1 mg/ml) assuming path length of 1 cm: "
		<< abs << endl << endl;
		
		cout << "Do you want to save your data (yes/no)? "; cin >> ans;
		
		if (ans == "yes")
		{
			ofstream fout;
			string ofile;
			cout << "Enter name of output file: "; cin >> ofile;
		
			fout.open(ofile.c_str());
			if (!fout) {cout << "ERR: Unable to open output file. Make sure you have read/write/execute permissions\n"
							 << "in this directory.\n"; pause(); return;}
		
			if(fout)
			{
				fout << endl 
					<< "Extinction coefficient of the native protein in water: "<< ext << endl;
				fout << "Absorbance (at 280nm at 1 mg/ml) assuming path length of 1 cm: "
					<< abs << endl;
			}
		}
		
		cout << endl;
		
//		pause();
}

void ext3()
{
	char* x;
	int start=0;
	int end=0;
	char q;
	int carbon = 0;
	int nitrogen = 0;
	int oxygen = 0;
	int hydrogen = 0;
	int sulphur = 0;
	float newline = 1;
		
	int ntyr = 0;
    int etyr = 1490;
    int ntrp = 0;
    int etrp = 5500;
    int ncys = 0;
    int ecys = 125;
    float mw = 0;
	float mw1 = 0;
	
	string ans;	
	int y=0;
	char r;
	bool warn=0;
	int nwarn=0;
	
	string file;

	ifstream inf;

	cout << "Enter name of sequence file: "; cin >> file;

	inf.open(file.c_str());
	
	if(!inf)
	{
		cout << "\nInvalid filename ...\n";
		pause();
		return;
	}

	while(inf)
	{
		inf >> r;
		++y;
	}

	inf.close();
	
	cout << "\nNumber of residues in sequence: " << y-1 << endl << endl;
	
	cout << "Enter number of the first residue (1 to start at first residue;  0 to exit): ";
	cin >> start;

	start = start-1;
	
	if((start+1) == 0)
	{
		cout << "Returning to main menu...\n";
		pause();
		return;
	}
	
	cout << "Enter number of the final residue (to end at the final residue, enter 0): "; 
	cin >> end;
	end = end-1;
	if((end+1) == 0)
	{
		end = y-2;
	}
	
	if (end < (start+1))
	{
		cout << "Error: End value is smaller than starting residue number\n\n";
		pause();
		return;
	}

	ifstream in;
	in.open(file.c_str());

	x=new char[y];
	
	for(int t=0; t<y; ++t)
	{
		in >> x[t];
	}
	
	in.close();
	
	float gly=0, ala=0,val=0,leu=0,ile=0,ser=0,thr=0,cys=0,met=0,phe=0,tyr=0,trp=0,pro=0,asn=0,gln=0,asp=0,glu=0,lys=0,his=0,arg=0,unk=0;
	
	cout << "\nSequence to be analyzed:\n";
	
	for (int tp = start; tp <= end; ++tp)
	{
		cout << x[tp];
		if ((newline/70) == int(newline/70))
		{
			cout << endl;
		}
		if (x[tp] == 'y' || x[tp] == 'Y')	   {++tyr; ++ntyr;carbon=carbon+9; nitrogen=nitrogen+1; oxygen=oxygen+2;hydrogen=hydrogen+9;sulphur=sulphur+0;}
		else if (x[tp] == 'w' || x[tp] == 'W') {++trp; ++ntrp;carbon=carbon+11; nitrogen=nitrogen+2; oxygen=oxygen+1;hydrogen=hydrogen+10;sulphur=sulphur+0;}
		else if (x[tp] == 'c' || x[tp] == 'C') {++cys; ++ncys;carbon=carbon+3; nitrogen=nitrogen+1; oxygen=oxygen+1;hydrogen=hydrogen+5;sulphur=sulphur+1;}
		else if (x[tp] == 'g' || x[tp] == 'G') {++gly; carbon=carbon+2; nitrogen=nitrogen+1; oxygen=oxygen+1;hydrogen=hydrogen+3;sulphur=sulphur+0;}
		else if (x[tp] == 'a' || x[tp] == 'A') {++ala; carbon=carbon+3; nitrogen=nitrogen+1; oxygen=oxygen+1;hydrogen=hydrogen+5;sulphur=sulphur+0;}
		else if (x[tp] == 'v' || x[tp] == 'V') {++val; carbon=carbon+5; nitrogen=nitrogen+1; oxygen=oxygen+1;hydrogen=hydrogen+9;sulphur=sulphur+0;}
		else if (x[tp] == 'l' || x[tp] == 'L') {++leu; carbon=carbon+6; nitrogen=nitrogen+1; oxygen=oxygen+1;hydrogen=hydrogen+11;sulphur=sulphur+0;}
		else if (x[tp] == 'i' || x[tp] == 'I') {++ile; carbon=carbon+6; nitrogen=nitrogen+1; oxygen=oxygen+1;hydrogen=hydrogen+11;sulphur=sulphur+0;}
		else if (x[tp] == 's' || x[tp] == 'S') {++ser; carbon=carbon+3; nitrogen=nitrogen+1; oxygen=oxygen+2;hydrogen=hydrogen+5;sulphur=sulphur+0;}
		else if (x[tp] == 't' || x[tp] == 'T') {++thr; carbon=carbon+4; nitrogen=nitrogen+1; oxygen=oxygen+2;hydrogen=hydrogen+7;sulphur=sulphur+0;}
		else if (x[tp] == 'm' || x[tp] == 'M') {++met; carbon=carbon+5; nitrogen=nitrogen+1; oxygen=oxygen+1;hydrogen=hydrogen+9;sulphur=sulphur+1;}
		else if (x[tp] == 'f' || x[tp] == 'F') {++phe; carbon=carbon+9; nitrogen=nitrogen+1; oxygen=oxygen+1;hydrogen=hydrogen+9;sulphur=sulphur+0;}
		else if (x[tp] == 'p' || x[tp] == 'P') {++pro; carbon=carbon+5; nitrogen=nitrogen+1; oxygen=oxygen+1;hydrogen=hydrogen+7;sulphur=sulphur+0;}
		else if (x[tp] == 'n' || x[tp] == 'N') {++asn; carbon=carbon+4; nitrogen=nitrogen+2; oxygen=oxygen+2;hydrogen=hydrogen+6;sulphur=sulphur+0;}
		else if (x[tp] == 'q' || x[tp] == 'Q') {++gln; carbon=carbon+5; nitrogen=nitrogen+2; oxygen=oxygen+2;hydrogen=hydrogen+8;sulphur=sulphur+0;}
		else if (x[tp] == 'd' || x[tp] == 'D') {++asp; carbon=carbon+4; nitrogen=nitrogen+1; oxygen=oxygen+3;hydrogen=hydrogen+5;sulphur=sulphur+0;}
		else if (x[tp] == 'e' || x[tp] == 'E') {++glu; carbon=carbon+5; nitrogen=nitrogen+1; oxygen=oxygen+3;hydrogen=hydrogen+7;sulphur=sulphur+0;}
		else if (x[tp] == 'k' || x[tp] == 'K') {++lys; carbon=carbon+6; nitrogen=nitrogen+2; oxygen=oxygen+1;hydrogen=hydrogen+12;sulphur=sulphur+0;}
		else if (x[tp] == 'h' || x[tp] == 'H') {++his; carbon=carbon+6; nitrogen=nitrogen+3; oxygen=oxygen+1;hydrogen=hydrogen+7;sulphur=sulphur+0;}
		else if (x[tp] == 'r' || x[tp] == 'R') {++arg; carbon=carbon+6; nitrogen=nitrogen+4; oxygen=oxygen+1;hydrogen=hydrogen+12;sulphur=sulphur+0;}
		else {if((tp+2)!=end) {cout << "\nWarning: Unknown residue at pos "<< tp+1 << endl; warn = 1; ++unk;++nwarn;}}
		++newline;
	}

	hydrogen = hydrogen+2;
	oxygen = oxygen+1;

	cout << endl<<endl;
	cout << "Warnings: " << nwarn << endl << endl;
	if(warn)
	{
		cout << "*** Warning [0xf6976]***: One or more warnings were issued during the analysis of your\n"
			 << "protein sequence. Please verify your sequence before using the calculated results below:\n\n";
	}
	
	y = end -start +1;
	
	cout << "Your protein's Amino Acid composition is as follows:\n\n"
		 << "Res\tNum\t%\n"
		 << "======================\n";
	cout << "Tyr:\t" << tyr << '\t' << roundf(100*(tyr/y)) << "%"<< endl;
	cout << "Trp:\t" << trp << '\t' << roundf(100*(trp/y)) << "%"<< endl;
	cout << "Cys:\t" << cys << '\t' << roundf(100*(cys/y)) << "%"<< endl;
	cout << "Gly:\t" << gly << '\t' << roundf(100*(gly/y)) << "%"<< endl;
	cout << "Ala:\t" << ala << '\t' << roundf(100*(ala/y)) << "%"<< endl;
	cout << "Val:\t" << val << '\t' << roundf(100*(val/y)) << "%"<< endl;
	cout << "Leu:\t" << leu << '\t' << roundf(100*(leu/y)) << "%"<< endl;
	cout << "Ile:\t" << ile << '\t' << roundf(100*(ile/y)) << "%"<< endl;
	cout << "Ser:\t" << ser << '\t' << roundf(100*(ser/y)) << "%"<< endl;
	cout << "Thr:\t" << thr << '\t' << roundf(100*(thr/y)) << "%"<< endl;
	cout << "Met:\t" << met << '\t' << roundf(100*(met/y)) << "%"<< endl;
	cout << "Phe:\t" << phe << '\t' << roundf(100*(phe/y)) << "%"<< endl;
	cout << "Pro:\t" << pro << '\t' << roundf(100*(pro/y)) << "%"<< endl;
	cout << "Asn:\t" << asn << '\t' << roundf(100*(asn/y)) << "%"<< endl;
	cout << "Gln:\t" << gln << '\t' << roundf(100*(gln/y)) << "%"<< endl;
	cout << "Asp:\t" << asp << '\t' << roundf(100*(asp/y)) << "%"<< endl;
	cout << "Glu:\t" << glu << '\t' << roundf(100*(glu/y)) << "%"<< endl;
	cout << "Lys:\t" << lys << '\t' << roundf(100*(lys/y)) << "%"<< endl;
	cout << "His:\t" << his << '\t' << roundf(100*(his/y)) << "%"<< endl;
	cout << "Arg:\t" << arg << '\t' << roundf(100*(arg/y)) << "%"<< endl;
	cout << "Unk:\t" << unk << '\t' << roundf(100*(unk/y)) << "%"<< endl;
	
	cout << "\nFormula of protein/peptide: "
		 << "C("<<carbon<<")H("<<hydrogen<<")O("<<oxygen<<")N("<<nitrogen<<")S("<<oxygen<<")"<< endl;
	
	/*cout << endl
		 << "Enter molecular weight of protein: "; cin >> mw; 
	*/
		mw1 = (12.011*carbon) + (14.007*nitrogen) + (15.999*oxygen) + (32.066*sulphur) + (1.008*hydrogen);
		
		mw = roundf(mw1);
		
		cout << "\nMolecular weight of protein/peptide (da): " << mw << endl;
      
		float ext = (ntyr * etyr) + (ntrp * etrp) + (ncys *ecys);
        float abs = ext / mw;

		ext = roundf(ext);

        cout << "\nExtinction coefficient of the native protein in water: "<< ext << endl << endl;
        cout << "Absorbance (at 280nm & 1 mg/ml) assuming a path length of 1 cm: "
		<< abs << endl << endl;
		
		string dummy;
		getline(cin, dummy);
		cout << "Do you want to save your data (yes/no)? "; getline(cin,ans);
		
		if (ans == "yes")
		{
			string pnme;
			ofstream fout;
			string ofile;
			cout << "Enter name of output file: "; cin >> ofile;
		
			fout.open(ofile.c_str());
			if (!fout) {cout << "ERR: Unable to open output file. Make sure you have read/write/execute permissions\n"
							 << "in this directory.\n"; pause(); return;}
			
			cout << "Enter a name or unique identifier for your protein/peptide: "; cin >> pnme;
			
			if(fout)
			{
				fout << "******************************************" << endl
					 << "|  PROTMATE REPORTER                     |\n"
					 << "******************************************\n\n";
				fout << "Sequence analysis report for " << pnme << "." << endl
					 << "Operator: "<< user << endl << endl;
				fout << "Your protein's Amino Acid composition is as follows:\n\n"
					 << "Res\tNum\t%\n"
					 << "======================\n";
				fout << "Tyr:\t" << tyr << '\t' << roundf(100*(tyr/y)) << "%"<< endl;
				fout << "Trp:\t" << trp << '\t' << roundf(100*(trp/y)) << "%"<< endl;
				fout << "Cys:\t" << cys << '\t' << roundf(100*(cys/y)) << "%"<< endl;
				fout << "Gly:\t" << gly << '\t' << roundf(100*(gly/y)) << "%"<< endl;
				fout << "Ala:\t" << ala << '\t' << roundf(100*(ala/y)) << "%"<< endl;
				fout << "Val:\t" << val << '\t' << roundf(100*(val/y)) << "%"<< endl;
				fout << "Leu:\t" << leu << '\t' << roundf(100*(leu/y)) << "%"<< endl;
				fout << "Ile:\t" << ile << '\t' << roundf(100*(ile/y)) << "%"<< endl;
				fout << "Ser:\t" << ser << '\t' << roundf(100*(ser/y)) << "%"<< endl;
				fout << "Thr:\t" << thr << '\t' << roundf(100*(thr/y)) << "%"<< endl;
				fout << "Met:\t" << met << '\t' << roundf(100*(met/y)) << "%"<< endl;
				fout << "Phe:\t" << phe << '\t' << roundf(100*(phe/y)) << "%"<< endl;
				fout << "Pro:\t" << pro << '\t' << roundf(100*(pro/y)) << "%"<< endl;
				fout << "Asn:\t" << asn << '\t' << roundf(100*(asn/y)) << "%"<< endl;
				fout << "Gln:\t" << gln << '\t' << roundf(100*(gln/y)) << "%"<< endl;
				fout << "Asp:\t" << asp << '\t' << roundf(100*(asp/y)) << "%"<< endl;
				fout << "Glu:\t" << glu << '\t' << roundf(100*(glu/y)) << "%"<< endl;
				fout << "Lys:\t" << lys << '\t' << roundf(100*(lys/y)) << "%"<< endl;
				fout << "His:\t" << his << '\t' << roundf(100*(his/y)) << "%"<< endl;
				fout << "Arg:\t" << arg << '\t' << roundf(100*(arg/y)) << "%"<< endl;
				fout << "Unk:\t" << unk << '\t' << roundf(100*(unk/y)) << "%"<< endl;
				fout << "\nFormula of protein/peptide: "
					 << "C("<<carbon<<")H("<<hydrogen<<")O("<<oxygen<<")N("<<nitrogen<<")S("<<oxygen<<")"<< endl;
				fout << "\nMolecular weight of peptide (da): " << mw << endl;
				fout << endl 
					<< "Extinction coefficient of the native protein in water: "<< ext << endl;
				fout << "Absorbance (at 280nm at 1 mg/ml) assuming path length of 1 cm: "
					<< abs << endl;
					
				cout << endl << "Your report was generated successfully." << endl;
			}
		}
		
/*		cout << endl;
		
		pause(); */
		return;
}


void p1()
{
	float wv;
	float vol;
	cout << "Enter weight to volume percent ( %(w/v)): ";
	cin >> wv;
	cout << "Enter desired volume (in ml): ";
	cin >> vol;
	float ans = (wv/100)*vol;			/*g=vol*((w/v)/100)*/
	cout << "\nMass of sample required = " << roundf(ans) <<" g\n";
        pause();
}

void p2()
{
	float w;
	float wv;
	cout << "Enter weight to volume percent ( %(w/v)): ";
	cin >> wv;
	cout << "Enter sample weight (in g): ";
	cin >> w;
	float ans = (w/(wv/100));			/*vol=g/((w/v)/100)*/
	cout << "\nVolume of liquid required = "<< roundf(ans) << " ml\n";
	pause();
}

void p3()
{
	float a;	/* A280 */
	float x;	/* Extinction coeff */
	float c;	/* Conc. of protein */
	float t;	/* Total volume in cuvette */
	float s;	/* Volume of protein in cuvette */
	float v;	/* Total volume of protien present */
	
	cout << "Enter total volume of protein (in ml): "; cin >> v;
	cout << "Enter A280 of protein: "; cin >> a;
	cout << "Enter total volume of liquid in cuvette (protien + buffer) (in ul): "; cin >> t;
	cout << "Enter volume of protein in cuvette (ul): "; cin >> s;
	cout << "Enter protein's extinction coefficient: "; cin >> x;

	c=t/s;
	float pc=(a*c)/x;
	pc = roundf(pc);

	cout << "\nDilution factor: " << c << endl;
	cout << endl;
	cout << "Protein concentration: " << pc << " mg/ml"<<endl;
	cout << "Total amount of protein present in " << v << " ml: " << pc*v << " mg\n";
        pause();
}

void p4()
{
	float* a;	/* A280 */
	float* pc;	/* Protein Concentration */
	float c;	/* dilution factor */
	float x;	/* Extinction coeff */
	float t;	/* Total vol in cuvette */
	float s;	/* Vol of protein in cuvette */
	float* v; 	/* Total volume of protein present */
	float* y;	/* Yield */
	int n;		/* Number of readings */	

	cout << "Enter total volume of liquid in cuvette (protien + buffer) (in ul): "; cin >> t;
	cout << "Enter volume of protein in cuvette (ul): "; cin >> s;
	cout << "Enter protein's extinction coefficient: "; cin >> x;
	cout << "Enter number of A280 readings: "; cin >> n;
	
	a = new float[n];
	pc = new float[n];
	v = new float[n];
	y = new float[n];

	c=t/s;	

	cout << endl;
	
	for (int q=0; q<n; ++q)
	{
		cout << "Enter A280 for sample " << q+1 << ": "; cin >> a[q];
		cout << "Enter yield volume for your sample " << q+1 << " (mL): "; cin >> v[q];
		
		pc[q]=(a[q]*c)/x;

		pc[q] = roundf(pc[q]);

		y[q]=pc[q]*v[q];
		
		cout << endl;
	} 
	
	cout << endl;
	
	cout << "Sample #\tA280\tDil.\t[Protein] (mg/ml)\tYield (mg)\n";
	for (int r=0; r<n; ++r)
	{
		cout << r+1 << '\t' << '\t' << a[r] << '\t' << c << '\t' << pc[r] << '\t' << '\t' << '\t' << y[r] << endl;
	}
	cout << endl;
	
	string ans;
	cout << "Do you want to save your data (yes/no)? "; cin >> ans;
	
	if (ans == "yes")
	{
		ofstream fout;
		string ofile;
		cout << "Enter name of output file: "; cin >> ofile;
		
		fout.open(ofile.c_str());
		if (!fout) {cout << "ERR: Unable to open output file. Make sure you have read/write/execute permissions\n"
						 << "in this directory.\n"; pause(); return;}
		
		if(fout)
		{
			fout << "Sample #\tA280\tDil.\t[Protein] (mg/ml)\tYield (mg)\n";
			for (int r=0; r<n; ++r)
				{
					fout << r+1 << '\t' << a[r] << '\t' << c << '\t' << pc[r] << '\t' << y[r] << endl;
				}
		}
	}
	
	pause();
    return;
}

void p5()
{
	float* a;	/* A280 */
	float* pc;	/* Protein Concentration */
	float c;	/* dilution factor */
	float x;	/* Extinction coeff */
	float t;	/* Total vol in cuvette */
	float s;	/* Vol of protein in cuvette */
	float* v; 	/* Total volume of protein present */
	float* y;	/* Yield */
	int n;		/* Number of readings */	
	string file;
	
	cout << "Enter total volume of liquid in cuvette (protien + buffer) (in ul): "; cin >> t;
	cout << "Enter volume of protein in cuvette (ul): "; cin >> s;
	cout << "Enter protein's extinction coefficient: "; cin >> x;
	cout << endl
		<< "Enter name of input file: "; cin >> file;
	
	n = getnum(file);
			
	ifstream in;
	in.open(file.c_str());
	
	if(!in) {cout << "Invalid file name ... \n\n"; pause(); return;}
	
	a = new float[n];
	pc = new float[n];
	v = new float[n];
	y = new float[n];

	c=t/s;	

	cout << endl;
	
	int q=0;
	while(in)
	{
		in >> a[q];
		in >>v[q];
		
		pc[q]=(a[q]*c)/x;

		pc[q] = roundf(pc[q]);

		y[q]=pc[q]*v[q];
		++q;
	} 
	
	cout << endl;
	
	cout << "Sample #\tA280\tDil.\t[Protein] (mg/ml)\tYield (mg)\n";
	for (int r=0; r<n; ++r)
	{
		cout << r+1 << '\t' << '\t' << a[r] << '\t' << c << '\t' << pc[r] << '\t' << '\t' << '\t' << y[r] << endl;
	}
	cout << endl;
	
	string ans;
	cout << "Do you want to save your data (yes/no)? "; cin >> ans;
	
	if (ans == "yes")
	{
		ofstream fout;
		string ofile;
		cout << "Enter name of output file: "; cin >> ofile;
		
		fout.open(ofile.c_str());
		if (!fout) { cout << "Unable to write output file\n";}
		
		if(fout)
		{
			fout << "Sample #\tA280\tDil.\t[Protein] (mg/ml)\tYield (mg)\n";
			for (int r=0; r<n; ++r)
				{
					fout << r+1 << '\t' << a[r] << '\t' << c << '\t' << pc[r] << '\t' << y[r] << endl;
				}
		}
	}
	
	pause(); 
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
//		pause();
}

void menu()
{
	system("clear");	
	cout << "ProtMate\n"
  		 << "Developed by Sanjeeva Wijeyesakere\nVer " <<ver << '\t'
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
				system("clear");
			}
			cout << "ProtMate\n"
			 	 << "Developed by Sanjeeva Wijeyesakere\nVer " <<ver << '\t'
				 << "Session started on " << date << endl << endl;
		}

		cout << user << "% > ";
		string x;
		getline(cin,x);
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
		else if (x=="0" || x =="quit" || x=="stop" || x=="exit" || x == "bye")	{system("clear"); cout << "ProtMate \nDeveloped by Sanjeeva Wijeyesakere\nVer " <<ver << '\n' << "Session Started: " << date << endl << endl << "Thank you for using ProtMate.\n\nToodle-oo...\n\n"; exit(0);}
		else if (x=="?" || x=="help")	{repeat = 1; cls=0; dspmenu = 0; dsptle = 0; help();}
		else if (x=="ls")				{repeat = 0; cls=0; dspmenu = 0; dsptle = 0; system("ls -alp"); select();}
		else if (x=="sys" || x=="$")	{repeat = 0; cls=0; dspmenu = 0; dsptle = 0; sys();}
		else if (x=="")					{repeat = 1; cls=0; dspmenu= 0; dsptle = 0;  }
		else if (x=="clear" || x=="clr"){system("clear"); dspmenu = 0; dsptle = 1;  return;}
		else if (x=="menu" || x=="mnu")	{system("clear"); dspmenu=1; dsptle = 1; return;}
		else if (x=="whoami")			{repeat = 0; cls=0; dspmenu=0; dsptle=0; cout << user << endl; return;}
		else if (x=="who")				{repeat = 0; cls=0; dspmenu=0; dsptle=0; system("who"); return;}
		else if (x=="pwd")				{repeat = 0; cls=0; dspmenu=0; dsptle=0; cout << "You are running ProtMate as " << user << " in" << endl; system("pwd"); cout << endl; return;}
		else if (x=="ps")				{repeat = 0; cls=0; dspmenu=0; dsptle=0; string ps="ps -a"; system(ps.c_str()); return;}
		else if (x=="11" || x=="fact")	{repeat = 0; cls=1; dspmenu=0; dsptle=1; fact(); return;}
		/*else if (x=="mnuon")			{fmnu = 1; dspmenu = 1; cls=1; return;}*/
		else if (x=="open" || x=="rasmol")	 {repeat = 0; cls = 0; dspmenu = 0; dsptle = 0; if (rasmol == 1) open(); else cout << "\n[ERR] Rasmol not available /n000x3q1429\n\n";}
		else if (x=="pmol" || x=="pmole") {repeat = 0; cls=1; dspmenu = 0; dsptle = 1; cout << endl; percentmole();}
        else							{cout << "Invalid Response ...\n"; repeat = 1; cls = 0; dspmenu = 0; dsptle = 0;}
        
	}
	return;
}
