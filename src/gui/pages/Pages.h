/* Factories for every ProtMate GUI page. */

#ifndef PMATE_GUI_PAGES_H
#define PMATE_GUI_PAGES_H

class Page;

/* Calculators (CalculatorPages.cpp) */
Page* createWeightVolumePage();
Page* createProteinConcPage();
Page* createMultiReadingPage();
Page* createExtinctionPage();
Page* createNucleicPage();
Page* createPercentMolarPage();
Page* createBmiPage();
Page* createFactorialPage();

/* Sequence tools (SequencePages.cpp) */
Page* createSequenceAnalysisPage();
Page* createMeltPage();

/* File utilities (FilePages.cpp) */
Page* createRenamePage();
Page* createVinaPage();
Page* createPdbqtPage();
Page* createXplorPage();

#endif
