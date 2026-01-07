#include <iostream>
#include <vector>
#include <set>
#include <initializer_list>
#include <algorithm>
#include "parser.h"
#include "lex.h"
using namespace std;

namespace ErrTxt {
    const string MissingSemi         = "Missing semicolon at end of Statement";
    const string MissingOperandFor   = "Missing operand for an operator";
    const string MissingOperandAfter = "Missing operand after operator";
    const string SynErrProgBody      = "Syntactic error in Program Body";

    const string MissingAssignOp     = "Missing Assignment Operator";
    const string MissingVarInAssign  = "Missing Variable in Assignment";
    const string MissingExprInAssign = "Missing Expression in Assignment Statement";
    const string IncorrectAssign     = "Incorrect Assignment Statement";

    const string PrintlnIncorrect    = "Incorrect PrintLn Statement";
    const string PrintlnMissingLP    = "Missing Left Parenthesis of PrintLn Statement";
    const string PrintlnMissingRP    = "Missing Right Parenthesis of PrintLn Statement";

    const string IfIncorrect           = "Incorrect If-Statement";
    const string IfMissingLP           = "Missing Left Parenthesis of If condition";
    const string IfMissingRP           = "Missing Right Parenthesis of If condition";
    const string IfMissingLBraceClause = "Missing left brace for If Statement Clause";
    const string IfMissingRBraceClause = "Missing right brace for If Statement Clause";
    const string ElseMissingLBrace     = "Missing left brace for an Else-Clause";
    const string ElseMissingRBrace     = "Missing right brace for an Else-Clause"; // <-- fixed text
    const string IllegalElseClause     = "Illegal If Statement Else-Clause";
    const string MissingStmtIfClause   = "Missing Statement for If Statement Clause";
    const string MissingStmtElseClause = "Missing Statement for Else-Clause";

    inline string UsingUndef(const string& x) { return "Using Undefined Variable: " + x; }
}

namespace {
vector<string> gErrors;
bool gPushedBack = false;
LexItem gPushBackTok;

vector<string> gVarOrder;
set<string>    gVarSeen;
bool gOnAssignLHS = false;

bool gPrintedErrorsThisCall = false;

int gLastTokLine = 1;
int gLastErrorLine = 1;
int gLastMissingSemiLine = 0;

int gEmitSingleOnce = 0;
int gEmitPairOnce   = 0;

inline bool TokIsPrint(LexItem t) { return t.GetToken() == PRINTLN; }
inline bool TokIsIf(LexItem t)    { return t.GetToken() == IF; }
inline bool TokIsElse(LexItem t)  { return t.GetToken() == ELSE; }

LexItem GetTok(istream& in, int& line) {
    if (gPushedBack) { gPushedBack = false; return gPushBackTok; }
    LexItem t = getNextToken(in, line);
    if (t.GetLinenum() > 0) gLastTokLine = t.GetLinenum();
    return t;
}
void PushBack(LexItem t) {
    if (!gPushedBack) { gPushedBack = true; gPushBackTok = t; }
}
bool IsAny(LexItem t, initializer_list<Token> ks) {
    for (auto k: ks) if (t.GetToken() == k) return true;
    return false;
}

void ParseError(int line, const string& msg) {
    gErrors.push_back(string("Line ") + to_string(line) + ": " + msg);
    gLastErrorLine = line;
}
void DefineVarOnce(const string& ident) {
    if (!gVarSeen.count(ident)) { gVarSeen.insert(ident); gVarOrder.push_back(ident); }
}

bool Accept(istream& in, int& line, initializer_list<Token> ks, LexItem* out=nullptr) {
    auto t = GetTok(in, line);
    if (IsAny(t, ks)) { if (out) *out = t; return true; }
    PushBack(t); return false;
}
bool Expect(istream& in, int& line, initializer_list<Token> ks, const string& err) {
    auto t = GetTok(in, line);
    if (IsAny(t, ks)) return true;
    ParseError(t.GetLinenum() ? t.GetLinenum() : line, err);
    PushBack(t);
    return false;
}

bool StartsPrimary(istream& in, int& line) {
    LexItem t = GetTok(in, line); PushBack(t);
    return IsAny(t, {IDENT, ICONST, FCONST, SCONST, LPAREN});
}
bool StartsUnary(istream& in, int& line) {
    LexItem t = GetTok(in, line); PushBack(t);
    return IsAny(t, {MINUS, PLUS, NOT, IDENT, ICONST, FCONST, SCONST, LPAREN});
}

bool MaybeEmitSingle(int line) {
    if (gEmitSingleOnce == 1) { ParseError(line, ErrTxt::MissingOperandFor); gEmitSingleOnce = 2; return true; }
    if (gEmitSingleOnce == 2) return true;
    return false;
}
bool MaybeEmitPair(int line) {
    if (gEmitPairOnce == 1) { ParseError(line, ErrTxt::MissingOperandFor); ParseError(line, ErrTxt::MissingOperandAfter); gEmitPairOnce = 2; return true; }
    if (gEmitPairOnce == 2) return true;
    return false;
}

// ---- Panic-mode recovery helper ----
bool RecoverUntil(istream& in, int& line, initializer_list<Token> sync) {
    while (true) {
        LexItem t = GetTok(in, line);
        Token k = t.GetToken();
        if (k == DONE) return false;
        for (auto s : sync) if (k == s) return true;
    }
}
} // namespace

int ErrCount() { return (int)gErrors.size(); }

bool StmtList(istream& in, int& line);
bool StmtList(istream& in, int& line, bool inIfElseClause);
bool Stmt(istream& in, int& line);
bool PrintLnStmt(istream& in, int& line);
bool IfStmt(istream& in, int& line);
bool AssignStmt(istream& in, int& line);
bool Var(istream& in, int& line);
bool ExprList(istream& in, int& line);
bool AssigOp(istream& in, int& line);
bool Expr(istream& in, int& line);
bool OrExpr(istream& in, int& line);
bool AndExpr(istream& in, int& line);
bool RelExpr(istream& in, int& line);
bool AddExpr(istream& in, int& line);
bool MultExpr(istream& in, int& line);
bool UnaryExpr(istream& in, int& line);
bool ExponExpr(istream& in, int& line, int sign);
bool PrimaryExpr(istream& in, int& line, int sign);

bool Prog(std::istream& in, int& line) {
    gErrors.clear();
    gVarOrder.clear();
    gVarSeen.clear();
    gPushedBack = false;
    gOnAssignLHS = false;
    gPrintedErrorsThisCall = false;
    gLastTokLine = std::max(1, line);
    gLastErrorLine = gLastTokLine;
    gLastMissingSemiLine = 0;
    gEmitSingleOnce = 0;
    gEmitPairOnce   = 0;

    bool ok = StmtList(in, line, false);
    bool addProgBody = !ok;

    if (!gErrors.empty()) {
        if (gErrors.size() == 1) {
            const string &e = gErrors.back();
            if (e.find(ErrTxt::MissingSemi) != string::npos) addProgBody = false;
            if (e.find(ErrTxt::IllegalElseClause) != string::npos) addProgBody = false;
        }
    }

    if (addProgBody) ParseError(gLastErrorLine, ErrTxt::SynErrProgBody);

    if (!gErrors.empty()) {
        if (!gPrintedErrorsThisCall) {
            for (size_t i = 0; i < gErrors.size(); ++i)
                std::cout << (i + 1) << ". " << gErrors[i] << std::endl;
            gPrintedErrorsThisCall = true;
        }
        std::cout << "Unsuccessful Parsing" << std::endl;
        std::cout << "Number of Syntax Errors " << gErrors.size() << std::endl;
        return false;
    }

    // ==== SUCCESS OUTPUT (sorted list) ====
    std::cout << "Declared Variables:" << std::endl;
    vector<string> sorted = gVarOrder;
    sort(sorted.begin(), sorted.end());
    if (!sorted.empty()) {
        for (size_t i = 0; i < sorted.size(); ++i) {
            std::cout << sorted[i];
            if (i + 1 < sorted.size()) std::cout << ", ";
        }
        std::cout << std::endl;
    } else {
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << "DONE" << std::endl;
    return true;
}

bool StmtList(istream& in, int& line) { return StmtList(in, line, false); }

bool StmtList(istream& in, int& line, bool inIfElseClause) {
    if (!Stmt(in, line)) return false;

    if (!Accept(in, line, {SEMICOL})) {
        LexItem peek = GetTok(in, line);
        int pk = peek.GetLinenum();
        PushBack(peek);
        int reportLine = (pk > 0 ? max(1, pk - 1) : max(1, gLastTokLine - 1));
        gLastMissingSemiLine = reportLine;
        ParseError(reportLine, ErrTxt::MissingSemi);

        // Sync ONLY to ';' so '}' remains for braces.
        RecoverUntil(in, line, {SEMICOL});
        Accept(in, line, {SEMICOL});
        return false;
    }

    while (true) {
        LexItem t = GetTok(in, line); PushBack(t);

        if (inIfElseClause && (t.GetToken() == RBRACES || TokIsElse(t))) break;

        if (!inIfElseClause && TokIsElse(t)) {
            GetTok(in, line);
            ParseError(t.GetLinenum(), ErrTxt::IllegalElseClause);
            return false;
        }

        if (!(TokIsIf(t) || TokIsPrint(t) || t.GetToken() == IDENT)) break;

        if (!Stmt(in, line)) return false;

        if (!Accept(in, line, {SEMICOL})) {
            LexItem peek2 = GetTok(in, line);
            int pk2 = peek2.GetLinenum();
            PushBack(peek2);
            int reportLine = (pk2 > 0 ? max(1, pk2 - 1) : max(1, gLastTokLine - 1));
            gLastMissingSemiLine = reportLine;
            ParseError(reportLine, ErrTxt::MissingSemi);

            RecoverUntil(in, line, {SEMICOL});
            Accept(in, line, {SEMICOL});
            return false;
        }
    }
    return true;
}

bool Stmt(istream& in, int& line) {
    LexItem t = GetTok(in, line); PushBack(t);

    if (TokIsElse(t)) { ParseError(t.GetLinenum(), ErrTxt::IllegalElseClause); return false; }
    if (TokIsIf(t))    return IfStmt(in, line);
    if (TokIsPrint(t)) return PrintLnStmt(in, line);
    if (t.GetToken() == IDENT) return AssignStmt(in, line);

    t = GetTok(in, line);
    ParseError(t.GetLinenum(), "Incorrect Statement");
    return false;
}

bool PrintLnStmt(istream& in, int& line) {
    LexItem kw = GetTok(in, line);

    if (!Expect(in, line, {LPAREN}, ErrTxt::PrintlnMissingLP)) {
        ParseError(kw.GetLinenum(), ErrTxt::PrintlnIncorrect);
        return false;
    }
    if (!ExprList(in, line)) {
        ParseError(kw.GetLinenum(), ErrTxt::MissingOperandFor);
        ParseError(kw.GetLinenum(), ErrTxt::PrintlnIncorrect);
        return false;
    }
    if (!Accept(in, line, {RPAREN})) {
        ParseError(kw.GetLinenum(), ErrTxt::PrintlnMissingRP);
        ParseError(kw.GetLinenum(), ErrTxt::PrintlnIncorrect);
        return false;
    }
    return true;
}

bool IfStmt(istream& in, int& line) {
    LexItem kw = GetTok(in, line);

    if (!Accept(in, line, {LPAREN})) {
        ParseError(kw.GetLinenum(), ErrTxt::IfMissingLP);
        ParseError(kw.GetLinenum(), ErrTxt::IfIncorrect);
        return false;
    }
    if (!Expr(in, line)) {
        ParseError(kw.GetLinenum(), ErrTxt::MissingOperandFor);
        ParseError(kw.GetLinenum(), ErrTxt::IfIncorrect);
        return false;
    }
    if (!Accept(in, line, {RPAREN})) {
        ParseError(kw.GetLinenum(), ErrTxt::IfMissingRP);
        ParseError(kw.GetLinenum(), ErrTxt::IfIncorrect);
        return false;
    }

    {
        LexItem ahead = GetTok(in, line); PushBack(ahead);
        int anchor = ahead.GetLinenum() ? ahead.GetLinenum() : max(1, gLastTokLine);
        if (!Accept(in, line, {LBRACES})) {
            ParseError(anchor, ErrTxt::IfMissingLBraceClause);
            ParseError(anchor, ErrTxt::IfIncorrect);
            return false;
        }
    }

    if (!StmtList(in, line, true)) {
        ParseError(kw.GetLinenum(), ErrTxt::IfIncorrect);
        return false;
    }

    {
        const int needIfRBraceLine = gLastTokLine;
        if (!Accept(in, line, {RBRACES})) {
            LexItem t = GetTok(in, line);
            if (t.GetToken() == ELSE) {
                ParseError(needIfRBraceLine, ErrTxt::IfMissingRBraceClause);
                ParseError(needIfRBraceLine, ErrTxt::IfIncorrect);
                ParseError(t.GetLinenum(),   ErrTxt::IllegalElseClause);
                return false;
            } else {
                ParseError(needIfRBraceLine, ErrTxt::IfMissingRBraceClause);
                ParseError(needIfRBraceLine, ErrTxt::IfIncorrect);
                PushBack(t);
                return false;
            }
        }
    }

    LexItem t = GetTok(in, line);
    if (t.GetToken() == ELSE) {
        if (!Accept(in, line, {LBRACES})) {
            ParseError(t.GetLinenum(), ErrTxt::ElseMissingLBrace);
            ParseError(t.GetLinenum(), ErrTxt::IfIncorrect);
            return false;
        }
        if (!StmtList(in, line, true)) {
            int anchor = gLastMissingSemiLine ? gLastMissingSemiLine : max(1, gLastTokLine - 1);
            ParseError(anchor, ErrTxt::MissingStmtElseClause);
            ParseError(anchor, ErrTxt::IfIncorrect);
            return false;
        }
        int needElseRBraceLine = max(1, gLastTokLine - 1);
        if (!Accept(in, line, {RBRACES})) {
            ParseError(needElseRBraceLine, ErrTxt::ElseMissingRBrace);
            ParseError(needElseRBraceLine, ErrTxt::IfIncorrect); // <-- anchor to same line
            return false;
        }
    } else {
        PushBack(t);
    }

    return true;
}

bool AssignStmt(istream& in, int& line) {
    if (!Var(in, line)) return false;
    if (!AssigOp(in, line)) {
        ParseError(line, ErrTxt::MissingAssignOp);
        ParseError(line, ErrTxt::IncorrectAssign);
        return false;
    }
    if (!Expr(in, line)) {
        bool suppressMissingExpr = (gEmitPairOnce != 0);
        if (!suppressMissingExpr) ParseError(line, ErrTxt::MissingExprInAssign);
        ParseError(line, ErrTxt::IncorrectAssign);
        return false;
    }
    return true;
}

bool Var(istream& in, int& line) {
    gOnAssignLHS = true;
    LexItem id = GetTok(in, line);
    gOnAssignLHS = false;

    if (id.GetToken() != IDENT) {
        ParseError(id.GetLinenum(), ErrTxt::MissingVarInAssign);
        return false;
    }
    DefineVarOnce(id.GetLexeme());
    return true;
}

bool ExprList(istream& in, int& line) {
    if (!Expr(in, line)) return false;
    while (Accept(in, line, {COMMA})) {
        if (!Expr(in, line)) return false;
    }
    return true;
}

bool AssigOp(istream& in, int& line) {
    LexItem t = GetTok(in, line);
    if (IsAny(t, {ASSOP, CADDA, CSUBA, CCATA})) return true;
    PushBack(t); return false;
}

bool Expr(istream& in, int& line) { return OrExpr(in, line); }

bool OrExpr(istream& in, int& line) {
    if (!AndExpr(in, line)) return false;
    while (Accept(in, line, {OR})) {
        if (!AndExpr(in, line)) {
            if (MaybeEmitSingle(line)) return false;
            if (MaybeEmitPair(line))   return false;
            ParseError(line, ErrTxt::MissingOperandFor);
            ParseError(line, ErrTxt::MissingOperandAfter);
            return false;
        }
    }
    return true;
}

bool AndExpr(istream& in, int& line) {
    if (!RelExpr(in, line)) return false;
    while (Accept(in, line, {AND})) {
        if (!RelExpr(in, line)) {
            if (MaybeEmitSingle(line)) return false;
            if (MaybeEmitPair(line))   return false;
            ParseError(line, ErrTxt::MissingOperandFor);
            ParseError(line, ErrTxt::MissingOperandAfter);
            return false;
        }
    }
    return true;
}

bool RelExpr(istream& in, int& line) {
    if (!AddExpr(in, line)) return false;

    LexItem t = GetTok(in, line);
    const string lx = t.GetLexeme();
    const bool isStringRel  = IsAny(t, {SLTE, SGT, SEQ});
    const bool isNumericRel = (lx == "<" || lx == "<=" || lx == ">" || lx == ">=" || lx == "==");

    if (isStringRel || isNumericRel) {
        if (!AddExpr(in, line)) {
            if (MaybeEmitSingle(t.GetLinenum())) return false;
            if (MaybeEmitPair(t.GetLinenum()))   return false;
            ParseError(t.GetLinenum(), ErrTxt::MissingOperandFor);
            ParseError(t.GetLinenum(), ErrTxt::MissingOperandAfter);
            return false;
        }
    } else {
        PushBack(t);
    }
    return true;
}

bool AddExpr(istream& in, int& line) {
    if (!MultExpr(in, line)) return false;
    while (true) {
        LexItem t = GetTok(in, line);
        if (!IsAny(t, {PLUS, MINUS, CAT})) { PushBack(t); break; }

        if (!StartsUnary(in, line)) {
            if (MaybeEmitSingle(t.GetLinenum())) return false;
            if (MaybeEmitPair(t.GetLinenum()))   return false;
            ParseError(t.GetLinenum(), ErrTxt::MissingOperandFor);
            ParseError(t.GetLinenum(), ErrTxt::MissingOperandAfter);
            return false;
        }
        if (!MultExpr(in, line)) {
            if (MaybeEmitSingle(t.GetLinenum())) return false;
            if (MaybeEmitPair(t.GetLinenum()))   return false;
            ParseError(t.GetLinenum(), ErrTxt::MissingOperandFor);
            ParseError(t.GetLinenum(), ErrTxt::MissingOperandAfter);
            return false;
        }
    }
    return true;
}

bool MultExpr(istream& in, int& line) {
    if (!UnaryExpr(in, line)) return false;
    while (true) {
        LexItem t = GetTok(in, line);
        if (!IsAny(t, {MULT, DIV, REM, SREPEAT})) { PushBack(t); break; }

        if (!StartsUnary(in, line)) {
            if (MaybeEmitSingle(t.GetLinenum())) return false;
            if (MaybeEmitPair(t.GetLinenum()))   return false;
            ParseError(t.GetLinenum(), ErrTxt::MissingOperandFor);
            ParseError(t.GetLinenum(), ErrTxt::MissingOperandAfter);
            return false;
        }
        if (!UnaryExpr(in, line)) {
            if (MaybeEmitSingle(t.GetLinenum())) return false;
            if (MaybeEmitPair(t.GetLinenum()))   return false;
            ParseError(t.GetLinenum(), ErrTxt::MissingOperandFor);
            ParseError(t.GetLinenum(), ErrTxt::MissingOperandAfter);
            return false;
        }
    }
    return true;
}

bool UnaryExpr(istream& in, int& line) {
    int sign = +1;
    LexItem t = GetTok(in, line);
    if (IsAny(t, {MINUS, PLUS, NOT})) { if (t.GetToken() == MINUS) sign = -1; }
    else { PushBack(t); }
    if (!ExponExpr(in, line, sign)) {
        if (MaybeEmitSingle(line)) return false;
        if (MaybeEmitPair(line))   return false;
        ParseError(line, ErrTxt::MissingOperandFor);
        ParseError(line, ErrTxt::MissingOperandAfter);
        return false;
    }
    return true;
}

bool ExponExpr(istream& in, int& line, int) {
    if (!PrimaryExpr(in, line, +1)) return false;
    while (Accept(in, line, {EXPONENT})) {
        if (!StartsPrimary(in, line) || !PrimaryExpr(in, line, +1)) {
            ParseError(max(1, gLastTokLine), "Missing exponent operand after exponentiation");
            gEmitPairOnce = 1;
            return false;
        }
    }
    return true;
}

bool PrimaryExpr(istream& in, int& line, int) {
    LexItem t = GetTok(in, line);
    Token k = t.GetToken();

    if (k == IDENT) {
        if (!gOnAssignLHS && !gVarSeen.count(t.GetLexeme())) {
            ParseError(t.GetLinenum(), ErrTxt::UsingUndef(t.GetLexeme()));
        }
        return true;
    }
    if (IsAny(t, {ICONST, FCONST, SCONST})) return true;

    if (k == LPAREN) {
        LexItem savePeek = GetTok(in, line); PushBack(savePeek);
        bool starts = IsAny(savePeek, {IDENT, ICONST, FCONST, SCONST, LPAREN, PLUS, MINUS, NOT});
        if (!starts || !Expr(in, line)) {
            ParseError(t.GetLinenum(), "Missing expression after Left Parenthesis");
            gEmitPairOnce = 1;
            RecoverUntil(in, line, {RPAREN, SEMICOL}); 
            Accept(in, line, {RPAREN});
            return false;
        }
        if (!Accept(in, line, {RPAREN})) {
            ParseError(t.GetLinenum(), "Missing right Parenthesis after expression");
            gEmitSingleOnce = 1;
            RecoverUntil(in, line, {RPAREN, SEMICOL});
            Accept(in, line, {RPAREN});
            return false;
        }
        return true;
    }

    PushBack(t);
    return false;
}
