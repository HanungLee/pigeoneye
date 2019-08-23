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


/*
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
*/

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

	double intrinsic_array[4];
	if (intrinsics.fail()) {
		OutputDebugString("no intrinsic error");
		return -1;
	}

	int line = 0;
	while (intrinsics.good()) {
		std::vector<std::string> row = csv_read_row(intrinsics, ',');
		intrinsic_array[line] = std::stod(row[1]);
		line++;

		std::cout << intrinsic_array[line - 1] << std::endl;
	}

	double fx_d = intrinsic_array[0];
	double fy_d = intrinsic_array[1];
	double cx_d = intrinsic_array[2];
	double cy_d = intrinsic_array[3];

	

	//read csv file
	std::ifstream file("../data/infrared_data/Label.csv");
	std::ofstream outputFile("../data/3Dposition.csv");

	if (file.fail()) {
		OutputDebugString("no file error");
		return -1;
	}

	int lineNum = 0;

	while (file.good()) {
		lineNum++;
		if (lineNum == 1) { 
			csv_read_row(file, ','); 
			continue; 
		}

		std::vector<std::string> row = csv_read_row(file, ',');
		std::cout << row[0] << std::endl;

		char* IR_name = const_cast<char*>(row[0].c_str());
	    std::string temp = const_cast<char*>(row[0].replace(row[0].find("jpg"), 3, "png").c_str());
		char* Depth_name = const_cast<char*>(temp.replace(0, 8, "../data/depth_data/depth2xyz_mapper").c_str());

		std::cout << Depth_name << std::endl;

	
		int label_x = std::stoi(row[2]);
		int label_y = std::stoi(row[3]);

		// load Depth image
		IplImage* cvDepthImage = cvCreateImage(cvSize(cDepthWidth, cDepthHeight), IPL_DEPTH_16U, 3);
		cvDepthImage = cvLoadImage(Depth_name, CV_LOAD_IMAGE_UNCHANGED);
		
		int x = CV_IMAGE_ELEM(cvDepthImage, int, label_y, label_x * 3);
		int y = CV_IMAGE_ELEM(cvDepthImage, int, label_y, label_x * 3 + 1);
		int z = CV_IMAGE_ELEM(cvDepthImage, int, label_y, label_x * 3 + 2);
		
		
		/*cv::Mat* cvDepthImage2 = cvCreateImage(cDepthHeight, cDepthWidth, CV_32FC3);
		cvDepthImage2 = cvLoadImage(Depth_name, CV_LOAD_IMAGE_UNCHANGED);
		double x = cvGet2D(cvDepthImage2, label_y, label_x).val[0];
		double y = cvGet2D(cvDepthImage2, label_y, label_x).val[1];
		double z = cvGet2D(cvDepthImage2, label_y, label_x).val[2];
		*/ 

		/*cv::FileStorage fs2(Depth_name, cv::FileStorage::READ);
		cv::Mat mat;
		fs2["yourMat"] >> mat;

		double x = mat.at<cv::Vec3b>(label_y, label_x)[0] ;
		double y = mat.at<cv::Vec3b>(label_y, label_x)[1];
		double z = mat.at<cv::Vec3b>(label_y, label_x)[2];
		*/



		/*std::ifstream depth2xyz(Depth_name);
		int index = label_y * 512 + label_x;
		int i = 0;
		while (depth2xyz.good()) {
			std::vector<std::string> row2 = csv_read_row(depth2xyz, ',');

			if (i == index) {
				outputFile << row[0] << "," << row2[0] << "," << row2[1] << "," << row2[2] << "\n";
				break;

			}
			i++;
		}*/





		/*
		double x = (double)cvDepthImage->imageData[label_y * cvDepthImage->widthStep + label_x*cvDepthImage->nChannels];
		double y = (double)cvDepthImage->imageData[label_y * cvDepthImage->widthStep + label_x * cvDepthImage->nChannels+1];

		double z = (double)cvDepthImage->imageData[label_y * cvDepthImage->widthStep + label_x * cvDepthImage->nChannels+2];
		*/

		//UINT16 depth = mat.at<UINT16>(label_y, label_x);


/*
		std::cout << depth << std::endl;

		double x_3d = (label_x - cx_d) * depth / fx_d;
		double y_3d = (label_y - cy_d) * depth / fy_d;
		double z_3d = depth;
		*/


		outputFile << row[0] << "," << x << "," << y << "," << z << "\n";


	}

	outputFile.close();
	file.close();


}