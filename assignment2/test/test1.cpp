#include <iostream>
#include <boost/test/minimal.hpp>
#include "../src/LoC.hpp"

using namespace std;

// Target file
static const string TARGET_FILE = "../src/target.cpp";

int test_main(int, char *[])
{
	Parser p(TARGET_FILE);
    Counter c;

    string s;
    while((s = p.next()) != "")
        c.analyzeLine(s);
    c.outputAnalysis(cout);
    
	
	BOOST_CHECK(c.getTotalLoC()==7);

	return 0;
}