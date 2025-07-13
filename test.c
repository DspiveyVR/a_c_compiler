int sum(int* a, int b);

int main() {
    int a;
    a = 4;

    return sum(&a, 2);
}

int sum(int* a, int b) { return *a + b; }
