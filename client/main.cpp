#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <iostream>
#include <vector>

class Background {
private:
    sf::Texture texture;
    sf::Sprite sprite1;
    sf::Sprite sprite2;
    float scrollSpeed;

public:
    Background() : scrollSpeed(200.0f) {}

    bool loadTexture(const std::string& path) { return texture.loadFromFile(path); }

    void init(sf::Vector2u windowSize) {
        sprite1.setTexture(texture);
        sprite2.setTexture(texture);

        sf::Vector2u textureSize = texture.getSize();
        float scale = static_cast<float>(windowSize.y) / textureSize.y;

        sprite1.setScale(scale, scale);
        sprite2.setScale(scale, scale);

        sprite1.setPosition(0, 0);
        sprite2.setPosition(sprite1.getGlobalBounds().width, 0);
    }

    void animate(float deltaTime) {
        sprite1.move(-scrollSpeed * deltaTime, 0);
        sprite2.move(-scrollSpeed * deltaTime, 0);

        if (sprite1.getPosition().x + sprite1.getGlobalBounds().width < 0) {
            sprite1.setPosition(sprite2.getPosition().x + sprite2.getGlobalBounds().width, 0);
        }
        if (sprite2.getPosition().x + sprite2.getGlobalBounds().width < 0) {
            sprite2.setPosition(sprite1.getPosition().x + sprite1.getGlobalBounds().width, 0);
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite1);
        window.draw(sprite2);
    }

    float getScrollSpeed() const { return scrollSpeed; }
    void setScrollSpeed(float speed) { scrollSpeed = speed; }
};

class Player {
private:
    sf::Texture texture;
    sf::Sprite sprite;
    sf::RectangleShape hitbox;
    float speed;
    float animationTime;
    int currentColumn;      // Colonne actuelle (0-4, où 2 = centre)
    int targetColumn;       // Colonne cible à atteindre
    int playerLine;         // Ligne du joueur dans la spritesheet
    float transitionSpeed;  // Vitesse de transition entre colonnes

    int SPRITE_WIDTH = 33;
    int SPRITE_HEIGHT = 17;
    int CENTER_COLUMN = 2;  // Colonne centrale (position neutre)

public:
    Player()
        : speed(500.0f), animationTime(0.0f), currentColumn(2), targetColumn(2), playerLine(0),
          transitionSpeed(0.15f) {}

    bool loadTexture(const std::string& path) { return texture.loadFromFile(path); }

    void init(int line = 0) {
        sprite.setTexture(texture);
        playerLine = line;
        currentColumn = 2;
        targetColumn = 2;

        // Position de départ : colonne centrale (2)
        sf::IntRect rect(33 * 2, playerLine * 17, 33, 17);
        sprite.setTextureRect(rect);

        sprite.setScale(3.0f, 3.0f);
        sprite.setPosition(100, 400);

        hitbox.setSize(sf::Vector2f(33 * 3.0f, 17 * 3.0f));
        hitbox.setFillColor(sf::Color::Transparent);
        hitbox.setOutlineColor(sf::Color::Red);
        hitbox.setOutlineThickness(2);
        hitbox.setPosition(sprite.getPosition());
    }

    void animate(float deltaTime, bool movingUp, bool movingDown) {
        // Déterminer la colonne cible selon les inputs
        if (movingUp) {
            targetColumn = 4;  // Colonne la plus à droite (incliné vers le haut)
        } else if (movingDown) {
            targetColumn = 0;  // Colonne la plus à gauche (incliné vers le bas)
        } else {
            targetColumn = 2;  // Retour au centre
        }

        // Transition progressive vers la colonne cible
        if (currentColumn != targetColumn) {
            animationTime += deltaTime;

            if (animationTime >= transitionSpeed) {
                animationTime = 0.0f;

                // Avancer d'une colonne vers la cible
                if (currentColumn < targetColumn) {
                    currentColumn++;
                } else if (currentColumn > targetColumn) {
                    currentColumn--;
                }

                // Mettre à jour le sprite
                sf::IntRect rect(33 * currentColumn, playerLine * 17, 33, 17);
                sprite.setTextureRect(rect);
            }
        }
    }

    void move(float deltaTime, sf::Vector2u windowSize) {
        float moveX = 0.0f;
        float moveY = 0.0f;
        bool movingUp = false;
        bool movingDown = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            moveY = -speed * deltaTime;
            movingUp = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            moveY = speed * deltaTime;
            movingDown = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            moveX = -speed * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            moveX = speed * deltaTime;

        // Animer le vaisseau en fonction du mouvement vertical
        animate(deltaTime, movingUp, movingDown);

        sf::Vector2f pos = sprite.getPosition();
        pos.x += moveX;
        pos.y += moveY;

        sf::FloatRect bounds = sprite.getGlobalBounds();
        if (pos.x < 0)
            pos.x = 0;
        if (pos.y < 0)
            pos.y = 0;
        if (pos.x + bounds.width > windowSize.x)
            pos.x = windowSize.x - bounds.width;
        if (pos.y + bounds.height > windowSize.y)
            pos.y = windowSize.y - bounds.height;

        sprite.setPosition(pos);
        hitbox.setPosition(pos);
    }

    void draw(sf::RenderWindow& window, bool drawHitbox = false) {
        window.draw(sprite);
        if (drawHitbox)
            window.draw(hitbox);
    }

    // Getters
    sf::Vector2f getPosition() const { return sprite.getPosition(); }
    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }
    float getSpeed() const { return speed; }

    // Setters
    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
        hitbox.setPosition(x, y);
    }
    void setSpeed(float s) { speed = s; }
};

class ShootEffect {
private:
    sf::Sprite sprite;
    float animationTime;
    float frameTime;
    int currentFrame;
    bool finished;
    sf::IntRect rect = {212, 80, 16, 16};
    sf::Vector2f offsetFromPlayer;

public:
    ShootEffect() : animationTime(0.0f), frameTime(0.05f), currentFrame(0), finished(false) {}

    void init(sf::Vector2f position, sf::Texture& texture, sf::Vector2f playerPos) {
        sprite.setTexture(texture);
        sprite.setTextureRect(rect);
        sprite.setScale(2.0f, 2.0f);
        sprite.setPosition(position);
        offsetFromPlayer = position - playerPos;
    }

    void update(float deltaTime, sf::Vector2f playerPos) {
        if (finished)
            return;
        sprite.setPosition(playerPos + offsetFromPlayer);
        animationTime += deltaTime;

        if (animationTime >= frameTime) {
            animationTime = 0.0f;
            currentFrame++;

            if (currentFrame >= 2) {
                finished = true;
            } else {
                sprite.setTextureRect(rect);
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        if (!finished)
            window.draw(sprite);
    }

    bool isFinished() const { return finished; }
};

class ChargedShootEffect {
private:
    sf::Sprite sprite;
    float animationTime;
    float frameTime;
    int currentFrame;
    bool finished;
    sf::IntRect baseRect = {0, 50, 29, 35};  // Position de départ sur la ligne 3
    sf::Vector2f offsetFromPlayer;

public:
    ChargedShootEffect()
        : animationTime(0.0f), frameTime(0.08f), currentFrame(0), finished(false) {}

    void init(sf::Vector2f position, sf::Texture& texture, sf::Vector2f playerPos) {
        sprite.setTexture(texture);
        sprite.setTextureRect(baseRect);
        sprite.setScale(2.5f, 2.5f);
        sprite.setPosition(position);
        offsetFromPlayer = position - playerPos;
        currentFrame = 0;
        finished = false;
    }

    void update(float deltaTime, sf::Vector2f playerPos) {
        sprite.setPosition(playerPos + offsetFromPlayer);

        animationTime += deltaTime;

        if (animationTime >= frameTime) {
            animationTime = 0.0f;
            currentFrame++;

            if (currentFrame >= 8)  // 8 frames d'animation (0 à 7)
            {
                currentFrame = 0;  // Remettre à zéro pour boucler
                finished = true;   // Marquer comme terminé une fois (pour getChargeLevel)
            }

            // Avancer dans l'animation (espacement de 34 pixels entre chaque frame)
            sf::IntRect newRect = baseRect;
            newRect.left = baseRect.left + (currentFrame * 34);
            sprite.setTextureRect(newRect);
        }
    }

    void draw(sf::RenderWindow& window) { window.draw(sprite); }

    bool isFinished() const { return finished; }
    int getCurrentFrame() const { return currentFrame; }
    bool isFullyCharged() const {
        return currentFrame >= 5;
    }  // Considéré comme chargé à partir de la frame 6/8

    // Calcule le niveau de charge (1-5) en fonction de la progression de l'animation
    int getChargeLevel() const {
        if (currentFrame < 2)
            return 1;  // Frames 0-1: Niveau 1 (ligne 5)
        else if (currentFrame < 3)
            return 2;  // Frame 2: Niveau 2 (ligne 6)
        else if (currentFrame < 5)
            return 3;  // Frames 3-4: Niveau 3 (ligne 7)
        else if (currentFrame < 6)
            return 4;  // Frame 5: Niveau 4 (ligne 8)
        else
            return 5;  // Frames 6-7: Niveau 5 (ligne 9)
    }
};

class ChargedMissile {
private:
    sf::Sprite sprite;
    sf::RectangleShape hitbox;
    float speed;
    float animationTime;
    float frameTime;
    int currentFrame;
    int chargeLevel;  // 1-5 (lignes 5-9)

    // Rectangles pour chaque niveau de charge (lignes 5 à 9)
    struct ChargeData {
        int xPos;        // Position X de départ dans la spritesheet
        int yPos;        // Position Y (ligne)
        int width;       // Largeur du sprite
        int height;      // Hauteur du sprite
        int frameCount;  // Nombre de frames d'animation
        int frameWidth;  // Espacement entre les frames
    };

    ChargeData chargeLevels[5] = {
        {233, 100, 15, 15, 2, 18},  // Niveau 1 (ligne 5) - 2 frames, espacement 15px
        {202, 117, 31, 15, 2, 32},  // Niveau 2 (ligne 6) - 2 frames, espacement 31px
        {170, 135, 47, 15, 2, 50},  // Niveau 3 (ligne 7) - 2 frames, espacement 47px
        {138, 155, 63, 15, 2, 65},  // Niveau 4 (ligne 8) - 2 frames, espacement 63px
        {105, 170, 79, 17, 2, 81}   // Niveau 5 (ligne 9) - 2 frames, espacement 79px
    };

public:
    ChargedMissile()
        : speed(1500.0f), animationTime(0.0f), frameTime(0.1f), currentFrame(0), chargeLevel(1) {}

    void init(const Player& player, sf::Texture& texture, int level) {
        sprite.setTexture(texture);
        chargeLevel = level;

        // Directement utiliser l'animation du niveau de charge
        ChargeData& data = chargeLevels[chargeLevel - 1];
        sf::IntRect initialRect(data.xPos, data.yPos, data.width, data.height);
        sprite.setTextureRect(initialRect);
        sprite.setScale(3.0f, 3.0f);

        sf::Vector2f playerPos = player.getPosition();
        sf::FloatRect playerBounds = player.getBounds();

        float missileHeight = data.height * 3.0f;
        float missileWidth = data.width * 3.0f;
        float missileX = playerPos.x + playerBounds.width;
        float missileY = playerPos.y + (playerBounds.height / 2.0f) - (missileHeight / 2.0f);

        sprite.setPosition(missileX, missileY);

        // Initialiser la hitbox
        hitbox.setSize(sf::Vector2f(missileWidth, missileHeight));
        hitbox.setFillColor(sf::Color::Transparent);
        hitbox.setOutlineColor(sf::Color::Cyan);
        hitbox.setOutlineThickness(2);
        hitbox.setPosition(missileX, missileY);

        animationTime = 0.0f;
        currentFrame = 0;
    }

    void animate(float deltaTime) {
        animationTime += deltaTime;

        if (animationTime >= frameTime) {
            animationTime = 0.0f;
            currentFrame++;

            // Animation en boucle selon le niveau de charge
            ChargeData& data = chargeLevels[chargeLevel - 1];

            if (currentFrame >= data.frameCount) {
                currentFrame = 0;
            }

            sf::IntRect newRect(data.xPos + (currentFrame * data.frameWidth), data.yPos, data.width,
                                data.height);
            sprite.setTextureRect(newRect);
        }
    }

    void move(float deltaTime) {
        sprite.move(speed * deltaTime, 0);
        hitbox.move(speed * deltaTime, 0);
        animate(deltaTime);
    }

    void draw(sf::RenderWindow& window, bool drawHitbox = false) {
        window.draw(sprite);
        if (drawHitbox)
            window.draw(hitbox);
    }

    sf::Vector2f getPosition() const { return sprite.getPosition(); }
    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }
    const sf::RectangleShape& getHitbox() const { return hitbox; }
    float getSpeed() const { return speed; }
    int getChargeLevel() const { return chargeLevel; }

    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
        hitbox.setPosition(x, y);
    }
    void setSpeed(float s) { speed = s; }
};

class Explosion {
private:
    sf::Sprite sprite;
    sf::Texture* texture;
    float animationTime;
    float frameTime;
    int currentFrame;
    int maxFrames;
    bool finished;

    // Coordonnées exactes de chaque frame d'explosion dans le spritesheet
    struct ExplosionFrame {
        int x;       // Position X dans la spritesheet
        int y;       // Position Y dans la spritesheet
        int width;   // Largeur du sprite
        int height;  // Hauteur du sprite
    };

    // Définir les 6 premières frames de l'explosion
    ExplosionFrame frames[6] = {
        {129, 0, 34, 35},  // Frame 0
        {160, 0, 34, 35},  // Frame 1
        {193, 0, 34, 35},  // Frame 2
        {226, 0, 34, 35},  // Frame 3
        {261, 0, 34, 35},  // Frame 4
        {293, 0, 34, 35}   // Frame 5
    };

public:
    Explosion()
        : animationTime(0.0f), frameTime(0.1f), currentFrame(0), maxFrames(6), finished(false) {}

    void init(sf::Texture& tex, sf::Vector2f position) {
        texture = &tex;
        sprite.setTexture(tex);

        sprite.setScale(2.5f, 2.5f);

        // Afficher la première frame immédiatement
        ExplosionFrame& frame = frames[0];
        sf::IntRect explosionRect(frame.x, frame.y, frame.width, frame.height);
        sprite.setTextureRect(explosionRect);

        // Centrer l'explosion sur la position
        sprite.setPosition(position);

        currentFrame = 0;
        animationTime = 0.0f;
    }

    void update(float deltaTime) {
        if (finished)
            return;

        animationTime += deltaTime;

        if (animationTime >= frameTime) {
            animationTime = 0.0f;

            // Passer à la frame suivante
            currentFrame++;

            // Vérifier si on a dépassé la dernière frame
            if (currentFrame >= maxFrames) {
                // Animation terminée
                finished = true;
                return;  // Important : ne pas essayer d'afficher une frame inexistante
            }

            // Mettre à jour avec la nouvelle frame
            ExplosionFrame& frame = frames[currentFrame];
            sf::IntRect newRect(frame.x, frame.y, frame.width, frame.height);
            sprite.setTextureRect(newRect);
        }
    }

    void draw(sf::RenderWindow& window) {
        if (!finished) {
            window.draw(sprite);
        }
    }

    bool isFinished() const { return finished; }
};

class Enemy {
public:
    enum class MovementPattern {
        STRAIGHT,       // Ligne droite de droite à gauche
        SINE_WAVE,      // Vague sinusoïdale
        ZIGZAG,         // Zigzag haut/bas
        CIRCULAR,       // Mouvement circulaire
        DIAGONAL_DOWN,  // Diagonale descendante
        DIAGONAL_UP     // Diagonale montante
    };

private:
    sf::Sprite sprite;
    sf::RectangleShape hitbox;
    sf::Texture* texture;
    float speed;
    float timeAlive;  // Temps écoulé depuis la création
    MovementPattern pattern;
    sf::Vector2f startPosition;  // Position de départ pour certains patterns
    float amplitude;             // Amplitude pour les mouvements ondulatoires
    float frequency;             // Fréquence pour les mouvements ondulatoires

    // Animation
    float animationTime;
    float frameTime;
    int currentFrame;
    int spriteWidth;
    int spriteHeight;
    int maxFrames;  // Seulement les 8 premières colonnes (partie gauche)

public:
    Enemy()
        : speed(300.0f), timeAlive(0.0f), pattern(MovementPattern::STRAIGHT), amplitude(100.0f),
          frequency(2.0f), animationTime(0.0f), frameTime(0.1f), currentFrame(0), spriteWidth(33),
          spriteHeight(32), maxFrames(8) {}

    void init(sf::Texture& tex, sf::Vector2f position,
              MovementPattern movePattern = MovementPattern::STRAIGHT) {
        texture = &tex;
        sprite.setTexture(tex);

        // Utiliser la première frame (colonne 0) de la partie gauche
        sf::IntRect enemyRect(0, 0, spriteWidth, spriteHeight);
        sprite.setTextureRect(enemyRect);

        sprite.setScale(2.5f, 2.5f);
        sprite.setPosition(position);

        startPosition = position;
        pattern = movePattern;
        timeAlive = 0.0f;
        animationTime = 0.0f;
        currentFrame = 0;

        hitbox.setSize(sf::Vector2f(spriteWidth * 2.5f, spriteHeight * 2.5f));
        hitbox.setFillColor(sf::Color::Transparent);
        hitbox.setOutlineColor(sf::Color::Green);
        hitbox.setOutlineThickness(2);
        hitbox.setPosition(position);
    }

    void animate(float deltaTime) {
        animationTime += deltaTime;

        if (animationTime >= frameTime) {
            animationTime = 0.0f;
            currentFrame++;

            // Boucler sur les 8 premières frames (partie gauche uniquement)
            if (currentFrame >= maxFrames) {
                currentFrame = 0;
            }

            // Mettre à jour la texture rect (colonnes 0 à 7)
            sf::IntRect newRect(currentFrame * spriteWidth, 0, spriteWidth, spriteHeight);
            sprite.setTextureRect(newRect);
        }
    }

    void update(float deltaTime, float, float screenHeight) {
        // Animer le sprite
        animate(deltaTime);

        timeAlive += deltaTime;
        sf::Vector2f currentPos = sprite.getPosition();
        sf::Vector2f newPos = currentPos;

        switch (pattern) {
            case MovementPattern::STRAIGHT:
                // Simple mouvement horizontal vers la gauche
                newPos.x -= speed * deltaTime;
                break;

            case MovementPattern::SINE_WAVE:
                // Mouvement en vague sinusoïdale
                newPos.x -= speed * deltaTime;
                newPos.y = startPosition.y + amplitude * std::sin(frequency * timeAlive);
                break;

            case MovementPattern::ZIGZAG:
                // Zigzag : change de direction verticale périodiquement
                newPos.x -= speed * deltaTime;
                newPos.y = startPosition.y + amplitude * std::sin(frequency * timeAlive * 2);
                break;

            case MovementPattern::CIRCULAR:
                // Mouvement circulaire tout en avançant
                newPos.x -= speed * deltaTime * 0.5f;  // Plus lent horizontalement
                newPos.x += amplitude * 0.3f * std::cos(frequency * timeAlive);
                newPos.y = startPosition.y + amplitude * std::sin(frequency * timeAlive);
                break;

            case MovementPattern::DIAGONAL_DOWN:
                // Diagonale descendante
                newPos.x -= speed * deltaTime;
                newPos.y += speed * deltaTime * 0.5f;
                break;

            case MovementPattern::DIAGONAL_UP:
                // Diagonale montante
                newPos.x -= speed * deltaTime;
                newPos.y -= speed * deltaTime * 0.5f;
                break;
        }

        // Limiter la position Y pour rester dans l'écran
        float spriteScaledHeight = spriteHeight * sprite.getScale().y;
        if (newPos.y < 0) {
            newPos.y = 0;
        } else if (newPos.y + spriteScaledHeight > screenHeight) {
            newPos.y = screenHeight - spriteScaledHeight;
        }

        sprite.setPosition(newPos);
        hitbox.setPosition(newPos);
    }

    void draw(sf::RenderWindow& window, bool drawHitbox = false) {
        window.draw(sprite);
        if (drawHitbox)
            window.draw(hitbox);
    }

    // Getters
    sf::Vector2f getPosition() const { return sprite.getPosition(); }
    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }
    const sf::RectangleShape& getHitbox() const { return hitbox; }
    float getSpeed() const { return speed; }

    // Setters
    void setSpeed(float s) { speed = s; }
    void setAmplitude(float a) { amplitude = a; }
    void setFrequency(float f) { frequency = f; }
};

class Missile {
private:
    sf::Sprite sprite;
    sf::RectangleShape hitbox;
    float speed;

public:
    Missile() : speed(1000.0f) {}

    void init(const Player& player, sf::Texture& texture) {
        sprite.setTexture(texture);

        sf::IntRect missileRect(245, 85, 20, 20);
        sprite.setTextureRect(missileRect);

        sprite.setScale(3.0f, 3.0f);

        sf::Vector2f playerPos = player.getPosition();
        sf::FloatRect playerBounds = player.getBounds();

        // Calculer la hauteur du missile après scale (20 * 3.0f = 60)
        float missileHeight = 20 * 3.0f;
        float missileWidth = 20 * 3.0f;

        // Positionner le missile à droite du joueur et centré verticalement
        float missileX = playerPos.x + playerBounds.width;
        float missileY =
            playerPos.y + (playerBounds.height / 2.0f) - (missileHeight / 2.0f) + 10.0f;

        sprite.setPosition(missileX, missileY);

        // Initialiser la hitbox
        hitbox.setSize(sf::Vector2f(missileWidth, missileHeight));
        hitbox.setFillColor(sf::Color::Transparent);
        hitbox.setOutlineColor(sf::Color::Blue);
        hitbox.setOutlineThickness(2);
        hitbox.setPosition(missileX, missileY);
    }

    void move(float deltaTime) {
        sprite.move(speed * deltaTime, 0);
        hitbox.move(speed * deltaTime, 0);
    }

    void draw(sf::RenderWindow& window, bool drawHitbox = false) {
        window.draw(sprite);
        if (drawHitbox)
            window.draw(hitbox);
    }

    // Getters
    sf::Vector2f getPosition() const { return sprite.getPosition(); }
    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }
    const sf::RectangleShape& getHitbox() const { return hitbox; }
    float getSpeed() const { return speed; }

    // Setters
    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
        hitbox.setPosition(x, y);
    }
    void setSpeed(float s) { speed = s; }
};

int main() {
    sf::Event ev;
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "R-Type");
    sf::Clock clock;

    // Initialiser le background
    Background background;
    if (!background.loadTexture("../../client/assets/background.png")) {
        std::cerr << "Error: Could not load background.png" << std::endl;
        return 1;
    }
    background.init(window.getSize());

    // Initialiser le player
    Player player;
    if (!player.loadTexture("../../client/assets/players/r-typesheet42.png")) {
        std::cerr << "Error: Could not load player sprite" << std::endl;
        return 1;
    }
    player.init();

    // Charger la texture des missiles
    sf::Texture missileTexture;
    if (!missileTexture.loadFromFile("../../client/assets/players/r-typesheet1.png")) {
        std::cerr << "Error: Could not load missile sprite" << std::endl;
        return 1;
    }

    // Charger le son de tir (vfx/shoot.ogg)
    sf::SoundBuffer shootBuffer;
    sf::Sound shootSound;
    if (!shootBuffer.loadFromFile("../../client/assets/vfx/shoot.ogg")) {
        std::cerr << "Warning: Could not load shoot.ogg (no shoot sound)" << std::endl;
        // Ne pas échouer la compilation/exécution si le son manque, on continue sans son
    } else {
        shootSound.setBuffer(shootBuffer);
        shootSound.setVolume(80.f);  // ajuster si besoin
    }

    // Charger la texture des ennemis (vaisseau rouge)
    sf::Texture enemyTexture;
    if (!enemyTexture.loadFromFile("../../client/assets/enemies/r-typesheet5.png")) {
        std::cerr << "Error: Could not load enemy sprite" << std::endl;
        return 1;
    }

    // Charger la texture des explosions
    sf::Texture explosionTexture;
    if (!explosionTexture.loadFromFile("../../client/assets/enemies/r-typesheet44.png")) {
        std::cerr << "Error: Could not load explosion sprite" << std::endl;
        return 1;
    }

    std::vector<Missile> missiles;
    std::vector<ChargedMissile> chargedMissiles;
    std::vector<Enemy> enemies;
    std::vector<Explosion> explosions;
    std::vector<ShootEffect> shootEffects;
    std::vector<ChargedShootEffect> chargedShootEffects;
    bool spacePressed = false;
    float spaceHoldTime = 0.0f;
    const float chargeStartTime = 0.1f;
    ChargedShootEffect* activeChargingEffect = nullptr;

    // Système de spawn d'ennemis
    float enemySpawnTimer = 0.0f;
    float enemySpawnInterval = 2.0f;  // Spawn un ennemi toutes les 2 secondes

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();

        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed)
                window.close();

            // Gestion du relâchement de la touche espace
            if (ev.type == sf::Event::KeyReleased && ev.key.code == sf::Keyboard::Space) {
                if (spacePressed) {
                    // Calculer le niveau de charge si l'animation existe
                    int chargeLevel = 0;
                    if (activeChargingEffect != nullptr) {
                        chargeLevel = activeChargingEffect->getChargeLevel();
                    }

                    if (chargeLevel > 0) {
                        // Tir chargé avec le niveau approprié (lignes 5-9)
                        ChargedMissile chargedMissile;
                        chargedMissile.init(player, missileTexture, chargeLevel);
                        chargedMissiles.push_back(chargedMissile);
                    } else {
                        // Tir normal si pas de charge
                        Missile missile;
                        missile.init(player, missileTexture);
                        missiles.push_back(missile);

                        // Jouer le son de tir si chargé
                        if (shootSound.getBuffer() != nullptr) {
                            shootSound.play();
                        }

                        ShootEffect effect;
                        sf::Vector2f playerPos = player.getPosition();
                        sf::FloatRect playerBounds = player.getBounds();
                        sf::Vector2f effectPos(playerPos.x + playerBounds.width - 10,
                                               playerPos.y + 10);
                        effect.init(effectPos, missileTexture, playerPos);
                        shootEffects.push_back(effect);
                    }

                    // Nettoyer l'effet de charge actif
                    if (activeChargingEffect != nullptr) {
                        delete activeChargingEffect;
                        activeChargingEffect = nullptr;
                    }

                    spacePressed = false;
                    spaceHoldTime = 0.0f;
                }
            }
        }

        // Gestion du maintien de la touche espace
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            if (!spacePressed) {
                spacePressed = true;
            }
            spaceHoldTime += deltaTime;

            // Démarrer l'animation de charge plus rapidement (0.1s au lieu de 0.5s)
            if (spaceHoldTime >= chargeStartTime && activeChargingEffect == nullptr) {
                activeChargingEffect = new ChargedShootEffect();
                sf::Vector2f playerPos = player.getPosition();
                sf::FloatRect playerBounds = player.getBounds();
                sf::Vector2f effectPos(playerPos.x + playerBounds.width, playerPos.y - 5);
                activeChargingEffect->init(effectPos, missileTexture, playerPos);
            }
        }

        // Mise à jour
        background.animate(deltaTime);
        player.move(deltaTime, window.getSize());

        // Système de spawn d'ennemis
        enemySpawnTimer += deltaTime;
        if (enemySpawnTimer >= enemySpawnInterval) {
            enemySpawnTimer = 0.0f;

            // Créer un ennemi avec un pattern aléatoire
            Enemy enemy;
            float spawnY = 100 + (rand() % 800);                     // Position Y aléatoire
            sf::Vector2f spawnPos(window.getSize().x + 50, spawnY);  // Spawn à droite de l'écran

            // Choisir un pattern aléatoire
            Enemy::MovementPattern patterns[] = {
                Enemy::MovementPattern::STRAIGHT,      Enemy::MovementPattern::SINE_WAVE,
                Enemy::MovementPattern::ZIGZAG,        Enemy::MovementPattern::CIRCULAR,
                Enemy::MovementPattern::DIAGONAL_DOWN, Enemy::MovementPattern::DIAGONAL_UP};

            int patternIndex = rand() % 6;
            enemy.init(enemyTexture, spawnPos, patterns[patternIndex]);

            // Varier la vitesse et les paramètres
            enemy.setSpeed(200.0f + (rand() % 200));
            enemy.setAmplitude(50.0f + (rand() % 100));
            enemy.setFrequency(1.0f + (rand() % 3));

            enemies.push_back(enemy);
        }

        // Mettre à jour et supprimer les ennemis hors écran
        for (auto it = enemies.begin(); it != enemies.end();) {
            it->update(deltaTime, window.getSize().x, window.getSize().y);

            // Supprimer si hors écran (à gauche)
            sf::Vector2f pos = it->getPosition();
            if (pos.x < -100) {
                it = enemies.erase(it);
            } else {
                ++it;
            }
        }

        // Détection de collision entre missiles normaux et ennemis
        for (auto missileIt = missiles.begin(); missileIt != missiles.end();) {
            bool missileHit = false;
            sf::FloatRect missileBounds = missileIt->getHitbox().getGlobalBounds();

            for (auto enemyIt = enemies.begin(); enemyIt != enemies.end();) {
                sf::FloatRect enemyBounds = enemyIt->getHitbox().getGlobalBounds();

                if (missileBounds.intersects(enemyBounds)) {
                    // Collision détectée !
                    // Créer une explosion à la position de l'ennemi
                    Explosion explosion;
                    explosion.init(explosionTexture, enemyIt->getPosition());
                    explosions.push_back(explosion);

                    // Supprimer l'ennemi
                    enemyIt = enemies.erase(enemyIt);

                    // Marquer le missile pour suppression
                    missileHit = true;
                    break;
                } else {
                    ++enemyIt;
                }
            }

            if (missileHit) {
                missileIt = missiles.erase(missileIt);
            } else {
                ++missileIt;
            }
        }

        // Détection de collision entre missiles chargés et ennemis
        for (auto missileIt = chargedMissiles.begin(); missileIt != chargedMissiles.end();) {
            bool missileHit = false;
            sf::FloatRect missileBounds = missileIt->getHitbox().getGlobalBounds();

            for (auto enemyIt = enemies.begin(); enemyIt != enemies.end();) {
                sf::FloatRect enemyBounds = enemyIt->getHitbox().getGlobalBounds();

                if (missileBounds.intersects(enemyBounds)) {
                    // Collision détectée !
                    // Créer une explosion à la position de l'ennemi
                    Explosion explosion;
                    explosion.init(explosionTexture, enemyIt->getPosition());
                    explosions.push_back(explosion);

                    // Supprimer l'ennemi
                    enemyIt = enemies.erase(enemyIt);

                    // Marquer le missile pour suppression
                    missileHit = true;
                    break;
                } else {
                    ++enemyIt;
                }
            }

            if (missileHit) {
                missileIt = chargedMissiles.erase(missileIt);
            } else {
                ++missileIt;
            }
        }

        // Déplacer et supprimer les missiles normaux hors écran
        for (auto it = missiles.begin(); it != missiles.end();) {
            it->move(deltaTime);
            if (it->getPosition().x > window.getSize().x) {
                it = missiles.erase(it);
            } else {
                ++it;
            }
        }

        // Déplacer et supprimer les missiles chargés hors écran
        for (auto it = chargedMissiles.begin(); it != chargedMissiles.end();) {
            it->move(deltaTime);
            if (it->getPosition().x > window.getSize().x) {
                it = chargedMissiles.erase(it);
            } else {
                ++it;
            }
        }

        // Mettre à jour les effets de tir normaux et supprimer ceux terminés
        for (auto it = shootEffects.begin(); it != shootEffects.end();) {
            it->update(deltaTime, player.getPosition());
            if (it->isFinished()) {
                it = shootEffects.erase(it);
            } else {
                ++it;
            }
        }

        // Mettre à jour les effets de tir chargés et supprimer ceux terminés
        for (auto it = chargedShootEffects.begin(); it != chargedShootEffects.end();) {
            it->update(deltaTime, player.getPosition());
            if (it->isFinished()) {
                it = chargedShootEffects.erase(it);
            } else {
                ++it;
            }
        }

        // Mettre à jour l'animation de charge active
        if (activeChargingEffect != nullptr) {
            activeChargingEffect->update(deltaTime, player.getPosition());
            // L'animation reste bloquée sur la dernière frame tant que la touche est maintenue
        }

        // Mettre à jour les explosions et supprimer celles terminées
        for (auto it = explosions.begin(); it != explosions.end();) {
            it->update(deltaTime);
            if (it->isFinished()) {
                it = explosions.erase(it);
            } else {
                ++it;
            }
        }

        // Rendu
        window.clear();
        background.draw(window);
        player.draw(window);

        // Dessiner les ennemis
        for (auto& enemy : enemies) {
            enemy.draw(window, false);  // true pour voir les hitbox
        }

        // Dessiner les missiles normaux
        for (auto& missile : missiles) {
            missile.draw(window, false);
        }

        // Dessiner les missiles chargés
        for (auto& chargedMissile : chargedMissiles) {
            chargedMissile.draw(window, false);
        }

        // Dessiner les effets de tir normaux
        for (auto& effect : shootEffects) {
            effect.draw(window);
        }

        // Dessiner les effets de tir chargés
        for (auto& chargedEffect : chargedShootEffects) {
            chargedEffect.draw(window);
        }

        // Dessiner l'animation de charge active
        if (activeChargingEffect != nullptr) {
            activeChargingEffect->draw(window);
        }

        // Dessiner les explosions
        for (auto& explosion : explosions) {
            explosion.draw(window);
        }

        window.display();
    }

    // Nettoyage
    if (activeChargingEffect != nullptr) {
        delete activeChargingEffect;
    }

    return 0;
}