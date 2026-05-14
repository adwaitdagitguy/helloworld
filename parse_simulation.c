#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NT 20
#define MAX_PROD 20
#define MAX_STR 256
#define MAX_SET 30
#define MAX_TERM 30
#define MAX_STACK 200

typedef struct {
    char name[MAX_STR];
    char productions[MAX_PROD][MAX_STR];
    int prod_count;
} NonTerminal;

typedef struct {
    char elems[MAX_SET][MAX_STR];
    int count;
} Set;

typedef struct {
    NonTerminal nts[MAX_NT];
    int count;
} Grammar;

int parse_table[MAX_NT][MAX_TERM];
char terminals[MAX_TERM][MAX_STR];
int term_count = 0;

Set FIRST[MAX_NT];
Set FOLLOW[MAX_NT];

// ----------------------------------------
// Utility
// ----------------------------------------
int set_contains(Set *s, const char *val) {
    for (int i = 0; i < s->count; i++)
        if (strcmp(s->elems[i], val) == 0) return 1;
    return 0;
}

int set_add(Set *s, const char *val) {
    if (!set_contains(s, val)) {
        strcpy(s->elems[s->count++], val);
        return 1;
    }
    return 0;
}

int set_union_exclude(Set *dst, Set *src, const char *exclude) {
    int changed = 0;
    for (int i = 0; i < src->count; i++) {
        if (exclude && strcmp(src->elems[i], exclude) == 0) continue;
        changed |= set_add(dst, src->elems[i]);
    }
    return changed;
}

int find_nt(Grammar *g, const char *name) {
    for (int i = 0; i < g->count; i++)
        if (strcmp(g->nts[i].name, name) == 0) return i;
    return -1;
}

int is_nt(Grammar *g, const char *name) {
    return find_nt(g, name) >= 0;
}

/* Parse next symbol from production string.
   - Lowercase words (id, num) => multi-char terminal
   - Uppercase letter optionally followed by ' => non-terminal (E, E')
   - Anything else => single-char terminal (+, *, =, etc.) */
void next_symbol(const char *prod, int *pos, char *sym) {
    int p = *pos;
    if (islower((unsigned char)prod[p])) {
        int start = p;
        while (prod[p] && (islower((unsigned char)prod[p]) || isdigit((unsigned char)prod[p])))
            p++;
        int len = p - start;
        strncpy(sym, prod + start, len);
        sym[len] = '\0';
        *pos = p;
        return;
    }
    sym[0] = prod[p];
    sym[1] = '\0';
    p++;
    if (isupper((unsigned char)sym[0]) && prod[p] == '\'') {
        sym[1] = '\'';
        sym[2] = '\0';
        p++;
    }
    *pos = p;
}

// ----------------------------------------
// Remove Direct Left Recursion
// ----------------------------------------
void remove_left_recursion(Grammar *original, Grammar *result) {
    result->count = 0;

    for (int i = 0; i < original->count; i++) {
        NonTerminal *A = &original->nts[i];

        char alpha[MAX_PROD][MAX_STR]; int alpha_count = 0;
        char beta[MAX_PROD][MAX_STR];  int beta_count = 0;

        for (int j = 0; j < A->prod_count; j++) {
            char *prod = A->productions[j];
            /* Check if production starts with the NT name (direct left recursion).
               Must match full name, not just prefix (E' does not recurse on E). */
            int alen = strlen(A->name);
            if (strncmp(prod, A->name, alen) == 0 && !isupper((unsigned char)prod[alen]) && prod[alen] != '\'') {
                strcpy(alpha[alpha_count++], prod + alen);
            } else {
                strcpy(beta[beta_count++], prod);
            }
        }

        if (alpha_count == 0) {
            NonTerminal *nt = &result->nts[result->count++];
            strcpy(nt->name, A->name);
            nt->prod_count = A->prod_count;
            for (int j = 0; j < A->prod_count; j++)
                strcpy(nt->productions[j], A->productions[j]);
        } else {
            char A_dash[MAX_STR];
            snprintf(A_dash, MAX_STR - 1, "%s'", A->name);

            NonTerminal *nt = &result->nts[result->count++];
            strcpy(nt->name, A->name);
            nt->prod_count = 0;
            for (int j = 0; j < beta_count; j++)
                snprintf(nt->productions[nt->prod_count++], MAX_STR - 1, "%s%s", beta[j], A_dash);

            NonTerminal *nt_dash = &result->nts[result->count++];
            strcpy(nt_dash->name, A_dash);
            nt_dash->prod_count = 0;
            for (int j = 0; j < alpha_count; j++)
                snprintf(nt_dash->productions[nt_dash->prod_count++], MAX_STR - 1, "%s%s", alpha[j], A_dash);
            strcpy(nt_dash->productions[nt_dash->prod_count++], "e");
        }
    }
}

// ----------------------------------------
// Compute FIRST
// ----------------------------------------
void compute_first(Grammar *g) {
    for (int i = 0; i < g->count; i++) FIRST[i].count = 0;

    int changed = 1;
    while (changed) {
        changed = 0;
        for (int ai = 0; ai < g->count; ai++) {
            NonTerminal *A = &g->nts[ai];
            for (int pi = 0; pi < A->prod_count; pi++) {
                char *prod = A->productions[pi];
                if (strcmp(prod, "e") == 0) {
                    changed |= set_add(&FIRST[ai], "e");
                    continue;
                }
                int pos = 0, plen = strlen(prod);
                while (pos < plen) {
                    char sym[MAX_STR];
                    next_symbol(prod, &pos, sym);
                    int si = find_nt(g, sym);
                    if (si < 0) {
                        changed |= set_add(&FIRST[ai], sym);
                        break;
                    } else {
                        changed |= set_union_exclude(&FIRST[ai], &FIRST[si], "e");
                        if (!set_contains(&FIRST[si], "e")) break;
                        if (pos >= plen) changed |= set_add(&FIRST[ai], "e");
                    }
                }
            }
        }
    }
}

// ----------------------------------------
// Compute FOLLOW
// ----------------------------------------
void compute_follow(Grammar *g, int start_idx) {
    for (int i = 0; i < g->count; i++) FOLLOW[i].count = 0;
    set_add(&FOLLOW[start_idx], "$");

    int changed = 1;
    while (changed) {
        changed = 0;
        for (int ai = 0; ai < g->count; ai++) {
            NonTerminal *A = &g->nts[ai];
            for (int pi = 0; pi < A->prod_count; pi++) {
                char *prod = A->productions[pi];
                int plen = strlen(prod), pos = 0;
                while (pos < plen) {
                    char B[MAX_STR];
                    next_symbol(prod, &pos, B);
                    int bi = find_nt(g, B);
                    if (bi < 0) continue;

                    Set first_beta; first_beta.count = 0;
                    int bpos = pos, all_nullable = 1;
                    while (bpos < plen) {
                        char sym[MAX_STR];
                        next_symbol(prod, &bpos, sym);
                        int si = find_nt(g, sym);
                        if (si < 0) {
                            set_add(&first_beta, sym);
                            all_nullable = 0;
                            break;
                        } else {
                            set_union_exclude(&first_beta, &FIRST[si], "e");
                            if (!set_contains(&FIRST[si], "e")) { all_nullable = 0; break; }
                        }
                    }
                    if (bpos >= plen && all_nullable) set_add(&first_beta, "e");

                    changed |= set_union_exclude(&FOLLOW[bi], &first_beta, "e");
                    if (set_contains(&first_beta, "e"))
                        changed |= set_union_exclude(&FOLLOW[bi], &FOLLOW[ai], NULL);
                }
            }
        }
    }
}

// ----------------------------------------
// Terminals
// ----------------------------------------
int find_terminal(const char *t) {
    for (int i = 0; i < term_count; i++)
        if (strcmp(terminals[i], t) == 0) return i;
    return -1;
}

int add_terminal(const char *t) {
    int idx = find_terminal(t);
    if (idx < 0) {
        strcpy(terminals[term_count++], t);
        return term_count - 1;
    }
    return idx;
}

void collect_terminals(Grammar *g) {
    term_count = 0;
    for (int i = 0; i < g->count; i++) {
        NonTerminal *nt = &g->nts[i];
        for (int j = 0; j < nt->prod_count; j++) {
            char *prod = nt->productions[j];
            if (strcmp(prod, "e") == 0) continue;
            int pos = 0, plen = strlen(prod);
            while (pos < plen) {
                char sym[MAX_STR];
                next_symbol(prod, &pos, sym);
                if (!is_nt(g, sym)) add_terminal(sym);
            }
        }
    }
    add_terminal("$");
    /* Sort */
    for (int i = 0; i < term_count - 1; i++)
        for (int j = i + 1; j < term_count; j++)
            if (strcmp(terminals[i], terminals[j]) > 0) {
                char tmp[MAX_STR];
                strcpy(tmp, terminals[i]);
                strcpy(terminals[i], terminals[j]);
                strcpy(terminals[j], tmp);
            }
}

// ----------------------------------------
// Build LL(1) Parse Table
// ----------------------------------------
int build_parse_table(Grammar *g) {
    for (int i = 0; i < g->count; i++)
        for (int j = 0; j < term_count; j++)
            parse_table[i][j] = -1;

    int is_ll1 = 1;
    for (int ai = 0; ai < g->count; ai++) {
        NonTerminal *A = &g->nts[ai];
        for (int pi = 0; pi < A->prod_count; pi++) {
            char *prod = A->productions[pi];

            Set first_prod; first_prod.count = 0;
            if (strcmp(prod, "e") == 0) {
                set_add(&first_prod, "e");
            } else {
                int pos = 0, plen = strlen(prod), all_nullable = 1;
                while (pos < plen && all_nullable) {
                    char sym[MAX_STR];
                    next_symbol(prod, &pos, sym);
                    int si = find_nt(g, sym);
                    if (si < 0) {
                        set_add(&first_prod, sym);
                        all_nullable = 0;
                    } else {
                        set_union_exclude(&first_prod, &FIRST[si], "e");
                        if (!set_contains(&FIRST[si], "e")) all_nullable = 0;
                    }
                }
                if (all_nullable) set_add(&first_prod, "e");
            }

            for (int fi = 0; fi < first_prod.count; fi++) {
                char *sym = first_prod.elems[fi];
                if (strcmp(sym, "e") == 0) {
                    for (int fli = 0; fli < FOLLOW[ai].count; fli++) {
                        int ti = find_terminal(FOLLOW[ai].elems[fli]);
                        if (ti < 0) continue;
                        if (parse_table[ai][ti] != -1 && parse_table[ai][ti] != pi) {
                            printf("[CONFLICT] M[%s, %s]\n", A->name, terminals[ti]);
                            is_ll1 = 0;
                        }
                        parse_table[ai][ti] = pi;
                    }
                } else {
                    int ti = find_terminal(sym);
                    if (ti < 0) continue;
                    if (parse_table[ai][ti] != -1 && parse_table[ai][ti] != pi) {
                        printf("[CONFLICT] M[%s, %s]\n", A->name, terminals[ti]);
                        is_ll1 = 0;
                    }
                    parse_table[ai][ti] = pi;
                }
            }
        }
    }
    return is_ll1;
}

// ----------------------------------------
// Print helpers
// ----------------------------------------
void sort_set(char sorted[][MAX_STR], Set *s) {
    int n = s->count;
    for (int i = 0; i < n; i++) strcpy(sorted[i], s->elems[i]);
    for (int i = 0; i < n-1; i++)
        for (int j = i+1; j < n; j++)
            if (strcmp(sorted[i], sorted[j]) > 0) {
                char tmp[MAX_STR]; strcpy(tmp, sorted[i]);
                strcpy(sorted[i], sorted[j]); strcpy(sorted[j], tmp);
            }
}

void set_to_str(Set *s, char *out) {
    char sorted[MAX_SET][MAX_STR];
    sort_set(sorted, s);
    strcpy(out, "{ ");
    for (int i = 0; i < s->count; i++) {
        if (i) strcat(out, ", ");
        /* Display 'e' as epsilon symbol */
        strcat(out, strcmp(sorted[i], "e") == 0 ? "epsilon" : sorted[i]);
    }
    strcat(out, " }");
}

void print_first_follow(Grammar *g) {
    printf("\n--------------------------------------------------\n");
    printf("%-10s %-26s %s\n", "NT", "FIRST SET", "FOLLOW SET");
    printf("--------------------------------------------------\n");
    for (int i = 0; i < g->count; i++) {
        char b1[512], b2[512];
        set_to_str(&FIRST[i], b1);
        set_to_str(&FOLLOW[i], b2);
        printf("%-10s %-26s %s\n", g->nts[i].name, b1, b2);
    }
    printf("--------------------------------------------------\n");
}

void print_parse_table(Grammar *g) {
    int col = 16;
    printf("\nLL(1) Parse Table:\n");
    printf("%-10s", "");
    for (int j = 0; j < term_count; j++)
        printf("%-*s", col, terminals[j]);
    printf("\n");
    int total = 10 + col * term_count;
    for (int i = 0; i < total; i++) printf("-");
    printf("\n");
    for (int i = 0; i < g->count; i++) {
        printf("%-10s", g->nts[i].name);
        for (int j = 0; j < term_count; j++) {
            int pi = parse_table[i][j];
            if (pi == -1) {
                printf("%-*s", col, "");
            } else {
                char cell[MAX_STR];
                const char *p = g->nts[i].productions[pi];
                snprintf(cell, sizeof(cell), "%s->%s", g->nts[i].name,
                         strcmp(p,"e")==0 ? "epsilon" : p);
                if ((int)strlen(cell) >= col) { cell[col-1] = '\0'; }
                printf("%-*s", col, cell);
            }
        }
        printf("\n");
    }
    for (int i = 0; i < total; i++) printf("-");
    printf("\n");
}

// ----------------------------------------
// Tokenize input string
// ----------------------------------------
int tokenize(const char *input, char tokens[][MAX_STR]) {
    int count = 0;
    int i = 0, len = strlen(input);
    while (i < len) {
        if (isspace((unsigned char)input[i])) { i++; continue; }
        if (isalpha((unsigned char)input[i])) {
            int start = i;
            while (i < len && isalnum((unsigned char)input[i])) i++;
            strncpy(tokens[count], input + start, i - start);
            tokens[count][i - start] = '\0';
        } else {
            tokens[count][0] = input[i];
            tokens[count][1] = '\0';
            i++;
        }
        count++;
    }
    strcpy(tokens[count++], "$");
    return count;
}

// ----------------------------------------
// LL(1) Predictive Parse
// ----------------------------------------
void parse_string(Grammar *g, const char *input_str) {
    char tokens[200][MAX_STR];
    int tok_count = tokenize(input_str, tokens);

    printf("\n==================================================\n");
    printf("Parsing: \"%s\"\n", input_str);
    printf("==================================================\n");
    printf("%-38s %-22s %s\n", "Stack (top -> bottom)", "Input", "Action");
    printf("--------------------------------------------------\n");

    char stack[MAX_STACK][MAX_STR];
    int top = 0;
    strcpy(stack[top++], "$");
    strcpy(stack[top++], g->nts[0].name);

    int ip = 0;

    while (1) {
        /* Build stack display string */
        char stk_str[512] = "";
        for (int i = top - 1; i >= 0; i--) {
            strcat(stk_str, stack[i]);
            if (i > 0) strcat(stk_str, " ");
        }
        /* Build remaining input string */
        char inp_str[256] = "";
        for (int i = ip; i < tok_count; i++) {
            if (i > ip) strcat(inp_str, " ");
            strcat(inp_str, tokens[i]);
        }
        char action[256] = "";

        if (top == 0) break;
        char *X = stack[top - 1];
        char *a = tokens[ip];

        if (strcmp(X, "$") == 0 && strcmp(a, "$") == 0) {
            snprintf(action, sizeof(action), "Accept");
            printf("%-38s %-22s %s\n", stk_str, inp_str, action);
            printf("\n  ==> String ACCEPTED\n");
            return;
        } else if (strcmp(X, "$") == 0) {
            snprintf(action, sizeof(action), "ERROR: extra input after end");
            printf("%-38s %-22s %s\n", stk_str, inp_str, action);
            printf("\n  ==> String REJECTED\n");
            return;
        } else if (!is_nt(g, X)) {
            /* Terminal on stack */
            if (strcmp(X, a) == 0) {
                snprintf(action, sizeof(action), "Match '%s'", a);
                printf("%-38s %-22s %s\n", stk_str, inp_str, action);
                top--; ip++;
            } else {
                snprintf(action, sizeof(action), "ERROR: expected '%s', got '%s'", X, a);
                printf("%-38s %-22s %s\n", stk_str, inp_str, action);
                printf("\n  ==> String REJECTED\n");
                return;
            }
        } else {
            /* Non-terminal on stack: consult parse table */
            int xi = find_nt(g, X);
            int ti = find_terminal(a);
            if (ti < 0) {
                snprintf(action, sizeof(action), "ERROR: unknown token '%s'", a);
                printf("%-38s %-22s %s\n", stk_str, inp_str, action);
                printf("\n  ==> String REJECTED\n");
                return;
            }
            int pi = parse_table[xi][ti];
            if (pi == -1) {
                snprintf(action, sizeof(action), "ERROR: no entry M[%s,%s]", X, a);
                printf("%-38s %-22s %s\n", stk_str, inp_str, action);
                printf("\n  ==> String REJECTED\n");
                return;
            }
            char *prod = g->nts[xi].productions[pi];
            snprintf(action, sizeof(action), "%s -> %s", X,
                     strcmp(prod,"e")==0 ? "epsilon" : prod);
            printf("%-38s %-22s %s\n", stk_str, inp_str, action);
            top--; /* pop X */
            if (strcmp(prod, "e") != 0) {
                char syms[MAX_PROD][MAX_STR];
                int sc = 0, pos = 0, plen = strlen(prod);
                while (pos < plen)
                    next_symbol(prod, &pos, syms[sc++]);
                for (int s = sc - 1; s >= 0; s--)
                    strcpy(stack[top++], syms[s]);
            }
        }
    }
    printf("\n  ==> String REJECTED (unexpected end)\n");
}

// ----------------------------------------
// Main
// ----------------------------------------
int main() {
    Grammar original, result;
    original.count = 0;

    int n;
    printf("Enter number of productions: ");
    scanf("%d", &n);
    getchar();

    printf("Enter productions (format: NT=body1|body2, e.g. E=E+T|T):\n");
    for (int i = 0; i < n; i++) {
        char line[MAX_STR];
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = '\0';

        /* Split on FIRST '=' */
        char *eq = strchr(line, '=');
        if (!eq) { fprintf(stderr, "Invalid input line: %s\n", line); return 1; }
        *eq = '\0';
        char *left = line;
        char *right = eq + 1;

        NonTerminal *nt = &original.nts[original.count++];
        strcpy(nt->name, left);
        nt->prod_count = 0;

        char *token = strtok(right, "|");
        while (token) {
            strcpy(nt->productions[nt->prod_count++], token);
            token = strtok(NULL, "|");
        }
    }

    /* Remove left recursion */
    remove_left_recursion(&original, &result);

    printf("\nGrammar after Removing Direct Left Recursion:\n");
    for (int i = 0; i < result.count; i++) {
        NonTerminal *nt = &result.nts[i];
        printf("  %s -> ", nt->name);
        for (int j = 0; j < nt->prod_count; j++) {
            if (j) printf(" | ");
            const char *p = nt->productions[j];
            printf("%s", strcmp(p,"e")==0 ? "epsilon" : p);
        }
        printf("\n");
    }

    /* Compute FIRST, FOLLOW */
    compute_first(&result);
    compute_follow(&result, 0);
    print_first_follow(&result);

    /* Build parse table */
    collect_terminals(&result);
    int ll1 = build_parse_table(&result);

    if (ll1)
        printf("\nGrammar is LL(1) -- no conflicts detected.\n");
    else
        printf("\nWarning: Grammar has conflicts and is NOT LL(1).\n");

    print_parse_table(&result);

    /* Predictive parsing */
    printf("\n");
    printf("##################################################\n");
    printf("#           PREDICTIVE PARSING TRACES           #\n");
    printf("##################################################\n");

    parse_string(&result, "id=id+num*id");
    parse_string(&result, "id=+id*num");

    return 0;
}