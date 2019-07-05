#ifndef DBF_H
#define DBF_H
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <exception>
#include "Segment.h"

class DBF {
private:
	std::map<std::string, std::vector<Segment> > segments;
	//std::map<std::string, std::vector<std::string> > segmentData;
	std::map<std::string, std::vector<std::string> > burSegData;
		
	std::vector<std::string> segmentKeys;
	std::vector<std::string> burFileSegKeys;
	std::vector<std::string> burData;
		
	std::string bureau;//Equifax, Experian, maybe TU at some point
	std::string OUTDBFPath;
	std::string dbfFileStr;
	std::string header;
	std::string endFiller;
	std::string editSeg;
		
	bool loadedDBF;
	bool didReadDBF;

public:
	DBF() {};
	DBF(const std::string& OUTDBFPath, const std::string& bur);

	static unsigned short int OUTDBFSectionLens[5];
	void parseBureauFile(const std::string&);
	void editBureauFile();
	
private:
	bool loadDBF();
	bool readDBF();
	static void trimContent(std::string&);
	bool pickSegToEdit();
	void populateTempTxt();
};


#endif
