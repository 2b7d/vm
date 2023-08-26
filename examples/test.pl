let file_a = 5;

proc inc(a) {
    a = a + 1;
    return a;
}

proc main() {
    let a = 3;
    let b = 5;

    a = inc(a) + inc(b);

    file_a = a + b + file_a;

    return;
}
