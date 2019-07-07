#ifndef SEGMENT_H
#define SEGMENT_H
#include <string>
#include <iostream>


struct Segment {
	std::string name;
	std::string desc;
	unsigned int pos;
	unsigned int len;
	std::string varName;

	Segment();
};

void printSeg(const Segment& seg);

#endif