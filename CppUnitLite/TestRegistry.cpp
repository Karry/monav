#include "TestRegistry.h"


#include "Test.h"
#include "TestResult.h"



void 
TestRegistry::addTest(Test* test) 
{
	instance().add (test);
}


void 
TestRegistry::runAllTests(TestResult* result) 
{
	instance().run(*result);
}


TestRegistry& 
TestRegistry::instance() 
{
	static TestRegistry registry;
	return registry;
}


void 
TestRegistry::add(Test *test) 
{
	if (tests == 0) {
		tests = test;
		return;
	}
	
	test->setNext(tests);
	tests = test;
}


void 
TestRegistry::run(TestResult& result) 
{
	result.testsStarted ();

	for (Test *test = tests; test != 0; test = test->getNext()) {
		numTests++;
		test->run(result);
	}

	result.testsEnded();
}


unsigned long
TestRegistry::GetNumTestsRun()
{
	return instance().numTests;
}
