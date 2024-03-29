#ifndef DBF_H
#define DBF_H
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <exception>
#include <ctime>
#include <sstream>
#include "Segment.h"

class DBF {
private:
	std::map<std::string, std::vector<Segment> > segments;
	std::map<std::string, std::vector<std::string> > burSegData;
	std::map<std::string, std::vector<std::string> > burSegVarNames;
	std::map<std::string, std::vector<int> > burSegVarNameSizes;
		
	std::vector<std::string> segmentKeys;
	std::vector<std::string> burFileSegKeys;
	std::vector<std::string> burData;
	std::vector<std::string> preEditFile;
	std::vector<std::string> postEditFile;
		
	std::string bureau;//Equifax, Experian, maybe TU at some point
	std::string burFilePath;
	std::string OUTDBFPath;
	std::string dbfFileStr;
	std::string header;
	std::string endFiller;
	std::string editSeg;
	std::string timeTag;
	std::string tmpFileName;
	std::string optArg;

	bool optionView;

public:
	DBF() {};
	DBF(const std::string& OUTDBFPath, const std::string& bur, const std::string& optArg);

	static unsigned short int OUTDBFSectionLens[5];
	
	bool loadDBF();
	bool readDBF();
	bool isView();
	void parseBureauFile(const std::string&);
	void viewBureauFile();
	void editBureauFile();
	
private:
	static void trimContent(std::string&);
	bool pickSegToEdit();
	void populateTempTxt();
	bool checkChanges();
	bool checkMissingBrackets();
	bool checkSegmentLengths();
	void rewriteBureauFile(const std::string&);
	void initOptions();
	void handleOption();
};


#endif
