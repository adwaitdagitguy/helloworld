#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
using namespace std;

struct Symbol {
    string name;
    int address;
};

int main() {
    int start = 0x4000;
    int locctr = start;

    vector<Symbol> sym;

    // Opcodes (SIC)
    int LDX = 0x04;
    int LDA = 0x00;
    int ADD = 0x18;
    int TIX = 0x2C;
    int JLT = 0x38;
    int STA = 0x0C;
    int RSUB = 0x4C;

    // -------- PASS 1 --------
    // FIRST
    sym.push_back({"FIRST", locctr});
    locctr += 3;

    locctr += 3; // LDA

    // LOOP
    sym.push_back({"LOOP", locctr});
    locctr += 3; // ADD
    locctr += 3; // TIX
    locctr += 3; // JLT
    locctr += 3; // STA
    locctr += 3; // RSUB

    // TABLE
    sym.push_back({"TABLE", locctr});
    locctr += 2000 * 3;

    // COUNT
    sym.push_back({"COUNT", locctr});
    locctr += 3;

    // ZERO
    sym.push_back({"ZERO", locctr});
    locctr += 3;

    // TOTAL
    sym.push_back({"TOTAL", locctr});
    locctr += 3;

    int programLength = locctr - start;

    // -------- PASS 2 --------
    cout << "LOCATION COUNTER / OBJECT CODE TABLE \n\n";
    cout << "LOC\tSOURCE\t\t\tOBJECT CODE\n";

    cout << hex << uppercase << setfill('0');

    cout << setw(4) << start << "\tFIRST LDX ZERO\t\t"
         << setw(2) << LDX << setw(4) << sym[4].address << endl;

    cout << setw(4) << start + 3 << "\tLDA ZERO\t\t"
         << setw(2) << LDA << setw(4) << sym[4].address << endl;

    cout << setw(4) << start + 6 << "\tLOOP ADD TABLE,X\t"
         << setw(2) << ADD << setw(4) << (sym[2].address + 0x8000) << endl;

    cout << setw(4) << start + 9 << "\tTIX COUNT\t\t"
         << setw(2) << TIX << setw(4) << sym[3].address << endl;

    cout << setw(4) << start + 12 << "\tJLT LOOP\t\t"
         << setw(2) << JLT << setw(4) << sym[1].address << endl;

    cout << setw(4) << start + 15 << "\tSTA TOTAL\t\t"
         << setw(2) << STA << setw(4) << sym[5].address << endl;

    cout << setw(4) << start + 18 << "\tRSUB\t\t\t"
         << setw(2) << RSUB << "0000\n";

    cout << setw(4) << sym[2].address << "\tTABLE RESW 2000\t\t-\n";
    cout << setw(4) << sym[3].address << "\tCOUNT RESW 1\t\t-\n";
    cout << setw(4) << sym[4].address << "\tZERO WORD 0\t\t000000\n";
    cout << setw(4) << sym[5].address << "\tTOTAL RESW 1\t\t-\n";

    // -------- PROGRAM LENGTH --------
    cout << "\n PROGRAM LENGTH n";
    cout << "Decimal Length = " << dec << programLength << endl;
    cout << "Hex Length = " << hex << uppercase << programLength << endl;

    // -------- HTE RECORD --------
    cout << "\n H T E RECORDS\n\n";

    cout << "H^SUM^"
         << setw(6) << setfill('0') << hex << start << "^"
         << setw(6) << programLength << endl;

    cout << "T^"
         << setw(6) << start << "^15^"; // 21 bytes = 15 hex

    cout << setw(2) << LDX << setw(4) << sym[4].address << "^";
    cout << setw(2) << LDA << setw(4) << sym[4].address << "^";
    cout << setw(2) << ADD << setw(4) << sym[2].address << "^";
    cout << setw(2) << TIX << setw(4) << sym[3].address << "^";
    cout << setw(2) << JLT << setw(4) << sym[1].address << "^";
    cout << setw(2) << STA << setw(4) << sym[5].address << "^";
    cout << setw(2) << RSUB << "0000\n";

    cout << "E^" << setw(6) << start << endl;

    // -------- SYMBOL TABLE --------
    cout << "\n SYMBOL TABLE \n\n";
    cout << "Symbol\tAddress\n";

    for (auto &s : sym) {
        cout << s.name << "\t" << setw(4) << setfill('0') << hex << s.address << endl;
    }

    return 0;
}