#ifndef VALUE_H
#define VALUE_H

#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <iomanip>
#include <stdexcept>
#include <cmath>
#include <sstream>

using namespace std;

enum ValType { VNUM, VSTRING, VBOOL, VERR };

class Value {
    ValType T;
    bool    Btemp;
    double  Ntemp;
    string  Stemp;

public:
    Value() : T(VERR), Btemp(false), Ntemp(0.0), Stemp("") {}
    Value(bool vb) : T(VBOOL), Btemp(vb), Ntemp(0.0), Stemp("") {}
    Value(double vr) : T(VNUM), Btemp(false), Ntemp(vr), Stemp("") {}
    Value(string vs) : T(VSTRING), Btemp(false), Ntemp(0.0), Stemp(vs) {}

    ValType GetType() const { return T; }
    bool IsErr() const { return T == VERR; }
    bool IsString() const { return T == VSTRING; }
    bool IsNum() const { return T == VNUM; }
    bool IsBool() const { return T == VBOOL; }

    string GetString() const {
        if (IsString()) return Stemp;
        throw "RUNTIME ERROR: Value not a string";
    }

    double GetNum() const {
        if (IsNum()) return Ntemp;
        throw "RUNTIME ERROR: Value not a number";
    }

    bool GetBool() const {
        if (IsBool()) return Btemp;
        throw "RUNTIME ERROR: Value not a boolean";
    }

    void SetType(ValType type) { T = type; }
    void SetNum(double val) { Ntemp = val; }
    void SetString(string val) { Stemp = val; }
    void SetBool(bool val) { Btemp = val; }

    // Numeric operations
    Value operator+(const Value& op) const;
    Value operator-(const Value& op) const;
    Value operator*(const Value& op) const;
    Value operator/(const Value& op) const;
    Value operator%(const Value& op) const;

    // Numeric relational
    Value operator==(const Value& op) const;
    Value operator>=(const Value& op) const;
    Value operator<(const Value& op) const;

    // Exponentiation
    Value Expon(const Value& oper) const;

    // String ops
    Value Catenate(const Value& oper) const;
    Value Repeat(const Value& oper) const;
    Value SEQ(const Value& oper) const;
    Value SGT(const Value& oper) const;
    Value SLE(const Value& oper) const;

    // Boolean logic
    Value operator&&(const Value& op) const;
    Value operator||(const Value& op) const;
    Value operator!() const;

    // Corrected output (NO newline)
    friend ostream& operator<<(ostream& out, const Value& op) {
        if (op.IsNum()) {
            out << fixed << setprecision(1) << op.GetNum();
        }
        else if (op.IsString()) {
            out << op.GetString();
        }
        else if (op.IsBool()) {
            out << (op.GetBool() ? "true" : "false");
        }
        return out;   // <<< IMPORTANT: no newline here
    }
};

#endif
