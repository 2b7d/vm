let file_a: u16 = 69;

proc inc(arg_a: u16): u16 {
    let block_a: u16 = 1;

    arg_a = arg_a + block_a;
    return arg_a;
}

proc main(): void {
    let block_a: u16 = 5;
    let block_b: u16 = 3;

    inc(block_a);

    block_b = inc(block_a); // 6
    block_b = inc(block_b); // 4

    file_a = file_a - (block_a + block_b) + 8; // 48

    return;
}
