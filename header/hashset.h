#include <iostream>

#define MAX_PID 99999

class Hashset
{
private: 
    int *data;
    int *indices;
    int length = 0;

public:
    Hashset();
    ~Hashset();
    void add(int value);
    bool contains(int value);
    int *toArray();
};