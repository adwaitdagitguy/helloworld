def remove_left_recursion(grammar):
    new_grammar = {}

    for non_terminal in grammar:
        productions = grammar[non_terminal]

        alpha = []  # left recursive
        beta = []   # non-left recursive

        for prod in productions:
            if prod.startswith(non_terminal):
                alpha.append(prod[len(non_terminal):])
            else:
                beta.append(prod)

        # If left recursion exists
        if alpha:
            new_nt = non_terminal + "'"

            new_grammar[non_terminal] = [
                b + new_nt for b in beta
            ]

            new_grammar[new_nt] = [
                a + new_nt for a in alpha
            ] + ['ε']

        else:
            new_grammar[non_terminal] = productions

    return new_grammar


# Example Grammar
grammar = {
    'A': ['Aa', 'Ab', 'c', 'd']
}

result = remove_left_recursion(grammar)

# Print Result
for nt in result:
    print(nt, "->", " | ".join(result[nt]))