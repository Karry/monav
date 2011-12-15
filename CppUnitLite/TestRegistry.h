#pragma once


class Test;
class TestResult;


class TestRegistry
{
public:
	TestRegistry() : tests(0), numTests(0)	{ }

	static void addTest (Test* test);
	static void runAllTests (TestResult* result);

	static unsigned long GetNumTestsRun();

private:
	static TestRegistry&	instance ();
	void					add (Test* test);
	void					run (TestResult& result);

	Test*	tests;
	unsigned long numTests;
};
