#include "val.h"
#include <cmath>
#include <sstream>
#include <cctype>

using namespace std;

static bool isNumericStr(const string& s) {
    if (s.empty()) return false;
    char* endptr = nullptr;
    strtod(s.c_str(), &endptr);
    return (*endptr == '\0');
}

Value Value::operator*(const Value& op) const {
    bool leftNumeric =
        IsNum() || (IsString() && isNumericStr(GetString()));

    bool rightNumeric =
        op.IsNum() || (op.IsString() && isNumericStr(op.GetString()));

    if (leftNumeric && rightNumeric) {
        double L = IsNum() ? GetNum() : stod(GetString());
        double R = op.IsNum() ? op.GetNum() : stod(op.GetString());
        return Value(L * R);
    }

    return Value();
}

Value Value::operator<(const Value& op) const {
    bool leftNumeric =
        IsNum() || (IsString() && isNumericStr(GetString()));

    bool rightNumeric =
        op.IsNum() || (op.IsString() && isNumericStr(op.GetString()));

    if (leftNumeric && rightNumeric) {
        double L = IsNum() ? GetNum() : stod(GetString());
        double R = op.IsNum() ? op.GetNum() : stod(op.GetString());
        return Value(L < R);
    }

    if (IsString() && op.IsString() &&
        !isNumericStr(GetString()) &&
        !isNumericStr(op.GetString())) {
        return Value();
    }

    return Value();
}

Value Value::Catenate(const Value& op) const {
    if (IsErr() || op.IsErr())
        return Value();

    if (IsBool() || op.IsBool())
        return Value();

    auto numToRaw = [](double n) {
        ostringstream ss;
        ss << n;
        return ss.str();
    };

    string L, R;

    if (IsString())
        L = GetString();
    else if (IsNum())
        L = numToRaw(GetNum());
    else
        return Value();

    if (op.IsString())
        R = op.GetString();
    else if (op.IsNum())
        R = numToRaw(op.GetNum());
    else
        return Value();

    return Value(L + R);
}

Value Value::Repeat(const Value& op) const {
    if (IsErr() || op.IsErr())
        return Value();

    if (IsBool() || op.IsBool())
        return Value();

    string rightStr;

    if (op.IsNum()) {
        ostringstream ss;
        ss << op.GetNum();
        rightStr = ss.str();
    }
    else if (op.IsString()) {
        rightStr = op.GetString();
    }
    else {
        return Value();
    }

    bool hasDigit = false;
    for (char c : rightStr)
        if (isdigit(static_cast<unsigned char>(c)))
            hasDigit = true;
    if (!hasDigit)
        return Value();

    size_t pos = rightStr.find('.');
    string intPart = (pos == string::npos ? rightStr : rightStr.substr(0, pos));

    while (!intPart.empty() && isspace(static_cast<unsigned char>(intPart.front())))
        intPart.erase(intPart.begin());
    while (!intPart.empty() && isspace(static_cast<unsigned char>(intPart.back())))
        intPart.pop_back();

    if (intPart.empty())
        return Value();

    for (char c : intPart)
        if (!isdigit(static_cast<unsigned char>(c)))
            return Value();

    int count = stoi(intPart);
    if (count < 0)
        return Value();

    string leftStr;

    if (IsString()) {
        leftStr = GetString();
    }
    else if (IsNum()) {
        ostringstream ss;
        ss << GetNum();
        leftStr = ss.str();
    }
    else {
        return Value();
    }

    string result;
    result.reserve(leftStr.size() * count);
    for (int i = 0; i < count; i++)
        result += leftStr;

    return Value(result);
}

Value Value::SEQ(const Value& op) const {
    if (IsErr() || op.IsErr())
        return Value();

    if (IsBool() || op.IsBool())
        return Value();

    bool leftNumeric =
        IsNum() || (IsString() && isNumericStr(GetString()));

    bool rightNumeric =
        op.IsNum() || (op.IsString() && isNumericStr(op.GetString()));

    if (leftNumeric && rightNumeric) {
        double L = IsNum() ? GetNum() : stod(GetString());
        double R = op.IsNum() ? op.GetNum() : stod(op.GetString());
        return Value(L == R);
    }

    if (IsString() && op.IsString() &&
        !isNumericStr(GetString()) &&
        !isNumericStr(op.GetString())) {
        return Value(GetString() == op.GetString());
    }

    if ((leftNumeric && op.IsString() && !isNumericStr(op.GetString())) ||
        (rightNumeric && IsString() && !isNumericStr(GetString()))) {
        return Value(false);
    }

    return Value();
}
