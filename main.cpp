#include <iostream>
#include <unistd.h>
#include <cstring>

#include <sys/stat.h>
#include <vector>
#include <fstream>

static const char options[] = "f:v:";


#define FILE_PATH_MAX_LEN 250


std::string GetFileName(std::string fullPath)
{
	//TODO::Windows has another Delimiter
	std::string delimiter = "/";
	
	size_t pos = 0;
	std::string token;
	std::vector<std::string> tokens;
	
	while ((pos = fullPath.find(delimiter)) != std::string::npos)
	{
		token = fullPath.substr(0, pos);
		tokens.push_back(token);
		fullPath.erase(0, pos + delimiter.length());
	}
	tokens.push_back(fullPath);

	return tokens.back();
}

std::string GetFilePath(std::string fullPath)
{
	//TODO::Windows has another Delimiter

#ifdef __linux__
	std::string delimiter = "/";
//	std::string delimiter = "\\";
#elif _WIN32
	std::string delimiter = "\\";
#endif
	
	
	
	unsigned int pos = 0;
	std::string token;
	std::vector<std::string> tokens;
	std::string result = "";
	
	while ((pos = fullPath.find(delimiter)) != std::string::npos)
	{
		token = fullPath.substr(0, pos);
		tokens.push_back(token);
		fullPath.erase(0, pos + delimiter.length());
	}
	//DO not push last!
	//tokens.push_back(fullPath);
	//result += delimiter;
	for(std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it)
	{
		if (*it != "")
			result += (delimiter + *it);
	}
	
	return result;
}


void ShowHelp()
{
	std::cerr << "Usage: ./ZigBeeOtaFilesVersionHacker -f \"{filepath}\"" << std::endl;
}


int main(int argc, char** argv)
{
	bool debug = true;
	
	std::cout << "ZigBee Ota-Files Version Hacker. Version: " << "1.0.0" << std::endl;
	std::string input = "";
	
	if ( (argc <= 1) || (argv[argc-1] == NULL) || (argv[argc-1][0] == '-') )
	{
		std::cerr << "No argument provided!" << std::endl;
		ShowHelp();
		return 1;
	}
	else
	{
		input = argv[argc-1];
	}

	
	int c;
	int optionCount = 0;
	char filePath[FILE_PATH_MAX_LEN];
	uint32_t version = 0;
	
	while (true)
	{
		c = getopt(argc, argv, options);
		if (c == -1) {
			if (optind != argc )
			{
				std::cout << "Invalid option! = " << argv[optind] << std::endl;
			}
			break;
		}
		
		optionCount++;

		switch (c)
		{
			case 'f':
				sscanf(optarg, "%[^\n]s", filePath);
				
				if (strlen(filePath) >= FILE_PATH_MAX_LEN - 1)
				{
					std::cerr << "FilePath is TOO LONG!" << filePath << std::endl;
					return 1;;
				}
				else
				{
					std::cout << "FilePath: " << filePath << std::endl;
				}
				break;
			case 'v':
				if (optarg)
				{
					version = atoi(optarg);
					std::cout << "Version to be set: " << version << std::endl;
				}
				break;
			default:
				std::cerr << "No argument provided!" << std::endl;
				return 1;;
				break;
		}   // end of switch (c)
	} //end while
	
	
	std::string givenFilePath{filePath};
	std::cout << "Given File Path: " << givenFilePath << std::endl;
	
	struct stat statInfo;
	if (0 != stat(givenFilePath.c_str(), &statInfo))
	{
		std::cerr << "Provided file " << givenFilePath <<" doesn't exist!" << std::endl;
		return 1;
	}
	
	//copy near
	std::string fileName = GetFileName(givenFilePath);
	std::cout << "Filename: " << fileName << std::endl;
	
	//rename file
	std::string versionUpgradedFileName = fileName + "-new_version";
	
	std::ifstream  src(givenFilePath.c_str(), std::ios::binary);
	std::ofstream  dst((GetFilePath(givenFilePath)+ "/" + versionUpgradedFileName).c_str(), std::ios::binary);
	
	std::cout << "ZigBeeUpdateDevice: Copied filePath: " + GetFilePath(givenFilePath)+ "/" + versionUpgradedFileName << std::endl;
	
	dst << src.rdbuf();
	
	std::string newFilePath = GetFilePath(givenFilePath)+ "/" + versionUpgradedFileName;
	
	struct stat newFileStatInfo;
	if (0 != stat(newFilePath.c_str(), &newFileStatInfo))
	{
		std::cerr << "Provided file " << newFilePath <<" doesn't exist!" << std::endl;
		return 1;
	}
	
	//https://stackoverflow.com/questions/37266047/modify-bytes-hex-of-a-save-file-with-c
	
	//TODO::Continue here, changing necesary byte.
	//https://stackoverflow.com/questions/37266047/modify-bytes-hex-of-a-save-file-with-c
	std::fstream binaryFile(newFilePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
	
	uint32_t versionFirstByte = 0x0e; //LSB!
	uint32_t versionSecondByte = 0x0f;
	uint32_t versionThirdByte = 0x10;
	uint32_t versionFourthByte = 0x11; //MSB!
	
	std::vector<char> versionAddresses { 0x0E, 0x0F, 0x10, 0x11 };
	
	std::vector<char> versionBytes;
	
	for (std::vector<char>::iterator it = versionAddresses.begin(); it != versionAddresses.end() ; ++it)
	{
		binaryFile.seekg(*it, std::ios::beg);
		char byte;
		binaryFile.read(&byte, 1);
		//std::cout << "Version byte: " << +byte << std::endl;
		versionBytes.push_back(byte);
	}
	
	unsigned int currentVersion = 0;
	unsigned int bias = 0;
	for (std::vector<char >::iterator it = versionBytes.begin(); it != versionBytes.end() ; ++it, ++bias)
	{
		//std::cout << "Version byte: " << +(*it) << std::endl;
		currentVersion |= ((*it) << (bias * 8));
	}
	
	std::cout << "Current version: " << currentVersion << std::endl;
	
//	std::vector<char> versionAddressesCheck { 0x00, 0x01, 0x02, 0x03 };
//	std::vector<char> versionBytesCheck;
//
//	for (std::vector<char>::iterator it = versionAddressesCheck.begin(); it != versionAddressesCheck.end() ; ++it)
//	{
//		binaryFile.seekg(*it, std::ios::beg);
//		char byte;
//		binaryFile.read(&byte, 1);
//		std::cout << "Version byte: " << std::hex << +(unsigned char)(byte) << std::endl;
//		versionBytesCheck.push_back(byte);
//	}
	

	//Change Version
	unsigned int nextVersion = ++currentVersion;
	
	std::cout << "Version will be: " << nextVersion << std::endl;
	
	std::vector<char> nextVersionData;
	
	for (int i = 0; i < 4; ++i)
	{
		nextVersionData.push_back((nextVersion >> i*8) & 0xFF);
	}
	
	int nextData = 0;
	for (std::vector<char>::iterator it = versionAddresses.begin(); it != versionAddresses.end(); ++it, ++nextData)
	{
		binaryFile.seekp(*it, std::ios::beg);
		binaryFile.write(&nextVersionData[nextData], 1);
	}
	
	return 0;
}