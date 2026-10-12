# gdb ./acc -ex 'set debuginfod enabled on' -ex "$1" -ex 'tty /dev/pts/1' -ex 'r test.c' \
#     -ex 'lay src'
gdb ./acc  -ex "$1" -ex 'set debuginfod enabled on' -ex 'lay src' -ex 'r test.c' 

