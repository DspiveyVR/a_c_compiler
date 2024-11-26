int main() {
    int a;
    a = 254;
    int* b;
    b = &a;
    int** f;
    int c;
    c = 4;
    f = &b;
    int* d;
    d = *f;
    return *d;
}

