#include "opencv2\opencv.hpp"

#undef min

#include <Windows.h>

#include <sstream>
#include <istream>
#include <fstream>

#include <iomanip>
#include <vector>
#include <string>
#include <iostream>

static const int        cDepthWidth = 512;
static const int        cDepthHeight = 424;
static const int        cColorWidth = 1920;
static const int        cColorHeight = 1080;


std::vector<std::string> csv_read_row(std::istream& file, char delimiter) {
	std::stringstream ss;
	bool inquotes = false;
	std::vector<std::string> row;

	while (file.good()) {
		char c = file.get();
		if (!inquotes && c == '"') inquotes = true;
		else if (inquotes && c == '"') {
			if (file.peek() == '"') {
				ss << (char)file.get();
			}
			else {
				inquotes = false;
			}
		}
		else if (!inquotes && c==delimiter){
			row.push_back(ss.str());
			ss.str("");
		}
		else if (!inquotes && (c == '\r' || c == '\n')) {
			if (file.peek()=='\n') {file.get();}
			row.push_back(ss.str());
			return row;
		}
		else {
			ss << c;
		}	
	}
}



int main(void)
{
	std::string datafolder = "../data";


	//read calibration parameters
	std::ifstream intrinsics("../data/intrinsics.csv");

	//read csv file
	std::ifstream file("../data/infrared_data/Label.csv");
	std::ofstream outputFile("../data/3Dposition.csv");

	if (file.fail()) {
		OutputDebugString("no file error");
		return -1;
	}

	int lineNum = 0;

	CvScalar reference;



	while (file.good()) {
		lineNum++;
		if (lineNum == 1) { 
			csv_read_row(file, ','); 
			continue; 
		}

		std::vector<std::string> row = csv_read_row(file, ',');
		std::cout << row[0] << std::endl;

		char* IR_name = const_cast<char*>(row[0].c_str());
	    std::string temp = const_cast<char*>(row[0].replace(row[0].find("jpg"), 6, "binary").c_str());
		char* Depth_name = const_cast<char*>(temp.replace(0, 8, "../data/depth_data/depth2xyz_mapper").c_str());

		std::cout << Depth_name << std::endl;

		int label_x = std::stoi(row[2]);
		int label_y = std::stoi(row[3]);

		CvScalar* pointArray = (CvScalar*)malloc(sizeof(CvScalar) * cDepthHeight * cDepthWidth);

		std::ifstream in(Depth_name, std::ios::in | std::ios::binary);
		OutputDebugString("read start");
		in.read((char*) pointArray, sizeof(CvScalar) * cDepthHeight * cDepthWidth);
		OutputDebugString("read end");

		if (lineNum == 2) {
			reference = pointArray[label_y * cDepthWidth + label_x];
		}

		CvScalar point = pointArray[label_y * cDepthWidth + label_x];

		outputFile << row[0] << "," << point.val[0] << "," << point.val[1] << "," << point.val[2] << ","  << point.val[0] - reference.val[0] << "," << point.val[1] - reference.val[1] << "," << point.val[2] - reference.val[2] << "\n";
		in.close();
		
		free(pointArray);


	}

	outputFile.close();
	file.close();


}