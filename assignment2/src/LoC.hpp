#include <string>
#include <list>
#include <stack>
#include <map>

using namespace std;

string trim(const string& str);

class Parser
{
private:
    ifstream* _file;

public:
    Parser(string file);
    ~Parser();

    const string next();
};

class Part
{
protected:
    string _name;
    int _depth;
    int _loc;

public:
    Part(string name, int depth, int loc = 1);
    const string getName();
    const int getDepth();
    virtual const int getLOC();
    virtual void addLine(string s);
};

class Class : public Part
{
private:
    list<Part*> _methods;

public:
    Class(string name, int depth);
    ~Class();
    virtual const int getLOC();
    virtual void addLine(string s);
    Part* newMethod(string name, int depth);
    const list<Part*> getMethods();
};

class Counter
{
private:
    stack<Part*> _stack;
    int _depth;
    bool _comment;  
    int _totalLoC;

    Part _preprocessing;
    Part _globals;
    list<Part*> _functions;
    map<string, Class*> _classes;

    bool checkForComments(const string& line);
    bool checkForPreProcessing(const string& line);
    bool checkForBraces(const string& line);
    bool checkForFunctions(const string& line);
    bool checkForHeader(const string& line);

public:
    Counter();

    void analyzeLine(string& line);
    void outputAnalysis(ostream& os);
    void setTotalLoC(int nb);
    int getTotalLoC();
};
