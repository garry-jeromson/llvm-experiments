// INTEGRATION-TEST
// EXPECT: 55

// Switch with many cases
int test_main(void) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        switch (i) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
            case 3: sum += 4; break;
            case 4: sum += 5; break;
            case 5: sum += 6; break;
            case 6: sum += 7; break;
            case 7: sum += 8; break;
            case 8: sum += 9; break;
            case 9: sum += 10; break;
            default: sum += 100; break;
        }
    }
    return sum;  // 1+2+3+4+5+6+7+8+9+10 = 55
}
