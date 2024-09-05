gcc -c -g ./main.c 
gcc -c -g ./lexer.c 
gcc -c -g ./dynamic_string.c 

gcc dynamic_string.c lexer.o main.o -g -o acc

rm dynamic_string.o
rm lexer.o
rm main.o
