struct A {
    int a;
    struct A *p;
};

int DoSomething(struct A* p1, struct A* p2) {
    return p1->a + p2->a;
}

int d = 0;
void TestVoidType(int a, int b, int c) {
    d = a + b + c;
    return;
}

int main() {
    struct A v1 = { 1991 };
    struct A v2 = { 2025 };
    v1.p = &v2;

    return DoSomething(&v1, &v2) + v1.p->a;
}
