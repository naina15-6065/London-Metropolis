#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <string>
#include <cstdlib>

using std::string;
using std::vector;

const int WINDOW_W = 1400;
const int WINDOW_H = 850;
const float PI = 3.14159265359f;

struct Color {
    float r, g, b, a;
    Color(float r=1, float g=1, float b=1, float a=1): r(r), g(g), b(b), a(a) {}
};

struct Vec2 {
    float x, y;
    Vec2(float x=0, float y=0): x(x), y(y) {}
};

struct Vehicle {
    float x, y;
    float speed;
    float width;
    float height;
    Color body;
    bool bus;
    bool dirRight;
};

struct Cloud {
    float x, y, speed, scale;
};

struct Boat {
    float x, y, speed;
};


float gTime = 0.0f;
float cameraX = 0.0f;

float dayBlend = 0.2f;
float targetDayBlend = 0.2f;

float eyeRotation = 0.0f;
float clockHourAngle = 0.0f;
float clockMinuteAngle = 0.0f;

bool rainEnabled = false;
bool trafficEnabled = true;
bool lightsForced = false;
bool fogEnabled = false;
bool boatEnabled = true;
bool snowEnabled = false;

vector<Vehicle> vehicles;
vector<Cloud> clouds;
vector<Boat> boats;


float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

Color mixColor(const Color &a, const Color &b, float t) {
    return Color(
        lerpf(a.r, b.r, t),
        lerpf(a.g, b.g, t),
        lerpf(a.b, b.b, t),
        lerpf(a.a, b.a, t)
    );
}

void setColor(const Color &c) {
    glColor4f(c.r, c.g, c.b, c.a);
}

void drawText(float x, float y, void* font, const string &text, const Color &c) {
    setColor(c);
    glRasterPos2f(x, y);
    for (char ch : text) glutBitmapCharacter(font, ch);
}

void rect(float x1, float y1, float x2, float y2) {
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void gradientRect(float x1, float y1, float x2, float y2, const Color &bottom, const Color &top) {
    glBegin(GL_QUADS);
    glColor4f(bottom.r, bottom.g, bottom.b, bottom.a);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glColor4f(top.r, top.g, top.b, top.a);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void drawHorizontalLine(int x1, int x2, int y) {
    glBegin(GL_LINES);
    glVertex2i(x1, y);
    glVertex2i(x2, y);
    glEnd();
}

void smoothCircle(float cx, float cy, float r, int segments = 100) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * PI * i / segments;
        float x = cx + r * cosf(angle);
        float y = cy + r * sinf(angle);
        glVertex2f(x, y);
    }
    glEnd();
}

void drawCirclePoints(int cx, int cy, int x, int y) {
    glVertex2i(cx + x, cy + y);
    glVertex2i(cx - x, cy + y);
    glVertex2i(cx + x, cy - y);
    glVertex2i(cx - x, cy - y);
    glVertex2i(cx + y, cy + x);
    glVertex2i(cx - y, cy + x);
    glVertex2i(cx + y, cy - x);
    glVertex2i(cx - y, cy - x);
}

void midpointCircle(float cx, float cy, float r) {
    int xc = (int)round(cx);
    int yc = (int)round(cy);
    int radius = (int)round(r);

    int x = 0;
    int y = radius;
    int p = 1 - radius;

    glBegin(GL_POINTS);
    while (x <= y) {
        drawCirclePoints(xc, yc, x, y);

        x++;
        if (p < 0) {
            p = p + 2 * x + 1;
        } else {
            y--;
            p = p + 2 * x - 2 * y + 1;
        }
    }
    glEnd();
}

void circle(float cx, float cy, float r, int segments = 100) {
    smoothCircle(cx, cy, r, segments);
}

void ring(float cx, float cy, float outerR, float innerR, int seg = 96) {
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= seg; ++i) {
        float a = 2.0f * PI * i / seg;
        glVertex2f(cx + cosf(a) * outerR, cy + sinf(a) * outerR);
        glVertex2f(cx + cosf(a) * innerR, cy + sinf(a) * innerR);
    }
    glEnd();
}

void line(float x1, float y1, float x2, float y2, float width = 1.0f) {
    int xStart = (int)round(x1);
    int yStart = (int)round(y1);
    int xEnd   = (int)round(x2);
    int yEnd   = (int)round(y2);

    int dx = abs(xEnd - xStart);
    int dy = abs(yEnd - yStart);

    int sx = (xStart < xEnd) ? 1 : -1;
    int sy = (yStart < yEnd) ? 1 : -1;

    int err = dx - dy;

    glPointSize(width);
    glBegin(GL_POINTS);

    while (true) {
        glVertex2i(xStart, yStart);

        if (xStart == xEnd && yStart == yEnd)
            break;

        int e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            xStart += sx;
        }

        if (e2 < dx) {
            err += dx;
            yStart += sy;
        }
    }

    glEnd();
    glPointSize(1.0f);
}

void triangle(float x1, float y1, float x2, float y2, float x3, float y3) {
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
}

void polygon(const vector<Vec2>& pts) {
    glBegin(GL_POLYGON);
    for (const auto& p : pts) glVertex2f(p.x, p.y);
    glEnd();
}

void glow(float x, float y, float r, const Color &c) {
    glEnable(GL_BLEND);
    for (int i = 5; i >= 1; --i) {
        float scale = i / 5.0f;
        setColor(Color(c.r, c.g, c.b, 0.05f * scale));
        circle(x, y, r * (1.0f + 0.45f * (6 - i)), 48);
    }
}

void setup2D() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_W, 0, WINDOW_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

float visibleLights() {
    return lightsForced ? 1.0f : clampf((dayBlend - 0.45f) / 0.55f, 0.0f, 1.0f);
}

void initScene() {
    vehicles.clear();
    vehicles.push_back({120, 110, 1.4f, 170, 55, Color(0.85f, 0.12f, 0.12f, 1), true, true});
    vehicles.push_back({560, 112, 1.0f, 110, 40, Color(0.08f, 0.08f, 0.08f, 1), false, true});
    vehicles.push_back({980, 110, 1.25f, 105, 42, Color(0.12f, 0.12f, 0.12f, 1), false, true});
    vehicles.push_back({1300, 110, 1.7f, 170, 55, Color(0.82f, 0.08f, 0.08f, 1), true, false});
    vehicles.push_back({760, 108, 1.35f, 110, 40, Color(0.10f, 0.35f, 0.85f, 1), false, true});

    clouds.clear();
    clouds.push_back({120, 710, 0.15f, 1.0f});
    clouds.push_back({380, 760, 0.11f, 1.35f});
    clouds.push_back({920, 705, 0.18f, 1.1f});
    clouds.push_back({1220, 760, 0.08f, 1.5f});
    clouds.push_back({200, 740, 0.09f, 0.9f});
    clouds.push_back({480, 730, 0.11f, 1.0f});
    clouds.push_back({760, 750, 0.08f, 0.85f});
    clouds.push_back({1050, 735, 0.10f, 1.1f});
    clouds.push_back({1300, 745, 0.07f, 0.95f});
    clouds.push_back({60, 760, 0.06f, 0.7f});
    clouds.push_back({300, 720, 0.08f, 0.85f});
    clouds.push_back({580, 740, 0.09f, 0.95f});

    boats.clear();
    boats.push_back({250, 255, 0.35f});
    boats.push_back({900, 220, 0.22f});
}

void drawSky() {
    Color dayBottom(0.67f, 0.86f, 0.98f, 1.0f);
    Color dayTop(0.93f, 0.97f, 1.0f, 1.0f);
    Color nightBottom(0.03f, 0.06f, 0.15f, 1.0f);
    Color nightTop(0.15f, 0.18f, 0.30f, 1.0f);

    Color skyBottom = mixColor(dayBottom, nightBottom, dayBlend);
    Color skyTop = mixColor(dayTop, nightTop, dayBlend);

    gradientRect(0, 0, WINDOW_W, WINDOW_H, skyBottom, skyTop);

    float sunX = 1040;
    float sunY = lerpf(680, 560, dayBlend);
    Color sunColor = mixColor(
        Color(1.0f, 0.9f, 0.45f, 0.95f),
        Color(0.85f, 0.90f, 1.0f, 0.9f),
        dayBlend
    );

    if (dayBlend < 0.9f) {
        glow(sunX, sunY, 26, Color(sunColor.r, sunColor.g, sunColor.b, 0.12f));
        setColor(sunColor);
        circle(sunX, sunY, 42, 64);
    }

    if (dayBlend > 0.25f) {
        float moonAlpha = (dayBlend - 0.25f) / 0.75f;

        float moonX = 500.0f;
        float moonY = 730.0f;

        setColor(Color(1.0f, 0.98f, 0.85f, 0.95f * moonAlpha));
        circle(moonX, moonY, 30.0f, 64);

        float t = moonY / WINDOW_H;
        Color localSky = mixColor(skyBottom, skyTop, t);

        setColor(localSky);
        circle(moonX + 11.0f, moonY + 4.0f, 26.0f, 64);
    }

    if (dayBlend > 0.45f) {
        float a = (dayBlend - 0.45f) / 0.55f;
        setColor(Color(1, 1, 1, 0.85f * a));
        for (int i = 0; i < 70; ++i) {
            float sx = fmodf(i * 187.0f + 53.0f, (float)WINDOW_W);
            float sy = 520 + fmodf(i * 97.0f + 77.0f, 280.0f);
            circle(sx, sy, (i % 3) + 1.0f, 8);
        }
    }
}

void drawCloud(float x, float y, float s) {
    Color c = mixColor(
      Color(1.0f, 1.0f, 1.0f, 1.0f),
      Color(0.62f, 0.66f, 0.74f, 1.0f),
      dayBlend
  );
    setColor(c);
    circle(x, y, 24 * s);
    circle(x + 26 * s, y + 10 * s, 28 * s);
    circle(x + 58 * s, y + 2 * s, 22 * s);
    circle(x + 14 * s, y - 8 * s, 22 * s);
    circle(x + 44 * s, y - 8 * s, 20 * s);
}

void drawClouds() {
    for (const auto &c : clouds) drawCloud(c.x, c.y, c.scale);
}

void drawSkylineLayer(float baseY, float alpha, float scale, float offset, const Color& color) {
    setColor(Color(color.r, color.g, color.b, alpha));
    float x = -50 + fmodf(offset, 180.0f) - 180.0f;
    while (x < WINDOW_W + 200) {
        float w = 60 + fmodf(x * 1.37f + 111, 75.0f);
        float h = 70 + fmodf(x * 0.97f + 91, 220.0f);
        rect(x, baseY, x + w * scale, baseY + h * scale);
        if ((int)x % 3 == 0) rect(x + w * 0.15f, baseY + h * scale, x + w * 0.35f, baseY + h * scale + 38 * scale);
        x += w * 0.8f;
    }
}

void drawDistantCity() {
    Color layer1 = mixColor(Color(0.58f, 0.67f, 0.78f, 1), Color(0.09f, 0.11f, 0.18f, 1), dayBlend);
    Color layer2 = mixColor(Color(0.44f, 0.52f, 0.62f, 1), Color(0.05f, 0.07f, 0.14f, 1), dayBlend);
    drawSkylineLayer(310, 0.45f, 1.0f, cameraX * 0.15f, layer1);
    drawSkylineLayer(285, 0.65f, 1.15f, cameraX * 0.25f + 50, layer2);
}

void drawWaterReflectionBand() {
    float nightFactor = dayBlend;

    setColor(Color(1, 1, 1, 0.04f + 0.05f * nightFactor));
    for (int i = 0; i < 10; ++i) {
        float y = 160 + i * 14;
        rect(0, y, WINDOW_W, y + 4);
    }

    float bridgeCenterX = 920;
    float width = 240;

    setColor(Color(0.9f, 0.9f, 1.0f, 0.06f + 0.08f * nightFactor));

    for (int i = 0; i < 8; ++i) {
        float y = 165 + i * 10;
        float wave = sinf(gTime * 2.0f + i) * 8;

        rect(bridgeCenterX - width + wave,
             y,
             bridgeCenterX + width + wave,
             y + 3);
    }

    if (nightFactor > 0.4f) {
        for (int i = 0; i < 8; ++i) {
            float lx = 748 + i * 45;

            for (int j = 0; j < 6; ++j) {
                float y = 150 + j * 10;
                float w = 2 + j * 0.5f;

                setColor(Color(1.0f, 0.9f, 0.6f, 0.08f * nightFactor));
                rect(lx - w, y, lx + w, y + 6);
            }
        }
    }
}

void drawThames() {
    Color top = mixColor(Color(0.23f, 0.58f, 0.84f, 1), Color(0.03f, 0.12f, 0.22f, 1), dayBlend);
    Color bottom = mixColor(Color(0.12f, 0.39f, 0.64f, 1), Color(0.01f, 0.08f, 0.16f, 1), dayBlend);

    gradientRect(0, 150, WINDOW_W, 320, bottom, top);

    drawWaterReflectionBand();

    setColor(Color(1, 1, 1, 0.06f + 0.06f * dayBlend));
    for (int i = 0; i < 24; ++i) {
        float y = 165 + i * 6;
        float shift = fmodf(gTime * (20 + i * 0.8f) + i * 37, 140.0f);

        glBegin(GL_LINES);
        for (float x = -80; x < WINDOW_W + 40; x += 120) {
            glVertex2f(x + shift, y);
            glVertex2f(x + 45 + shift, y);
        }
        glEnd();
    }

    setColor(Color(1, 1, 1, 0.04f + 0.05f * dayBlend));
    for (int i = 0; i < 18; ++i) {
        float y = 170 + i * 8;
        float shift = fmodf(gTime * (28 + i) + i * 19, 100.0f);

        glBegin(GL_LINES);
        for (float x = -50; x < WINDOW_W + 30; x += 90) {
            glVertex2f(x + shift, y);
            glVertex2f(x + 20 + shift, y);
        }
        glEnd();
    }

    setColor(Color(1, 1, 1, 0.06f + 0.08f * dayBlend));
    for (int i = 0; i < 10; ++i) {
        float y = 185 + i * 12;
        float shift = fmodf(gTime * 32 + i * 53, 180.0f);

        glBegin(GL_LINES);
        for (float x = -100; x < WINDOW_W; x += 170) {
            glVertex2f(x + shift, y);
            glVertex2f(x + 70 + shift, y);
        }
        glEnd();
    }
}

void drawBridge() {
    Color bridgeMain  = mixColor(Color(0.66f, 0.68f, 0.75f, 1), Color(0.20f, 0.22f, 0.28f, 1), dayBlend);
    Color bridgeDark  = mixColor(Color(0.45f, 0.48f, 0.58f, 1), Color(0.12f, 0.14f, 0.20f, 1), dayBlend);
    Color bridgeLight = mixColor(Color(0.82f, 0.84f, 0.90f, 1), Color(0.35f, 0.38f, 0.48f, 1), dayBlend);

    float deckX1 = 700, deckX2 = 1140;
    float deckY1 = 320, deckY2 = 342;

    float towerBaseY = 342;
    float towerW = 52, towerH = 180;

    float leftTowerX  = 780;
    float rightTowerX = 980;

    float leftTopX  = leftTowerX + towerW / 2.0f;
    float leftTopY  = towerBaseY + towerH + 42;

    float rightTopX = rightTowerX + towerW / 2.0f;
    float rightTopY = towerBaseY + towerH + 42;

    float deckY = deckY2;

    setColor(bridgeLight);
    rect(deckX1, deckY2, deckX2, deckY2 + 5);

    setColor(bridgeLight);
    for (int i = 0; i < 10; ++i) {
        float segW = (deckX2 - deckX1) / 10.0f;
        rect(deckX1 + i * segW + 2, deckY2 - 5,
             deckX1 + (i + 1) * segW - 2, deckY2 - 1);
    }

    setColor(bridgeMain);
    rect(deckX1, deckY1, deckX2, deckY2);

    setColor(bridgeDark);
    rect(deckX1, deckY2 - 4, deckX2, deckY2);

    setColor(bridgeDark);
    rect(760, 300, 806, 320);
    rect(1034, 300, 1080, 320);

    setColor(bridgeMain);
    rect(750, 292, 816, 300);
    rect(1024, 292, 1090, 300);

    Color cableColor = mixColor(
        bridgeLight,
        Color(1.0f, 1.0f, 0.9f, 1.0f),
        dayBlend * 0.45f
    );

    setColor(cableColor);
    for (int i = -1; i <= 1; ++i) {
        line(leftTopX + i, leftTopY, 730 + i, deckY, 2.0f);
        line(rightTopX + i, rightTopY, 1060 + i, deckY, 2.0f);
        line(leftTopX + i, leftTopY, 885 + i, deckY, 2.0f);
        line(rightTopX + i, rightTopY, 900 + i, deckY, 2.0f);
    }

    setColor(bridgeMain);
    rect(leftTowerX, towerBaseY, leftTowerX + towerW, towerBaseY + towerH);
    rect(rightTowerX, towerBaseY, rightTowerX + towerW, towerBaseY + towerH);

    setColor(bridgeDark);
    rect(leftTowerX + 10, towerBaseY + 18, leftTowerX + towerW - 10, towerBaseY + towerH - 18);
    rect(rightTowerX + 10, towerBaseY + 18, rightTowerX + towerW - 10, towerBaseY + towerH - 18);

    setColor(bridgeMain);
    rect(leftTowerX - 6, towerBaseY - 10, leftTowerX + towerW + 6, towerBaseY);
    rect(rightTowerX - 6, towerBaseY - 10, rightTowerX + towerW + 6, towerBaseY);

    float lightA = visibleLights();
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 2; ++c) {
            float wx1 = leftTowerX + 14 + c * 14;
            float wy1 = towerBaseY + 34 + r * 34;
            setColor(mixColor(Color(0.18f, 0.20f, 0.24f, 1),
                              Color(1.0f, 0.88f, 0.55f, 1),
                              lightA * 0.7f));
            rect(wx1, wy1, wx1 + 8, wy1 + 18);

            float wx2 = rightTowerX + 14 + c * 14;
            float wy2 = towerBaseY + 34 + r * 34;
            rect(wx2, wy2, wx2 + 8, wy2 + 18);
        }
    }

    setColor(bridgeLight);
    rect(leftTowerX + 14, towerBaseY + 118, leftTowerX + towerW - 14, towerBaseY + 154);
    rect(rightTowerX + 14, towerBaseY + 118, rightTowerX + towerW - 14, towerBaseY + 154);

    setColor(bridgeMain);
    rect(leftTowerX + towerW - 2, towerBaseY + 138, rightTowerX + 2, towerBaseY + 156);

    setColor(bridgeDark);
    rect(leftTowerX + towerW - 2, towerBaseY + 132, rightTowerX + 2, towerBaseY + 138);

    setColor(bridgeLight);
    triangle(leftTowerX - 10, towerBaseY + towerH,
             leftTowerX + towerW / 2.0f, towerBaseY + towerH + 58,
             leftTowerX + towerW + 10, towerBaseY + towerH);

    triangle(rightTowerX - 10, towerBaseY + towerH,
             rightTowerX + towerW / 2.0f, towerBaseY + towerH + 58,
             rightTowerX + towerW + 10, towerBaseY + towerH);

    setColor(bridgeDark);
    rect(leftTowerX + 18, towerBaseY + towerH + 58, leftTowerX + 34, towerBaseY + towerH + 82);
    rect(rightTowerX + 18, towerBaseY + towerH + 58, rightTowerX + 34, towerBaseY + towerH + 82);

    setColor(cableColor);

    for (int i = 0; i <= 8; ++i) {
        float x = 730 + i * 14.0f;
        if (x >= leftTowerX - 4) break;
        float t = (x - 730.0f) / (leftTopX - 730.0f);
        float topY = deckY + (leftTopY - deckY) * t;
        line(x, topY, x, deckY, 1.4f);
    }

    for (int i = 0; i <= 9; ++i) {
        float x = leftTowerX + towerW + 8.0f + i * 9.0f;
        if (x >= 885.0f) break;
        float t = (x - leftTopX) / (885.0f - leftTopX);
        float topY = leftTopY + (deckY - leftTopY) * t;
        line(x, topY, x, deckY, 1.4f);
    }

    for (int i = 0; i <= 9; ++i) {
        float x = 900.0f + i * 9.0f;
        if (x >= rightTowerX - 8) break;
        float t = (x - 900.0f) / (rightTopX - 895.0f);
        float topY = deckY + (rightTopY - deckY) * t;
        line(x, topY, x, deckY, 1.4f);
    }

    for (int i = 0; i <= 8; ++i) {
        float x = rightTowerX + towerW + 6.0f + i * 14.0f;
        if (x > 1060.0f) break;
        float t = (x - rightTopX) / (1060.0f - rightTopX);
        float topY = rightTopY + (deckY - rightTopY) * t;
        line(x, topY, x, deckY, 1.4f);
    }

    float lampA = visibleLights();
    if (lampA > 0.01f) {
        for (int i = 0; i < 8; ++i) {
            float lx = 748 + i * 45;
            glow(lx, deckY + 10, 5, Color(1.0f, 0.88f, 0.50f, 0.18f * lampA));
            setColor(Color(1.0f, 0.92f, 0.60f, 0.95f * lampA));
            circle(lx, deckY + 10, 3, 20);
        }
    }
}

void drawBigBen() {
    Color base = mixColor(Color(0.78f, 0.71f, 0.49f, 1), Color(0.25f, 0.22f, 0.16f, 1), dayBlend);
    Color dark = mixColor(Color(0.55f, 0.46f, 0.25f, 1), Color(0.16f, 0.14f, 0.10f, 1), dayBlend);
    float x = 180, y = 320;

    setColor(base);
    rect(x, y, x + 95, y + 280);

    setColor(dark);
    rect(x + 8, y + 18, x + 87, y + 250);

    setColor(base);
    rect(x - 12, y + 280, x + 107, y + 330);
    rect(x + 14, y + 330, x + 81, y + 420);

    setColor(dark);
    rect(x + 22, y + 340, x + 73, y + 406);
    triangle(x + 12, y + 420, x + 48, y + 505, x + 84, y + 420);
    rect(x + 39, y + 505, x + 57, y + 560);

    float cx = x + 47.5f;
    float cy = y + 372;

    Color clockFace = mixColor(
        Color(0.96f, 0.93f, 0.76f, 1),
        Color(0.98f, 0.86f, 0.50f, 0.95f),
        visibleLights()
    );

    setColor(clockFace);
    circle(cx, cy, 17, 64);

    setColor(dark);
    midpointCircle(cx, cy, 18);

    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glRotatef(-clockHourAngle, 0, 0, 1);

    setColor(dark);
    line(0, 0, 0, 8, 3);

    glPopMatrix();

    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glRotatef(-clockMinuteAngle, 0, 0, 1);

    setColor(dark);
    line(0, 0, 0, 12, 2);

    glPopMatrix();

    setColor(dark);
    midpointCircle(cx, cy, 2);

    float lightA = visibleLights();
    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 3; ++c) {
            float wx = x + 18 + c * 20;
            float wy = y + 40 + r * 36;
            setColor(mixColor(
                Color(0.23f, 0.17f, 0.10f, 1),
                Color(1.0f, 0.82f, 0.38f, 1),
                lightA * (0.75f + 0.25f * ((r + c) % 2))
            ));
            rect(wx, wy, wx + 10, wy + 18);
        }
    }
}

void drawLondonEye() {
    float cx = 1220, cy = 405, radius = 118;

    Color frame = mixColor(Color(0.84f, 0.88f, 0.92f, 1), Color(0.55f, 0.62f, 0.72f, 1), dayBlend);
    Color spoke = mixColor(Color(0.76f, 0.82f, 0.88f, 1), Color(0.45f, 0.52f, 0.62f, 1), dayBlend);

    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glRotatef(eyeRotation, 0, 0, 1);

    setColor(frame);
    ring(0, 0, radius + 4, radius - 4, 128);

    setColor(Color(frame.r * 0.9f, frame.g * 0.9f, frame.b * 0.95f, 1));
    ring(0, 0, radius - 18, radius - 21, 128);

    for (int i = 0; i < 12; ++i) {
        float a = i * (2.0f * PI / 12.0f);
        float x = cosf(a) * radius;
        float y = sinf(a) * radius;

        setColor(spoke);
        line(0, 0, x, y, 2);

        float x2 = cosf(a) * (radius - 12);
        float y2 = sinf(a) * (radius - 12);
        line(x2, y2, x, y, 1);

        setColor(mixColor(Color(0.70f, 0.85f, 1.0f, 1),
                          Color(1.0f, 0.88f, 0.55f, 1),
                          visibleLights() * 0.6f));
        circle(x, y, 8, 24);

        setColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
        midpointCircle(x, y, 8);
    }

    setColor(frame);
    circle(0, 0, 10, 24);

    setColor(Color(0.92f, 0.95f, 0.98f, 1));
    circle(0, 0, 4, 16);

    setColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    midpointCircle(0, 0, 10);

    glPopMatrix();

    setColor(frame);
    line(cx, cy - radius, cx - 48, 220, 4);
    line(cx, cy - radius, cx + 48, 220, 4);

    rect(cx - 45, 210, cx + 45, 225);
}

void drawBuilding(float x, float y, float w, float h, const Color& body, int rows, int cols, float roof = 0.0f) {
    setColor(body);
    rect(x, y, x + w, y + h);
    if (roof > 0.0f) {
        setColor(Color(body.r * 0.8f, body.g * 0.8f, body.b * 0.8f, body.a));
        triangle(x, y + h, x + w / 2.0f, y + h + roof, x + w, y + h);
    }

    float padX = w * 0.12f;
    float padY = h * 0.08f;
    float gapX = (w - 2 * padX) / cols;
    float gapY = (h - 2 * padY) / rows;
    float lightA = visibleLights();

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            float wx = x + padX + c * gapX + gapX * 0.18f;
            float wy = y + padY + r * gapY + gapY * 0.15f;
            float ww = gapX * 0.44f;
            float wh = gapY * 0.52f;

            bool litPattern = ((r + c + ((int)x / 13)) % 3 != 0);
            Color off(0.10f, 0.13f, 0.18f, 1);
            Color on(1.0f, 0.82f, 0.40f, 1);
            setColor(litPattern ? mixColor(off, on, lightA) : off);
            rect(wx, wy, wx + ww, wy + wh);
        }
    }
}

void drawForegroundBuildings() {
    Color b1 = mixColor(Color(0.65f, 0.58f, 0.62f, 1), Color(0.11f, 0.12f, 0.16f, 1), dayBlend);
    Color b2 = mixColor(Color(0.74f, 0.68f, 0.72f, 1), Color(0.13f, 0.14f, 0.19f, 1), dayBlend);
    Color b3 = mixColor(Color(0.58f, 0.61f, 0.70f, 1), Color(0.09f, 0.10f, 0.14f, 1), dayBlend);

    drawBuilding(0, 320, 145, 225, b1, 5, 4, 0);
    drawBuilding(300, 320, 125, 185, b2, 4, 3, 18);
    drawBuilding(420, 320, 155, 250, b3, 6, 4, 0);
    drawBuilding(575, 320, 140, 205, b1, 5, 4, 14);
    drawBuilding(1095, 320, 105, 165, b2, 4, 3, 12);
    drawBuilding(1198, 320, 120, 195, b3, 5, 3, 0);
    drawBuilding(1316, 320, 84, 150, b1, 4, 2, 10);
}

void drawRoadAndWalkway() {
    Color pavement = mixColor(Color(0.70f, 0.68f, 0.66f, 1), Color(0.18f, 0.18f, 0.19f, 1), dayBlend);
    Color road = mixColor(Color(0.18f, 0.20f, 0.22f, 1), Color(0.08f, 0.08f, 0.10f, 1), dayBlend);
    Color lane = mixColor(Color(0.92f, 0.89f, 0.62f, 1), Color(0.80f, 0.74f, 0.45f, 1), dayBlend);

    rect(0, 130, WINDOW_W, 150);
    setColor(pavement);
    rect(0, 130, WINDOW_W, 210);
    setColor(road);
    rect(0, 0, WINDOW_W, 130);

    setColor(lane);
    for (int i = 0; i < WINDOW_W; i += 90)
        rect(i + 15, 60, i + 60, 67);
}

void drawTrees() {
    for (int i = 0; i < 6; ++i) {
        float x = 460 + i * 110;
        setColor(Color(0.33f, 0.20f, 0.10f, 1));
        rect(x, 150, x + 10, 190);
        Color leaves = mixColor(Color(0.17f, 0.52f, 0.22f, 1), Color(0.08f, 0.20f, 0.10f, 1), dayBlend);
        setColor(leaves);
        circle(x + 5, 202, 22, 32);
        circle(x - 10, 194, 16, 24);
        circle(x + 20, 194, 16, 24);
    }
}

void drawLampPost(float x, float y) {
    float lampA = visibleLights();
    setColor(Color(0.16f, 0.16f, 0.18f, 1));

    rect(x, y, x + 6, y + 78);
    rect(x - 6, y + 78, x + 12, y + 84);
    rect(x + 5, y + 78, x + 24, y + 74);
    rect(x + 20, y + 68, x + 28, y + 78);

    if (lampA > 0.01f) {
        glow(x + 24, y + 68, 10, Color(1.0f, 0.86f, 0.46f, 0.26f * lampA));
        setColor(Color(1.0f, 0.90f, 0.55f, 0.95f * lampA));
        circle(x + 24, y + 68, 4, 20);
    }
}

void drawStreetFurniture() {
    for (int i = 0; i < 5; ++i) drawLampPost(90 + i * 250, 150);

    drawLampPost(1340, 150);

    setColor(Color(0.36f, 0.22f, 0.08f, 1));
    rect(980, 160, 1040, 168);
    rect(982, 174, 1038, 180);
    rect(990, 150, 996, 174);
    rect(1024, 150, 1030, 174);

    setColor(Color(0.05f, 0.14f, 0.55f, 1));
    rect(1112, 150, 1118, 235);
    setColor(Color(0.85f, 0.08f, 0.10f, 1));
    ring(1115, 244, 24, 13, 72);
    setColor(Color(0.02f, 0.20f, 0.64f, 1));
    rect(1095, 238, 1135, 250);
}

void drawBusStop() {
    setColor(Color(0.18f, 0.18f, 0.20f, 1));
    rect(265, 150, 270, 235);
    rect(355, 150, 360, 235);
    rect(258, 230, 367, 238);
    setColor(Color(0.60f, 0.75f, 0.95f, 0.35f));
    rect(272, 160, 353, 226);
    setColor(Color(0.83f, 0.12f, 0.12f, 1));
    circle(312, 255, 12, 32);
    setColor(Color(1, 1, 1, 1));
    rect(302, 251, 322, 259);
}

void drawVehicle(const Vehicle &v) {
    float x = v.x, y = v.y, w = v.width, h = v.height;

    setColor(v.body);
    rect(x, y, x + w, y + h * 0.72f);

    if (v.bus) {
        rect(x + 18, y + h * 0.72f, x + w - 12, y + h);
        setColor(Color(0.65f, 0.88f, 1.0f, 0.8f));
        for (int i = 0; i < 5; ++i) rect(x + 22 + i * 27, y + h * 0.79f, x + 42 + i * 27, y + h * 0.95f);
        for (int i = 0; i < 5; ++i) rect(x + 18 + i * 27, y + h * 0.22f, x + 38 + i * 27, y + h * 0.52f);
        setColor(Color(0.92f, 0.92f, 0.92f, 1));
        rect(x + w - 32, y + 8, x + w - 10, y + 40);
    } else {
        polygon({{x + 14, y + h * 0.72f}, {x + 40, y + h}, {x + 82, y + h}, {x + w - 8, y + h * 0.72f}});
        setColor(Color(0.65f, 0.88f, 1.0f, 0.8f));
        rect(x + 25, y + h * 0.72f, x + 48, y + h * 0.92f);
        rect(x + 52, y + h * 0.72f, x + 78, y + h * 0.92f);
    }

    float lightA = v.dirRight ? 1.0f : 0.35f;
    setColor(Color(1.0f, 0.95f, 0.6f, 1));
    rect(x + w - 4, y + 16, x + w, y + 24);
    setColor(Color(0.85f, 0.10f, 0.10f, 1));
    rect(x, y + 16, x + 4, y + 24);

    if (dayBlend > 0.55f) {
        glow(x + w + 2, y + 20, 10, Color(1.0f, 0.95f, 0.7f, 0.18f * lightA));
    }

    setColor(Color(0.08f, 0.08f, 0.08f, 1));
    circle(x + 28, y, 13, 32);
    circle(x + w - 28, y, 13, 32);
    setColor(Color(0.65f, 0.65f, 0.65f, 1));
    circle(x + 28, y, 5, 20);
    circle(x + w - 28, y, 5, 20);
}

void drawBoat(const Boat &b) {
    float x = b.x;
    float y = b.y;

    float s = 0.6f;

    setColor(Color(1, 1, 1, 0.12f));
    for (int i = 0; i < 4; ++i) {
        float wy = y - 2 - i * 4;
        float wx = x - 8 - i * 10;
        rect(wx, wy, wx + 18 - i * 2, wy + 2);
    }

    setColor(Color(0.05f, 0.05f, 0.08f, 1));
    polygon({
        {x, y},
        {x + 140*s, y},
        {x + 160*s, y + 15*s},
        {x + 20*s, y + 15*s}
    });

    setColor(Color(0.92f, 0.92f, 0.95f, 1));
    rect(x + 20*s, y + 15*s, x + 150*s, y + 40*s);

    setColor(Color(0.55f, 0.75f, 0.95f, 0.85f));
    rect(x + 25*s, y + 25*s, x + 145*s, y + 38*s);

    setColor(Color(0.85f, 0.85f, 0.88f, 1));
    rect(x + 25*s, y + 40*s, x + 145*s, y + 48*s);

    setColor(Color(0.9f, 0.9f, 0.95f, 1));
    rect(x + 110*s, y + 40*s, x + 145*s, y + 60*s);

    setColor(Color(0.5f, 0.7f, 0.9f, 0.9f));
    rect(x + 115*s, y + 45*s, x + 140*s, y + 58*s);

    setColor(Color(0.85f, 0.1f, 0.1f, 1));
    rect(x + 25*s, y + 18*s, x + 145*s, y + 22*s);

    setColor(Color(0.85f, 0.1f, 0.1f, 1));
    triangle(x + 140*s, y + 65*s, x + 165*s, y + 55*s, x + 140*s, y + 45*s);

    setColor(Color(1, 1, 1, 0.08f));
    rect(x + 10*s, y - 4, x + 150*s, y);
}

void drawRain() {
    if (!rainEnabled) return;

    glEnable(GL_BLEND);

    setColor(Color(0.75f, 0.85f, 1.0f, 0.25f + 0.15f * dayBlend));
    glLineWidth(1.5f);

    glBegin(GL_LINES);
    for (int i = 0; i < 550; ++i) {
        float speed = 220.0f;

        float x = fmodf(i * 47.0f + gTime * speed, WINDOW_W + 100.0f) - 50.0f;
        float y = fmodf(i * 83.0f - gTime * speed * 0.9f, WINDOW_H + 100.0f);

        glVertex2f(x, y);
        glVertex2f(x - 4, y - 10);
    }
    glEnd();
}

void drawFog() {
    if (!fogEnabled) return;

    glEnable(GL_BLEND);

    float strength = 0.035f + dayBlend * 0.05f + (rainEnabled ? 0.02f : 0.0f);

    gradientRect(0, 120, WINDOW_W, 430,
        Color(0.88f, 0.91f, 0.95f, strength),
        Color(0.88f, 0.91f, 0.95f, 0.0f));

    gradientRect(0, 150, WINDOW_W, 520,
        Color(0.90f, 0.93f, 0.96f, strength * 0.7f),
        Color(0.90f, 0.93f, 0.96f, 0.0f));

    gradientRect(0, 220, WINDOW_W, 620,
        Color(0.92f, 0.94f, 0.97f, strength * 0.35f),
        Color(0.92f, 0.94f, 0.97f, 0.0f));

    for (int i = 0; i < 14; ++i) {
        float y = 135 + i * 22;
        float shift = fmodf(gTime * (2.0f + i * 0.15f) + i * 31.0f, 240.0f);

        setColor(Color(0.92f, 0.94f, 0.97f, 0.012f + 0.003f * (i % 4)));
        rect(-260 + shift, y, WINDOW_W + shift, y + 24);
    }
}

void drawSnow() {
    if (!snowEnabled) return;

    glEnable(GL_BLEND);
    setColor(Color(1.0f, 1.0f, 1.0f, 0.85f));

    for (int i = 0; i < 550; ++i) {
        float x = fmodf(i * 53.0f + gTime * 18.0f + sinf(i * 0.7f + gTime) * 12.0f,
                        WINDOW_W + 60.0f) - 30.0f;

        float y = fmodf(i * 91.0f - gTime * 60.0f,
                        WINDOW_H + 80.0f);

        float r = 1.5f + (i % 3) * 0.6f;
        circle(x, y, r, 10);
    }
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glPushMatrix();
    glTranslatef(-cameraX, 0, 0);

    drawSky();
    drawClouds();
    drawDistantCity();
    drawThames();
    drawBridge();

    if (boatEnabled) {
        for (const auto &b : boats) drawBoat(b);
    }

    drawBigBen();
    drawForegroundBuildings();
    drawLondonEye();
    drawRoadAndWalkway();
    drawTrees();
    drawStreetFurniture();
    drawBusStop();

    if (trafficEnabled) {
        for (const auto &v : vehicles) drawVehicle(v);
    } else {
        for (auto v : vehicles) {
            v.speed = 0;
            drawVehicle(v);
        }
    }

    drawRain();
    drawFog();
    drawSnow();

    glPopMatrix();

    glutSwapBuffers();
}

void update(int) {
    gTime += 0.016f;

    eyeRotation += 0.2f;

    if (eyeRotation > 360.0f)
    eyeRotation -= 360.0f;

    clockHourAngle   = lerpf(30.0f, 250.0f, dayBlend);
    clockMinuteAngle = lerpf(60.0f, 120.0f, dayBlend);

    dayBlend = lerpf(dayBlend, targetDayBlend, 0.015f);

    for (auto &c : clouds) {
        c.x += c.speed * (rainEnabled ? 1.4f : 1.0f);
        if (c.x > WINDOW_W + 120) c.x = -150;
    }

    if (boatEnabled) {
        for (auto &b : boats) {
            b.x += b.speed;
            if (b.x > WINDOW_W + 120) b.x = -120;
        }
    }

if (trafficEnabled) {
    for (size_t i = 0; i < vehicles.size(); ++i) {
        auto &v = vehicles[i];
        float currentSpeed = v.speed;

        if (i == 0 && v.x > 200 && v.x < 280) currentSpeed = 0.25f;
        if (i == 0 && v.x >= 280 && v.x < 325) currentSpeed = 0.03f;
        if (i == 0 && v.x >= 325) currentSpeed = v.speed;

        if (!v.bus) {
            currentSpeed += 0.08f * sin(gTime * 2.0f + i);
        }

        if (v.dirRight) {
            v.x += currentSpeed;
            if (v.x > WINDOW_W + 220) v.x = -220;
        } else {
            v.x -= currentSpeed;
            if (v.x < -220) v.x = WINDOW_W + 220;
        }
    }
}

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int, int) {
    switch (key) {
        case 'd': case 'D': targetDayBlend = 0.0f; break;
        case 'n': case 'N': targetDayBlend = 1.0f; break;
        case 't': case 'T': trafficEnabled = !trafficEnabled; break;
        case 'l': case 'L': lightsForced = !lightsForced; break;
        case 'r': case 'R':
    rainEnabled = !rainEnabled;

    if (rainEnabled) {
        snowEnabled = false;
        fogEnabled  = false;
    }
    break;

case 's': case 'S':
    snowEnabled = !snowEnabled;

    if (snowEnabled) {
        rainEnabled = false;
        fogEnabled  = false;
    }
    break;

case 'f': case 'F':
    fogEnabled = !fogEnabled;

    if (fogEnabled) {
        rainEnabled = false;
        snowEnabled = false;
    }
    break;
        case 27: std::exit(0); break;
    }
    glutPostRedisplay();
}


void specialKeys(int key, int, int) {
    switch (key) {
        case GLUT_KEY_LEFT:  cameraX = clampf(cameraX - 18, 0, 200); break;
        case GLUT_KEY_RIGHT: cameraX = clampf(cameraX + 18, 0, 200); break;
        case GLUT_KEY_UP:    targetDayBlend = clampf(targetDayBlend - 0.08f, 0, 1); break;
        case GLUT_KEY_DOWN:  targetDayBlend = clampf(targetDayBlend + 0.08f, 0, 1); break;
    }
    glutPostRedisplay();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    setup2D();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WINDOW_W, WINDOW_H);
    glutCreateWindow("London Metropolis");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glPointSize(1.0f);

    setup2D();
    initScene();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutReshapeFunc(reshape);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
