// INTEGRATION-TEST
// EXPECT: 150

// Array of structs with field access
typedef struct {
    int x;
    int y;
    int z;
} Point3D;

int test_main(void) {
    Point3D points[3];

    points[0].x = 10; points[0].y = 20; points[0].z = 30;
    points[1].x = 15; points[1].y = 25; points[1].z = 35;
    points[2].x = 5;  points[2].y = 5;  points[2].z = 5;

    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += points[i].x + points[i].y + points[i].z;
    }
    return sum;  // 60 + 75 + 15 = 150
}
