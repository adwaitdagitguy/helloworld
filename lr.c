#include <bits/stdc++.h>
using namespace std;

// ════════════════════════════════════════════════════════════════════
//  SECTION 1 — Core data types
// ════════════════════════════════════════════════════════════════════

// A single LR(1) item:
struct Item {
    int    prod;       // production index (0 = augmented S'→S)
    int    dot;        // dot position (0 = before first symbol of rhs)
    string lookahead;  // single terminal lookahead
};

// A grammar production
struct Production {
    string lhs;
    vector<string> rhs;  // empty rhs = epsilon
    string str() const {
        string s = lhs + " → ";
        for (auto& x : rhs) s += x + " ";
        return s;
    }
};

// One LR(1) state = a set of items + its GOTO transitions
struct State {
    int              id;
    vector<Item>     items;
    map<string,int>  next;   // symbol → target state id
};

// ════════════════════════════════════════════════════════════════════
//  SECTION 2 — Grammar & hardcoded states
// ════════════════════════════════════════════════════════════════════

// Productions (index 0 = augmented)
const vector<Production> GRAMMAR = {
    { "S'", {"S"}           },   // 0  S' → S
    { "S",  {"id","=","E"}  },   // 1  S  → id = E
    { "E",  {"E","+","T"}   },   // 2  E  → E + T
    { "E",  {"T"}           },   // 3  E  → T
    { "T",  {"T","*","F"}   },   // 4  T  → T * F
    { "T",  {"F"}           },   // 5  T  → F
    { "F",  {"id"}          },   // 6  F  → id
    { "F",  {"num"}         },   // 7  F  → num
};

// Terminals and non-terminals
const vector<string> TERMINALS    = { "id","num","=","+","*","$" };
const vector<string> NONTERMINALS = { "S'","S","E","T","F" };

bool isTerminal(const string& s) {
    return find(TERMINALS.begin(), TERMINALS.end(), s) != TERMINALS.end();
}

/*
  ── Hardcoded LR(1) Itemsets 
  Format of each item: { prod_index, dot_position, "lookahead" }

  The GOTO transitions (next) are also hardcoded — these encode the
  edges of the LR(1) automaton.  The program will read these and
  automatically build the ACTION and GOTO tables.
*/
vector<State> buildStates() {
    vector<State> S(13);
    for (int i = 0; i < 13; i++) S[i].id = i;

    // ── I0 
    S[0].items = {
        {0, 0, "$"},   // S' → • S,  $
        {1, 0, "$"},   // S  → • id=E, $
    };
    S[0].next = { {"S",1}, {"id",2} };

    // ── I1 
    S[1].items = {
        {0, 1, "$"},   // S' → S •,  $   ← ACCEPT
    };
    // no transitions

    // ── I2 
    S[2].items = {
        {1, 1, "$"},   // S → id • = E,  $
    };
    S[2].next = { {"=",3} };

    // ── I3 
    S[3].items = {
        {1, 2, "$"},   // S → id = • E,  $
        {2, 0, "$"},   // E → • E+T,     $
        {2, 0, "+"},   // E → • E+T,     +
        {3, 0, "$"},   // E → • T,       $
        {3, 0, "+"},   // E → • T,       +
        {4, 0, "$"},   // T → • T*F,     $
        {4, 0, "+"},   // T → • T*F,     +
        {4, 0, "*"},   // T → • T*F,     *
        {5, 0, "$"},   // T → • F,       $
        {5, 0, "+"},   // T → • F,       +
        {5, 0, "*"},   // T → • F,       *
        {6, 0, "$"},   // F → • id,      $
        {6, 0, "+"},   // F → • id,      +
        {6, 0, "*"},   // F → • id,      *
        {7, 0, "$"},   // F → • num,     $
        {7, 0, "+"},   // F → • num,     +
        {7, 0, "*"},   // F → • num,     *
    };
    S[3].next = { {"E",6}, {"T",7}, {"F",8}, {"id",4}, {"num",5} };

    // ── I4 
    S[4].items = {
        {6, 1, "$"},   // F → id •,  $
        {6, 1, "+"},   // F → id •,  +
        {6, 1, "*"},   // F → id •,  *
    };
    // dot at end → reduce only, no transitions

    // ── I5 
    S[5].items = {
        {7, 1, "$"},   // F → num •,  $
        {7, 1, "+"},   // F → num •,  +
        {7, 1, "*"},   // F → num •,  *
    };

    // ── I6 
    S[6].items = {
        {1, 3, "$"},   // S → id=E •,   $    ← reduce 1
        {2, 1, "$"},   // E → E • +T,   $
        {2, 1, "+"},   // E → E • +T,   +
    };
    S[6].next = { {"+",9} };

    // ── I7 
    S[7].items = {
        {3, 1, "$"},   // E → T •,      $    ← reduce 3
        {3, 1, "+"},   // E → T •,      +    ← reduce 3
        {4, 1, "$"},   // T → T • *F,   $
        {4, 1, "+"},   // T → T • *F,   +
        {4, 1, "*"},   // T → T • *F,   *
    };
    S[7].next = { {"*",10} };

    // ── I8 
    S[8].items = {
        {5, 1, "$"},   // T → F •,  $    ← reduce 5
        {5, 1, "+"},   // T → F •,  +    ← reduce 5
        {5, 1, "*"},   // T → F •,  *    ← reduce 5
    };

    // ── I9 
    S[9].items = {
        {2, 2, "$"},   // E → E+ • T,   $
        {2, 2, "+"},   // E → E+ • T,   +
        {4, 0, "$"},   // T → • T*F,    $
        {4, 0, "+"},   // T → • T*F,    +
        {4, 0, "*"},   // T → • T*F,    *
        {5, 0, "$"},   // T → • F,      $
        {5, 0, "+"},   // T → • F,      +
        {5, 0, "*"},   // T → • F,      *
        {6, 0, "$"},   // F → • id,     $
        {6, 0, "+"},   // F → • id,     +
        {6, 0, "*"},   // F → • id,     *
        {7, 0, "$"},   // F → • num,    $
        {7, 0, "+"},   // F → • num,    +
        {7, 0, "*"},   // F → • num,    *
    };
    S[9].next = { {"T",11}, {"F",8}, {"id",4}, {"num",5} };

    // ── I10 
    S[10].items = {
        {4, 2, "$"},   // T → T* • F,   $
        {4, 2, "+"},   // T → T* • F,   +
        {4, 2, "*"},   // T → T* • F,   *
        {6, 0, "$"},   // F → • id,     $
        {6, 0, "+"},   // F → • id,     +
        {6, 0, "*"},   // F → • id,     *
        {7, 0, "$"},   // F → • num,    $
        {7, 0, "+"},   // F → • num,    +
        {7, 0, "*"},   // F → • num,    *
    };
    S[10].next = { {"F",12}, {"id",4}, {"num",5} };

    // ── I11 
    S[11].items = {
        {2, 3, "$"},   // E → E+T •,    $    ← reduce 2
        {2, 3, "+"},   // E → E+T •,    +    ← reduce 2
        {4, 1, "$"},   // T → T • *F,   $
        {4, 1, "+"},   // T → T • *F,   +
        {4, 1, "*"},   // T → T • *F,   *
    };
    S[11].next = { {"*",10} };

    // ── I12 
    S[12].items = {
        {4, 3, "$"},   // T → T*F •,    $    ← reduce 4
        {4, 3, "+"},   // T → T*F •,    +    ← reduce 4
        {4, 3, "*"},   // T → T*F •,    *    ← reduce 4
    };

    return S;
}

// ════════════════════════════════════════════════════════════════════
//  SECTION 3 — Automatic ACTION / GOTO table construction
// ════════════════════════════════════════════════════════════════════

struct TableEntry {
    enum Kind { ERR, SHIFT, REDUCE, ACCEPT } kind = ERR;
    int val = 0;   // shift → target state;  reduce → production index
    string str() const {
        switch(kind){
            case SHIFT:  return "s" + to_string(val);
            case REDUCE: return "r" + to_string(val);
            case ACCEPT: return "acc";
            default:     return "err";
        }
    }
};

struct ParseTables {
    // action[state][terminal]  goto_[state][nonterminal]
    map<int, map<string,TableEntry>> action;
    map<int, map<string,int>>        goto_;
    vector<string> conflicts;
};

ParseTables buildTables(const vector<State>& states) {
    ParseTables T;

    for (auto& st : states) {
        int sid = st.id;

        // ── Step A: SHIFT / GOTO from transition edges ──────────────
        for (auto& [sym, tgt] : st.next) {
            if (isTerminal(sym)) {
                // terminal transition → SHIFT
                TableEntry e;
                e.kind = TableEntry::SHIFT;
                e.val  = tgt;
                // conflict check
                if (T.action[sid].count(sym) &&
                    T.action[sid][sym].kind != TableEntry::ERR) {
                    T.conflicts.push_back(
                        "State " + to_string(sid) + " on '" + sym +
                        "': conflict between " + T.action[sid][sym].str() +
                        " and " + e.str());
                }
                T.action[sid][sym] = e;
            } else if (sym != "S'") {
                // non-terminal transition → GOTO
                T.goto_[sid][sym] = tgt;
            }
        }

        // ── Step B: REDUCE / ACCEPT from completed items ────────────
        for (auto& item : st.items) {
            const Production& p = GRAMMAR[item.prod];
            bool atEnd = (item.dot == (int)p.rhs.size());
            if (!atEnd) continue;

            // S' → S •  on $ → ACCEPT
            if (item.prod == 0 && item.lookahead == "$") {
                TableEntry e; e.kind = TableEntry::ACCEPT;
                T.action[sid]["$"] = e;
                continue;
            }

            // Otherwise → REDUCE on lookahead
            TableEntry e;
            e.kind = TableEntry::REDUCE;
            e.val  = item.prod;

            if (T.action[sid].count(item.lookahead) &&
                T.action[sid][item.lookahead].kind != TableEntry::ERR) {
                T.conflicts.push_back(
                    "State " + to_string(sid) + " on '" + item.lookahead +
                    "': conflict between " + T.action[sid][item.lookahead].str() +
                    " and " + e.str());
            } else {
                T.action[sid][item.lookahead] = e;
            }
        }
    }
    return T;
}

// ════════════════════════════════════════════════════════════════════
//  SECTION 4 — Pretty-print the tables
// ════════════════════════════════════════════════════════════════════

void printTables(const ParseTables& T) {
    const vector<string> terms = {"id","num","=","+","*","$"};
    const vector<string> nonts = {"S","E","T","F"};   // exclude S'

    int W = 7;
    cout << "\n  ── ACTION Table ──────────────────────────────────────────\n";
    cout << "  " << setw(6) << "State";
    for (auto& t : terms) cout << setw(W) << t;
    cout << "\n  " << string(6 + W*terms.size(), '-') << "\n";

    for (int s = 0; s <= 12; s++) {
        cout << "  " << setw(6) << s;
        for (auto& t : terms) {
            string cell = "  .";
            if (T.action.count(s) && T.action.at(s).count(t))
                cell = "  " + T.action.at(s).at(t).str();
            cout << setw(W) << cell;
        }
        cout << "\n";
    }

    cout << "\n  ── GOTO Table ────────────────────────────────────────────\n";
    cout << "  " << setw(6) << "State";
    for (auto& n : nonts) cout << setw(W) << n;
    cout << "\n  " << string(6 + W*nonts.size(), '-') << "\n";

    for (int s = 0; s <= 12; s++) {
        cout << "  " << setw(6) << s;
        for (auto& n : nonts) {
            string cell = "  .";
            if (T.goto_.count(s) && T.goto_.at(s).count(n))
                cell = "  " + to_string(T.goto_.at(s).at(n));
            cout << setw(W) << cell;
        }
        cout << "\n";
    }
}

// ════════════════════════════════════════════════════════════════════
//  SECTION 5 — Lexer
// ════════════════════════════════════════════════════════════════════

struct Token { string type; string val; };

vector<Token> tokenize(const string& s) {
    vector<Token> v;
    size_t i = 0;
    while (i < s.size()) {
        if (isspace((unsigned char)s[i])) { i++; continue; }
        if (isalpha((unsigned char)s[i])) {
            string w;
            while (i < s.size() && isalnum((unsigned char)s[i])) w += s[i++];
            v.push_back({"id", w});
        } else if (isdigit((unsigned char)s[i])) {
            string w;
            while (i < s.size() && isdigit((unsigned char)s[i])) w += s[i++];
            v.push_back({"num", w});
        } else if (s[i]=='=')  { v.push_back({"=",  "="}); i++; }
        else if  (s[i]=='+')   { v.push_back({"+",  "+"}); i++; }
        else if  (s[i]=='*')   { v.push_back({"*",  "*"}); i++; }
        else { cerr << "Unknown char: " << s[i] << "\n"; i++; }
    }
    v.push_back({"$", "$"});
    return v;
}

// ════════════════════════════════════════════════════════════════════
//  SECTION 6 — Parser driver
// ════════════════════════════════════════════════════════════════════

void parse(const string& input, const ParseTables& T) {
    cout << "┌──────────────────────────────────────────────────────────────┐\n";
    printf("│  Input: \"%-53s│\n", (input + "\"").c_str());
    cout << "└──────────────────────────────────────────────────────────────┘\n";
    cout << left << setw(28) << "  State Stack"
                 << setw(28) << "  Remaining Input"
                 << "  Action\n";
    cout << string(80, '-') << "\n";

    auto tokens = tokenize(input);
    int pos = 0;

    stack<int>    ss;
    stack<string> sym;
    ss.push(0);

    auto stackStr = [&]() {
        vector<int> v; stack<int> tmp = ss;
        while (!tmp.empty()) { v.push_back(tmp.top()); tmp.pop(); }
        reverse(v.begin(), v.end());
        string s;
        for (int x : v) s += to_string(x) + " ";
        return s;
    };
    auto remStr = [&]() {
        string s;
        for (size_t i = pos; i < tokens.size(); i++) s += tokens[i].val + " ";
        return s;
    };
    auto print = [&](const string& act) {
        cout << "  " << left << setw(26) << stackStr()
             << "  " << setw(26) << remStr()
             << "  " << act << "\n";
    };

    bool accepted = false;
    while (true) {
        int state = ss.top();
        string tok = tokens[pos].type;
        string val = tokens[pos].val;

        // Lookup action
        if (!T.action.count(state) || !T.action.at(state).count(tok)) {
            print("ERROR: no action for state " + to_string(state)
                  + " on '" + val + "'");
            break;
        }

        const TableEntry& e = T.action.at(state).at(tok);

        if (e.kind == TableEntry::ERR) {
            print("ERROR: unexpected '" + val + "'");
            break;
        }
        if (e.kind == TableEntry::ACCEPT) {
            print("ACCEPT ✓");
            accepted = true;
            break;
        }
        if (e.kind == TableEntry::SHIFT) {
            print("Shift '" + val + "'  → state " + to_string(e.val));
            ss.push(e.val);
            sym.push(val);
            pos++;
        } else {  // REDUCE
            const Production& p = GRAMMAR[e.val];
            print("Reduce by  " + p.str());
            for (int i = 0; i < (int)p.rhs.size(); i++) {
                ss.pop();
                if (!sym.empty()) sym.pop();
            }
            int top = ss.top();
            if (!T.goto_.count(top) || !T.goto_.at(top).count(p.lhs)) {
                print("ERROR: no GOTO from state " + to_string(top)
                      + " on " + p.lhs);
                break;
            }
            ss.push(T.goto_.at(top).at(p.lhs));
            sym.push(p.lhs);
        }
    }

    cout << string(80, '-') << "\n";
    cout << "  Result: "
         << (accepted ? "✅  VALID  — accepted by the grammar"
                      : "❌  INVALID — rejected by the parser")
         << "\n\n\n";
}

// ════════════════════════════════════════════════════════════════════
//  MAIN
// ════════════════════════════════════════════════════════════════════
int main() {
    cout << "    LR(1) Parser — Auto Table Construction     \n";


    // ── Build itemsets (hardcoded) 
    auto states = buildStates();
    cout << "\n Loaded " << states.size() << " LR(1) states.\n";

    // ── Construct ACTION/GOTO tables from itemsets 
    auto tables = buildTables(states);
    cout << "  Tables constructed automatically from itemsets.\n";

    // ── Report any conflicts 
    if (tables.conflicts.empty()) {
        cout << " No conflicts — this is a valid LR(1) grammar.\n";
    } else {
        cout << "\n Conflicts detected:\n";
        for (auto& c : tables.conflicts) cout << "      " << c << "\n";
    }

    // ── Print the constructed tables 
    printTables(tables);

    // ── Parse the test strings 
    cout << "    Parsing Test Strings\n";
    parse("id = id + num * id", tables);
    parse("id = + id * num",    tables);

    return 0;
}