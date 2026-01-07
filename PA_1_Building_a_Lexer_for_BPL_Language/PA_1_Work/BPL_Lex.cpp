
#include <iostream>
#include <string>
#include <cctype>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <set>
using namespace std;

enum class TokenType {
    ICONST, FCONST, STRING, IDENT,
 
    PLUS, MINUS, MULT, REM, EXPONENT, NEQ, NGTE, NLT, ASSOP, AND, OR, NOT,
    SREPEAT, CAT, CADDA, CSUBA, CCATA,
    SEQ, SLTE, SGT, 
    COMMA, SEMICOL,

    LPAREN, RPAREN, LBRACES, RBRACES,
  
    IFKW, ELSEKW, PRINTLNKW,
    ERR, END
};

struct Token {
    TokenType type;
    string lexeme;
    int line;
};

static string toLowerStr(string v){
    for(char &c: v) c = (char)tolower((unsigned char)c);
    return v;
}

static string fmtNumber(double x) {
    long long r = llround(x);
    if (fabs(x - (double)r) < 1e-12) {
        return to_string(r);
    } else {
        ostringstream oss;
        oss.setf(std::ios::fmtflags(0), std::ios::floatfield);
        oss << setprecision(15) << x;
        string s = oss.str();
        if (s.find('.') != string::npos) {
            while (!s.empty() && s.back() == '0') s.pop_back();
            if (!s.empty() && s.back() == '.') s.pop_back();
        }
        return s;
    }
}

class Lexer {
    string s;
    size_t pos;
    int line;
    bool errorFlag = false;

    char peek(int ahead = 0) const {
        return (pos + ahead < s.size()) ? s[pos + ahead] : '\0';
    }
    void advance(int n=1) { while(n-- > 0 && pos < s.size()) pos++; }

    Token make(TokenType t, const string &lex) { return {t, lex, line}; }
    Token errTok(const string &lex, int ln) { errorFlag = true; return {TokenType::ERR, lex, ln}; }

public:
    Lexer(const string &src) : s(src), pos(0), line(1) {}

    Token next() {
        if (errorFlag) return make(TokenType::END, "");

        while (isspace((unsigned char)peek())) {
            if (peek() == '\n') line++;
            advance();
        }

        if (peek() == '#') {
            while (peek() != '\n' && peek() != '\0') advance();
            return next();
        }

        char c = peek();
        if (c == '\0') return make(TokenType::END, "");

        
        if (isdigit((unsigned char)c)) {
            size_t start = pos;
            int start_line = line;
            bool hasDot = false;

            while (isdigit((unsigned char)peek())) { advance(); }

            if (peek() == '.') {
                if (isdigit((unsigned char)peek(1))) {
                    hasDot = true;
                    advance();
                    while (isdigit((unsigned char)peek())) advance();
                } else {
                    string intpart = s.substr(start, pos - start);
                    return make(TokenType::FCONST, intpart);
                }
            }

            if (hasDot && (peek() == 'e' || peek() == 'E')) {
                size_t save = pos;
                advance();
                if (peek() == '+' || peek() == '-') advance();
                if (!isdigit((unsigned char)peek())) {
                    pos = save;
                } else {
                    while (isdigit((unsigned char)peek())) advance();
                }
            }

            if (peek() == '.') {
                string bad = s.substr(start, pos - start + 1);
                return errTok(bad, start_line);
            }

            string lex = s.substr(start, pos - start);
            if (hasDot)
                return make(TokenType::FCONST, lex);
            else
                return make(TokenType::ICONST, lex);
        }

       
        if (isalpha((unsigned char)c) || c == '$') {
            size_t start = pos;
            advance();
            while (isalnum((unsigned char)peek()) || peek() == '_' || peek() == '$')
                advance();
            string lex = s.substr(start, pos - start);

            string low = toLowerStr(lex);
            if (low == "if")       return make(TokenType::IFKW, lex);
            if (low == "else")     return make(TokenType::ELSEKW, lex);
            if (low == "println")  return make(TokenType::PRINTLNKW, lex);

            return make(TokenType::IDENT, lex);
        }

        
        if (c == '<') {
            if (isalnum((unsigned char)peek(1))) {
                int start_line = line;
                size_t open = pos;
                advance();
                size_t start = pos;
                while (peek() != '>' && peek() != '\0' && peek() != '\n') advance();
                if (peek() == '>') {
                    string val = s.substr(start, pos - start);
                    advance();
                    return make(TokenType::STRING, val);
                } else {
                    string bad = s.substr(open, pos - open);
                    return errTok(bad, start_line);
                }
            } else {
                advance();
                return make(TokenType::NLT, "<");
            }
        }

        
        if (c == '"') {
            int start_line = line;
            size_t open = pos;
            advance();
            size_t start = pos;
            while (peek() != '"' && peek() != '\0' && peek() != '\n') advance();
            if (peek() == '"') {
                string val = s.substr(start, pos - start);
                advance();
                return make(TokenType::STRING, val);
            } else {
                string bad = s.substr(open, pos - open);
                return errTok(bad, start_line);
            }
        }

     
        if (c == '\'') {
            int start_line = line;
            size_t open = pos;
            advance();
            size_t start = pos;
            while (peek() != '\'' && peek() != '\0' && peek() != '\n') advance();
            if (peek() == '\'') {
                string val = s.substr(start, pos - start);
                advance();
                return make(TokenType::STRING, val);
            } else {
                string bad = s.substr(open, pos - open);
                return errTok(bad, start_line);
            }
        }

       
        if (c == '.') {
            if ((peek(1) == 'x' || peek(1) == 'X') && peek(2) == '.') {
                string lex = s.substr(pos, 3);
                advance(3);
                return make(TokenType::SREPEAT, lex);
            }
            if (peek(1) == '=') {
                string lex = s.substr(pos, 2);
                advance(2);
                return make(TokenType::CCATA, lex);
            }
            advance();
            return make(TokenType::CAT, ".");
        }

        if (c == '*') {
            if (peek(1) == '*') {
                string lex = s.substr(pos, 2);
                advance(2);
                return make(TokenType::EXPONENT, lex);
            }
            advance();
            return make(TokenType::MULT, "*");
        }
        if (c == '%') { advance(); return make(TokenType::REM, "%"); }

        if (c == '=') {
            if (peek(1) == '=') {
                string lex = s.substr(pos, 2);
                advance(2);
                return make(TokenType::NEQ, lex);
            }
            advance();
            return make(TokenType::ASSOP, "=");
        }
        if (c == '+') {
            if (peek(1) == '=') {
                string lex = s.substr(pos, 2);
                advance(2);
                return make(TokenType::CADDA, lex);
            }
            advance();
            return make(TokenType::PLUS, "+");
        }
        if (c == '-') {
            if (peek(1) == '=') {
                string lex = s.substr(pos, 2);
                advance(2);
                return make(TokenType::CSUBA, lex);
            }
            advance();
            return make(TokenType::MINUS, "-");
        }

        if (c == '>') {
            if (peek(1) == '=') {
                string lex = s.substr(pos, 2);
                advance(2);
                return make(TokenType::NGTE, lex);
            }
            string bad(1, c);
            advance();
            return errTok(bad, line);
        }
        if (c == '&' && peek(1) == '&') {
            string lex = s.substr(pos, 2);
            advance(2);
            return make(TokenType::AND, lex);
        }
        if (c == '|' && peek(1) == '|' ) {
            string lex = s.substr(pos, 2);
            advance(2);
            return make(TokenType::OR, lex);
        }
        if (c == '!') { advance(); return make(TokenType::NOT, "!"); }

        if (c == '@') {
            int start_line = line;
            advance();
            if (!isalpha((unsigned char)peek())) {
                return errTok("@", start_line);
            }
            string word;
            while (isalpha((unsigned char)peek())) { word.push_back(peek()); advance(); }
            string low = toLowerStr(word);
            if (low == "eq") return make(TokenType::SEQ, string("@") + word);
            if (low == "le") return make(TokenType::SLTE, string("@") + word);
            if (low == "gt") return make(TokenType::SGT, string("@") + word);
            return errTok("@", start_line);
        }

        if (c == ',') { advance(); return make(TokenType::COMMA, ","); }
        if (c == ';') { advance(); return make(TokenType::SEMICOL, ";"); }
        if (c == '(') { advance(); return make(TokenType::LPAREN, "("); }
        if (c == ')') { advance(); return make(TokenType::RPAREN, ")"); }
        if (c == '{') { advance(); return make(TokenType::LBRACES, "{"); }
        if (c == '}') { advance(); return make(TokenType::RBRACES, "}"); }

        if (c == '_') { advance(); return errTok("_", line); }

        string bad(1, c);
        advance();
        return errTok(bad, line);
    }

    int getLine() const { return line; }
};

int main(int argc, char **argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    bool verbose = false;
    bool numflag = false;
    bool idsflag = false;
    bool strflag = false;
    string filename;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-v") verbose = true;
        else if (arg == "-num") numflag = true;
        else if (arg == "-ids") idsflag = true;
        else if (arg == "-str") strflag = true;
        else if (arg[0] == '-') {
            cout << "UNRECOGNIZED FLAG {" << arg << "}\n";
            return 0;
        } else {
            if (!filename.empty()) {
                cout << "ONLY ONE FILE NAME IS ALLOWED.\n";
                return 0;
            }
            filename = arg;
        }
    }

    if (filename.empty()) {
        cout << "No specified input file.\n";
        return 0;
    }

    ifstream in(filename);
    if (!in) {
        cout << "CANNOT OPEN THE FILE " << filename << "\n";
        return 0;
    }

    string src((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    if (src.empty()) {
        cout << "Empty file.\n";
        return 0;
    }

    Lexer lex(src);
    Token t;
    bool hadError = false;

    int totalTokens = 0, identifiers = 0, numLiterals = 0, strLiterals = 0;
    vector<string> identList, numLexList, strList;
    vector<double> numValues;

    auto pushNumeric = [&](const Token& tok){
        numLexList.push_back(tok.lexeme);
        try {
            double v = stod(tok.lexeme);
            numValues.push_back(v);
        } catch (...) {
            double v = 0.0;
            bool neg = false;
            size_t p = 0;
            if (!tok.lexeme.empty() && tok.lexeme[0]=='-'){ neg=true; p=1; }
            while (p<tok.lexeme.size() && isdigit((unsigned char)tok.lexeme[p])) {
                v = v*10 + (tok.lexeme[p]-'0'); p++;
            }
            if (neg) v = -v;
            numValues.push_back(v);
        }
    };

    int lastTokenLine = 1;

    while ((t = lex.next()).type != TokenType::END) {
        lastTokenLine = t.line;

        totalTokens++;
        switch (t.type) {
            case TokenType::ICONST: numLiterals++; pushNumeric(t); break;
            case TokenType::FCONST: numLiterals++; pushNumeric(t); break;
            case TokenType::STRING: strLiterals++; strList.push_back(t.lexeme); break;
            case TokenType::IDENT:  identifiers++; identList.push_back(t.lexeme); break;
            case TokenType::ERR:
                cout << "ERR: Error-Unrecognized Lexeme {" << t.lexeme << "} in line " << t.line << "\n";
                hadError = true;
                break;
            default: break;
        }

        if (t.type == TokenType::ERR) break;

        if (verbose) {
            switch (t.type) {
                case TokenType::ICONST: cout << "ICONST: [" << t.lexeme << "]\n"; break;
                case TokenType::FCONST: cout << "FCONST: [" << t.lexeme << "]\n"; break;
                case TokenType::STRING: cout << "SCONST: <" << t.lexeme << ">\n"; break;
                case TokenType::IDENT:  cout << "IDENT: (" << t.lexeme << ")\n"; break;

                case TokenType::PLUS: cout << "PLUS: \"+\"\n"; break;
                case TokenType::MINUS: cout << "MINUS: \"-\"\n"; break;
                case TokenType::MULT: cout << "MULT: \"*\"\n"; break;
                case TokenType::REM: cout << "REM: \"%\"\n"; break;
                case TokenType::EXPONENT: cout << "EXPONENT: \"**\"\n"; break;
                case TokenType::NEQ: cout << "NEQ: \"==\"\n"; break;
                case TokenType::NGTE: cout << "NGTE: \">=\"\n"; break;
                case TokenType::NLT: cout << "NLT: \"<\"\n"; break;
                case TokenType::ASSOP: cout << "ASSOP: \"=\"\n"; break;
                case TokenType::AND: cout << "AND: \"&&\"\n"; break;
                case TokenType::OR: cout << "OR: \"||\"\n"; break;
                case TokenType::NOT: cout << "NOT: \"!\"\n"; break;
                case TokenType::SREPEAT: cout << "SREPEAT: \".x.\"\n"; break;
                case TokenType::CAT: cout << "CAT: \".\"\n"; break;
                case TokenType::CADDA: cout << "CADDA: \"+=\"\n"; break;
                case TokenType::CSUBA: cout << "CSUBA: \"-=\"\n"; break;
                case TokenType::CCATA: cout << "CCATA: \".=\"\n"; break;
                case TokenType::SEQ: cout << "SEQ: \"" << t.lexeme << "\"\n"; break;
                case TokenType::SLTE: cout << "SLTE: \"" << t.lexeme << "\"\n"; break;
                case TokenType::SGT: cout << "SGT: \"" << t.lexeme << "\"\n"; break;

                case TokenType::COMMA: cout << "COMMA: \",\"\n"; break;
                case TokenType::SEMICOL: cout << "SEMICOL: \";\"\n"; break;

                case TokenType::LPAREN:  cout << "LPAREN: \"(\"\n";  break;
                case TokenType::RPAREN:  cout << "RPAREN: \")\"\n";  break;
                case TokenType::LBRACES: cout << "LBRACES: \"{\"\n"; break;
                case TokenType::RBRACES: cout << "RBRACES: \"}\"\n"; break;

                case TokenType::IFKW: cout << "IF: \"" << t.lexeme << "\"\n"; break;
                case TokenType::ELSEKW: cout << "ELSE: \"" << t.lexeme << "\"\n"; break;
                case TokenType::PRINTLNKW: cout << "PRINTLN: \"" << t.lexeme << "\"\n"; break;

                default: break;
            }
        }
    }

    if (!hadError) {
        int printedLines = lastTokenLine;

        if (!verbose && !numflag && !idsflag && !strflag) {
            cout << "\n"; 
            cout << "Lines: " << printedLines << "\n";
            cout << "Total Tokens: " << totalTokens << "\n";
            cout << "Identifiers: " << identifiers << "\n";
            cout << "Numeric Literals: " << numLiterals << "\n";
            cout << "String Literals: " << strLiterals << "\n";
        }
        else if (idsflag) {
            cout << "\n";

            set<string> uniqIds(identList.begin(), identList.end());
            vector<string> sortedIds(uniqIds.begin(), uniqIds.end());

            set<double> uniqNums(numValues.begin(), numValues.end());
            vector<double> sortedNums(uniqNums.begin(), uniqNums.end());
            sort(sortedNums.begin(), sortedNums.end());

            set<string> uniqStrs(strList.begin(), strList.end());
            vector<string> sortedStrs(uniqStrs.begin(), uniqStrs.end());
            sort(sortedStrs.begin(), sortedStrs.end());

            cout << "Lines: " << printedLines << "\n";
            cout << "Total Tokens: " << totalTokens << "\n";
            cout << "Identifiers: " << sortedIds.size() << "\n";
            cout << "Numeric Literals: " << sortedNums.size() << "\n";
            cout << "String Literals: " << sortedStrs.size() << "\n";

            cout << "IDENTIFIERS:\n";
            for (size_t i = 0; i < sortedIds.size(); ++i) {
                cout << sortedIds[i];
                if (i + 1 < sortedIds.size()) cout << ", ";
            }
            cout << "\n";

            cout << "NUMERIC LITERALS:\n";
            for (double v : sortedNums) cout << fmtNumber(v) << "\n";

            if (!sortedStrs.empty()) {
                cout << "STRING LITERALS:\n";
                for (auto &s : sortedStrs) cout << "<" << s << ">\n";
            }
        }
        else if (strflag) {
            cout << "\n";
            cout << "Lines: " << printedLines << "\n";
            cout << "Total Tokens: " << totalTokens << "\n";
            cout << "Identifiers: " << identifiers << "\n";
            cout << "Numeric Literals: " << numLiterals << "\n";
            cout << "String Literals: " << strLiterals << "\n";

            cout << "STRING LITERALS:\n";
            vector<string> sorted = strList;
            sort(sorted.begin(), sorted.end());
            for (auto &s : sorted) cout << "<" << s << ">\n";
        }
        else if (numflag) {
            cout << "\n";
            cout << "Lines: " << printedLines << "\n";
            cout << "Total Tokens: " << totalTokens << "\n";
            cout << "Identifiers: " << identifiers << "\n";
            cout << "Numeric Literals: " << numLiterals << "\n";
            cout << "String Literals: " << strLiterals << "\n";

            cout << "NUMERIC LITERALS:\n";
            vector<double> sortedVals = numValues;
            sort(sortedVals.begin(), sortedVals.end());
            for (double v : sortedVals) cout << fmtNumber(v) << "\n";
        }
        else if (verbose) {
            if (identifiers == 0 && numLiterals == 0 && strLiterals == 0) {
                cout << "\n";
                cout << "Lines: " << printedLines << "\n";
                cout << "Total Tokens: " << totalTokens << "\n";
                cout << "Identifiers: " << identifiers << "\n";
                cout << "Numeric Literals: " << numLiterals << "\n";
                cout << "String Literals: " << strLiterals << "\n";
            } else {
                cout << "EOF: []\n";
            }
        }
    }

    return 0;
}
