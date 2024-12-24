int sum(int a, int b, int c, int d);

int main() {
    int a;
    a = 3;
    if (a == 3) {
        a = sum(1, 2, a, 4);
    }
    int* b;
    b = &a;
    return *b;
}

int sum(int a, int b, int c, int d) {
    return a + b + c + d;
}
