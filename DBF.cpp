#include "DBF.h"

unsigned short int DBF::OUTDBFSectionLens[5] = {7, 17, 4, 4, 20};
bool DBF::isView() { return this->optionView; }

DBF::DBF(const std::string& OUTDBFPath, const std::string& bur, const std::string& optArg)
	: OUTDBFPath(OUTDBFPath)
	, burFilePath("")
	, bureau(bur)
	, optArg(optArg)
{
	if (this->bureau == "E1") {
		this->OUTDBFPath += "CBIOUT.DBF";
		this->endFiller = "  ";
	}
	else if (this->bureau == "E2") {
		this->OUTDBFPath += "TRWOUT.DBF";
		this->endFiller = "@";
	}

	time_t curTm;
	time(&curTm);
	struct tm* tmObj;

	tmObj = localtime(&curTm);

	std::stringstream ss;
	ss << tmObj->tm_sec << tmObj->tm_min << tmObj->tm_hour << tmObj->tm_yday << tmObj->tm_year;

	this->timeTag = ss.str();
	//std::cout << this->timeTag << std::endl;
	this->tmpFileName = "tmp" + this->timeTag + "z.txt";
}

bool DBF::loadDBF() {
	//std::cout << "DBF Path is: " << this->OUTDBFPath << std::endl;
	std::ifstream dbfIn(this->OUTDBFPath.c_str());

	//std::cout << "Loading DBF..." << std::endl;

	if (dbfIn.is_open()) {
		//std::cout << "Successfully opened the dbf. Reading file..." << std::endl;

		std::getline(dbfIn, this->dbfFileStr);
		dbfIn.close();
	}
	else {
		std::cout << "Could not open the output DBF. Check the path for errors:\n";
		std::cout << this->OUTDBFPath << std::endl;
		return false;
	}
	return true;
}// end loadDBF func

bool DBF::readDBF() {

	size_t pos = this->dbfFileStr.find_first_of(' ');
	if (pos != std::string::npos) {
		++pos;// Include the space
		this->header = this->dbfFileStr.substr(0, pos);
	}
	else {
		std::cout << "No space after the header for some reason. Can't parse." << std::endl;
		return false;
	}

	int i = 0;
	std::string sectionContent = "";
	std::string oldSegName = "";
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
				if (seg.name != oldSegName) {
					oldSegName = seg.name;
					segmentKeys.push_back(seg.name);
				}
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
				this->segments[seg.name].push_back(seg);
				//printSeg(seg);
				break;
			}
		}
		++i;
		i %= 5;
		
	}//end of while loop

	if (segments.size() == 0) {
		std::cout << "There were no segments to read, or something went wrong." << std::endl;
		return false;
	}
	return true;
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
	this->burFilePath = burFilePath;

	// open bureauFile
	std::ifstream burIn(burFilePath.c_str());
		
	//std::cout << "Parsing bureau file. Starting to open file..." << std::endl;


	if (burIn.is_open()) {
		//std::cout << "Bureau file opened. Ready to parse." << std::endl;
			
		std::string burFile = "";
		std::getline(burIn, burFile);
			
		burIn.close();
		if (burFile == "") {
			std::cout << "The bureau file was read, but the result is an empty string. Exiting..." << std::endl;
			return;
		}

		size_t pos = 0;
		//std::cout << "Starting to loop through data now...\n" << std::endl;
		for (int i = 0; i < segmentKeys.size(); ++i) {
			// these need to be reset for every new segment name
			int segLen = 0;
			int segPos = 0;
			int subSegPos = 0;
			int subSegLen = 0;
			int nextLen = -1;
			int len = 0;//doesn't really need the default
			int subSegIndx = -1;
			std::string lastSubSeg = "";
			bool repeat = false;
			
			//std:: cout << "\nStarting to loop through the sections in segment \"" << segmentKeys[i] << "\"..." << std::endl;

			for (int j = 0; j < segments[segmentKeys[i]].size(); ++j) {
				if (nextLen != -1) {
					len = nextLen;
						if (subSegPos != 0 && subSegLen != 0 && subSegPos != subSegLen && len + subSegPos > subSegLen)
							len = subSegLen - subSegPos;
					nextLen = -1;
					//std::cout << "\nnextLen was not -1, so setting the len to " << len << "." << std::endl;
				}
				else {
					len = this->segments[segmentKeys[i]][j].len;
					if (subSegPos != 0 && subSegLen != 0 && subSegPos != subSegLen && len + subSegPos > subSegLen)
						len = subSegLen - subSegPos;
					//std::cout << "\nnextLen was -1, so setting the len to " << len << "." << std::endl;
				}
				
				if (segLen != 0 && len + segPos > segLen)
					len = segLen - segPos;
				
				std::string varName = this->segments[segmentKeys[i]][j].varName;
				std::string burReadLine = burFile.substr(pos, len);
					
				//std::cout << "The varName from the dbf is currently " << varName << ", and the data just read was:\n[" << burReadLine << "]" << std::endl;
					
				if (j == 0 && burReadLine != segmentKeys[i]) {
					//std::cout << "Segment \"" << segmentKeys[i] << "\" is not found in this bureau file. Skipping to next segment..." << std::endl;
					break;
				}
				else if (j == 0 && burSegData.find(segmentKeys[i]) == burSegData.end()) {
					this->burFileSegKeys.push_back(burReadLine);
				}
				
				if (varName.substr(0, 2) == "%|" && j == 1) {//varName.substr(2,7) == "Seg_Len") {
					segLen = atoi(burReadLine.c_str());
					//std::cout << "Found the current segment's length data. Setting that to " << segLen << std::endl;
				}
				else if (varName.substr(0, 2) == "%|") {
					subSegLen = atoi(burReadLine.c_str());
				}
				else if (varName[0] == '%') {
					nextLen = atoi(burReadLine.c_str());
					//std::cout << "Found length data for variable length attribute, " << varName << ". Setting the next data fetch length to " << nextLen << std::endl;
				}
				else if (varName.find("@|") != std::string::npos) {
					subSegPos = 0;
					subSegLen = 0;
					do {
						//std::cout << "Found a conditional subsegment in DBF, " << varName << "." << std::endl;
						size_t condSubPos = varName.find("@|");
						std::string subSegName = varName.substr(0, condSubPos);
							
						if (subSegName != burReadLine) {
							//std::cout << "The line read from bureau file does not have this subsegment. Finding the next one..." << std::endl;
							do {
								varName = this->segments[segmentKeys[i]][++j].varName;
							} while (j < segments[segmentKeys[i]].size() - 1 && varName.find("@|") == std::string::npos);
								
						}
						else {
							//std::cout << "This bureau file contains the subsegment " << subSegName << "." << std::endl;
							subSegPos = subSegName.length();
							lastSubSeg = subSegName;
							subSegIndx = j;
							break;
						}
						
					} while (j != segments[segmentKeys[i]].size() - 1);//varName != "&EOS");
					
					if (j == segments[segmentKeys[i]].size() - 1) {
						//std::cout << "Reached the end of the \"" << segmentKeys[i] << "\" segment. Continuing..." << std::endl;
						// this will extract the end of segments, like the '@' symbol for Experian bureau files
						len = this->segments[segmentKeys[i]][j].len;
						burReadLine = burFile.substr(pos, len);
						subSegIndx = -1;
					}
				}
				
				// TODO: only push back the segments the user wants to edit. Maybe just allow editing of the segments they want to see
				if (this->burSegData[segmentKeys[i]].capacity() < 512)
					this->burSegData[segmentKeys[i]].reserve(512);
						
				this->burSegData[segmentKeys[i]].push_back(burReadLine);
				this->burSegVarNames[segmentKeys[i]].push_back(varName);
				
				if (j == segments[segmentKeys[i]].size() - 1) {
					this->burSegVarNameSizes[segmentKeys[i]].push_back(this->burSegVarNames[segmentKeys[i]].size());
				}

				segPos += len;
				pos += len;
				if (subSegLen != 0 && subSegLen != subSegPos)
					subSegPos += len;
					
				//std::cout << "Saving the data read:[" << burReadLine << "]" << std::endl;
				//std::cout << "Current segment: " << segmentKeys[i] << std::endl;
				//std::cout << "i = " << i << " | Seg Len = " << segLen << " | segPos = " << segPos << " | nextLen = " << nextLen << " | len = " << len << " | subSegLen: " << subSegLen << " | subSegPos: " << subSegPos << std::endl;
				
				if (subSegIndx != -1 && burFile.substr(pos, segments[segmentKeys[i]][subSegIndx].len) == lastSubSeg) {
					//std::cout << "Found another subsegment \"" << lastSubSeg << "\" in bureau file. Parsing it again." << std::endl;
					//std::cout << "Last subseg: " << lastSubSeg << " | burFileSub: " << burFile.substr(pos, segments[segmentKeys[i]][subSegIndx].len) << std::endl;
					j = subSegIndx;
				}
				else if (j == segments[segmentKeys[i]].size() - 1 && burFile.substr(pos, segments[segmentKeys[i]][0].len) == segmentKeys[i]) {
					//std::cout << "Found another segment \"" << segmentKeys[i] << "\" in bureau file. Parsing it again." << std::endl;
					//std::cout << "Last seg: " << segmentKeys[i] << " | burFileSub: " << burFile.substr(pos, segments[segmentKeys[i]][0].len) << std::endl;
					repeat = true;
				}
			}// end of inner for-loop
			if (repeat) --i;
			//std::cout << "End of segment \"" << segmentKeys[i] << "\". Moving to next one..." << std::endl;
		}//end of outer for-loop

	}
	else {
		std::cout << "Could not open bureau file in current directory: " << burFilePath << std::endl;
	}
	
	
	//for (int i = 0; i < this->burSegData[burFileSegKeys[2]].size(); ++i) {
		//std::cout << "[" << this->burSegData[burFileSegKeys[2]][i] << "]" << std::endl;
	//}
}

bool DBF::pickSegToEdit() {
	
	std::cout << "\nWhich of these segments in the bureau file do you want to edit?\n" << std::endl;
	
	int i;
	for (i = 0; i < this->burFileSegKeys.size(); ++i) {
		std::cout << i + 1 << ": [" << this->burFileSegKeys[i] << "]" << std::endl;
	}
	std::cout << i + 1 << ": Cancel and Exit" << std::endl;
	
	std::cout << "\n>> ";
	int menuChoice = 0;
	std::string consoleIn;
	std::getline(std::cin, consoleIn);
		
	menuChoice = atoi(consoleIn.c_str());
	
	
	while (menuChoice > this->burFileSegKeys.size() + 1 || menuChoice < 1) {
		std::cout << "Enter a number that exists in the menu.\n>> ";
		std::getline(std::cin, consoleIn);
		menuChoice = atoi(consoleIn.c_str());
	}
	
	if (menuChoice == this->burFileSegKeys.size() + 1) {
		std::cout << "Quitting the program." << std::endl;
		return true;
	}
	
	std::cout << "\nEditing the \"" << this->burFileSegKeys[menuChoice - 1] << "\" segment." << std::endl;
	this->editSeg = this->burFileSegKeys[menuChoice - 1];
	
	return false;
}

void DBF::populateTempTxt() {
	this->preEditFile.reserve(256);
	std::ofstream editOut(this->tmpFileName.c_str());
		
	if (editOut.is_open()) {
		editOut << "Edit the segment data in between the brackets. It works best if you are in replace mode.\nSave and quit to continue.\n" << std::endl;
		this->preEditFile.push_back("Edit the segment data in between the brackets. It works best if you are in replace mode.\nSave and quit to continue\n\n");
		int j = 0;
		for (int i = 0; i < this->burSegData[this->editSeg].size(); ++i) {
			int sz = this->burSegVarNameSizes[this->editSeg][j];
			if (i == sz || i == 0) {
				this->preEditFile.push_back("************ NEW SEGMENT ************\n");
				editOut << "************ NEW SEGMENT ************" << std::endl;
				if (i != 0)
					++j;
			}
			// burSegData and burSegVarNames should be the same size
			std::string curName = "zzz";
			if (i <= this->burSegVarNames[this->editSeg].size()) {
				curName = this->burSegVarNames[this->editSeg][i];
			}
			
			editOut << curName << ": [" << this->burSegData[this->editSeg][i] << "]" << std::endl;
			this->preEditFile.push_back(curName + ": [" + this->burSegData[this->editSeg][i] + "]\n");
		}
		
		editOut.close();

	}
	else {
		std::cout << "Failed to open or create the editable text file, \"" << this->tmpFileName << "\"." << std::endl;
	}
}

void DBF::editBureauFile() {
	bool quit = false;
	
	do {
		quit = this->pickSegToEdit();
		bool done = false;
		if (!quit) {
			this->populateTempTxt();
			while (!done) {
				std::string cmd = "vi " + this->tmpFileName;
				system(cmd.c_str());
				//done = confirm changes (this will also check to see if the user accidentally deleted brackets [ or ]
				done = this->checkChanges();
				if (done) {
					std::cout << "\nWould you like to overwrite the bureau file? (y/n)>> ";
					std::string yn = "";
					std::string newName = this->burFilePath;

					std::getline(std::cin, yn);

					if (yn == "n") {
						std::cout << "What name do want the new file to have?\n>> ";
						std::getline(std::cin, newName);
					}

					this->rewriteBureauFile(newName);
					quit = true;
					break;
				}
				else {
					std::string any;
					std::cout << "Press any key and hit ENTER to continue..." << std::endl;
					std::getline(std::cin, any);
				}
			}
		}
	} while (!quit);
}

bool DBF::checkChanges() {
	// don't want to be appending the same vector each iteration in editBureauFile()
	if (this->postEditFile.size() == 0)
		this->postEditFile.reserve(256);
	else
		this->postEditFile.clear();

	// need to read the file each time after an edit
	std::ifstream checkIn(this->tmpFileName.c_str());

	if (checkIn.is_open()) {
		std::string ln;
		std::getline(checkIn, ln);
		while (checkIn.good()) {
			this->postEditFile.push_back(ln);
			std::getline(checkIn, ln);
		}
		checkIn.close();

		//unsigned int preSize = this->preEditFile.size();
		unsigned int postSize = this->postEditFile.size();
		
		// segment length check won't work if there are missing brackets, so they need to be divided this way
		// it will be annoying for the user to have to fix one thing only to find another error, but such is life
		if (this->checkMissingBrackets()) {
			return false;
		}
		else if (this->checkSegmentLengths()) {
			return false;
		}
		
	}
	else {
		std::cout << "Could not open or create \"tmpBur.txt\" to check your changes." << std::endl;
	}
	
	return true;
}

bool DBF::checkMissingBrackets() {
	//std::cout << "Hello there: " << this->postEditFile.size() << std::endl;
	bool hasMissingBracks = false;
	unsigned int postSize = this->postEditFile.size();

	for (int i = 0; i < postSize; ++i) {//std::string ln: this->postEditFile) {
		size_t start = this->postEditFile[i].find("[");
		size_t end = this->postEditFile[i].find("]");
		size_t colon = this->postEditFile[i].find(":");
			
		// Detect missing brackets. Every line with a colon will have data, and the same with one or two brackets
		if ((start != std::string::npos && end == std::string::npos)
			|| (start == std::string::npos && end != std::string::npos)
			|| (colon != std::string::npos && start == std::string::npos && end == std::string::npos))
		{
			std::cout << "!! It appears there are some brackets that have been deleted. Please fix line " << i + 1 << std::endl;
			hasMissingBracks = true;
		}
	}
	
	return hasMissingBracks;
}

bool DBF::checkSegmentLengths() {
	bool segLenMisMatch = false;
	int totalSegLen = 0;
	int statedSegLen = 0;
	int subSegLen = 0;
	int idIndex = 0;
	unsigned int postSize = this->postEditFile.size();
	
	for (int i = 0; i < postSize; ++i) {//std::string ln: this->postEditFile) {
		// TODO
	}
	
	return segLenMisMatch;
}

void DBF::rewriteBureauFile(const std::string& fileName) {
	std::string filePrefix = "";
	std::string fileNameUse = fileName;
	int pos = fileName.find_last_of('/');

	if (pos != std::string::npos) {
		filePrefix = fileName.substr(0, pos + 1);
		fileNameUse = fileName.substr(pos + 1);
		std::cout << "Original file name: " << fileNameUse << std::endl;
		std::cout << "File prefix: " << filePrefix << std::endl;
	}

	pos = fileNameUse.find('.');
	if (pos == std::string::npos) {
		std::cout << "No file extension provided. Your new file will be of \".txt\"." << std::endl;
		fileNameUse += ".txt";
	}

	// reattach prefix for writing to the file in the correct directory
	fileNameUse = filePrefix + fileNameUse;

	std::ifstream tmpIn(this->tmpFileName.c_str());
	std::ofstream rewriteOut(fileNameUse.c_str());


	if (tmpIn.is_open() && rewriteOut.is_open()) {
		// write to the bureau file all the segments before the one the user edited
		int keyIndex;
		//std::cout << "burFileSegKeys.size() = " << this->burFileSegKeys.size() << std::endl;
		for (int i = 0; i < this->burFileSegKeys.size(); ++i) {
			if (this->burFileSegKeys[i] != this->editSeg) {
				for (int j = 0; j < this->burSegData[this->burFileSegKeys[i]].size(); ++j) {
					rewriteOut << this->burSegData[this->burFileSegKeys[i]][j];
				}
			}
			else {
				keyIndex = i;
				break;
			}
		}

		std::string tmpLine;
		std::getline(tmpIn, tmpLine);
		while (tmpIn.good()) {
			int start = tmpLine.find("[");
			int end = tmpLine.find("]");

			if (start != std::string::npos && end != std::string::npos) {
				tmpLine = tmpLine.substr(start + 1, end - start - 1);
				rewriteOut << tmpLine;
			}
			std::getline(tmpIn, tmpLine);
		}

		// write to the bureau file all the segments after the one the user edited
		for (int i = keyIndex + 1; i < this->burFileSegKeys.size(); ++i) {
			for (int j = 0; j < this->burSegData[this->burFileSegKeys[i]].size(); ++j) {
				rewriteOut << this->burSegData[this->burFileSegKeys[i]][j];
			}
		}

		tmpIn.close();
		rewriteOut.close();

		// remove the temp file that was created for editing
		std::string rmCmd = "rm -f " + this->tmpFileName;
		system(rmCmd.c_str());
	}
	else {
		std::cout << "Could not open one of the two files: \"" << fileName << "\" or \"tmpBur.txt\"." << std::endl;
	}
}

void DBF::viewBureauFile() {
	this->pickSegToEdit();
	this->populateTempTxt();
	std::string cmd = "less " + this->tmpFileName;

	system(cmd.c_str());

	cmd = "rm -f " + this->tmpFileName;

	system(cmd.c_str());
}

void DBF::handleOption() {
	if (this->optArg == "--view")
		this->optionView = true;
}

void DBF::initOptions() {
	this->optionView = false;
}
