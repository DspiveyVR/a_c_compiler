int square(int a) {
    return a * a;
}

int main() {
    int a;
    a = 3;
    a = square(a) * 5;
    int* b;
    b = &a;
    return *b;
}

