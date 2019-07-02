#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <exception>
#include "Segment.h"
using namespace std;

/*
 * Author: Ezekiel Williams
 * Date: June 26, 2019
 * Version: 1.0 alpha
 * Version Info: can parse Equifax bureau files, open them in vim, save the changes, and output
 *				 back into the bureau file. Currently saves to temporary bureau file that you must overwrite.
 * 
 * Planned Features: ask to overwrite file, automatically tell which bureau the bureau file contains, confirm
 *					 changes the user made, and possibly options to edit different segments (FULL or 350 e.g.).
 *					 There are also plans to make this available to Experian bureau files as well.
 *
 * Description: This was meant to be a tool like dbfedit but for bureau files so editing them is much easier
*/


// global variables
const string DBF_LOCATION = "/home/ewilliams/cvs/wfsp/crit/";
// this map will contain the segment names as keys (PT or 357 e.g.) and the description, data parsing info, and var name for each section in the vector
map<string, vector<Segment> > dbfMap;
// old file is the bureau file you start with and the new file will have the changes the user makes
vector<string> oldFile;
vector<string> newFile;
// string for the bureau file name that comes in as an argument
string burFile;
// bureau specific vars
string trSeg;// = " PT ";//"@357";//" PT ";
string DBF;// = DBF_LOCATION + "CBIOUT.DBF";//"TRWOUT.DBF";//"CBIOUT.DBF"
string MAP_SEG;// = "PT";//"357";//"PT";

// function prototypes
// automatically tell which type of bureau it is
void whichBur();
// read the proper dbf to get parsing information
void readDBF();
// parse the bureau file using the dbf info and store into oldFile
string parse();
// right trim function for dbf info
void rtrim(string&);
// since c++11 is not available, my own stoi function
int strtoi(const string&);
// output and save the changes to the bureau file TODO: actually overwrite the bureau file with the changes
void save(string);

int main(int argc, char* argv[]) {
	// this will save processing power and time when pushing segments into these
	newFile.reserve(256);
	oldFile.reserve(256);

	// make sure an argument was entered TODO: check the file's name for RX (make sure it's a bureau file)
	if (argc != 2) {
		cout << "Enter the bureau file you want to edit as the argument!" << endl;
		return 1;
	}
	
	// pass the argument into a string (might want for later)
	burFile = argv[1];
	
	whichBur();
	
	// read from CBI equifax dbf and populate our dbfMap with segments TODO: automatically differentiate between CBI and TRW
	readDBF();

	// parse the bureau file and output to tempBur.txt
	string beforeTrades = parse();

	// open the tempBur.txt file we made in parse() for the user to edit
	
	system("vi tempBur.txt");

	// when the user is done editing, we will save back to the bureau file
	save(beforeTrades);
	

	return 0;
}

void readDBF() {
	string fullPath = DBF_LOCATION + DBF;
	ifstream dbfIn(fullPath.c_str());
	string ln = "";
	// get and throw away the header
	std::getline(dbfIn, ln, ' ');
	char abc[50];

	Segment segCpy;
	
	short int i = 1;
	do {
		char* getln = new char[25];
		int pos = 0, len = 0;
	
		switch (i) {
		case 1:
			dbfIn.get(getln, 8);
			ln = getln;
			rtrim(ln);
			segCpy.name = ln;
			break;
		case 2:
			//dbfIn.readsome(getln, 17);
			dbfIn.get(getln, 18);
			ln = getln;
			rtrim(ln);
			segCpy.desc = ln;
			break;
		case 3:
			dbfIn.get(getln, 5);
			ln = getln;
			rtrim(ln);
			pos = strtoi(ln);
			segCpy.pos = pos;
			break;
		case 4:
			dbfIn.get(getln, 5);
			ln = getln;
			rtrim(ln);
			len = strtoi(ln);
			segCpy.len = len;
			break;
		case 5:
			dbfIn.get(getln, 21);
			ln = getln;
			rtrim(ln);
			segCpy.varName = ln;
			dbfMap[segCpy.name].push_back(segCpy);
			while (dbfIn.get() == ' ') {}
			if (segCpy.name == "[DOWN" && ln == "&Filler") {
				// on the final segment, we need to reach eof to exit the loop
				char a = dbfIn.get();
			}
			else {
				dbfIn.seekg(-1, dbfIn.cur);
			}
			i = 0;
			break;
		}
		
		delete [] getln;
		getln = NULL;

		++i;

	} while (dbfIn.good());
	
	dbfIn.close();

}

void rtrim(string& s) {
	size_t end = s.find_last_not_of(" \n\r\t\f\v");
	s = (end == string::npos) ? "" : s.substr(0, end + 1);
}

int strtoi(const string& s) {
	stringstream ss(s);
	int i;
	ss >> i;
	return i;
}

void whichBur() {
	ifstream burIn(burFile.c_str());
	string line = "";
	
	std::getline(burIn, line);
	burIn.close();
	
	if (line.find(" PT ") != string::npos) {
		// do stuff
		trSeg = " PT ";
		DBF = "CBIOUT.DBF";
		MAP_SEG = "PT";
	}
	else if (line.find("@357") != string::npos) {
		// do stuff
		trSeg = "@357";
		DBF = "TRWOUT.DBF";
		MAP_SEG = "357";
	}
}

string parse() {
	// open the file associated with the string TODO: maybe check the file exists in current directory
	ifstream burIn(burFile.c_str());
	// open a temp file that the user will write changes to in vi
	ofstream tradeOut("tempBur.txt");

	int trSegSz = dbfMap[MAP_SEG].size();
	string entireFile;
	string beforeTrades;
	std::getline(burIn, entireFile);
	burIn.close();

	size_t pos = 0;
	int trCnt = 1;
	bool firstPos = true;
	// reopen after the getline to eof. TODO: find a more efficient way to reset the eof flag or something
	burIn.open(burFile.c_str());
	
	do {
		pos = entireFile.find(trSeg, pos);
		if (firstPos && pos != string::npos) {
			beforeTrades = entireFile.substr(0, pos + 1);
			firstPos = false;
		}

		if (pos == string::npos) {
			break;
		}
		else {
			++pos;
		}
			
		stringstream ss;
		ss << trCnt;

		tradeOut << "*************  Trade Number " << trCnt << "  ************" << endl;
		oldFile.push_back("*************  Trade Number " + ss.str() + "  ************\n");
		burIn.seekg(pos);
		int nextLen = -1;

		for (int i = 0; i < trSegSz; ++i) {
			char* data = new char[1002];
			Segment* pcurseg = &(dbfMap[MAP_SEG][i]);


			if (nextLen != -1)
				burIn.get(data, nextLen + 1);
			else
				burIn.get(data, pcurseg->len + 1);

			oldFile.push_back(pcurseg->desc + ": [" + data + "]\n");
			tradeOut << pcurseg->desc << ": [" <<  data << "]" << endl;

			if (MAP_SEG == "357" && pcurseg->varName.find("|") == string::npos && pcurseg->varName.find("_Len") != string::npos) {
				cout << "Found variable len: " << pcurseg->varName << " | len: " << pcurseg->len << endl;
				string dataStr = data;
				nextLen = strtoi(dataStr);
			}
			else {
				nextLen = -1;
			}

			delete [] data;
			data = NULL;
		}
		
		++pos;
		++trCnt;
	
	} while (true);

	burIn.close();
	tradeOut.close();
	return beforeTrades;
}

void save(string beforeTrades) {
	string burStr = beforeTrades;
	ifstream tempIn("tempBur.txt");
	string ln = "";

	do {
		std::getline(tempIn, ln);
		if (tempIn.good())
			newFile.push_back(ln + "\n");
		size_t dataStart = ln.find("[");
		if (dataStart == string::npos) continue;
		size_t dataEnd = ln.find("]");
		
		ln = ln.substr(dataStart + 1, dataEnd - dataStart - 1);
		burStr += ln;
	} while (tempIn.good());

	tempIn.close();

	//if (oldFile.size() == newFile.size()) {
	//	for (int i = 0; i < oldFile.size(); ++i) {
	//		if (oldFile[i] != newFile[i]) {
	//			cout << "Old > \033[0;36m" << oldFile[i] << "\033[0m";
	//			cout << "New > \033[1;32m" << newFile[i] << "\033[0m";
	//		}
	//		else {
	//			cout << "\033[1;37m" << oldFile[i] << "\033[0m";
	//		}
	//	}
	//}
	//else {
	//	cout << "The files are somehow not the same size..." << endl;
	//	cout << oldFile.size() << " : " << newFile.size() << endl;
	//}
	
	string newVersion = burFile + ".tmp";
	cout << "Now writing to file, '" << newVersion << "'. The original will remain untouched." << endl;
	
	ofstream burOut(newVersion.c_str());
	burOut << burStr;
	burOut.close();

	system("rm -f tempBur.txt");
}