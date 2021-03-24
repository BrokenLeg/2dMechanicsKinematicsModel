#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>

const float g = 200, DT = 0.5;
class Vector2f {
public:
    float x, y, Length;

    Vector2f() : x(0), y(0), Length(0) {};
    Vector2f(float _x, float _y) : x(_x), y(_y), Length(sqrt(_x*_x + _y*_y)) {};
    Vector2f(const Vector2f& u) : x(u.x), y(u.y), Length(u.Length) {};
    Vector2f operator + (Vector2f u) {
        return Vector2f(this->x + u.x, this->y + u.y);
    }
    Vector2f(const Vector2f& u, const Vector2f& v) : x(u.x - v.x), y(u.y - v.y){
        Length = sqrt(x * x + y * y);
    };

    Vector2f operator * (float k) {
        return Vector2f(k*this->x, k*this->y);
    }

    float operator * (Vector2f u) {
        return this->x * u.x + this->y * u.y;
    }

    float operator ^ (Vector2f u) {
        return this->x * u.y - this->y * u.x;
    }

    friend std::ostream& operator << (std::ostream& os, const Vector2f& u) {
        os << u.x << " " << u.y <<std::endl;
        return os;
    }

    void Normalize() {
        x = x / Length;
        y = y / Length;
        Length = 1.0f;
    }

    Vector2f Normalized() {
        return Vector2f(x / Length, y / Length);
    }
};

Vector2f operator * (float k, const Vector2f& u) {
    return Vector2f(k * u.x, k * u.y);
}

Vector2f NullV(0, 0);

Vector2f operator * (const Vector2f& u, float k) {
    return Vector2f(k * u.x, k * u.y);
}

class SurfaceLine {
public:
    Vector2f Begin, End, Along, N;
    float AngleSin, AngleTan;

    SurfaceLine(): AngleSin(0) {};

    SurfaceLine(float x1, float y1, float x2, float y2) : Begin(Vector2f(x1, y1)),
        End(Vector2f(x2, y2)) {
        Along = Vector2f(End, Begin);
        Along.Normalize();

        N = Vector2f((-1) * Along.y, Along.x);
        
        float dx = x2 - x1, dy = y1 - y2;

        AngleSin = dy / sqrt(dx*dx + dy*dy);
    };
};

class Body {
public:
    Vector2f Position, Velocity, Acceleration, Force;
    float R, m;

    Body(Vector2f Pos, Vector2f Vel, Vector2f Acc, float r, float _m) : Position(Pos),
        Velocity(Vel),  Acceleration(Acc), R(r), m(_m), Force(Vector2f(0, _m*-g)){};

    void move(float dt) {
        //Acceleration = Force * (1 / m);
        Velocity = Velocity + Acceleration * dt;
        Position = Position + Velocity * dt;
    }

    float sgetDistance(const SurfaceLine& Surface) {
        Vector2f AP(this->Position, Surface.Begin);
        float h = abs(AP ^ Surface.Along);
        return h;
    }

    bool sIntersects(const SurfaceLine& Surface) {
        return this->sgetDistance(Surface) <= this->R;
    }

    void ReactSurface(const SurfaceLine& Surface) {

        float dh = (this->R - this->sgetDistance(Surface));

        this->Position = this->Position + dh * Surface.N ;
        this->Velocity = Surface.Along * (this->Velocity * Surface.Along) ;
        this->Acceleration = Surface.Along * (Surface.AngleSin * g);
    }

    bool bIntersects(const Body& b) {
        Vector2f D(b.Position, this->Position);
        return D.Length <= this->R + b.R;
    }
};

class Model {
public:
    std::vector <SurfaceLine> lines;
    Body body;

    Model(const std::vector<SurfaceLine>& Surfaces, const Body& b) : lines(Surfaces),
        body(b){};

    void run(float dt) {
        body.move(dt);

        for (SurfaceLine Surface : lines) {
            if (body.sIntersects(Surface)) {
                body.ReactSurface(Surface);
            }
        }

    }

};

void drawTerrain(sf::RenderWindow* w, const std::vector<SurfaceLine>& lines) {
    for (SurfaceLine s : lines) {

        sf::Vertex line[2] =
        {
            sf::Vertex(sf::Vector2f(s.Begin.x + 400, -s.Begin.y + 300)),
            sf::Vertex(sf::Vector2f(s.End.x + 400, -s.End.y + 300))
        };

        w->draw(line, 2, sf::Lines);

    }
}


int main()
{
    
    Body b(Vector2f(0, 200), NullV, Vector2f(0, -g), 25, 10), b1(Vector2f(0, 2), NullV, NullV, 0, 1);
    SurfaceLine s(-300, 100, 100, -50);
    SurfaceLine s1(100, -50, 200, -50);
    SurfaceLine s2(200, -50, 300, 50);
    std::vector<SurfaceLine> terrain = {s, s1, s2};

    Model M(terrain, b);

    sf::RenderWindow window(sf::VideoMode(800, 600), "Physics");
    window.setFramerateLimit(60);


    sf::CircleShape circle(M.body.R);
    circle.setOrigin(M.body.R, M.body.R);

    sf::Clock clock;
   
 
    while (window.isOpen()) {

        sf::Event event;
        while (window.pollEvent(event))
        {
            // Request for closing the window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        float Dt = clock.getElapsedTime().asSeconds();
        clock.restart();

        M.run(Dt);

        window.clear();

        circle.setPosition(M.body.Position.x + 400, -M.body.Position.y + 300);

        window.draw(circle);
        drawTerrain(&window, M.lines);
        window.display();
    }

}
