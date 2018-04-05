# Todo

- Incorporate symbol table ds into cfg builder 
    - Every time a new function is entered, run the visitor to get global, local, free vars 
    - For each free var in the child, create a ref var or a free var in the parent 
    - Use this list to create the variable arrays for the child and perhaps a map ds to get indices more efficiently. 
    - Also run this at the root and make everything a global. 
    - Write an easy lookup function.

- CFG: Handle variable writing/storing (depends on the above) 

- CFG: Handling function definitions/calls (related to above)

- CFG: Records 
