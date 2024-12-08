int square(int a, int b, int c, int d) {
    return a + b + c + d;
}

int main() {
    int a;
    a = 3;
    a = square(1, 2, a, 4);
    int* b;
    b = &a;
    return *b;
}

