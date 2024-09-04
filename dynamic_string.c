#include <stdlib.h>
#include <string.h>

typedef struct String {
    char* string;
    int length;
} String;

void initialize_string(String* string, int length) {
    string->string = malloc(length);
    string->length = length;
}

void set_string(String* string, char* input_string, int bytes) {
    if (bytes >= string->length) {
        string->string = realloc((void*)(string->string), string->length + 5);
        string->length += 5;
    }
    strncpy(, string, 50);
}
