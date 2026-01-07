#include <iostream>
#include <map>
#include <queue>
#include "parserInt.h"
#include "lex.h"
#include "val.h"

using namespace std;

extern void ParseError(int line, string msg);
extern int ErrCount();

extern map<string, bool> defVar;
extern map<string, Value> TempsResults;
extern queue<Value>* ValQue;

namespace Parser {
    extern bool pushed_back;
    extern LexItem pushed_token;
    extern LexItem GetNextToken(istream& in, int& line);
    extern void PushBackToken(LexItem& t);
}

static bool IsDefined(const string& name) {
    return TempsResults.find(name) != TempsResults.end();
}

static bool BplTruth(const Value& v) {
    if (v.IsNum()) return v.GetNum() != 0.0;
    if (v.IsString()) {
        string s = v.GetString();
        return !(s == "" || s == "0");
    }
    if (v.IsBool()) return v.GetBool();
    return false;
}

bool Prog(istream& in, int& line) {

    if (!StmtList(in, line)) {
        cout << "\nUnsuccessful Interpretation" << endl;
        cout << "Number of Errors " << ErrCount() << endl;
        return false;
    }

    LexItem t = Parser::GetNextToken(in, line);
    if (t.GetToken() != DONE) {
        ParseError(line, "Unexpected token after program end");
        cout << "\nUnsuccessful Interpretation" << endl;
        cout << "Number of Errors " << ErrCount() << endl;
        return false;
    }

    cout << endl << endl;
    cout << "DONE" << endl;
    return true;
}

bool StmtList(istream& in, int& line) {

    if (!Stmt(in, line)) return false;

    LexItem tok = Parser::GetNextToken(in, line);

    if (tok.GetToken() != SEMICOL) {
        Parser::PushBackToken(tok);
        return true;
    }

    tok = Parser::GetNextToken(in, line);
    Parser::PushBackToken(tok);

    Token nxt = tok.GetToken();

    if (nxt == IDENT || nxt == IF || nxt == PRINTLN)
        return StmtList(in, line);

    return true;
}

bool Stmt(istream& in, int& line) {

    LexItem t = Parser::GetNextToken(in, line);
    Parser::PushBackToken(t);

    switch (t.GetToken()) {
        case IF:        return IfStmt(in, line);
        case PRINTLN:   return PrintLnStmt(in, line);
        case IDENT:     return AssignStmt(in, line);
        default:
            ParseError(line, "Invalid Statement");
            return false;
    }
}

bool PrintLnStmt(istream& in, int& line) {

    Parser::GetNextToken(in, line);

    if (Parser::GetNextToken(in, line).GetToken() != LPAREN) {
        ParseError(line, "Missing '(' in PrintLn");
        return false;
    }

    ValQue = new queue<Value>();

    if (!ExprList(in, line)) {
        delete ValQue;
        ValQue = nullptr;
        ParseError(line, "Invalid expression list in PrintLn");
        return false;
    }

    if (Parser::GetNextToken(in, line).GetToken() != RPAREN) {
        delete ValQue;
        ValQue = nullptr;
        ParseError(line, "Missing ')' in PrintLn");
        return false;
    }

    while (!ValQue->empty()) {
        cout << ValQue->front();
        ValQue->pop();
    }
    cout << endl;

    delete ValQue;
    ValQue = nullptr;

    return true;
}

bool IfStmt(istream& in, int& line) {

    Parser::GetNextToken(in, line);

    if (Parser::GetNextToken(in, line).GetToken() != LPAREN) {
        ParseError(line, "Missing '(' in If condition");
        return false;
    }

    Value cond;
    if (!Expr(in, line, cond)) {
        ParseError(line, "Invalid If condition");
        return false;
    }

    if (Parser::GetNextToken(in, line).GetToken() != RPAREN) {
        ParseError(line, "Missing ')' in If condition");
        return false;
    }

    if (Parser::GetNextToken(in, line).GetToken() != LBRACES) {
        ParseError(line, "Missing '{' after If condition");
        return false;
    }

    bool condTruth = BplTruth(cond);
    int startLine = line;

    if (condTruth) {
        if (!StmtList(in, line)) return false;
    }
    else {
        int bc = 1;
        while (bc > 0) {
            LexItem x = Parser::GetNextToken(in, line);
            Token tk = x.GetToken();

            if (tk == DONE) {
                ParseError(startLine, "Missing '}' in If");
                return false;
            }
            if (tk == LBRACES) bc++;
            if (tk == RBRACES) bc--;
        }
    }

    if (condTruth) {
        LexItem endTrue = Parser::GetNextToken(in, line);
        if (endTrue.GetToken() != RBRACES) {
            ParseError(line, "Missing '}' after If block");
            return false;
        }
    }

    LexItem nxt = Parser::GetNextToken(in, line);

    if (nxt.GetToken() == ELSE) {
        if (Parser::GetNextToken(in, line).GetToken() != LBRACES) {
            ParseError(line, "Missing '{' in Else clause");
            return false;
        }

        if (!condTruth) {
            if (!StmtList(in, line)) return false;

            LexItem endElse = Parser::GetNextToken(in, line);
            if (endElse.GetToken() != RBRACES) {
                ParseError(line, "Missing '}' in Else clause");
                return false;
            }
        }
        else {
            int bc = 1;
            while (bc > 0) {
                LexItem x = Parser::GetNextToken(in, line);
                Token tk = x.GetToken();

                if (tk == DONE) {
                    ParseError(line, "Missing '}' in Else");
                    return false;
                }
                if (tk == LBRACES) bc++;
                if (tk == RBRACES) bc--;
            }
        }
    }
    else {
        Parser::PushBackToken(nxt);
    }

    return true;
}

bool AssignStmt(istream& in, int& line) {

    LexItem var = Parser::GetNextToken(in, line);
    if (var.GetToken() != IDENT) {
        ParseError(line, "Missing variable in assignment");
        return false;
    }

    defVar[var.GetLexeme()] = true;

    LexItem op = Parser::GetNextToken(in, line);
    Token optok = op.GetToken();
    if (!(optok == ASSOP || optok == CADDA || optok == CSUBA || optok == CCATA)) {
        ParseError(line, "Missing assignment operator");
        return false;
    }

    Value rval;
    if (!Expr(in, line, rval)) {
        ParseError(line, "Missing Expression in Assignment");
        return false;
    }

    Value ans;

    if (optok == ASSOP) {
        if (rval.IsBool()) {
            ParseError(line, "Run-Time Error-Illegal assignment of Boolean");
            return false;
        }
        ans = rval;
    }
    else if (optok == CADDA) {
        if (!IsDefined(var.GetLexeme())) {
            ParseError(line, "Using Undefined Variable: " + var.GetLexeme());
            return false;
        }
        ans = TempsResults[var.GetLexeme()] + rval;
    }
    else if (optok == CSUBA) {
        if (!IsDefined(var.GetLexeme())) {
            ParseError(line, "Using Undefined Variable: " + var.GetLexeme());
            return false;
        }
        ans = TempsResults[var.GetLexeme()] - rval;
    }
    else if (optok == CCATA) {
        if (!IsDefined(var.GetLexeme())) {
            ParseError(line, "Using Undefined Variable: " + var.GetLexeme());
            return false;
        }
        ans = TempsResults[var.GetLexeme()].Catenate(rval);
    }

    if (ans.IsErr()) {
        ParseError(line, "Run-Time Error-Illegal Assignment Operation");
        return false;
    }

    TempsResults[var.GetLexeme()] = ans;
    return true;
}

bool ExprList(istream& in, int& line) {
    Value v;

    if (!Expr(in, line, v)) return false;
    ValQue->push(v);

    LexItem t = Parser::GetNextToken(in, line);
    while (t.GetToken() == COMMA) {
        if (!Expr(in, line, v)) return false;
        ValQue->push(v);
        t = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(t);
    return true;
}

bool Expr(istream& in, int& line, Value &retVal) {
    return OrExpr(in, line, retVal);
}

bool OrExpr(istream& in, int& line, Value &retVal) {

    if (!AndExpr(in, line, retVal)) return false;

    LexItem t = Parser::GetNextToken(in, line);
    while (t.GetToken() == OR) {

        Value rhs;
        if (!AndExpr(in, line, rhs)) {
            ParseError(line, "Missing operand for ||");
            return false;
        }

        retVal = retVal || rhs;
        if (retVal.IsErr()) {
            ParseError(line, "Run-Time Error-Illegal OR Operation");
            return false;
        }

        t = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(t);
    return true;
}

bool AndExpr(istream& in, int& line, Value &retVal) {

    if (!RelExpr(in, line, retVal)) return false;

    LexItem t = Parser::GetNextToken(in, line);
    while (t.GetToken() == AND) {

        Value rhs;
        if (!RelExpr(in, line, rhs)) {
            ParseError(line, "Missing operand for &&");
            return false;
        }

        retVal = retVal && rhs;
        if (retVal.IsErr()) {
            ParseError(line, "Run-Time Error-Illegal AND Operation");
            return false;
        }

        t = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(t);
    return true;
}

bool RelExpr(istream& in, int& line, Value &retVal) {

    if (!AddExpr(in, line, retVal)) return false;

    LexItem op = Parser::GetNextToken(in, line);
    Token t = op.GetToken();
    string lex = op.GetLexeme();

    if (t == SEQ || t == SLTE || t == SGT) {

        Value rhs;
        if (!AddExpr(in, line, rhs)) {
            ParseError(line, "Missing relational operand");
            return false;
        }

        if (!retVal.IsString() || !rhs.IsString()) {
            ParseError(line, "Illegal Relational operation.");
            return false;
        }

        Value result =
            (t == SEQ ? retVal.SEQ(rhs) :
            (t == SLTE ? retVal.SLE(rhs) :
                         retVal.SGT(rhs)));

        if (result.IsErr()) {
            ParseError(line, "Illegal Relational operation.");
            return false;
        }

        retVal = result;
        return true;
    }

    if (lex == "<" || lex == ">=" || lex == "==") {

        Value rhs;
        if (!AddExpr(in, line, rhs)) {
            ParseError(line, "Missing relational operand");
            return false;
        }

        if (!retVal.IsNum() || !rhs.IsNum()) {
            ParseError(line, "Illegal Relational operation.");
            return false;
        }

        Value result =
            (lex == "<"  ? retVal < rhs :
            (lex == ">=" ? retVal >= rhs :
                           retVal == rhs));

        if (result.IsErr()) {
            ParseError(line, "Illegal Relational operation.");
            return false;
        }

        retVal = result;
        return true;
    }

    Parser::PushBackToken(op);
    return true;
}

bool AddExpr(istream& in, int& line, Value &retVal) {
    if (!MultExpr(in, line, retVal)) return false;

    LexItem t = Parser::GetNextToken(in, line);
    while (t.GetToken() == PLUS || t.GetToken() == MINUS || t.GetToken() == CAT) {

        Token op = t.GetToken();
        Value rhs;

        if (!MultExpr(in, line, rhs)) {
            ParseError(line, "Missing operand for + or - or .");
            return false;
        }

        if (op == PLUS || op == MINUS) {
            if (!retVal.IsNum() || !rhs.IsNum()) {
                ParseError(line, "Illegal operand type for the operation.");
                return false;
            }
        }

        Value ans =
            (op == PLUS ? retVal + rhs :
            (op == MINUS ? retVal - rhs :
                           retVal.Catenate(rhs)));

        if (ans.IsErr()) {
            ParseError(line, "Run-Time Error-Illegal Additive Operation");
            return false;
        }

        retVal = ans;
        t = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(t);
    return true;
}

bool MultExpr(istream& in, int& line, Value &retVal) {
    if (!UnaryExpr(in, line, retVal)) return false;

    LexItem t = Parser::GetNextToken(in, line);
    while (t.GetToken() == MULT || t.GetToken() == DIV ||
           t.GetToken() == REM || t.GetToken() == SREPEAT)
    {
        Token op = t.GetToken();
        Value rhs;

        if (!UnaryExpr(in, line, rhs)) {
            ParseError(line, "Missing operand for multiplicative operator");
            return false;
        }

        if (op == REM) {
            if (rhs.IsString()) {
                ParseError(line, "Illegal operand type for the operation.");
                return false;
            }

            if (retVal.IsString()) {
                try { stod(retVal.GetString()); }
                catch (...) {
                    ParseError(line, "Illegal operand type for the operation.");
                    return false;
                }
            }
        }

        if (op == SREPEAT) {

            if (!rhs.IsNum() && !rhs.IsString()) {
                ParseError(line, "Illegal operand type for the string repetition operation.");
                return false;
            }

            if (rhs.IsString()) {
                try { stod(rhs.GetString()); }
                catch (...) {
                    ParseError(line, "Illegal operand type for the string repetition operation.");
                    return false;
                }
            }
        }

        Value ans =
            (op == MULT ? retVal * rhs :
            (op == DIV  ? retVal / rhs :
            (op == REM  ? retVal % rhs :
                           retVal.Repeat(rhs))));

        if (ans.IsErr()) {
            ParseError(line, "Run-Time Error-Illegal Multiplicative Operation");
            return false;
        }

        retVal = ans;
        t = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(t);
    return true;
}

bool UnaryExpr(istream& in, int& line, Value &retVal) {
    LexItem t = Parser::GetNextToken(in, line);
    Token tok = t.GetToken();

    int sign = +1;
    bool isNot = false;

    if (tok == MINUS) sign = -1;
    else if (tok == PLUS) sign = +1;
    else if (tok == NOT) isNot = true;
    else Parser::PushBackToken(t);

    if (!ExponExpr(in, line, sign, retVal)) return false;

    if (isNot) {
        Value v = !retVal;
        if (v.IsErr()) {
            ParseError(line, "Run-Time Error-Illegal NOT operation");
            return false;
        }
        retVal = v;
    }

    return true;
}

bool ExponExpr(istream& in, int& line, int sign, Value &retVal) {
    if (!PrimaryExpr(in, line, sign, retVal)) return false;

    LexItem t = Parser::GetNextToken(in, line);
    if (t.GetToken() != EXPONENT) {
        Parser::PushBackToken(t);
        return true;
    }

    Value rhs;
    if (!ExponExpr(in, line, +1, rhs)) {
        ParseError(line, "Missing exponent operand");
        return false;
    }

    if (!retVal.IsNum() || !rhs.IsNum()) {
        ParseError(line, "Run-Time Error-Illegal Exponentiation");
        return false;
    }

    retVal = retVal.Expon(rhs);

    if (retVal.IsErr()) {
        ParseError(line, "Run-Time Error-Illegal Exponentiation");
        return false;
    }

    return true;
}

bool PrimaryExpr(istream& in, int& line, int sign, Value &retVal) {
    LexItem t = Parser::GetNextToken(in, line);
    Token tt = t.GetToken();

    if (tt == IDENT) {
        string var = t.GetLexeme();
        if (!IsDefined(var)) {
            ParseError(line, "Using Undefined Variable: " + var);
            return false;
        }
        retVal = TempsResults[var];
        if (sign == -1) {
            if (!retVal.IsNum()) {
                ParseError(line, "Run-Time Error-Illegal operand type for sign operation");
                return false;
            }
            retVal = Value(-retVal.GetNum());
        }
        return true;
    }

    if (tt == ICONST) {
        retVal = Value(sign * stod(t.GetLexeme()));
        return true;
    }

    if (tt == FCONST) {
        retVal = Value(sign * stod(t.GetLexeme()));
        return true;
    }

    if (tt == SCONST) {
        if (sign != 1) {
            ParseError(line, "Run-Time Error-Illegal operand type for sign operation");
            return false;
        }
        retVal = Value(t.GetLexeme());
        return true;
    }

    if (tt == LPAREN) {
        if (!Expr(in, line, retVal)) return false;
        if (Parser::GetNextToken(in, line).GetToken() != RPAREN) {
            ParseError(line, "Missing closing parenthesis");
            return false;
        }
        if (sign == -1) {
            if (!retVal.IsNum()) {
                ParseError(line, "Run-Time Error-Illegal operand type for sign operation");
                return false;
            }
            retVal = Value(-retVal.GetNum());
        }
        return true;
    }

    ParseError(line, "Invalid Primary Expression");
    return false;
}
