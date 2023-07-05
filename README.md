# Project Microshell

## Features
    - printing prompt (path and login)
    - colours 
    - reads commands in quotation mark
    - redirections (>, >>)
    - builtin commands (echo, cat, ls, ...)
    - implementations:
        - exit 
        - help
        - cd [ , -, ~, dir]
        - cp
        - history [-c] [-r] [-w] [-a] [-d offset] [(number)]
         -- removing repeats and blanks spaces (enter)
         -- removing if space is the first character
        - !!
        - !(number)