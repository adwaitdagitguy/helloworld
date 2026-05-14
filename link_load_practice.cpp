#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;

struct Symbol
{
    string name;
    int value;
};

int main()
{
    ifstream fin("link_load.asm");
    if(!fin) // i.e. fin equals 0
    {
        cout<<"Failed to read file, exiting\n";
        return 0;
    }
    string line;
    vector<Symbol> defTable;
    vector<string> refTable;
    int locctr=0;
    while(getline(fin, line)) // read from fin into line
    {
        stringstream ss(line);
        string word;
        vector<string> tokens;
        while(ss >> word) // while the string stream spits out words (strings)
        {
            tokens.push_back(word);
        }
        if(tokens.size()==0)
        {
            continue;
        }

        // EXTREF
        if(tokens[0]=="EXTDEF")
        {
            for(int i=1; i<tokens.size(); i++)
            {
                string sym = tokens[i];
                if(sym.back()==',')
                {
                    sym.pop_back();
                }
                Symbol s;
                s.value=locctr;
                s.name=sym;
                defTable.push_back(s);
                locctr+=3;
            }

        }
        // EXTDEF
        else if(tokens[0]=="EXTREF")
        {
            for(int i=1; i<tokens.size(); i++)
            {
                string sym = tokens[i];
                if(sym.back()==',') // do not compare string with char
                {
                    sym.pop_back();                        
                }    
                refTable.push_back(sym);   
            }
        }
        // START/END
        else if(tokens[0]=="START" || tokens[0]=="END")
        {
            continue;
        }
        // Other INSTR
        else
        {
            locctr+=3; // assuming each INSTR is of 3B
        }
    }
    fin.close();

    // Printing the records and the symbol tables...
    cout<<"\n\n LOCAL SYMBOL TABLE\n\n";
    
    cout<<left<<setw(15)<<"Symbol Name"<<"Value\n";

    for(auto s:defTable)
    {
        cout<<left<<setw(15)<<s.name;
        cout<<right<<setw(6)<<setfill('0')<<hex<<uppercase<<s.value<<endl;
        cout << setfill(' ');
    }

    // D
    cout<<"\nRecords of the Definition Table\n";
    cout<<"D";
    for(auto d: defTable)
    {
        cout<<"^"<<d.name<<"^"
        <<setw(6)<<setfill('0')
        <<hex<<uppercase<<d.value;
        cout << setfill(' ');
    }

    // R
    cout<<"\nRecords of the reference table\n";
    cout<<"R";
    for(auto r: refTable)
    {
        cout<<"^"<<r;
    }
    // Symbol Table
    return 0;
}