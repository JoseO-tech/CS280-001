#include "val.h"
#include <sstream>
#include <cmath>
#include <iomanip>

static double StringToNum(const string &s) {
    try {
        return stod(s);
    } catch (...) {
        return 0.0;
    }
}

static string ToString(const Value &v) {
    if (v.IsString()) return v.GetString();

    if (v.IsNum()) {
        double n = v.GetNum();

        if (floor(n) == n) {
            std::ostringstream oss;
            oss << (long long)n;
            return oss.str();
        }

        std::ostringstream oss;
        oss << fixed << setprecision(1) << n;
        return oss.str();
    }

    if (v.IsBool())
        return v.GetBool() ? "true" : "false";

    return "";
}

static double ToNum(const Value &v) {
    if (v.IsNum()) return v.GetNum();
    if (v.IsString()) return StringToNum(v.GetString());
    if (v.IsBool()) return v.GetBool() ? 1.0 : 0.0;
    return 0.0;
}

static bool ToBool(const Value &v) {
    if (v.IsBool()) return v.GetBool();
    if (v.IsNum()) return v.GetNum() != 0.0;

    if (v.IsString()) {
        string s = v.GetString();
        return !(s == "" || s == "0");
    }

    return false;
}

Value Value::operator+(const Value &op) const {
    return Value(ToNum(*this) + ToNum(op));
}

Value Value::operator-(const Value &op) const {
    return Value(ToNum(*this) - ToNum(op));
}

Value Value::operator*(const Value &op) const {
    return Value(ToNum(*this) * ToNum(op));
}

Value Value::operator/(const Value &op) const {
    double rhs = ToNum(op);
    if (rhs == 0.0) return Value();
    return Value(ToNum(*this) / rhs);
}

Value Value::operator%(const Value &op) const {
    bool lhsStr = IsString();
    bool rhsStr = op.IsString();

    if (!lhsStr && rhsStr) return Value();

    if (lhsStr && rhsStr) return Value();

    if (!lhsStr && !rhsStr) {
        int a = (int)ToNum(*this);
        int b = (int)ToNum(op);
        if (b == 0) return Value();
        return Value((double)(a % b));
    }

    if (lhsStr && !rhsStr) {
        int a = (int)StringToNum(GetString());
        int b = (int)ToNum(op);
        if (b == 0) return Value();
        return Value((double)(a % b));
    }

    return Value();
}

Value Value::operator==(const Value &op) const {
    return Value(ToNum(*this) == ToNum(op));
}

Value Value::operator>=(const Value &op) const {
    return Value(ToNum(*this) >= ToNum(op));
}

Value Value::operator<(const Value &op) const {
    return Value(ToNum(*this) < ToNum(op));
}

Value Value::Expon(const Value& oper) const {
    if (!IsNum() || !oper.IsNum())
        return Value();
    return Value(pow(GetNum(), oper.GetNum()));
}

Value Value::Catenate(const Value &op) const {
    return Value(ToString(*this) + ToString(op));
}

Value Value::Repeat(const Value &op) const {
    if (!op.IsNum() && !op.IsString())
        return Value();  

    double raw = ToNum(op);
    int n = (int)raw;

    if (n < 0) return Value();

    string base = ToString(*this);

    string out = "";
    for (int i = 0; i < n; i++)
        out += base;

    return Value(out);
}

Value Value::SEQ(const Value &oper) const {
    return Value(ToString(*this) == ToString(oper));
}

Value Value::SGT(const Value &oper) const {
    return Value(ToString(*this) > ToString(oper));
}

Value Value::SLE(const Value &oper) const {
    return Value(ToString(*this) <= ToString(oper));
}

Value Value::operator&&(const Value &op) const {
    return Value(ToBool(*this) && ToBool(op));
}

Value Value::operator||(const Value &op) const {
    return Value(ToBool(*this) || ToBool(op));
}

Value Value::operator!() const {
    return Value(!ToBool(*this));
}
