// INTEGRATION-TEST
// EXPECT: 30

typedef struct {
    int value;
} Item;

int test_main(void) {
    Item items[3];
    items[0].value = 10;
    items[1].value = 20;
    items[2].value = 30;
    return items[2].value;
}
