int square() {
    return 4 * 4;
}

int main() {
    int a;
    a = square() * 5;
    int* b;
    b = &a;
    return *b;
}

