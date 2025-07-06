clang -Wall -c -g ./main.c 
clang -Wall -c -g ./lexer.c 
clang -Wall -c -g ./dynamic_string.c 
clang -Wall -c -g ./parser.c 
clang -Wall -c -g ./code_generation.c 
clang -Wall -c -g ./colors.c

clang colors.o code_generation.o parser.o dynamic_string.o lexer.o main.o -Wall -g -o acc

rm dynamic_string.o
rm lexer.o
rm main.o
rm parser.o
rm code_generation.o
rm colors.o
