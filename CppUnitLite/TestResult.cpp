#include "TestResult.h"


#include "Failure.h"

#include <cstring>


const char* FileNameFromPath(const char* path, char* outFileName);


TestResult::TestResult()
	:	resultStr(""), failureCount(0)
{
}


TestResult::~TestResult()
{
}


void
TestResult::testsStarted ()
{
}


void
TestResult::addFailure(const Failure& failure)
{
	resultStr += "Error:\t";
	resultStr += failure.mMessage.GetCString();
	resultStr += "\n\tline ";
	resultStr += StringFrom(failure.mLineNumber).GetCString();
	resultStr += " in ";

	char fileNameBuf[256];
	FileNameFromPath(failure.mFileName.GetCString(), fileNameBuf);
	SimpleString fileName = fileNameBuf;
	fileName += "\n\n";
	resultStr += fileName.GetCString();

	failureCount++;
}


void
TestResult::testsEnded()
{
	if (failureCount > 0)
		resultStr += "\n";

	resultStr += "There were ";
	if (failureCount > 0)
		resultStr += StringFrom(failureCount).GetCString();
	else
		resultStr += "no test";
	resultStr += " failures\n";
}


const char* FileNameFromPath(const char* path, char* outFileName)
{
	size_t fileNameStrLen = 0;
	size_t strLenPath = strlen(path);

	size_t i = strLenPath - 1;
	while (path[i] != '\\' && path[i] != '/') {
		i--;
		fileNameStrLen++;
		if (i == 0)
		    break;
	}

	size_t actPathPos = strLenPath - fileNameStrLen;
	for (i=0; i < fileNameStrLen; i++) {
		if (path[actPathPos + i] == '\\')
			break;

		outFileName[i] = path[actPathPos + i];
	}

	outFileName[i] = '\0';

	return outFileName;
}
