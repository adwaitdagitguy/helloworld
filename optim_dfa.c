#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#define MAXPOS 50
#define MAXSTATES 50
// This program reads a regular expression from standard input
// and as an output for each node 
// it prints the nullable, firstpos, lastpos and followpos sets.
// It also prints the state transition table of the minimized DFA
// The DFA D recognizes L(r) where r is the input regular expression.
// Format:
/*
State_Name  Symbol 1 Symbol 2 
*/

int followpos[MAXPOS][MAXPOS];
int followpos_count[MAXPOS];

typedef struct Node
{
// typedef: a user defined data type, in this case it is a structure that represents a node in the syntax tree of the regular expression
    char symbol;
    int position;
    int nullable;

    int firstpos[100];
    int lastpos[100];

    int firstpos_count;
    int lastpos_count;
    struct Node* left;
    struct Node* right;
} Node;


//DFA Related Data structures
char pos_symbol[MAXPOS];   // pos_symbol[p] = 'a' or 'b' or '#'
int hash_pos;              // position of #
typedef struct 
{
    int pos[MAXPOS];
    int count;
} DFAState;
DFAState dfa_states[MAXSTATES]; // assuming we have max
int dfa_count = 0;
int dfa_trans[MAXSTATES][2]; // transition table as we have 2 symbols.
int dfa_marked[MAXSTATES];

int same_state(DFAState *a, DFAState *b)
{
    if(a->count != b->count)
    {
        return 0;
    }
    for(int i=0; i<a->count; i++)
    {
        int found=0;
        int pos_a = a->pos[i];
        for(int j=0; j<b->count; j++)
        {
            if(b->pos[j] == pos_a)
            {
                found = 1;
                break;
            }
        }
        if(!found)
        {
            return 0;
        }
    }
    return 1;

}

int find_state(DFAState *s)
{
    for(int i=0; i<dfa_count; i++)
    {
        if(same_state(&dfa_states[i], s))
        {
            return i;
        }
    }
    return -1;
}

void union_into_state(DFAState *dst, int *src, int src_count)
{   
    int found;
    for(int i=0; i<src_count; i++)
    {
        int p = src[i];
        found = 0;
        for(int j=0; j<dst->count; j++)
        {
            if(dst->pos[j] == p)
            {
                found = 1;
                break;
            }
        }
        if(!found)
        {
            dst->pos[dst->count++] =p;
        }
    }
}

void init_dfa(Node *root)
{
    // initialize the DFA with S0
    dfa_states[0].count = root->firstpos_count;
    for (int i = 0; i < root->firstpos_count; i++) {
        dfa_states[0].pos[i] = root->firstpos[i];
    }
   dfa_marked[0]=0;
   dfa_count = 1;
}

void build_dfa()
{
    for (int i = 0; i < MAXSTATES; i++)
        dfa_marked[i] = 0;

    int found;

    do {
        found = 0;

        for (int i = 0; i < dfa_count; i++) {
            if (dfa_marked[i] == 0) {
                found = 1;
                dfa_marked[i] = 1;

                for (int sym = 0; sym < 2; sym++) {   // 0=a, 1=b
                    char c = (sym == 0) ? 'a' : 'b';
                    DFAState U;
                    U.count = 0;

                    for (int k = 0; k < dfa_states[i].count; k++) {
                        int p = dfa_states[i].pos[k];
                        if (pos_symbol[p] == c) {
                            union_into_state(&U,
                                followpos[p],
                                followpos_count[p]);
                        }
                    }

                    if (U.count == 0) {
                        dfa_trans[i][sym] = -1;
                    } else {
                        int idx = find_state(&U);
                        if (idx == -1) {
                            dfa_states[dfa_count] = U;
                            dfa_marked[dfa_count] = 0;
                            dfa_trans[i][sym] = dfa_count;
                            dfa_count++;
                        } else {
                            dfa_trans[i][sym] = idx;
                        }
                    }
                }
                break;
            }
        }
    } while (found);
}

void print_dfa()
{
    printf("\nDFA States and their positions:\n");
    printf("========================================\n");
    for (int i = 0; i < dfa_count; i++) {
        printf("S%d = {", i);
        for (int j = 0; j < dfa_states[i].count; j++) {
            printf("%d", dfa_states[i].pos[j]);
            if (j < dfa_states[i].count - 1)
                printf(", ");
        }
        printf("}\n");
    }
    
    printf("\nDFA Transition Table:\n");
    printf("====================================================\n");
    printf("State\tPositions\ta\tb\tAccepting\n");
    printf("====================================================\n");

    for (int i = 0; i < dfa_count; i++) {
        int accepting = 0;

        for (int j = 0; j < dfa_states[i].count; j++) {
            if (dfa_states[i].pos[j] == hash_pos) {
                accepting = 1;
                break;
            }
        }

        printf("S%d\t{", i);
        for (int j = 0; j < dfa_states[i].count; j++) {
            printf("%d", dfa_states[i].pos[j]);
            if (j < dfa_states[i].count - 1)
                printf(",");
        }
        printf("}");
        
        // Add extra tab for sets with 3 or fewer elements to align properly
        if (dfa_states[i].count <= 3) {
            printf("\t\t");
        } else {
            printf("\t");
        }

        if (dfa_trans[i][0] == -1) printf("-\t");
        else printf("S%d\t", dfa_trans[i][0]);

        if (dfa_trans[i][1] == -1) printf("-\t");
        else printf("S%d\t", dfa_trans[i][1]);

        printf("%s\n", accepting ? "YES" : "NO");
    }
}




typedef struct CharStack
{
    int top;
    char stack[100];
} CharStack;

void pushChar(char c, CharStack* stack)
{
    printf("Pushing char %c\n", c);
    stack->stack[++stack->top] = c;   
}
char popChar(CharStack* stack)
{
    return stack->stack[stack->top--];
}
char peekChar(CharStack* stack)
{
    // return the top element of the stack without removing it
    return stack->stack[stack->top];
}



typedef struct NodeStack
{
    int top;
    Node *stack[100];
} NodeStack;

Node* create_node(char c)
{
    Node* node = (Node*) malloc (sizeof(Node));
    node->position=-1;
    node->left= node->right=NULL;
    node->nullable=0;
    node->symbol=c;
    node->firstpos_count=node->lastpos_count=0;
    return node;
}
void push(Node* node, NodeStack* stack) // pointers to node and the nodestack
{
    printf("Pushing node with symbol %c and pos %d\n", node->symbol, node->position);
    stack->stack[++stack->top] = node;   
}

Node* pop(NodeStack* stack)
{
    return stack->stack[stack->top--];
}

void peek(NodeStack* stack)
{
    // return the top element of the stack without removing it
    printf("Peeking node with symbol %c and pos %d\n", stack->stack[stack->top]->symbol, stack->stack[stack->top]->position);
}

int precedence(char c)
{
    if(c=='*')
    {
        return 3;
    }
    else if(c=='.')
    {
        return 2;
    }
    else if(c=='|')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

char* infixToPostfix(char * str) // standard Shunting Yard algorithm
{
    CharStack opstack;
    opstack.top=-1;
    char* postfix = (char*) malloc (sizeof(char)*200);
    int j=0;
    for(int i=0; str[i]!='\0'; i++)
    {
        if(isalpha(str[i]) || str[i]=='#')
        {
            postfix[j++] = str[i];
        }
        else if(str[i]=='*')
        {
            postfix[j++]= str[i];
        }
        else if(str[i]=='(')
        {
            pushChar(str[i], &opstack);
        }
        else if(str[i]==')')
        {
            while(opstack.top >= 0 && peekChar(&opstack)!='(')  // FIX: Added stack empty check
            {
                postfix[j++] = popChar(&opstack);
            }   
            if(opstack.top >= 0)  // FIX: Added check before popping
                popChar(&opstack);
        }
        else
        {
            while(opstack.top >= 0 && precedence(peekChar(&opstack)) >= precedence(str[i]))  // FIX: Added stack empty check
            {
                postfix[j++]=popChar(&opstack);
            }
            pushChar(str[i], &opstack);
        }
    }
    // pop anything that remains
    while(opstack.top >= 0)  // FIX: Added proper condition
    {
        postfix[j++]=popChar(&opstack);
    }
    postfix[j] = '\0';
    return postfix;
}

char* insertConcatInString(char * str)
{
    // inserting the concatenation operation in the string 
    char* newStr= (char*)malloc(sizeof(char)*200);
    int j=0;
    for(int i=0; str[i]!='\0'; i++)
    {
        if((str[i]=='a'||str[i]=='b'||str[i]=='#') && str[i+1]!='\0')
        {
            newStr[j++] = str[i];
            if(str[i+1]=='a'||str[i+1]=='b'||str[i+1]=='#' || str[i+1]=='(') // adjust for beginning bracket
            {
                newStr[j++]='.';
            }
        }
        else if(str[i]==')' && str[i+1]!='\0')
        {
            newStr[j++] = str[i];
            if(str[i+1]=='a'||str[i+1]=='b'||str[i+1]=='(' || str[i+1]=='#')
            {
                newStr[j++]='.';
            }
        }
        else if(str[i]=='*' && str[i+1]!='\0') // adjusting for kleene star
        {
            newStr[j++] = str[i];
            if(str[i+1]=='a'||str[i+1]=='b'||str[i+1]=='(' || str[i+1]=='#')
            {
                newStr[j++]='.';    
            }
        }
        else // includes operators and beginning bracket
        {
            newStr[j++] = str[i];
        }
    }
    newStr[j] = '\0';
    // it is not possible in C to return local arrays, we can only return references
    return  newStr;
}

Node* construct_syntax_tree(char * pstr)
{
    NodeStack nstack;
    nstack.top=-1;
    int position_number=1;
    for(int i=0; pstr[i]!='\0'; i++)
    {
        if(pstr[i]=='a'||pstr[i]=='b'||pstr[i]=='#')
        {
            
            Node* node = create_node(pstr[i]);
            node->right = node->left= NULL;
            if(pstr[i]=='#')
            {
                hash_pos=position_number;   
            }
            node->position=position_number;
            pos_symbol[position_number]=pstr[i];
            position_number++;
            push(node, &nstack);
        }
        else if(pstr[i]=='*') // it is a left associative operator
        {
            Node* node = create_node(pstr[i]);
            node->left=pop(&nstack);
            node->right=NULL;
            push(node, &nstack);
        }
        else if(pstr[i]=='|' || pstr[i]=='.') // bidirectional operator
        {
            Node* node = create_node(pstr[i]);
            node->right = pop(&nstack);
            node->left = pop(&nstack);
            push(node, &nstack);
        }
        else
        {
            printf("Invalid character encounterred %c... breaking",pstr[i]);
            exit(1);
        }
    }
    return nstack.stack[nstack.top];
}

void compute_nullable(Node *n)
{

    if(n==NULL)
    {
        return;
    }
    compute_nullable(n->left);
    compute_nullable(n->right);
  
    if(n->symbol=='a' || n->symbol=='b' || n->symbol=='#')
    {
        n->nullable=0;
    }
    else if(n->symbol=='|')
    {
        n->nullable= n->left->nullable || n->right->nullable;
    }
    else if(n->symbol=='.')
    {
        n->nullable= n->left->nullable && n->right->nullable;
    }
    else if(n->symbol=='*')
    {
        n->nullable= 1;
    }
    else
    {
        printf("Invalid character encounterred\n");
    }
}

void copy_set(int *dest, int *dest_cnt, int * src, int src_cnt)
{
    *dest_cnt=0;
    for(int i=0; i<src_cnt; i++)
    {
        dest[(*dest_cnt)++] = src[i];
    }

}

void union_set(int * dest, int *dest_cnt, int*a, int a_cnt, int*b, int b_cnt)
{
    *dest_cnt=0;
    for(int i=0; i<a_cnt; i++)
    {
        dest[(*dest_cnt)++] = a[i];  // FIX: Changed from dest[*(dest_cnt)++] to dest[(*dest_cnt)++]
    }

    for(int i=0; i<b_cnt; i++)
    {
        int b_elem = b[i];
        int exists=0;
        for(int j=0; j<*dest_cnt; j++)
        {
            if(dest[j]==b_elem)
            {
                exists=1;
                break;
            }
        }
        if(!exists)
        {
            dest[(*dest_cnt)++] = b_elem;
        }
    }
}
void compute_firstpos(Node *n)
{
    if(n==NULL)
    {
        return;
    }
    compute_firstpos(n->left);
    compute_firstpos(n->right);
    n->firstpos_count=0;
    if(n->symbol=='a'|| n->symbol=='b'||n->symbol=='#')
    {
        
        n->firstpos[n->firstpos_count++]= n->position;
    }
    else if(n->symbol=='|')
    {
        // perform set UNION(between left and right)
        union_set(n->firstpos, &(n->firstpos_count) ,n->left->firstpos, n->left->firstpos_count ,n->right->firstpos, n->right->firstpos_count);
    }
    else if(n->symbol=='.')
    {
        if(n->left->nullable)
        {
            union_set(n->firstpos, &(n->firstpos_count) ,n->left->firstpos, n->left->firstpos_count ,n->right->firstpos, n->right->firstpos_count);
        }
        else
        {
            copy_set(n->firstpos, &(n->firstpos_count), n->left->firstpos, n->left->firstpos_count);
        }
    }
    else if(n->symbol=='*') // kleene star
    {
        copy_set(n->firstpos, &(n->firstpos_count), n->left->firstpos, n->left->firstpos_count);
    }
    else
    {
        printf("ERROR\n");
    }
}

void compute_lastpos(Node *n)
{
    if(n==NULL)
    {
        return;
    }
    compute_lastpos(n->left);
    compute_lastpos(n->right);
    n->lastpos_count=0;
    if(n->symbol=='a'|| n->symbol=='b'||n->symbol=='#')
    {
        
        n->lastpos[n->lastpos_count++]= n->position;
    }
    else if(n->symbol=='|')
    {
        // perform set UNION(between left and right)
        union_set(n->lastpos, &(n->lastpos_count) ,n->left->lastpos, n->left->lastpos_count ,n->right->lastpos, n->right->lastpos_count);
    }
    else if(n->symbol=='.')
    {
        if(n->right->nullable)
        {
            union_set(n->lastpos, &(n->lastpos_count) ,n->left->lastpos, n->left->lastpos_count ,n->right->lastpos, n->right->lastpos_count);
        }
        else
        {
            copy_set(n->lastpos, &(n->lastpos_count), n->right->lastpos, n->right->lastpos_count);
        }
    }
    else if(n->symbol=='*') // kleene star
    {
        copy_set(n->lastpos, &(n->lastpos_count), n->left->lastpos, n->left->lastpos_count);
    }
    else
    {
        printf("ERROR\n");
    }
}

void compute_followpos(Node *n)
{
    if(n==NULL)
    {
        return;
    }
    compute_followpos(n->left);
    compute_followpos(n->right);

    if(n->symbol=='.')
    {
        /*if n.symbol == '.':
            for each i in n.left.lastpos:
                followpos[i] += n.right.firstpos

        */
        for(int i=0; i<n->left->lastpos_count; i++)
        {
            // copy the full n.rightpos onto followpos[i]
            int p = n->left->lastpos[i];
        union_set(
        followpos[p],
        &followpos_count[p],
        followpos[p],
        followpos_count[p],
        n->right->firstpos,
        n->right->firstpos_count);
        }
    }
    else if(n->symbol=='*')
    {
        /*if n.symbol == '*':
        for each i in n.left.lastpos:
            followpos[i] += n.left.firstpos
        */
        for (int i = 0; i < n->left->lastpos_count; i++) {
        int p = n->left->lastpos[i];
        union_set(
        followpos[p],
        &followpos_count[p],
        followpos[p],
        followpos_count[p],
        n->left->firstpos,
        n->left->firstpos_count);
        }
    }
}

int node_counter = 0;

void print_node_info(Node *n)
{
    if(n == NULL)
    {
        return;
    }
    
    print_node_info(n->left);
    print_node_info(n->right);
    
    node_counter++;
    
    printf("\nNode %d:\n", node_counter);
    printf("  Symbol: %c\n", n->symbol);
    
    if(n->position != -1)
    {
        printf("  Position: %d\n", n->position);
    }
    else
    {
        printf("  Position: (internal node)\n");
    }
    
    printf("  Nullable: %s\n", n->nullable ? "true" : "false");
    
    printf("  Firstpos: {");
    for(int i=0; i<n->firstpos_count; i++)
    {
        printf("%d", n->firstpos[i]);
        if(i < n->firstpos_count - 1)
            printf(", ");
    }
    printf("}\n");
    
    printf("  Lastpos: {");
    for(int i=0; i<n->lastpos_count; i++)
    {
        printf("%d", n->lastpos[i]);
        if(i < n->lastpos_count - 1)
            printf(", ");
    }
    printf("}\n");
}

void print_followpos()
{
    printf("\n========================================\n");
    printf("Followpos for each position:\n");
    printf("========================================\n");
    
    for(int i=1; i<MAXPOS; i++)
    {
        if(followpos_count[i] > 0)
        {
            printf("Position %d (symbol '%c'): {", i, pos_symbol[i]);
            for(int j=0; j<followpos_count[i]; j++)
            {
                printf("%d", followpos[i][j]);
                if(j < followpos_count[i] - 1)
                    printf(", ");
            }
            printf("}\n");
        }
    }
}


int main()
{
    char str[100];
    for(int i=0; i<MAXPOS; i++)
    {
        followpos_count[i] = 0;
    }
    printf("Enter the regular expression: ");
    scanf("%s", str);
    size_t len = strlen(str);
    // Augmenting the regular expression r with a special end symbol #
    str[len] = '#';
    str[len+1] = '\0';
    // insert the concatenation operator '.' in the regular expression where needed
    char* newStr= insertConcatInString(str);
    printf("the regular expression after inserting the concatenation operator is: %s\n", newStr);
    // Convert the regex to an equivalent postfix operation
    char* postFixStr = infixToPostfix(newStr);
    printf("the regular expression in postfix form is: %s\n", postFixStr);
    // Construct a syntax Tree from r#, asusme that expression has only 2 symbols a and b
    Node* root = construct_syntax_tree(postFixStr);
    printf("the syntax tree is successfully constructed\n");
    // Traverse the syntax tree to compute nullable, firstpos, lastpos and followpos sets for each node 
    compute_nullable(root);
    printf("The nullable is computed\n");

    compute_firstpos(root);
    printf("The firstpos is successfully computed\n");

    compute_lastpos(root);
    printf("The lastpos is successfully computed\n");

    compute_followpos(root);
    printf("The followpos is successfully completed\n");

    // Print information for all nodes
    printf("\n========================================\n");
    printf("NODE INFORMATION\n");
    printf("========================================\n");
    node_counter = 0;
    print_node_info(root);
    
    // Print followpos for each position
    print_followpos();

    // Construct D states the states of DFA and DTrans the transition table of DFA and print it
    printf("\n========================================\n");
    printf("DFA TRANSITION TABLE\n");
    printf("========================================\n");
    init_dfa(root);
    build_dfa();
    print_dfa();
    
    // FIX: Added memory cleanup
    free(newStr);
    free(postFixStr);
    
    return 0;
}