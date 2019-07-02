#include "Segment.h"

Segment::Segment()
	: name("")
	, desc("")
	, pos(0)
	, len(0)
	, varName("")
{

}

void printSeg(const Segment& seg) {
	std::cout << "Name: " << seg.name << std::endl;
	std::cout << "Desc: " << seg.desc << std::endl;
	std::cout << "Pos: " << seg.pos << std::endl;
	std::cout << "Len: " << seg.len << std::endl;
	std::cout << "varName: " << seg.varName << std::endl;
}