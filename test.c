int square(int a) {
    return a * a;
}

int main() {
    int* a;
    int b;
    b = 5;
    *a = b;

    return square(*a);
}

