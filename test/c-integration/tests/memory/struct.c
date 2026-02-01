// INTEGRATION-TEST
// EXPECT: 15

typedef struct {
    int x;
    int y;
    int z;
} Point;

int sum_point(Point *p) {
    return p->x + p->y + p->z;
}

int test_main(void) {
    Point pt;
    pt.x = 3;
    pt.y = 5;
    pt.z = 7;
    return sum_point(&pt);  // 3+5+7 = 15
}
