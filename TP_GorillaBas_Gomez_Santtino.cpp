#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>

using namespace std;

// --- Constantes del Juego ---
const int ANCHO = 110;  // Modificado: Ampliado para que quepan 8 edificios
const int ALTO = 25;
const double GRAVEDAD = 10.0; // CAMBIO 1: Gravedad modificada de 9.8 a 10.0
const double PI = 3.14159265;
const int NUM_EDIFICIOS = 8;  // CAMBIO 2: Total de edificios en el escenario
const int ANCHO_EDIFICIO = 12; // CAMBIO 2: Ancho ampliado (10 + 2 columnas)

// --- Estructuras ---
struct Punto {
    int x, y;
};

struct Jugador {
    string nombre;
    Punto pos;
    char simbolo;
    bool esCPU; // CAMBIO 5: Variable para identificar si el jugador es la computadora
};

// --- Prototipos de Funciones ---
void limpiarPantalla();
void mostrarMenuPrincipal();
double gradosARadianes(double grados);
void inicializarMapa(char mapa[ALTO][ANCHO], Jugador &j1, Jugador &j2);
void limpiarCielo(char mapa[ALTO][ANCHO], Jugador j1, Jugador j2);
void dibujarMapa(char mapa[ALTO][ANCHO], int victoriasJ1, int victoriasJ2, bool mostrarMarcador);
double leerEntradaValida(string mensaje);
bool disparar(char mapa[ALTO][ANCHO], Jugador tirador, Jugador objetivo, double angulo, double velocidad, bool disparaHaciaDerecha);
void jugar(bool contraCPU, bool mejorDeTres);

// --- Función Principal ---
int main() {
    srand(time(NULL)); 
    int opcionModo, opcionRondas;

    // CAMBIO 3: Opción de reiniciar partida controlada por este bucle principal
    do {
        mostrarMenuPrincipal();
        opcionModo = leerEntradaValida("Elige el modo de juego: ");

        if (opcionModo == 1 || opcionModo == 2) {
            bool contraCPU = (opcionModo == 1); // 1 = vs CPU, 2 = vs J2
            
            limpiarPantalla();
            cout << "=======================================\n";
            cout << "          TIPO DE PARTIDA              \n";
            cout << "=======================================\n";
            cout << "1. Ronda unica (Muerte subita)\n";
            cout << "2. Al mejor de 3\n";
            cout << "=======================================\n";
            
            do {
                opcionRondas = leerEntradaValida("Elige el tipo de partida (1 o 2): ");
            } while (opcionRondas != 1 && opcionRondas != 2);

            bool mejorDeTres = (opcionRondas == 2);
            
            jugar(contraCPU, mejorDeTres);
            
            cout << "\nPresiona Enter para volver al menu principal y jugar de nuevo...";
            cin.ignore();
            cin.get();
        } else if (opcionModo != 3) {
            cout << "Opcion no valida. Intenta de nuevo.\n";
            cin.ignore();
            cin.get();
        }
    } while (opcionModo != 3);

    cout << "\nGracias por jugar. ¡Hasta la proxima!\n";
    return 0;
}

// --- Implementación de Funciones ---

void limpiarPantalla() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void mostrarMenuPrincipal() {
    limpiarPantalla();
    cout << "=======================================\n";
    cout << "          BATALLA DE PROYECTILES       \n";
    cout << "=======================================\n";
    cout << "1. Jugar: Jugador vs Computadora (CPU)\n"; // CAMBIO 5
    cout << "2. Jugar: Jugador vs Jugador\n";
    cout << "3. Salir\n";
    cout << "=======================================\n";
}

double gradosARadianes(double grados) {
    return grados * (PI / 180.0);
}

double leerEntradaValida(string mensaje) {
    double valor;
    while (true) {
        cout << mensaje;
        if (cin >> valor) {
            break; 
        } else {
            cout << "Error: Entrada invalida. Por favor, ingresa un numero.\n";
            cin.clear(); 
            cin.ignore(10000, '\n'); 
        }
    }
    return valor;
}

void inicializarMapa(char mapa[ALTO][ANCHO], Jugador &j1, Jugador &j2) {
    for (int i = 0; i < ALTO; i++) {
        for (int j = 0; j < ANCHO; j++) mapa[i][j] = ' ';
    }

    // CAMBIO 2: Generar 8 edificios en lugar de 2
    int alturas[NUM_EDIFICIOS];
    for(int i = 0; i < NUM_EDIFICIOS; i++) {
        alturas[i] = rand() % 7 + 4; // Alturas aleatorias entre 4 y 10
    }

    int espacioEntreEdificios = 2; // Espacio libre entre edificios

    // Construir los 8 edificios
    for(int b = 0; b < NUM_EDIFICIOS; b++) {
        int inicioX = b * (ANCHO_EDIFICIO + espacioEntreEdificios) + 2; 
        for (int j = inicioX; j < inicioX + ANCHO_EDIFICIO && j < ANCHO; j++) {
            for (int i = ALTO - 1; i >= ALTO - alturas[b]; i--) {
                mapa[i][j] = '#';
            }
        }
    }

    // Posicionar J1 en el primer edificio (índice 0)
    int inicioX_J1 = 0 * (ANCHO_EDIFICIO + espacioEntreEdificios) + 2;
    j1.pos.x = inicioX_J1 + (ANCHO_EDIFICIO / 2); 
    j1.pos.y = ALTO - alturas[0] - 1; 

    // Posicionar J2 en el octavo edificio (índice 7)
    int inicioX_J2 = 7 * (ANCHO_EDIFICIO + espacioEntreEdificios) + 2;
    j2.pos.x = inicioX_J2 + (ANCHO_EDIFICIO / 2); 
    j2.pos.y = ALTO - alturas[7] - 1; 

    mapa[j1.pos.y][j1.pos.x] = j1.simbolo;
    mapa[j2.pos.y][j2.pos.x] = j2.simbolo;
}

void limpiarCielo(char mapa[ALTO][ANCHO], Jugador j1, Jugador j2) {
    for (int i = 0; i < ALTO; i++) {
        for (int j = 0; j < ANCHO; j++) {
            if (mapa[i][j] == '*' || mapa[i][j] == 'X') {
                mapa[i][j] = ' ';
            }
        }
    }
    mapa[j1.pos.y][j1.pos.x] = j1.simbolo;
    mapa[j2.pos.y][j2.pos.x] = j2.simbolo;
}

void dibujarMapa(char mapa[ALTO][ANCHO], int victoriasJ1, int victoriasJ2, bool mostrarMarcador) {
    limpiarPantalla();
    
    // CAMBIO 3: Mostrar marcador solo si es al mejor de 3
    if (mostrarMarcador) {
        cout << "MARCADOR -> J1: " << victoriasJ1 << " | J2: " << victoriasJ2 << "\n";
        cout << "--------------------------------------------------\n";
    }

    for (int i = 0; i < ALTO; i++) {
        for (int j = 0; j < ANCHO; j++) {
            cout << mapa[i][j];
        }
        cout << endl;
    }
    for (int j = 0; j < ANCHO; j++) cout << "=";
    cout << endl;
}

bool disparar(char mapa[ALTO][ANCHO], Jugador tirador, Jugador objetivo, double angulo, double velocidad, bool disparaHaciaDerecha) {
    double anguloRad = gradosARadianes(angulo);
    double t = 0.0;
    bool impacto = false;
    bool enVuelo = true;

    while (enVuelo) {
        t += 0.1; 
        int xActual, yActual;

        if (disparaHaciaDerecha) {
            xActual = tirador.pos.x + (int)(velocidad * cos(anguloRad) * t);
        } else {
            xActual = tirador.pos.x - (int)(velocidad * cos(anguloRad) * t);
        }

        // CAMBIO 1: La fórmula utiliza la constante GRAVEDAD (ahora en 10.0)
        yActual = tirador.pos.y - (int)((velocidad * sin(anguloRad) * t) - (0.5 * GRAVEDAD * t * t));

        if (xActual < 0 || xActual >= ANCHO || yActual >= ALTO) {
            cout << "\nEl proyectil cayo fuera del escenario.\n";
            enVuelo = false;
        } else if (yActual >= 0) { 
            if (mapa[yActual][xActual] == objetivo.simbolo) {
                mapa[yActual][xActual] = 'X'; 
                impacto = true;
                enVuelo = false;
            } else if (mapa[yActual][xActual] == '#') {
                mapa[yActual][xActual] = 'X'; 
                cout << "\nEl proyectil choco con un edificio.\n";
                enVuelo = false;
            } else if (mapa[yActual][xActual] == ' ') {
                mapa[yActual][xActual] = '*'; 
            }
        }
    }
    return impacto;
}

void jugar(bool contraCPU, bool mejorDeTres) {
    Jugador j1 = {"Jugador 1 (A)", {0,0}, 'A', false};
    Jugador j2 = {"Jugador 2 (B)", {0,0}, 'B', contraCPU}; 

    int victoriasJ1 = 0, victoriasJ2 = 0;
    int tirosJ1 = 0, tirosJ2 = 0; // CAMBIO 4: Contadores de tiros
    int victoriasNecesarias = mejorDeTres ? 2 : 1; // CAMBIO 3: Lógica de Mejor de 3
    int ronda = 1;

    while (victoriasJ1 < victoriasNecesarias && victoriasJ2 < victoriasNecesarias) {
        char mapa[ALTO][ANCHO];
        inicializarMapa(mapa, j1, j2);
        
        bool rondaTerminada = false;
        bool turnoJ1 = true; 
        
        while (!rondaTerminada) {
            limpiarCielo(mapa, j1, j2);
            dibujarMapa(mapa, victoriasJ1, victoriasJ2, mejorDeTres);
            
            Jugador actual = turnoJ1 ? j1 : j2;
            Jugador oponente = turnoJ1 ? j2 : j1;
            
            cout << "\n--- Ronda " << ronda << " ---\n";
            cout << "Turno actual: " << actual.nombre << "\n"; // CAMBIO 4: Mostrar turno
            
            double angulo, vel;

            // CAMBIO 5: Lógica de la CPU vs Lógica humana
            if (actual.esCPU) {
                angulo = rand() % 50 + 20; // Ángulo aleatorio entre 20 y 70
                vel = rand() % 25 + 10;    // Velocidad aleatoria entre 10 y 35
                cout << "La CPU esta calculando...\n";
                cout << "La CPU ingreso - Angulo: " << angulo << " | Velocidad: " << vel << "\n"; // CAMBIO 4
            } else {
                angulo = leerEntradaValida("Ingresa el angulo (0-90): ");
                vel = leerEntradaValida("Ingresa la velocidad: ");
                cout << "Datos confirmados - Angulo: " << angulo << " | Velocidad: " << vel << "\n"; // CAMBIO 4
            }
            
            if (turnoJ1) tirosJ1++; else tirosJ2++; // Contar tiro
            
            bool acierto = disparar(mapa, actual, oponente, angulo, vel, turnoJ1);
            dibujarMapa(mapa, victoriasJ1, victoriasJ2, mejorDeTres); 
            
            if (acierto) {
                cout << "\n¡IMPACTO DIRECTO! ¡" << actual.nombre << " gana la ronda!\n";
                if (turnoJ1) victoriasJ1++; else victoriasJ2++;
                rondaTerminada = true;
                ronda++;
            } else {
                cout << "\nHas fallado.\n";
                turnoJ1 = !turnoJ1; 
            }
            
            cout << "Presiona Enter para continuar...";
            if (!actual.esCPU) cin.ignore();
            cin.get();
        }
    }

    // CAMBIO 4: Resumen final
    limpiarPantalla();
    cout << "=======================================\n";
    cout << "           FIN DE LA PARTIDA           \n";
    cout << "=======================================\n";
    if (victoriasJ1 == victoriasNecesarias) {
        cout << "¡El GANADOR absoluto es " << j1.nombre << "!\n";
        cout << "Acerto el tiro ganador en su intento numero " << tirosJ1 << ".\n";
    } else {
        cout << "¡El GANADOR absoluto es " << j2.nombre << "!\n";
        cout << "Acerto el tiro ganador en su intento numero " << tirosJ2 << ".\n";
    }
    cout << "\n--- Estadisticas Finales ---\n";
    cout << "Total de tiros de " << j1.nombre << ": " << tirosJ1 << "\n";
    cout << "Total de tiros de " << j2.nombre << ": " << tirosJ2 << "\n";
}