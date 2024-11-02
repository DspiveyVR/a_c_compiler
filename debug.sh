# gdb ./acc -ex 'set debuginfod enabled on' -ex "$1" -ex 'tty /dev/pts/1' -ex 'r test.c' \
#     -ex 'lay src'
gdb ./acc -ex 'set debuginfod enabled on' -ex "$1" -ex 'r test.c' \
    -ex 'lay src'
