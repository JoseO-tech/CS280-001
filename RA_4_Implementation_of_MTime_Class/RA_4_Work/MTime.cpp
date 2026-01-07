#include "MTime.h"
using namespace std;

void MTime::Normalize() {
    if (seconds >= 60) {
        minutes += seconds / 60;
        seconds %= 60;
    }
    if (minutes >= 60) {
        hours += minutes / 60;
        minutes %= 60;
    }
    if (hours >= 24) {
        hours %= 24;
    }

    if (seconds < 0) seconds = 0;
    if (minutes < 0) minutes = 0;
    if (hours < 0) hours = 0;
}

MTime::MTime(int h, int m, int s) : hours(h), minutes(m), seconds(s) {
    Normalize();
}

int MTime::GetHours() const { return hours; }
int MTime::GetMinutes() const { return minutes; }
int MTime::GetSeconds() const { return seconds; }

void MTime::SetTime(int h, int m, int s) {
    hours = h;
    minutes = m;
    seconds = s;
    Normalize();
}

void MTime::DisplayTime() const {
    cout << setw(2) << setfill('0') << hours << ":"
         << setw(2) << setfill('0') << minutes << ":"
         << setw(2) << setfill('0') << seconds;
}

bool MTime::operator==(const MTime& other) const {
    return hours == other.hours &&
           minutes == other.minutes &&
           seconds == other.seconds;
}

bool MTime::operator>(const MTime& other) const {
    if (hours != other.hours) return hours > other.hours;
    if (minutes != other.minutes) return minutes > other.minutes;
    return seconds > other.seconds;
}

ostream& operator<<(ostream& out, const MTime& t) {
    int h12 = t.hours % 12;
    if (h12 == 0) h12 = 12;

    string suffix = (t.hours < 12) ? "AM" : "PM";

    out << h12 << ":"
        << setw(2) << setfill('0') << t.minutes << ":"
        << setw(2) << setfill('0') << t.seconds << " "
        << suffix;

    return out;
}
