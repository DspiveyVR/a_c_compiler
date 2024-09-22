gcc -Wall -c -g ./main.c 
gcc -Wall -c -g ./lexer.c 
gcc -Wall -c -g ./dynamic_string.c 
gcc -Wall -c -g ./parser.c 

gcc parser.o dynamic_string.o lexer.o main.o -Wall -g -o acc

rm dynamic_string.o
rm lexer.o
rm main.o
rm parser.o
