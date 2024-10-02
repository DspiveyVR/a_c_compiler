gcc -Wall -c -g ./main.c 
gcc -Wall -c -g ./lexer.c 
gcc -Wall -c -g ./dynamic_string.c 
gcc -Wall -c -g ./parser.c 
gcc -Wall -c -g ./code_generation.c 
gcc -Wall -c -g ./colors.c

gcc colors.o code_generation.o parser.o dynamic_string.o lexer.o main.o -Wall -g -o acc

rm dynamic_string.o
rm lexer.o
rm main.o
rm parser.o
rm code_generation.o
rm colors.o
