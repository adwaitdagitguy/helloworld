#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

struct Macro
{
    string name;
    int start;
    int end;
};

int main()
{
    ifstream fin("macro.asm");

    if(!fin)
    {
        cout << "File not found\n";
        return 0;
    }

    vector<string> deftab;
    vector<Macro> namtab;

    string line;

    bool insideMacro = false;

    Macro currentMacro;

    while(getline(fin, line))
    {
        stringstream ss(line);

        string t1, t2, t3;

        ss >> t1 >> t2 >> t3;

        // MACRO start
        if(t1 == "MACRO")
        {
            insideMacro = true;

            currentMacro.start = deftab.size();

            continue;
        }

        // Macro header
        if(insideMacro && currentMacro.name == "")
        {
            currentMacro.name = t1;

            deftab.push_back(line);

            continue;
        }

        // Inside macro body
        if(insideMacro)
        {
            deftab.push_back(line);

            // MEND
            if(t1 == "MEND")
            {
                currentMacro.end = deftab.size() - 1;

                namtab.push_back(currentMacro);

                currentMacro = {"", 0, 0};

                insideMacro = false;
            }
        }
    }

    fin.close();

    // Print DEFTAB
    cout << "\nDEFTAB\n\n";

    for(int i = 0; i < deftab.size(); i++)
    {
        cout << i << "\t" << deftab[i] << "\n";
    }

    // Print NAMTAB
    cout << "\nNAMTAB\n\n";

    cout << "NAME\tSTART\tEND\n";

    for(auto m : namtab)
    {
        cout << m.name << "\t"
             << m.start << "\t"
             << m.end << "\n";
    }

    return 0;
}