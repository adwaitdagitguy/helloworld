#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;

struct Block
{
    int no;
    int start;
    int length;
};

int main()
{
    ifstream fin("input.asm");

    if(!fin)
    {
        cout << "File not found\n";
        return 0;
    }

    map<string, Block> blockTable;

    string currentBlock = "DEFAULT";

    blockTable[currentBlock] = {0, 0, 0};

    int blockNo = 1;

    string line;

    while(getline(fin, line))
    {
        stringstream ss(line);

        string t1, t2, t3;

        ss >> t1 >> t2 >> t3;

        // Empty line
        if(t1 == "")
            continue;

        // USE statement
        if(t1 == "USE")
        {
            if(t2 == "")
                currentBlock = "DEFAULT";
            else
                currentBlock = t2;

            // Create block if new
            if(blockTable.find(currentBlock) == blockTable.end())
            {
                blockTable[currentBlock] =
                {
                    blockNo,
                    0,
                    0
                };

                blockNo++;
            }
        }

        // Ignore START and END
        else if(t2 == "START" || t1 == "END")
        {
            continue;
        }

        // RESW handling
        else if(t2 == "RESW")
        {
            int n = stoi(t3);

            blockTable[currentBlock].length += (3 * n);
        }

        // RESB handling
        else if(t2 == "RESB")
        {
            int n = stoi(t3);

            blockTable[currentBlock].length += n;
        }

        // WORD
        else if(t2 == "WORD")
        {
            blockTable[currentBlock].length += 3;
        }

        // BYTE
        else if(t2 == "BYTE")
        {
            blockTable[currentBlock].length += 1;
        }

        // Normal instruction
        else
        {
            blockTable[currentBlock].length += 3;
        }
    }

    fin.close();

    // Assign starting addresses
    int addr = 0;

    for(auto &b : blockTable)
    {
        b.second.start = addr;
        addr += b.second.length;
    }

    // Print BLOCK TABLE
    cout << "\nBLOCK TABLE\n\n";

    cout << "BLOCK\tNO\tSTART\tLENGTH\n";

    for(auto b : blockTable)
    {
        cout << b.first << "\t"
             << b.second.no << "\t"
             << b.second.start << "\t"
             << b.second.length << "\n";
    }

    return 0;
}