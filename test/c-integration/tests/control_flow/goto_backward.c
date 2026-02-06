// INTEGRATION-TEST
// EXPECT: 55
int test_main(void) {
    int sum = 0;
    int i = 1;
loop:
    sum += i;
    i++;
    if (i <= 10) goto loop;
    return sum;
}
