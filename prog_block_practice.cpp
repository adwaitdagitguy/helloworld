#include<iostream>
#include<fstream>
#include<sstream>
#include<map>

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
        cout<<"Failed to read file\n";
        return 0;
    }
    map<string, Block> block_table;
    string current_block="DEFAULT";
    block_table[current_block]={0,0,0};
    int block_no=1;
    string line;
    while(getline(fin, line))
    {
        stringstream ss(line);
        string t1,t2,t3;
        ss>>t1>>t2>>t3;
        if(t1=="")
        continue;
        if(t1=="USE")
        {
            if(t2=="")
            current_block="DEFAULT";
            else
            current_block=t2;
            if(block_table.find(current_block)==block_table.end())
            {
                block_table[current_block]={block_no, 0, 0};
                block_no++;
            }
        }
        else if(t2=="START"||t2=="END")
        {
            continue;
        }
        else if(t2=="RESW")
        {
            int n = stoi(t3);
            block_table[current_block].length+=3*n;
        }
        else if(t2=="RESB")
        {
            int n = stoi(t3);
            block_table[current_block].length+=n;
        }
        else
        {
            block_table[current_block].length+=3;
        }

        int addr=0;
        for(auto &b: block_table)
        {
            b.second.start=addr;
            addr+=b.second.length;
        }
        
    }
    cout<<"BLOCK TABLE\n\n";
    cout<<"BLOCK\tNO\tSTART\tLENGTH\n";
    for(auto &b: block_table)
    {
        cout<<b.first<<"\t"<<b.second.no<<"\t"<<b.second.start<<"\t"<<b.second.length<<endl;
    }
    return 0;
}