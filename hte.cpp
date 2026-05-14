#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;

int main()
{
    ifstream fin("input.asm");

    if(!fin)
    {
        cout << "File not found\n";
        return 0;
    }

    map<string, int> symtab;

    string line;

    string programName = "";
    int startAddr = 0;

    int locctr = 0;

    while(getline(fin, line))
    {
        stringstream ss(line);

        string t1, t2, t3;

        ss >> t1 >> t2 >> t3;

        if(t1 == "")
            continue;

        // START
        if(t2 == "START")
        {
            programName = t1;

            startAddr = stoi(t3, nullptr, 16);

            locctr = startAddr;
        }

        // END
        else if(t1 == "END")
        {
            break;
        }

        // Label present
        else if(t2 != "")
        {
            // Store label
            symtab[t1] = locctr;

            // RESW
            if(t2 == "RESW")
            {
                locctr += (3 * stoi(t3));
            }

            // RESB
            else if(t2 == "RESB")
            {
                locctr += stoi(t3);
            }

            // WORD
            else if(t2 == "WORD")
            {
                locctr += 3;
            }

            // BYTE
            else if(t2 == "BYTE")
            {
                locctr += 1;
            }

            // Instruction
            else
            {
                locctr += 3;
            }
        }

        // Instruction without label
        else
        {
            locctr += 3;
        }
    }

    fin.close();

    int programLength = locctr - startAddr;

    // H Record
    cout << "\nH RECORD\n";

    cout << "H^"
         << programName
         << "^"
         << startAddr
         << "^"
         << programLength
         << "\n";

    // E Record
    cout << "\nE RECORD\n";

    cout << "E^" << startAddr << "\n";

    // Symbol Table
    cout << "\nSYMBOL TABLE\n\n";

    cout << "SYMBOL\tVALUE\n";

    for(auto s : symtab)
    {
        cout << s.first
             << "\t"
             << s.second
             << "\n";
    }

    return 0;
}