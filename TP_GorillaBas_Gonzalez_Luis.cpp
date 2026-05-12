/*
*** Apellidos:     Gonzalez Vargas
*** Nombres:       Luis Mario
*** Cedula:        7.761.778
*** Tema:          Juego de Artilleria inspirado en Gorilla.bas, con edificios generados aleatoriamente, dos jugadores y efectos de viento.
*/

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <thread>
#include <vector>

namespace {

constexpr int kWidth = 60;
constexpr int kHeight = 20;
constexpr int kGroundRow = kHeight - 1;
constexpr int kPointsToWin = 5;
constexpr int kMaxAngle = 89;
constexpr int kMinAngle = 1;
constexpr int kMaxSpeed = 40;
constexpr int kMinSpeed = 1;
constexpr int kMaxShotSteps = 140;
constexpr double kPi = 3.14159265358979323846;
constexpr double kGravity = 0.16;
constexpr double kSpeedScale = 0.48;
constexpr double kWindMin = -0.10;
constexpr double kWindMax = 0.10;

struct Building {
    int x;
    int width;
    int height;
};

struct Player {
    std::string name;
    char symbol;
    int x;
    int y;
    int score = 0;
};

struct GameState {
    std::vector<Building> buildings;
    Player player1;
    Player player2;
    int currentTurn = 0;
    double wind = 0.0;
};

int randomInt(int minValue, int maxValue, std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(minValue, maxValue);
    return dist(rng);
}

double randomDouble(double minValue, double maxValue, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(minValue, maxValue);
    return dist(rng);
}

void clearScreen() {
    std::cout << "\x1B[2J\x1B[H";
}

std::vector<Building> generateBuildings(std::mt19937& rng) {
    std::vector<Building> buildings;
    int x = 0;

    while (x < kWidth) {
        int remaining = kWidth - x;
        int width = std::min(randomInt(3, 5, rng), remaining);
        int height = randomInt(6, 14, rng);

        buildings.push_back({x, width, height});
        x += width;
    }

    return buildings;
}

int buildingTopRow(const Building& building) {
    return kGroundRow - building.height;
}

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

bool collidesWithBuilding(const std::vector<Building>& buildings, int x, int y) {
    for (const Building& building : buildings) {
        if (x >= building.x && x < building.x + building.width) {
            return y >= buildingTopRow(building) && y < kGroundRow;
        }
    }
    return false;
}

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
    std::cout << "GORILLA C++ - Juego de artilleria\n";
    std::cout << "Puntaje: " << state.player1.name << " " << state.player1.score
              << " - " << state.player2.score << " " << state.player2.name
              << " | Viento: " << std::fixed << std::setprecision(2) << state.wind << "\n";

    if (state.wind > 0.0)
        std::cout << "Viento hacia la derecha\n";
    else if (state.wind < 0.0)
        std::cout << "Viento hacia la izquierda\n";
    else
        std::cout << "Sin viento\n";

    std::cout << "Turno: " << (state.currentTurn == 0 ? state.player1.name : state.player2.name) << "\n";
    std::cout << "Consejo: Usa angulos entre 30 y 60 grados y velocidad moderada\n";
    std::cout << message << "\n\n";

    for (const std::string& row : grid) {
        std::cout << row << '\n';
    }
}

double readDouble(const std::string& prompt, double minValue, double maxValue) {
    while (true) {
        std::cout << prompt;
        double value;
        std::cin >> value;

        if (std::cin.fail() || value < minValue || value > maxValue) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Entrada invalida. Intenta de nuevo.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return value;
    }
}

int readMenuOption(const std::string& prompt, int minValue, int maxValue) {
    while (true) {
        std::cout << prompt;
        int option;
        std::cin >> option;

        if (std::cin.fail() || option < minValue || option > maxValue) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Opcion invalida. Intenta de nuevo.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return option;
    }
}

std::string readString(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) {
        return "Jugador";
    }
    return line;
}

void startRound(GameState& state, std::mt19937& rng) {
    state.buildings = generateBuildings(rng);
    placePlayers(state);
    state.wind = randomDouble(kWindMin, kWindMax, rng);
}

bool simulateShot(GameState& state, double angleDeg, double speed) {
    const double radians = angleDeg * kPi / 180.0;
    const Player& shooter = state.currentTurn == 0 ? state.player1 : state.player2;
    const Player& target = state.currentTurn == 0 ? state.player2 : state.player1;
    double direction = state.currentTurn == 0 ? 1.0 : -1.0;

    double x = shooter.x;
    double y = shooter.y;
    double vx = std::cos(radians) * speed * kSpeedScale * direction;
    double vy = -std::sin(radians) * speed * kSpeedScale;

    for (int step = 0; step < kMaxShotSteps; ++step) {
        x += vx;
        y += vy;
        vx += state.wind;
        vy += kGravity;

        int drawX = static_cast<int>(std::round(x));
        int drawY = static_cast<int>(std::round(y));

        drawScene(state, "Disparo en curso...", drawX, drawY);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));

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

void showIntro() {
    clearScreen();
    std::cout << "GORILLA C++\n";
    std::cout << "Bienvenido al juego de artilleria. El primer jugador en alcanzar " << kPointsToWin << " puntos gana.\n";
    std::cout << "Controla angulo y velocidad. El viento modifica la trayectoria.\n\n";
}

bool showMainMenu() {
    std::cout << "1. Jugar\n2. Salir\n";
    return readMenuOption("Opcion: ", 1, 2) == 1;
}

void playMatch(std::mt19937& rng) {
    GameState state;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    state.player1.name = readString("Nombre jugador 1: ");
    state.player2.name = readString("Nombre jugador 2: ");
    state.player1.symbol = '1';
    state.player2.symbol = '2';
    state.player1.score = 0;
    state.player2.score = 0;
    state.currentTurn = 0;

    while (state.player1.score < kPointsToWin && state.player2.score < kPointsToWin) {
        startRound(state, rng);
        bool hit = false;

        while (!hit) {
            Player& current = state.currentTurn == 0 ? state.player1 : state.player2;
            std::string turnMessage = current.name + ", elije angulo y velocidad.";
            drawScene(state, turnMessage);

            double angle = readDouble("Angulo (" + std::to_string(kMinAngle) + "-" + std::to_string(kMaxAngle) + "): ", kMinAngle, kMaxAngle);
            double speed = readDouble("Velocidad (" + std::to_string(kMinSpeed) + "-" + std::to_string(kMaxSpeed) + "): ", kMinSpeed, kMaxSpeed);

            hit = simulateShot(state, angle, speed);
            if (hit) {
                current.score++;
                drawScene(state, "Impacto! " + current.name + " anota.");
                std::this_thread::sleep_for(std::chrono::milliseconds(700));
            } else {
                state.currentTurn = 1 - state.currentTurn;
            }
        }
    }

    std::string winner = state.player1.score > state.player2.score ? state.player1.name : state.player2.name;
    drawScene(state, "Partida terminada! Ganador: " + winner);
    std::cout << "Felicitaciones, " << winner << "!\n";
    std::cout << "Presiona Enter para continuar...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} // namespace

int main() {
    std::random_device rd;
    std::mt19937 rng(rd());

    while (showMainMenu()) {
        showIntro();
        playMatch(rng);
    }

    return 0;
}




