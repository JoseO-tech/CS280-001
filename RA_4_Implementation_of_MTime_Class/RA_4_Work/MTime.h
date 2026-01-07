#ifndef MTIME_H
#define MTIME_H

#include <iostream>
#include <iomanip>
#include <string>

class MTime {
private:
    int hours;
    int minutes;
    int seconds;

    void Normalize(); // keep normalization private

public:
    // Single flexible constructor (covers all cases)
    MTime(int h = 0, int m = 0, int s = 0);

    // Getters
    int GetHours() const;
    int GetMinutes() const;
    int GetSeconds() const;

    // Setter
    void SetTime(int h, int m, int s);

    // Display in 24-hour format
    void DisplayTime() const;

    // Operators
    bool operator==(const MTime& other) const;
    bool operator>(const MTime& other) const;

    // Friend for 12-hour format printing
    friend std::ostream& operator<<(std::ostream& out, const MTime& t);
};

#endif
