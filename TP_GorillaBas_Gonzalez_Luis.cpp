/*
*** Apellidos:     Gonzalez Vargas
*** Nombres:       Luis Mario
*** Cedula:        7.761.778
*** Tema:          Juego de Artilleria inspirado en Gorilla.bas, con edificios generados aleatoriamente, dos jugadores y efectos de viento.
*/  

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

namespace {

// Dimensiones del escenario de texto y constantes generales del juego.
constexpr int kWidth = 60;
constexpr int kHeight = 20;
constexpr int kGroundRow = kHeight - 1; 

// CAMBIO MANUAL 3: Se aumenta la cantidad de puntos para ganar
constexpr int kPointsToWin = 5; // antes 3

constexpr double kPi = 3.14159265358979323846;

// Representa un edificio del escenario.
struct Building {
    int x;
    int width;
    int height;
};

// Representa a un jugador con su posicion y puntaje.
struct Player {
    std::string name;
    char symbol;
    int x;
    int y;
    int score = 0;
};

// Guarda el estado general de una partida.
struct GameState {
    std::vector<Building> buildings;
    Player player1;
    Player player2;
    int currentTurn = 0;
    double wind = 0.0;
};

// Genera un numero entero aleatorio dentro de un rango.
int randomInt(int minValue, int maxValue, std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(minValue, maxValue);
    return dist(rng);
}

// Genera un numero decimal aleatorio dentro de un rango.
double randomDouble(double minValue, double maxValue, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(minValue, maxValue);
    return dist(rng);
}

// Limpia la consola para redibujar la escena completa.
void clearScreen() {
    std::cout << "\x1B[2J\x1B[H";
}

// Crea una lista de edificios con ancho y altura aleatorios.
std::vector<Building> generateBuildings(std::mt19937& rng) {
    std::vector<Building> buildings;
    int x = 0;

    while (x < kWidth) {
        int remaining = kWidth - x;

        // CAMBIO MANUAL 2: Edificios más angostos y altos
        int width = std::min(randomInt(3, 5, rng), remaining);  // antes 4-6
        int height = randomInt(6, 14, rng); // antes 4-10

        buildings.push_back({x, width, height});
        x += width;
    }

    return buildings;
}

// Calcula la fila superior de un edificio dentro de la grilla.
int buildingTopRow(const Building& building) {
    return kGroundRow - building.height;
}

// Ubica a los dos jugadores sobre edificios.
void placePlayers(GameState& state) {
    int leftIndex = 1;
    int rightIndex = static_cast<int>(state.buildings.size()) - 2;

    if (leftIndex >= static_cast<int>(state.buildings.size())) {
        leftIndex = 0;
    }
    if (rightIndex < 0) {
        rightIndex = static_cast<int>(state.buildings.size()) - 1;
    }

    const Building& leftBuilding = state.buildings[leftIndex];
    const Building& rightBuilding = state.buildings[rightIndex];

    state.player1.x = leftBuilding.x + leftBuilding.width / 2;
    state.player1.y = buildingTopRow(leftBuilding) - 1;
    state.player2.x = rightBuilding.x + rightBuilding.width / 2;
    state.player2.y = buildingTopRow(rightBuilding) - 1;
}

// Colisiones con edificios
bool collidesWithBuilding(const std::vector<Building>& buildings, int x, int y) {
    for (const Building& building : buildings) {
        if (x >= building.x && x < building.x + building.width) {
            return y >= buildingTopRow(building) && y < kGroundRow;
        }
    }
    return false;
}

// Dibuja la escena
void drawScene(const GameState& state, const std::string& message, int projectileX = -1, int projectileY = -1) {
    std::vector<std::string> grid(kHeight, std::string(kWidth, ' '));

    for (const Building& building : state.buildings) {
        for (int y = buildingTopRow(building); y < kGroundRow; ++y) {
            for (int x = building.x; x < building.x + building.width; ++x) {
                grid[y][x] = '#';
            }
        }
    }

    grid[state.player1.y][state.player1.x] = state.player1.symbol;
    grid[state.player2.y][state.player2.x] = state.player2.symbol;

    if (projectileX >= 0 && projectileX < kWidth && projectileY >= 0 && projectileY < kHeight) {
        grid[projectileY][projectileX] = '*';
    }

    for (int x = 0; x < kWidth; ++x) {
        grid[kGroundRow][x] = '=';
    }

    clearScreen();
    std::cout << "GORILLA C++ - Version simple\n";

    std::cout << "Puntaje: " << state.player1.name << " " << state.player1.score
              << " - " << state.player2.score << " " << state.player2.name
              << " | Viento: " << std::fixed << std::setprecision(2) << state.wind << "\n";

    // CAMBIO MANUAL 5: Dirección del viento
    if (state.wind > 0)
        std::cout << "Viento hacia la derecha\n";
    else if (state.wind < 0)
        std::cout << "Viento hacia la izquierda\n";
    else
        std::cout << "Sin viento\n";

    std::cout << "Turno: " << (state.currentTurn == 0 ? state.player1.name : state.player2.name) << "\n";

    // CAMBIO MANUAL 4: Consejo al jugador
    std::cout << "Consejo: Usa angulos entre 30 y 60 grados\n";

    std::cout << message << "\n\n";

    for (const std::string& row : grid) {
        std::cout << row << '\n';
    }
}

// Lectura segura
double readDouble(const std::string& prompt, double minValue, double maxValue) {
    while (true) {
        std::cout << prompt;
        double value;
        std::cin >> value;

        if (std::cin.fail() || value < minValue || value > maxValue) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Entrada invalida\n";
            continue;
        }
        return value;
    }
}

// Menu
int readMenuOption(const std::string& prompt, int minValue, int maxValue) {
    while (true) {
        int option;
        std::cout << prompt;
        std::cin >> option;

        if (std::cin.fail() || option < minValue || option > maxValue) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Opcion invalida\n";
            continue;
        }
        return option;
    }
}

// Nueva ronda
void startRound(GameState& state, std::mt19937& rng) {
    state.buildings = generateBuildings(rng);
    placePlayers(state);
    state.wind = randomDouble(-0.05, 0.05, rng);
}

// Simulación del disparo
bool simulateShot(GameState& state, double angleDeg, double speed) {

    // CAMBIO MANUAL 1: Física modificada
    const double gravity = 0.15;   // antes 0.22
    const double speedScale = 0.45; // antes 0.35

    const double radians = angleDeg * kPi / 180.0;

    const Player& shooter = state.currentTurn == 0 ? state.player1 : state.player2;
    const Player& target = state.currentTurn == 0 ? state.player2 : state.player1;

    double direction = state.currentTurn == 0 ? 1.0 : -1.0;

    double x = shooter.x;
    double y = shooter.y;

    double vx = std::cos(radians) * speed * speedScale * direction;
    double vy = -std::sin(radians) * speed * speedScale;

    for (int step = 0; step < 120; ++step) {
        x += vx;
        y += vy;
        vx += state.wind;
        vy += gravity;

        int drawX = std::round(x);
        int drawY = std::round(y);

        drawScene(state, "Disparo en curso...", drawX, drawY);

        if (drawX < 0 || drawX >= kWidth || drawY >= kHeight)
            return false;

        if (drawY >= kGroundRow)
            return false;

        if (drawX == target.x && drawY == target.y)
            return true;

        if (collidesWithBuilding(state.buildings, drawX, drawY))
            return false;
    }

    return false;
}

// Intro
void showIntro() {
    clearScreen();
    std::cout << "GORILLA C++\n";
}

// Menu principal
bool showMainMenu() {
    std::cout << "1. Jugar\n2. Salir\n";
    return readMenuOption("Opcion: ", 1, 2) == 1;
}

// Juego
void playMatch(std::mt19937& rng) {
    GameState state;
    state.player1 = {"Jugador 1", '1'};
    state.player2 = {"Jugador 2", '2'};

    while (state.player1.score < kPointsToWin && state.player2.score < kPointsToWin) {
        startRound(state, rng);
        bool hit = false;

        while (!hit) {
            Player& current = state.currentTurn == 0 ? state.player1 : state.player2;

            drawScene(state, "Dispara");

            double angle = readDouble("Angulo: ", 1, 89);
            double speed = readDouble("Velocidad: ", 1, 40);

            hit = simulateShot(state, angle, speed);

            if (hit) {
                current.score++;
            } else {
                state.currentTurn = 1 - state.currentTurn;
            }
        }
    }
}

} // namespace

int main() {
    std::random_device rd;
    std::mt19937 rng(rd());

    while (showMainMenu()) {
        showIntro();
        playMatch(rng);
    }
}




