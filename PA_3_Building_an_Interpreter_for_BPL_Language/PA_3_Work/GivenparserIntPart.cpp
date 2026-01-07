/* Implementation of Interpreter
 * for the Basic Perl-Like (BPL) Language
 * parserInt.cpp
 * Programming Assignment 3
 * Fall 2025
*/


#include "parserInt.h"

map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> TempsResults; //Container of temporary locations of Value objects for results of expressions, variables values and constants 
queue <Value> * ValQue; //declare a pointer variable to a queue of Value objects

namespace Parser {

    bool pushed_back = false;
    LexItem pushed_token;
// for the other code dont forget to remove static
     LexItem GetNextToken(istream& in, int& line) {
        if (pushed_back) {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }
//same here
    void PushBackToken(LexItem &t) {
        if (pushed_back) {
            cerr << "PushBackToken(): double push" << endl;
            exit(1);
        }
        pushed_back = true;
        pushed_token = t;
    }
}


static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
	++error_count;
	cout << error_count << ". Line " << line << ": " << msg << endl;
}



