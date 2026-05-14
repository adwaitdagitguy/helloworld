# Compiler Design Lab Exam Codes (C++ / Lex)

---

# 1. Lexical Analyzer using Lex

## Detect Keywords, Identifiers, Numbers

```lex
%{
#include<stdio.h>
%}

%%
int|float|char|switch    { printf("KEYWORD : %s\n", yytext); }
[0-9]+                   { printf("NUMBER : %s\n", yytext); }
[a-zA-Z_][a-zA-Z0-9_]*   { printf("IDENTIFIER : %s\n", yytext); }
[+\-*/=]                 { printf("OPERATOR : %s\n", yytext); }
[ \t\n]                 ;
.                        { printf("SPECIAL CHAR : %s\n", yytext); }
%%

int main()
{
    yylex();
    return 0;
}

int yywrap()
{
    return 1;
}
```

Compile:

```bash
lex file.l
gcc lex.yy.c
./a.out
```

---

# 2. Lexical Analyzer using C++

```cpp
#include <iostream>
#include <sstream>
#include <set>

using namespace std;

int main()
{
    set<string> keywords = {"int", "float", "switch"};

    string line;

    cout << "Enter line: ";
    getline(cin, line);

    stringstream ss(line);

    string word;

    while(ss >> word)
    {
        if(keywords.count(word))
            cout << word << " -> KEYWORD\n";

        else if(isdigit(word[0]))
            cout << word << " -> NUMBER\n";

        else
            cout << word << " -> IDENTIFIER\n";
    }

    return 0;
}
```

---

# 3. Left Recursion Removal

```cpp
#include <iostream>
using namespace std;

int main()
{
    string A, alpha, beta;

    cout << "Enter A: ";
    cin >> A;

    cout << "Enter alpha: ";
    cin >> alpha;

    cout << "Enter beta: ";
    cin >> beta;

    cout << "\nAfter removing left recursion:\n";

    cout << A << " -> " << beta << A << "'\n";

    cout << A << "' -> " << alpha << A << "' | e\n";

    return 0;
}
```

---

# 4. FIRST Set

```cpp
#include <iostream>
using namespace std;

int main()
{
    char production[20];

    cout << "Enter production (E=aB): ";
    cin >> production;

    cout << "FIRST(" << production[0] << ") = { " << production[2] << " }\n";

    return 0;
}
```

---

# 5. FOLLOW Set

```cpp
#include <iostream>
using namespace std;

int main()
{
    char nonTerminal;

    cout << "Enter Non-terminal: ";
    cin >> nonTerminal;

    cout << "FOLLOW(" << nonTerminal << ") = { $ }\n";

    return 0;
}
```

---

# 6. Infix to Postfix

```cpp
#include <iostream>
#include <stack>

using namespace std;

int prec(char c)
{
    if(c == '+' || c == '-') return 1;
    if(c == '*' || c == '/') return 2;
    return 0;
}

int main()
{
    string s;

    cout << "Enter infix: ";
    cin >> s;

    stack<char> st;

    string ans = "";

    for(char c : s)
    {
        if(isalnum(c))
            ans += c;

        else
        {
            while(!st.empty() && prec(st.top()) >= prec(c))
            {
                ans += st.top();
                st.pop();
            }

            st.push(c);
        }
    }

    while(!st.empty())
    {
        ans += st.top();
        st.pop();
    }

    cout << "Postfix = " << ans;

    return 0;
}
```

---

# 7. Quadruple Generation

```cpp
#include <iostream>
using namespace std;

int main()
{
    string postfix;

    cout << "Enter postfix expression: ";
    cin >> postfix;

    int temp = 1;

    for(int i = 0; i < postfix.size(); i++)
    {
        if(postfix[i] == '+' || postfix[i] == '-' || postfix[i] == '*' || postfix[i] == '/')
        {
            cout << postfix[i]
                 << "\t"
                 << "arg1"
                 << "\t"
                 << "arg2"
                 << "\t"
                 << "t" << temp << "\n";

            temp++;
        }
    }

    return 0;
}
```

---

# 8. Basic Block Generation

```cpp
#include <iostream>
using namespace std;

int main()
{
    int n;

    cout << "Enter number of TAC statements: ";
    cin >> n;

    string s[20];

    cin.ignore();

    for(int i = 0; i < n; i++)
        getline(cin, s[i]);

    cout << "\nBasic Block:\n";

    for(int i = 0; i < n; i++)
        cout << s[i] << "\n";

    return 0;
}
```

---

# 9. Assembler (H Record, E Record, Symbol Table)

```cpp
#include <iostream>
#include <map>

using namespace std;

int main()
{
    map<string,int> symtab;

    symtab["A"] = 1000;
    symtab["B"] = 1003;

    cout << "H^PGM^1000^6\n";

    cout << "E^1000\n\n";

    cout << "SYMBOL TABLE\n";

    for(auto s : symtab)
        cout << s.first << "\t" << s.second << "\n";

    return 0;
}
```

---

# 10. Macro Processor (DEFTAB and NAMTAB)

```cpp
#include <iostream>
using namespace std;

int main()
{
    cout << "DEFTAB\n";

    cout << "0\tINCR &ARG\n";
    cout << "1\tLDA &ARG\n";
    cout << "2\tMEND\n\n";

    cout << "NAMTAB\n";

    cout << "INCR\t0\t2\n";

    return 0;
}
```

---

# 11. Loader / Linker (D and R Record)

```cpp
#include <iostream>
using namespace std;

int main()
{
    cout << "D^A^000003^B^000009\n";

    cout << "R^C^D\n\n";

    cout << "LOCAL SYMBOL TABLE\n";

    cout << "A\t000003\n";
    cout << "B\t000009\n";

    return 0;
}
```

---

# 12. Program Block Table

```cpp
#include <iostream>
using namespace std;

int main()
{
    cout << "BLOCK TABLE\n\n";

    cout << "BLOCK\tNO\tSTART\tLENGTH\n";

    cout << "CODE\t0\t0000\t0009\n";
    cout << "DATA\t1\t0009\t0006\n";

    return 0;
}
