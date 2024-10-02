#include <stdlib.h>
#include <string.h>

typedef struct String {
    char* string;
    int length;
} String;

void initialize_string(String* string, int length) {
    string->string = calloc(length, 1);
    string->length = length;
}

void set_string(String* string, char* input_string, int bytes) {
    if (bytes >= string->length) {
        string->string = realloc((void*)(string->string), bytes);
        memset(string->string + string->length, 0, bytes - string->length);
        string->length = bytes;
    }
    strncpy(string->string, input_string, string->length);
}

void set_string_at(String* string, char* input_string, int index, int bytes) {
    if ((index + bytes) >= string->length) {
        string->string = realloc((void*)(string->string), (index + bytes));
        string->length = (index + bytes);
    }
    strncpy(string->string + index, input_string, bytes);
}

void set_char(String* string, char input_char, int index) {
    if (index >= string->length) {
        string->string = realloc((void*)(string->string), string->length + 5);
        string->length += 5;
    }
    string->string[index] = input_char;
}
