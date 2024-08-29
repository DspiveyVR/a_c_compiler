gcc -c ./main.c 
gcc -c ./lexer.c 

gcc lexer.o main.o -o acc

rm lexer.o
rm main.o