fn main(x: pub Field) {
    assert(x + 7 == 13);
}

#[test]
fn test_valid_input() {
    main(6);
}
