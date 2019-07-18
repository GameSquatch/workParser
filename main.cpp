#include <iostream>
#include "DBF.h"
using namespace std;

// automatically tell which type of bureau it is
string whichBur(const string&);
string readConfigFile();

int main(int argc, char* argv[]) {

	if (argc < 2) {
	 	cout << "Enter the bureau file you want to edit as the argument." << endl;
	 	return 1;
	}
	
	// pass the argument into a string (might want for later)
	string burFile = argv[1];//"./B123MYRX.3Lo";//argv[1]
	
	string bureau = whichBur(burFile);
	//cout << "Bureau is " << bureau << endl;

	if (bureau == "Error")
		return 2;

	string dbfPath = readConfigFile();

	DBF dbf(dbfPath, bureau);

	bool loadedDBF = dbf.loadDBF();
	if (loadedDBF) {
		bool didReadDBF = dbf.readDBF();
		if (didReadDBF) {
			dbf.parseBureauFile(burFile);
			dbf.editBureauFile();
		}
		else {
			std::cout << "Failed reading the DBF. Report as potential bug." << std::endl;
			return 4;
		}
	}
	else {
		std::cout << "Failed loading the DBF. Not proceeding. Check your config file first." << std::endl;
		return 3;
	}

	return 0;
}

string whichBur(const string& burFilePath) {
	ifstream burIn(burFilePath.c_str());

	if (burIn.is_open()) {
		string line;
		
		std::getline(burIn, line);
		burIn.close();
		
		if (line.find("FULL") == 0) {
			return "Equifax";
		}
		else if (line.find("110") == 0) {
			return "Experian";
		}
		else {
			return "Error";
		}
	}
	else {
		cout << "Could not open \"" << burFilePath << "\"." << endl;
		return "Error";
	}
}

string readConfigFile() {
	// default path location
	string defaultPath = "/home/ewilliams/cvs/wfsp/crit/";
	
	// get current user's name to create the config file in their home directory
	system("echo $HOME > envtmp");
	
	ifstream userIn("envtmp");

	if (userIn.is_open()) {
		string ln;
		std::getline(userIn, ln);
		userIn.close();
		//cout << "Home directory obtained is: " << ln << endl;
		// create a path string to the user's home directory
		string configFile = ln + "/.burEdCfg";
	
		// remove it once the username has been obtained.
		system("rm -f envtmp");
		
		// attempt to open an existing config file for reading. If that fails, they don't have one, so create a config file
		ifstream cfgIn(configFile.c_str());
		if (!cfgIn.is_open()) {
			cout << "Creating config file in your home directory: \"" << configFile << "\"" << endl;
			ofstream cfgOut(configFile.c_str());
			cfgOut << defaultPath;
			cfgOut.close();
		}
		else {
			// if user had a config file, read the path they want for their parse-map files and return it
			cout << "A config file exists. Reading preferred path to parse-map file...";
			string path = "";
			std::getline(cfgIn, path);
			cfgIn.close();
			
			if (path == "")
				path = defaultPath;
			
			cout << "\"" << path << "\"" << endl;
			return path;
		}
	}
	else {
		cout << "Could not open temporary user file. Permissions to create it possibly have something to do with it??" << endl;
	}
	
	return defaultPath;
}
