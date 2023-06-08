#include "../header/hashset.h"

Hashset::Hashset()
{
    Hashset::data = new Window[MAX_PID];
    Hashset::indices = new Window[MAX_PID];
}

Hashset::~Hashset()
{
    delete[] Hashset::data;
    delete[] Hashset::indices;
}

void Hashset::add(Window value)
{
    Hashset::data[value.pid] = value;
    Hashset::indices[Hashset::length] = value;
    Hashset::length++;
}

bool Hashset::contains(Window value)
{
    if (Hashset::data[value.pid].pid == value.pid)
    {
        return true;
    }

    return false;
}

Window *Hashset::toArray()
{
    Window *copy = new Window[Hashset::length];
    for (int i = 0; i < Hashset::length; i++)
    {
        copy[i] = Hashset::indices[i];
        copy[i].length = Hashset::length;
    }
    return copy;
}