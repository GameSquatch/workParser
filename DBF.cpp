#include "DBF.h"

unsigned short int DBF::OUTDBFSectionLens[5] = {7, 17, 4, 4, 20};

DBF::DBF(const std::string& OUTDBFPath, const std::string& bur)
	: OUTDBFPath(OUTDBFPath)
	, bureau(bur)
{
	if (this->bureau == "Equifax") {
		this->OUTDBFPath += "CBIOUT.DBF";
		this->endFiller = "  ";
	}
	else if (this->burea == "Experian") {
		this->OUTDBFPath += "TRWOUT.DBF";
		this->endFiller = "@";
	}

	loadDBF();
	readDBF();
}

void DBF::loadDBF() {
	std::cout << "DBF Path is: " << this->OUTDBFPath << std::endl;
	std::ifstream dbfIn;
	std::filebuf* pdbfIn = dbfIn.rdbuf();
	pdbfIn->open(this->OUTDBFPath.c_str(), std::ios::in);


	if (pdbfIn->is_open()) {
		// Find the end of the file, read the entire thing, and put it in a string
		long dbfSize = pdbfIn->pubseekoff(0, dbfIn.end);
		pdbfIn->pubseekpos(0);
		try {
			char* buff = new char[dbfSize + 1];
			pdbfIn->sgetn(buff, dbfSize);
			this->dbfFileStr = buff;
			delete [] buff;//don't forget to delete! :D
		}
		catch (std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
		}

		pdbfIn->close();
	}
	else {
		std::cout << "Could not open the output DBF. Check the path for errors:\n";
		std::cout << this->OUTDBFPath << std::endl;
	}
}// end loadDBF func

void DBF::readDBF() {
	size_t pos = this->dbfFileStr.find_first_of(' ');
	if (pos != std::string::npos) {
		++pos;// Include the space
		this->header = this->dbfFileStr.substr(0, pos);
		std::cout << "[" << this->header << "]" << std::endl;
	}
	else {
		std::cout << "No space after the header for some reason. Can't parse." << std::endl;
	}

	int i = 0;
	std::string sectionContent = "";
	int sectionLen = 0;
	Segment seg;
	while (true) {
		// length of the section in the dbf we are about to read
		sectionLen = DBF::OUTDBFSectionLens[i];

		if (pos + sectionLen > this->dbfFileStr.length() || pos == std::string::npos) {
			break;
		}
		else {
			sectionContent = this->dbfFileStr.substr(pos, sectionLen);
			DBF::trimContent(sectionContent);

			unsigned int contentInt = 0;
			pos += sectionLen;
			if (i == 2 || i == 3) {
				contentInt = (unsigned int)atoi(sectionContent.c_str());
			}

			switch(i) {
			case 0:
				seg.name = sectionContent;
				break;
			case 1:
				seg.desc = sectionContent;
				break;
			case 2:
				seg.pos = contentInt;
				break;
			case 3:
				seg.len = contentInt;
				break;
			case 4:
				seg.varName = sectionContent;
				pos = this->dbfFileStr.find_first_not_of(' ', pos);
				this->segmentSection[seg.name].push_back(seg);
				//printSeg(seg);
				break;
			}
		}
		++i;
		i %= 5;
	}//end of while loop
}//end readDBF func

void DBF::trimContent(std::string& content) {
	size_t startPos = content.find_first_not_of(' ', 0);
	size_t endPos = content.find_last_not_of(' ');

	if (startPos != std::string::npos && endPos != std::string::npos)
		content = content.substr(startPos, endPos - startPos + 1);
	else
		content = "  ";
}

void DBF::parseBureauFile(const std::string& burFilePath) {
	// open bureauFile
	std::ifstream burIn;
	std::filebuf* pBurIn = burIn.rdbuf();

	pBurIn->open(burFilePath.c_str());

	if (pBurIn->is_open()) {
		long sz = pBurIn->pubseekoff(0, burIn.end);
		pBurIn->pubseekpos(0);
		char* burBuff = new char[sz];
		std::string burFile = burBuff;
		delete [] burBuff;

		size_t pos = 0;
		std::map<std::string, std::vector<Segment> >::iterator it = segmentSection.begin();

		while (pos < burFile.length()) {
			std::string key = it->first;
			pos = burFile.find(this->endFiller + key);
			if (pos != std::string::npos) {
				for (int i = 0; i < it->second.size(); ++i) {
					std::string data = burFile.substr(pos, it->second[i].len);
				}
			}
		}

	}
	else {
		std::cout << "Could not open bureau file in current directory: " << burFilePath << std::endl;
	}
}