// INTEGRATION-TEST
// EXPECT: 60

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point start;
    Point end;
} Line;

int test_main(void) {
    Line line;
    line.start.x = 10;
    line.start.y = 20;
    line.end.x = 30;
    line.end.y = 0;
    return line.start.x + line.start.y + line.end.x;  // 10+20+30 = 60
}
