void assign(int** a);

int main() {
    int a;
    a = 4;

    int* b;
    b = &a;

    assign(&b);
    return a;
}

void assign(int** a) { **a = 7; }
