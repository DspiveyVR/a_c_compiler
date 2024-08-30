gcc -c ./main.c 
gcc -c ./lexer.c 
gcc -c ./dynamic_array.c

gcc dynamic_array.o lexer.o main.o -o acc

rm dynamic_array.o
rm lexer.o
rm main.o
