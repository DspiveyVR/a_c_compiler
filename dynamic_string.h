#ifndef DYN_STR_H
#define DYN_STR_H
typedef struct String {
    char* string;
    int length;
} String;

void initialize_string(String* string, int length);
void set_string(String* string, char* input_string, int bytes);
void set_char(String* string, char input_char, int index);

#endif