#include <iostream>
#include "LoC.hpp"


using namespace std;

// Target file
static const string TARGET_FILE = "../src/target.cpp";

int main()
{
    Parser p(TARGET_FILE);
    Counter c;

    string s;
    while((s = p.next()) != "")
        c.analyzeLine(s);
    c.outputAnalysis(cout);

    return 0;
}
