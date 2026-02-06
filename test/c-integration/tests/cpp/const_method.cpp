// INTEGRATION-TEST
// EXPECT: 100
// Test const member functions

class Rectangle {
    int width;
    int height;
public:
    Rectangle(int w, int h) : width(w), height(h) {}

    int area() const {
        return width * height;
    }

    int perimeter() const {
        return 2 * (width + height);
    }

    void scale(int factor) {
        width *= factor;
        height *= factor;
    }
};

extern "C" int test_main(void) {
    Rectangle r(5, 10);

    int a1 = r.area();       // 50

    r.scale(2);  // Now 10x20

    int a2 = r.area();       // 200

    // 50 + 200 - 150 = 100
    return a1 + a2 - (a2 - a1);
}
