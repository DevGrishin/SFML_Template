#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <font_data.h>
#include <random>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>


class Player: public sf::Sprite{
    public:
        sf::Vector2f velocity;
        sf::RectangleShape collisionBox;
        bool onGround;

        float accel = 6000;
        float maxSpeed = 150;
        float jumpStrength = 480;
        float friction = 3000;

        Player(const sf::Texture& texture) : sf::Sprite(texture) {
            setScale(sf::Vector2f(2,2));
            velocity = {0.0f, 0.0f};
            onGround = false;
            collisionBox.setSize(sf::Vector2f(getGlobalBounds().size.x-10,getGlobalBounds().size.y));
            collisionBox.setPosition(sf::Vector2f(getPosition().x+5, getPosition().y));
        }

        void shift(sf::Vector2f offset){
            move(offset);
            collisionBox.move(offset);
        }

        void setPos(sf::Vector2f position){
            setPosition(position);
            collisionBox.setPosition(sf::Vector2f(position.x+5, position.y));
        }

        void applyGravity(float g, float dt){
            if (!onGround) velocity.y += g * dt;
            // else velocity.y = 0;
        }

        void jump(){
                velocity.y = -jumpStrength;
                onGround = false;
        }

        void applyHorizontalInput(int dir, float dt){
            // dir: -1 left, 1 right
            if(dir != 0){
                velocity.x += dir * accel * dt;
                velocity.x = std::clamp(velocity.x, -maxSpeed, maxSpeed);
            } else {
                // Friction
                if (std::abs(velocity.x) > 0.f){
                    float sign = velocity.x > 0 ? 1.f : -1.f;
                    velocity.x -= sign * friction * dt;
                    if (sign > 0 && velocity.x < 0) velocity.x = 0;
                    if (sign < 0 && velocity.x > 0) velocity.x = 0;
                }
            }
        }
};

bool colliding(sf::RectangleShape &rec1, const sf::RectangleShape &rec2){
    sf::FloatRect a = rec1.getGlobalBounds();
    sf::FloatRect b = rec2.getGlobalBounds();
    return a.findIntersection(b) != std::nullopt;
}

void resolveCollision(Player &rec1, const sf::RectangleShape &rec2){
    sf::Vector2f pos1 = rec1.collisionBox.getPosition();
    sf::Vector2f pos2 = rec2.getPosition();
    sf::Vector2f size1 = rec1.collisionBox.getSize();
    sf::Vector2f size2 = rec2.getSize();
    
    // Calculate penetration
    float penX = std::min(pos1.x + size1.x, pos2.x + size2.x) - std::max(pos1.x, pos2.x);
    float penY = std::min(pos1.y + size1.y, pos2.y + size2.y) - std::max(pos1.y, pos2.y);    

    // Only resolve on axis where we're moving into the collision
    bool resolveX = false;
    bool resolveY = false;

    if (penX < penY){
        // Check if moving toward collision on X axis
        if ((pos1.x < pos2.x && rec1.velocity.x > 0) || 
            (pos1.x > pos2.x && rec1.velocity.x < 0)) {
            resolveX = true;
        }
    }
    else {
        // Check if moving toward collision on Y axis
        if ((pos1.y < pos2.y && rec1.velocity.y > 0) || 
            (pos1.y > pos2.y && rec1.velocity.y < 0)) {
            resolveY = true;
        }
    }

    if (resolveX){
        if (pos1.x < pos2.x)
            pos1.x = pos2.x - size1.x;
        else
            pos1.x = pos2.x + size2.x;
        
        rec1.velocity.x = 0.f;
    }
    
    if (resolveY) {
        if (pos1.y < pos2.y){
            pos1.y = pos2.y - size1.y;
            rec1.onGround = true;
        }
        else
            pos1.y = pos2.y + size2.y;

        rec1.velocity.y = 0;
    }
    
    rec1.collisionBox.setPosition(pos1);
}





int main() {
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    unsigned int windowWidth = 1050;
    unsigned int windowHeight = 650;
    sf::RenderWindow window(sf::VideoMode({windowWidth,windowHeight}), "Pico Park", sf::Style::Default);
    window.setVerticalSyncEnabled(true);

    float dt;
    sf::Clock clock;
    sf::Texture test;
    if (!test.loadFromFile("sprites\\Blue Cat.png")){
        exit(0);
    }
    
    Player player(test);
    Player player2(test);
    player2.setPos(sf::Vector2f(110,500));

    sf::RectangleShape ground(sf::Vector2f(windowWidth,5));
    ground.setPosition(sf::Vector2f(0,windowHeight-5));
    const float MAX_DT = 1.0f/60.0f;
    float gravity = 1500;

    std::vector<sf::RectangleShape*> collisionList = {&ground};
    std::vector<Player*> players = {&player, &player2};

    sf::Font font;
    if (!font.openFromMemory(arial_ttf, arial_ttf_len )) {
        std::cerr << "Failed to load font\n";
        return -1;
    }

    sf::Text grav(font);
    grav.setPosition(sf::Vector2f(windowWidth-200,5));
    grav.setCharacterSize(24);

    bool jumpKeyLast = false;
    const float COYOTE_TIME = 0.08f;
    float coyoteTimer = 0.f;
    const float JUMP_BUFFER_TIME = 0.10f;
    float jumpBufferTimer = 0.f;


    while (window.isOpen())
    {
        dt = clock.restart().asSeconds();
        dt = std::min(dt, MAX_DT);

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                // if (keyPressed->scancode == sf::Keyboard::Scancode::W)
                //     player.jump();
            }

        }
        int dir = 0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)){
            dir += -1;
        } 
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)){
            dir += 1;  
        } 

        {
            bool jumpKey = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W);
            if (jumpKey && !jumpKeyLast) {
                jumpBufferTimer = JUMP_BUFFER_TIME;
            }
            jumpKeyLast = jumpKey;
        }

        // player.applyGravity(gravity, dt);
        // player.applyHorizontalInput(dir, dt);
        // player.shift(player.velocity * dt);
        
        for (Player* p: players){
            p->applyGravity(gravity, dt);
            if (p != &player) p->applyHorizontalInput(0, dt);
            else p->applyHorizontalInput(dir, dt);
            p->shift(p->velocity * dt);

            p->onGround = false;
            
            for (sf::RectangleShape* entity: collisionList){
                if (colliding(p->collisionBox, *entity)){
                    resolveCollision(*p, *entity);
                    p->setPosition(sf::Vector2f(p->collisionBox.getPosition().x-5, p->collisionBox.getPosition().y));
                    // p->onGround = true;

                }
            }
            for (Player* p2: players){
                if (p == p2) continue;
                if (colliding(p->collisionBox, p2->collisionBox)){
                    resolveCollision(*p, p2->collisionBox);
                    p->setPosition(sf::Vector2f(p->collisionBox.getPosition().x-5, p->collisionBox.getPosition().y));
                    // p->onGround = true;
                }
            }
        }

        if (player.onGround) coyoteTimer = COYOTE_TIME;
        else coyoteTimer = std::max(0.f, coyoteTimer - dt);
        jumpBufferTimer = std::max(0.f, jumpBufferTimer - dt);

        if ((player.onGround || coyoteTimer > 0.f) && jumpBufferTimer > 0.f) {
            player.jump();
            jumpBufferTimer = 0.f;
            coyoteTimer = 0.f;
        }
        
        
        grav.setString(std::to_string(player.velocity.y));

        window.clear(sf::Color{211, 185, 87, 255});
        
        window.draw(player);
        window.draw(player2);
        window.draw(ground);
        
        window.draw(grav);

        // window.draw(player.collisionBox);
        // window.draw(player2.collisionBox);

        window.display();


    }
    return 0;
}


