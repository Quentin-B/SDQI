/******************************************************************
 * Program Assignment:  2
 * Name:                Quentin Bitschene, Guilhem Quilici
 * Date:                09/02/2014
 * Description:         counts the LOC of a C++ program
 ******************************************************************/

#include <algorithm>
#include <cctype>
#include <iostream>
#include <fstream>
#include <functional>
#include <list>
#include <locale>
#include <map>
#include <sstream>
#include <stack>

#include "LoC.hpp"

using namespace std;


/******************************************************************
 * Compilation instructions
 *   $ g++ -o main BitscheneQuiliciAssign2.cpp -std=c++11
 * 
 * Class declarations:
 *   Class
 *   Counter
 *   Parser
 *   Part
 ******************************************************************/

/******************************************************************
 * string trim(const string& str)
 *   Purpose:       erase heading and trailing blanks of str
 *   Return:        copy of the trimmed string
 ******************************************************************/
string trim(const string& str)
{
    // Copy the string
    string s = str;
    
    // Perform a left-trim
    s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));

    // Perform a right-trim
    s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
    return s;
}


Parser::Parser(string file)
{
    // Open the file in reading mode
    _file = new ifstream(file, ios::in);

    // Ensure file is correctly open
    if(_file == nullptr || !_file->is_open())
        cout << "IO error while opening file <" << file << ">" << endl;
}

Parser::~Parser()
{
    if (_file->is_open())
        _file->close();
    delete _file;
}

    


/******************************************************************
 * const string Parser::next()
 *   Purpose:   returns the next non-empty line of the file
 *   Return:    next line or "" if the eof has been reached
 ******************************************************************/
const string Parser::next()
{
    string line = "";

    while (line == "")
    {
        if (!getline(*_file, line))
            return "";
        
        line = trim(line);
    }

    return line;
}


Part::Part(string name, int depth, int loc) : 
    _name(name), 
    _depth(depth), 
    _loc(loc)
{
}

const string Part::getName()
{
    return _name;
}

const int Part::getDepth()
{
    return _depth;
}

const int Part::getLOC()
{
    return _loc;
}

void Part::addLine(string s)
{
    _loc++;
}



Class::Class(string name, int depth) : 
    Part(name, depth), 
    _methods()
{
}

Class::~Class()
{
    for_each(_methods.begin(), _methods.end(), [] (Part* m) { delete m; });
    _methods.clear();
}

const int Class::getLOC()
{
    int total = _loc;
    for_each(_methods.begin(), _methods.end(), [&total] (Part* m) { total += m->getLOC(); });
    return total;
}



Part* Class::newMethod(string name, int depth)
{
    Part* m = new Part(name, depth);
    _methods.push_back(m);
    return m;
}

const list<Part*> Class::getMethods()
{
    return _methods;
}


void Class::addLine(string s)
{
    if (s.find("public") != string::npos)
        return;
    if (s.find("private") != string::npos)
        return;
    _loc++;
}


Counter::Counter() : 
    _stack(), 
    _depth(0), 
    _comment(false), 
    _preprocessing("Pre-processing", 0, 0), 
    _globals("Global declarations", 0, 0), 
    _functions(), 
    _classes()
{
}

    


/******************************************************************
 * bool Counter::checkForComments(const string& line)
 *   Purpose:   checks if the line is part of a comment
 *   Return:    true if it is, false otherwise
 ******************************************************************/
bool Counter::checkForComments(const string& line)
{
    // Check for multi-line comments opening
    if (line.find("/*") == 0)
    {
        _comment = true;
        return true;
    }
    
    // Check for multi-line comments closing
    if (_comment && line.find("*/") != string::npos)
    {
        _comment = false;
        return true;
    }

    if (_comment)
        return true;

    // Check for line comments
    return line.find("//") == 0;
}

/******************************************************************
 * bool Counter::checkForPreProcessing(const string& line)
 *   Purpose:   checks if the line is a pre-processing instruction
 *   Return:    true if it is, false otherwise
 ******************************************************************/
bool Counter::checkForPreProcessing(const string& line)
{
    // Check for pre-processing instructions
    if (line.find("#") == 0)
    {
        _preprocessing.addLine(line);
        return true;
    }
    return false;
}

/******************************************************************
 * bool Counter::checkForBraces(const string& line)
 *   Purpose:   checks if the line is a brace
 *   Return:    true if it is, false otherwise
 ******************************************************************/
bool Counter::checkForBraces(const string& line)
{
    // Check for opening braces
    if (line.compare("{") == 0)
    {
        _depth++;
        return true;
    }

    // Check for closing braces
    if (line.compare("}") == 0 || line.compare("};") == 0)
    {
        _depth--;
        if (_depth == _stack.top()->getDepth())
            _stack.pop();
        return true;
    }
    return false;
}

/******************************************************************
 * bool Counter::checkForFunctions(const string& line)
 *   Purpose:   checks if the line is a declaration of a function
 *   Return:    true if it is, false otherwise
 ******************************************************************/
bool Counter::checkForFunctions(const string& line)
{
    // Check the stack state
    if (!_stack.empty())
        return false;

    // Test if it's a global declaration
    if (line.find(";") == line.size() - 1)
        return false;

    // Test if it's a member-function declaration
    if (line.find("::") != string::npos)
    {
        // Get the name of the class
        int end = line.find("::");
        int begin = line.substr(0, end).find_last_of(" ");
        string className = line.substr(begin + 1, end - begin - 1);
        
        // Create a new part for the member-function
        Class* c = _classes[className];
        _stack.push(c->newMethod(line, _depth));
        return true;
    }
    
    // It's a free-function declaration
    Part* f = new Part(line, _depth);
    _functions.push_back(f);
    _stack.push(f);
    return true;
}

/******************************************************************
 * bool Counter::checkForHeader(const string& line)
 *   Purpose:   checks if the line is a declaration of a header
 *   Return:    true if it is, false otherwise
 ******************************************************************/
bool Counter::checkForHeader(const string& line)
{
    // Check for class header
    istringstream iss(line);
    string keyWord;
    iss >> keyWord;

    // Check the first word of the line
    if (keyWord == "class")
    {
        string className;
        iss >> className;
        
        // Create a new part for the class
        Class* c = new Class(className, _depth);
        _classes[className] = c;
        _stack.push(c);
        return true;
    }
    
    return false;
}

/******************************************************************
 * void Counter::analyzeLine(string& line)
 *   Purpose:   analyze the line to determine its logic part
 ******************************************************************/
void Counter::analyzeLine(string& line)
{
    // Check for comments
    if (checkForComments(line))
        return;

    // Check for pre-processing instructions
    if (checkForPreProcessing(line))
        return;

    // Check for braces
    if (checkForBraces(line))
        return;

    // Check for class header
    if (checkForHeader(line))
        return;

    // Check for functions
    if (checkForFunctions(line))
        return;

    // Add line to the current part
    if (!_stack.empty())
        _stack.top()->addLine(line);
    else
        _globals.addLine(line);
}

/******************************************************************
 * void Counter::outputAnalysis(ostream& os)
 *   Purpose:   display the LOC of each part of the program
 ******************************************************************/
void Counter::outputAnalysis(ostream& os)
{
    // Count the total LOC
    int total = _globals.getLOC();

    // Print the analysis part by part
    os << "Preprocessing instructions : " << _preprocessing.getLOC() << endl;
    os << "Global declarations        : " << _globals.getLOC() << endl;

    os << "Free functions" << endl;
    for_each(_functions.begin(), _functions.end(), [&total, &os] (Part* f) {
        total += f->getLOC();
        os << "\t" << f->getName() << " : " << f->getLOC() << endl;
    });

    os << "Classes" << endl;
    for_each(_classes.begin(), _classes.end(), [&total, &os] (pair<string, Class*> p) {
        Class* c = p.second;
        total += c->getLOC();
        os << "\t" << c->getName() << " : " << c->getLOC() << endl;

        list<Part*> methods = c->getMethods();
        for_each(methods.begin(), methods.end(), [&os] (Part* m) {
            os << "\t\t" << m->getName() << " : " << m->getLOC() << endl;
        });
    });
    
    // Print the total LOC
    os << endl << "TOTAL LOC = " << total << endl;
    setTotalLoC(total);
    os << "(ignoring pre-processing instructions and visibility modifiers)" << endl;
}

void Counter::setTotalLoC(int nb)
{
    _totalLoC=nb;
}

int Counter::getTotalLoC()
{
    return _totalLoC;
}

