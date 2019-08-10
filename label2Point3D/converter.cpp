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

std::vector<std::string> ListDirectoryContents(const char* sDir, char* filename_pattern)
{
	std::vector<std::string> allfiles;

	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	char sPath[2048];

	//Specify a file mask. *.* = We want everything!
	sprintf(sPath, "%s\\*.*", sDir);

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		printf("Path not found: [%s]\n", sDir);
		//return false;
		return allfiles;
	}

	do
	{
		//Find first file will always return "."
		//    and ".." as the first two directories.
		if (strcmp(fdFile.cFileName, ".") != 0
			&& strcmp(fdFile.cFileName, "..") != 0)
		{
			//Build up our file path using the passed in
			//  [sDir] and the file/foldername we just found:
			sprintf(sPath, "%s\\%s", sDir, fdFile.cFileName);

			////Is the entity a File or Folder?
			//if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
			//{
			//    printf("Directory: %s\n", sPath);
			//    ListDirectoryContents(sPath); //Recursion, I love it!
			//}
			//else{
			//    printf("File: %s\n", sPath);
			//}
			if (strstr(sPath, filename_pattern))
				allfiles.push_back(sPath);
		}
	} while (FindNextFile(hFind, &fdFile)); //Find the next file.

	FindClose(hFind); //Always, Always, clean things up!

	return allfiles;
}

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

std::string replaceAll(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)  //from�� ã�� �� ���� ������
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // �ߺ��˻縦 ���ϰ� from.length() > to.length()�� ��츦 ���ؼ�
	}
	return str;
}

int main(void)
{
	std::string datafolder = "./data";

	
	//read calibration parameters
	double cx_d = 1, cy_d = 1, fx_d = 1, fy_d = 1;


	//read csv file
	std::ifstream file("./data/label.csv");
	std::ofstream outputFile("./data/result.csv");

	if (file.fail()) {
		OutputDebugString("no file error");
		return -1;
	}

	int lineNum = 0;

	while (file.good()) {
		lineNum++;

		std::vector<std::string> row = csv_read_row(file, ',');
		std::cout << row[0] << std::endl;

		char* IR_name = const_cast<char*>(row[0].c_str());
		char* Depth_name = const_cast<char*>(row[0].replace(0, 2, "raw_depth").c_str());
		
		int label_x = std::stoi(row[1]);
		int label_y = std::stoi(row[2]);

		// load Depth image
		IplImage* cvDepthImage = cvCreateImage(cvSize(cDepthWidth, cDepthHeight), IPL_DEPTH_16U, 1);
		cvDepthImage = cvLoadImage(Depth_name, CV_LOAD_IMAGE_ANYDEPTH);
		

		unsigned int depth = cvDepthImage->imageData[label_y * cvDepthImage->widthStep + label_x];

		double x_3d = (label_x - cx_d) * depth / fx_d;
		double y_3d = (label_y - cy_d) * depth / fy_d;
		double z_3d = depth;

		outputFile << IR_name << "," << x_3d << "," << y_3d << "," << z_3d << "\n";


	}

	outputFile.close();
	file.close();


}