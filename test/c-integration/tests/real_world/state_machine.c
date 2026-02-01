// INTEGRATION-TEST
// EXPECT: 6

// Simple state machine pattern
int test_main(void) {
    int state = 0;
    int count = 0;

    for (int i = 0; i < 10; i++) {
        switch (state) {
            case 0:
                if (i > 2) state = 1;
                break;
            case 1:
                count = count + 1;
                if (i > 5) state = 2;
                break;
            case 2:
                count = count + 1;
                break;
        }
    }
    // i=0,1,2: state 0, no count
    // i=3: state 0->1
    // i=4,5: state 1, count+=2
    // i=6: state 1, count+=1, state->2
    // i=7,8,9: state 2, count+=3
    // Total count = 6
    return count;
}
