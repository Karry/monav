#include "TestHarness.h"


#include "TestRegistry.h"


TestHarness testHarness;


const char*
TestHarness::RunAllTests()
{
	TestRegistry::runAllTests(&mTR);
	return(mTR.resultStr.GetCString());
}


unsigned long
TestHarness::GetNumTestsRun()
{
	return TestRegistry::GetNumTestsRun();
}
