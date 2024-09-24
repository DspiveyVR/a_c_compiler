gdb ./acc -ex 'set debuginfod enabled on' -ex "$1" -ex 'r test.c' \
    -ex 'lay src'
