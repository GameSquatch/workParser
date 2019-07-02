#include <iostream>
#include "DBF.h"
using namespace std;

// automatically tell which type of bureau it is
string whichBur(const string&);

int main(int argc, char* argv[]) {

	// if (argc != 2) {
	// 	cout << "Enter the bureau file you want to edit as the argument!" << endl;
	// 	return 1;
	// }
	
	// pass the argument into a string (might want for later)
	string burFile = "../B123MYRX.3JJ";//argv[1];
	string bureau = whichBur(burFile);
	cout << "Bureau is " << bureau << endl;

	if (bureau == "Error")
		return 2;

	string dbfPath = "../";

	DBF dbf(dbfPath, bureau);

	return 0;
}

string whichBur(const string& burFilePath) {
	ifstream burIn(burFilePath.c_str());

	if (burIn.is_open()) {
		string line;
		
		std::getline(burIn, line);
		burIn.close();
		
		if (line.find("PT") != string::npos) {
			return "Equifax";
		}
		else if (line.find("@357") != string::npos) {
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