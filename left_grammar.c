#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NT 20          // max non-terminals
#define MAX_PROD 20        // max productions per non-terminal
#define MAX_STR 256        // max string length
#define MAX_SET 30         // max elements in FIRST/FOLLOW set

// ----------------------------------------
// Data Structures
// ----------------------------------------

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

Set FIRST[MAX_NT];
Set FOLLOW[MAX_NT];

// ----------------------------------------
// Utility Functions
// ----------------------------------------

int set_contains(Set *s, const char *val) {
    for (int i = 0; i < s->count; i++)
        if (strcmp(s->elems[i], val) == 0) return 1;
    return 0;
}

int set_add(Set *s, const char *val) {
    if (!set_contains(s, val)) {
        strcpy(s->elems[s->count++], val);
        return 1; // changed
    }
    return 0;
}

// Union: add all from src into dst, excluding 'exclude' (pass NULL to exclude nothing)
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

// Returns 1 if 'name' is a non-terminal in grammar
int is_nt(Grammar *g, const char *name) {
    return find_nt(g, name) >= 0;
}

// Parse next symbol from production string at position *pos
// Handles primed symbols like E'
void next_symbol(const char *prod, int *pos, char *sym) {
    int p = *pos;
    sym[0] = prod[p];
    sym[1] = '\0';
    p++;
    if (prod[p] == '\'') {
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
            int alen = strlen(A->name);
            if (strncmp(prod, A->name, alen) == 0) {
                // Left-recursive: α = remainder after A
                strcpy(alpha[alpha_count++], prod + alen);
            } else {
                strcpy(beta[beta_count++], prod);
            }
        }

        if (alpha_count == 0) {
            // No left recursion: copy as-is
            NonTerminal *nt = &result->nts[result->count++];
            strcpy(nt->name, A->name);
            nt->prod_count = A->prod_count;
            for (int j = 0; j < A->prod_count; j++)
                strcpy(nt->productions[j], A->productions[j]);
        } else {
            // A' name
            char A_dash[MAX_STR];
            snprintf(A_dash, sizeof(A_dash), "%s'", A->name);

            // A → βA'
            NonTerminal *nt = &result->nts[result->count++];
            strcpy(nt->name, A->name);
            nt->prod_count = 0;
            for (int j = 0; j < beta_count; j++) {
                snprintf(nt->productions[nt->prod_count++], MAX_STR, "%s%s", beta[j], A_dash);
            }

            // A' → αA' | ε
            NonTerminal *nt_dash = &result->nts[result->count++];
            strcpy(nt_dash->name, A_dash);
            nt_dash->prod_count = 0;
            for (int j = 0; j < alpha_count; j++) {
                snprintf(nt_dash->productions[nt_dash->prod_count++], MAX_STR, "%s%s", alpha[j], A_dash);
            }
            strcpy(nt_dash->productions[nt_dash->prod_count++], "ε");
        }
    }
}

// ----------------------------------------
// Compute FIRST
// ----------------------------------------

void compute_first(Grammar *g) {
    for (int i = 0; i < g->count; i++)
        FIRST[i].count = 0;

    int changed = 1;
    while (changed) {
        changed = 0;
        for (int ai = 0; ai < g->count; ai++) {
            NonTerminal *A = &g->nts[ai];
            for (int pi = 0; pi < A->prod_count; pi++) {
                char *prod = A->productions[pi];

                if (strcmp(prod, "ε") == 0) {
                    changed |= set_add(&FIRST[ai], "ε");
                    continue;
                }

                int pos = 0;
                int plen = strlen(prod);
                while (pos < plen) {
                    char sym[MAX_STR];
                    next_symbol(prod, &pos, sym);

                    int si = find_nt(g, sym);
                    if (si < 0) {
                        // Terminal
                        changed |= set_add(&FIRST[ai], sym);
                        break;
                    } else {
                        // Non-terminal
                        changed |= set_union_exclude(&FIRST[ai], &FIRST[si], "ε");
                        if (!set_contains(&FIRST[si], "ε"))
                            break;
                        if (pos >= plen)
                            changed |= set_add(&FIRST[ai], "ε");
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
    for (int i = 0; i < g->count; i++)
        FOLLOW[i].count = 0;

    set_add(&FOLLOW[start_idx], "$");

    int changed = 1;
    while (changed) {
        changed = 0;
        for (int ai = 0; ai < g->count; ai++) {
            NonTerminal *A = &g->nts[ai];
            for (int pi = 0; pi < A->prod_count; pi++) {
                char *prod = A->productions[pi];
                int plen = strlen(prod);
                int pos = 0;

                while (pos < plen) {
                    char B[MAX_STR];
                    next_symbol(prod, &pos, B);

                    int bi = find_nt(g, B);
                    if (bi < 0) continue; // terminal, skip

                    // Compute FIRST of beta (remainder)
                    Set first_beta;
                    first_beta.count = 0;

                    int bpos = pos;
                    int all_nullable = 1;
                    while (bpos < plen) {
                        char sym[MAX_STR];
                        next_symbol(prod, &bpos, sym);

                        int si = find_nt(g, sym);
                        if (si < 0) {
                            set_add(&first_beta, sym);
                            all_nullable = 0;
                            break;
                        } else {
                            set_union_exclude(&first_beta, &FIRST[si], "ε");
                            if (!set_contains(&FIRST[si], "ε")) {
                                all_nullable = 0;
                                break;
                            }
                        }
                    }
                    if (bpos >= plen && all_nullable)
                        set_add(&first_beta, "ε");

                    changed |= set_union_exclude(&FOLLOW[bi], &first_beta, "ε");
                    if (set_contains(&first_beta, "ε"))
                        changed |= set_union_exclude(&FOLLOW[bi], &FOLLOW[ai], NULL);
                }
            }
        }
    }
}

// ----------------------------------------
// Print set sorted (simple insertion sort on display)
// ----------------------------------------

void print_set_sorted(Set *s) {
    // Copy and sort
    char sorted[MAX_SET][MAX_STR];
    int n = s->count;
    for (int i = 0; i < n; i++) strcpy(sorted[i], s->elems[i]);
    for (int i = 0; i < n-1; i++)
        for (int j = i+1; j < n; j++)
            if (strcmp(sorted[i], sorted[j]) > 0) {
                char tmp[MAX_STR];
                strcpy(tmp, sorted[i]);
                strcpy(sorted[i], sorted[j]);
                strcpy(sorted[j], tmp);
            }
    for (int i = 0; i < n; i++) {
        if (i) printf(", ");
        printf("%s", sorted[i]);
    }
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
    getchar(); // consume newline

    printf("Enter productions (Example: E=E+T|T):\n");
    for (int i = 0; i < n; i++) {
        char line[MAX_STR];
        fgets(line, sizeof(line), stdin);
        // Remove trailing newline
        line[strcspn(line, "\n")] = '\0';

        // Split on '='
        char *eq = strchr(line, '=');
        if (!eq) { fprintf(stderr, "Invalid input\n"); return 1; }
        *eq = '\0';
        char *left = line;
        char *right = eq + 1;

        NonTerminal *nt = &original.nts[original.count++];
        strcpy(nt->name, left);
        nt->prod_count = 0;

        // Split right on '|'
        char *token = strtok(right, "|");
        while (token) {
            strcpy(nt->productions[nt->prod_count++], token);
            token = strtok(NULL, "|");
        }
    }

    // Remove left recursion
    remove_left_recursion(&original, &result);

    printf("\nAfter Removing Direct Left Recursion:\n");
    for (int i = 0; i < result.count; i++) {
        NonTerminal *nt = &result.nts[i];
        printf("%s -> ", nt->name);
        for (int j = 0; j < nt->prod_count; j++) {
            if (j) printf(" | ");
            printf("%s", nt->productions[j]);
        }
        printf("\n");
    }

    compute_first(&result);
    compute_follow(&result, 0);

    printf("\n--------------------------------------------------\n");
    printf("%-6s%-20s%s\n", "NT", "FIRST SET", "FOLLOW SET");
    printf("--------------------------------------------------\n");

    for (int i = 0; i < result.count; i++) {
        printf("%-6s", result.nts[i].name);
        // Print FIRST inline padded to 20 chars
        char buf[256] = "";
        Set tmp = FIRST[i];
        // Build comma-separated sorted string
        char sorted[MAX_SET][MAX_STR];
        int sn = tmp.count;
        for (int j = 0; j < sn; j++) strcpy(sorted[j], tmp.elems[j]);
        for (int j = 0; j < sn-1; j++)
            for (int k = j+1; k < sn; k++)
                if (strcmp(sorted[j], sorted[k]) > 0) {
                    char t[MAX_STR]; strcpy(t, sorted[j]);
                    strcpy(sorted[j], sorted[k]); strcpy(sorted[k], t);
                }
        for (int j = 0; j < sn; j++) {
            if (j) strcat(buf, ", ");
            strcat(buf, sorted[j]);
        }
        printf("%-20s", buf);

        // Print FOLLOW
        Set tmp2 = FOLLOW[i];
        char buf2[256] = "";
        int sn2 = tmp2.count;
        char sorted2[MAX_SET][MAX_STR];
        for (int j = 0; j < sn2; j++) strcpy(sorted2[j], tmp2.elems[j]);
        for (int j = 0; j < sn2-1; j++)
            for (int k = j+1; k < sn2; k++)
                if (strcmp(sorted2[j], sorted2[k]) > 0) {
                    char t[MAX_STR]; strcpy(t, sorted2[j]);
                    strcpy(sorted2[j], sorted2[k]); strcpy(sorted2[k], t);
                }
        for (int j = 0; j < sn2; j++) {
            if (j) strcat(buf2, ", ");
            strcat(buf2, sorted2[j]);
        }
        printf("%s\n", buf2);
    }
    printf("--------------------------------------------------\n");

    return 0;
}