#include "../header/hashset.h"

Hashset::Hashset()
{
    Hashset::data = new int[MAX_PID];
    Hashset::indices = new int[MAX_PID];
}

Hashset::~Hashset()
{
    delete[] Hashset::data;
    delete[] Hashset::indices;
}

void Hashset::add(int value)
{
    Hashset::data[value] = value;
    Hashset::indices[Hashset::length] = value;
    Hashset::length++;
}

bool Hashset::contains(int value)
{
    if (Hashset::data[value] == value)
    {
        return true;
    }

    return false;
}

int *Hashset::toArray()
{
    int *copy = new int[Hashset::length + 1];
    copy[0] = Hashset::length;
    for (int i = 1; i < Hashset::length; i++)
    {
        copy[i] = Hashset::indices[i];
    }
    return copy;
}