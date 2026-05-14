#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>

using namespace std;

struct Symbol
{
    string name;
    int value;
};

int main()
{
    ifstream fin("link_load.asm");

    if(!fin)
    {
        cout << "Cannot open file.\n";
        return 0;
    }

    string line;
    vector<Symbol> defTable;
    vector<string> refTable;

    int locctr = 0;

    while(getline(fin, line))
    {
        stringstream ss(line);

        string word;
        vector<string> tokens;

        while(ss >> word)
        {
            tokens.push_back(word);
        }

        if(tokens.size() == 0)
            continue;

        // EXTDEF Processing
        if(tokens[0] == "EXTDEF")
        {
            for(int i = 1; i < tokens.size(); i++)
            {
                string sym = tokens[i];

                // Remove comma if present
                if(sym.back() == ',')
                    sym.pop_back();

                Symbol s;
                s.name = sym;
                s.value = locctr;

                defTable.push_back(s);

                locctr += 3; // each instruction assumed 3 bytes
            }
        }

        // EXTREF Processing
        else if(tokens[0] == "EXTREF")
        {
            for(int i = 1; i < tokens.size(); i++)
            {
                string sym = tokens[i];

                if(sym.back() == ',')
                    sym.pop_back();

                refTable.push_back(sym);
            }
        }

        // Skip START and END
        else if(tokens[0] == "START" || tokens[0] == "END")
        {
            continue;
        }

        // Normal instruction
        else
        {
            locctr += 3;
        }
    }

    ///////////////////////

    fin.close();

    // D RECORD
    cout << "----- D RECORD -----\n";

    cout << "D";

    for(auto s : defTable)
    {
        cout << "^" << s.name << "^"
             << setw(6)
             << setfill('0')
             << hex
             << uppercase
             << s.value;
    }

    // R RECORD
    cout << "\n\n----- R RECORD -----\n";

    cout << "R";

    for(auto r : refTable)
    {
        cout << "^" << r;
    }

    // LOCAL SYMBOL TABLE
    cout << "\n\n----- LOCAL SYMBOL TABLE -----\n";

    cout << left << setw(15) << "Symbol Name"
         << "Value\n";

    for(auto s : defTable)
    {
        cout << left << setw(15)
             << s.name;

        cout << setw(6)
             << setfill('0')
             << hex
             << uppercase
             << s.value
             << "\n";
    }

    return 0;
}