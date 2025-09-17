#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <string>

using namespace std;

// Estructura para representar un proceso (PCB)
struct Proceso {
    int pid;
    int llegada;
    int servicio;
    int inicio;
    int fin;
    int tiempoRestante;  // Para Round Robin
    bool iniciado;
    
    // Constructor
    Proceso(int p, int ll, int s) : pid(p), llegada(ll), servicio(s), 
                                    inicio(-1), fin(-1), tiempoRestante(s), iniciado(false) {}
    
    // Métodos para calcular métricas
    int respuesta() const { return inicio - llegada; }
    int espera() const { return fin - llegada - servicio; }
    int retorno() const { return fin - llegada; }
};

// Estructura para bloques de memoria
struct BloqueMemoria {
    int id;
    int tamano;
    bool ocupado;
    int pidAsignado;
    
    BloqueMemoria(int i, int t) : id(i), tamano(t), ocupado(false), pidAsignado(-1) {}
};

class SimuladorSO {
private:
    vector<Proceso> procesos;
    vector<BloqueMemoria> memoria;
    int tamanoMemoria;
    string algoritmoMemoria;
    
public:
    SimuladorSO() {}
    
    // Cargar procesos
    void cargarProcesos(const vector<Proceso>& procs) {
        procesos = procs;
    }
    
    // Inicializar memoria
    void inicializarMemoria(int tamano, const string& estrategia) {
        tamanoMemoria = tamano;
        algoritmoMemoria = estrategia;
        // Crear un bloque inicial con toda la memoria disponible
        memoria.clear();
        memoria.push_back(BloqueMemoria(0, tamano));
    }
    
    // Algoritmo FCFS (First Come First Served)
    void ejecutarFCFS() {
        cout << "\n=== ALGORITMO FCFS ===" << endl;
        
        // Ordenar por tiempo de llegada
        sort(procesos.begin(), procesos.end(), 
             [](const Proceso& a, const Proceso& b) { return a.llegada < b.llegada; });
        
        int tiempoActual = 0;
        
        for (auto& proceso : procesos) {
            // Si el tiempo actual es menor que la llegada, avanzar al tiempo de llegada
            if (tiempoActual < proceso.llegada) {
                tiempoActual = proceso.llegada;
            }
            
            proceso.inicio = tiempoActual;
            tiempoActual += proceso.servicio;
            proceso.fin = tiempoActual;
        }
        
        mostrarResultados();
    }
    
    // Algoritmo SPN (Shortest Process Next)
    void ejecutarSPN() {
        cout << "\n=== ALGORITMO SPN ===" << endl;
        
        vector<Proceso> cola;
        vector<Proceso> ejecutados;
        int tiempoActual = 0;
        
        while (ejecutados.size() < procesos.size()) {
            // Agregar procesos que han llegado a la cola
            for (auto& proceso : procesos) {
                if (proceso.llegada <= tiempoActual && 
                    find_if(cola.begin(), cola.end(), 
                           [&proceso](const Proceso& p) { return p.pid == proceso.pid; }) == cola.end() &&
                    find_if(ejecutados.begin(), ejecutados.end(),
                           [&proceso](const Proceso& p) { return p.pid == proceso.pid; }) == ejecutados.end()) {
                    cola.push_back(proceso);
                }
            }
            
            if (cola.empty()) {
                tiempoActual++;
                continue;
            }
            
            // Ordenar por servicio (y por llegada en caso de empate)
            sort(cola.begin(), cola.end(), [](const Proceso& a, const Proceso& b) {
                if (a.servicio == b.servicio) return a.llegada < b.llegada;
                return a.servicio < b.servicio;
            });
            
            // Ejecutar el proceso más corto
            Proceso& procesoActual = cola.front();
            procesoActual.inicio = tiempoActual;
            tiempoActual += procesoActual.servicio;
            procesoActual.fin = tiempoActual;
            
            ejecutados.push_back(procesoActual);
            cola.erase(cola.begin());
        }
        
        procesos = ejecutados;
        mostrarResultados();
    }
    
    // Algoritmo Round Robin
    void ejecutarRoundRobin(int quantum) {
        cout << "\n=== ALGORITMO ROUND ROBIN (Quantum=" << quantum << ") ===" << endl;
        
        // Crear copias de los procesos para no modificar los originales
        vector<Proceso> procesosRR = procesos;
        queue<int> colaListos; // Índices de procesos
        vector<bool> enCola(procesosRR.size(), false);
        int tiempoActual = 0;
        int procesosTerminados = 0;
        
        while (procesosTerminados < procesosRR.size()) {
            // Agregar procesos que han llegado a la cola
            for (int i = 0; i < procesosRR.size(); i++) {
                if (procesosRR[i].llegada <= tiempoActual && 
                    procesosRR[i].tiempoRestante > 0 && 
                    !enCola[i]) {
                    colaListos.push(i);
                    enCola[i] = true;
                }
            }
            
            if (colaListos.empty()) {
                tiempoActual++;
                continue;
            }
            
            // Tomar el primer proceso de la cola
            int indice = colaListos.front();
            colaListos.pop();
            enCola[indice] = false;
            
            Proceso& procesoActual = procesosRR[indice];
            
            // Marcar inicio si es la primera vez
            if (!procesoActual.iniciado) {
                procesoActual.inicio = tiempoActual;
                procesoActual.iniciado = true;
            }
            
            // Ejecutar por quantum o hasta terminar
            int tiempoEjecucion = min(quantum, procesoActual.tiempoRestante);
            tiempoActual += tiempoEjecucion;
            procesoActual.tiempoRestante -= tiempoEjecucion;
            
            // Si terminó
            if (procesoActual.tiempoRestante == 0) {
                procesoActual.fin = tiempoActual;
                procesosTerminados++;
            } else {
                // Volver a agregar a la cola si no terminó
                colaListos.push(indice);
                enCola[indice] = true;
            }
        }
        
        procesos = procesosRR;
        mostrarResultados();
    }
    
    // Gestión de memoria - First Fit
    bool asignarMemoriaFirstFit(int pid, int tamano) {
        for (auto& bloque : memoria) {
            if (!bloque.ocupado && bloque.tamano >= tamano) {
                cout << "Memoria asignada: PID " << pid << " -> Bloque ID " << bloque.id 
                     << " (Tamano: " << bloque.tamano << ")" << endl;
                
                // Si el bloque es más grande, dividirlo
                if (bloque.tamano > tamano) {
                    memoria.insert(memoria.begin() + (&bloque - &memoria[0]) + 1,
                                   BloqueMemoria(memoria.size(), bloque.tamano - tamano));
                }
                
                bloque.ocupado = true;
                bloque.pidAsignado = pid;
                bloque.tamano = tamano;
                return true;
            }
        }
        cout << "No se pudo asignar memoria para PID " 
		<< pid << " (Tamano: " << tamano << ")" << endl;
        return false;
    }
    
    // Gestión de memoria - Best Fit
    bool asignarMemoriaBestFit(int pid, int tamano) {
        int mejorBloque = -1;
        int menorDesperdicio = tamanoMemoria + 1;
        
        for (int i = 0; i < memoria.size(); i++) {
            if (!memoria[i].ocupado && memoria[i].tamano >= tamano) {
                int desperdicio = memoria[i].tamano - tamano;
                if (desperdicio < menorDesperdicio) {
                    menorDesperdicio = desperdicio;
                    mejorBloque = i;
                }
            }
        }
        
        if (mejorBloque != -1) {
            auto& bloque = memoria[mejorBloque];
            cout << "Memoria asignada: PID " << pid << " -> Bloque ID " << bloque.id 
                 << " (Tamano: " << bloque.tamano << ")" << endl;
            
            // Si el bloque es más grande, dividirlo
            if (bloque.tamano > tamano) {
                memoria.insert(memoria.begin() + mejorBloque + 1,
                               BloqueMemoria(memoria.size(), bloque.tamano - tamano));
            }
            
            bloque.ocupado = true;
            bloque.pidAsignado = pid;
            bloque.tamano = tamano;
            return true;
        }
        
        cout << "No se pudo asignar memoria para PID " 
		<< pid << " (Tamano: " << tamano << ")" << endl;
        return false;
    }
    
    // Mostrar resultados de planificación
    void mostrarResultados() {
        cout << "\nResultados de Planificacion:" << endl;
        cout << "PID | Llegada | Servicio | Inicio | Fin | Respuesta | Espera | Retorno" << endl;
        cout << "--- | ------- | -------- | ------ | --- | --------- | ------ | -------" << endl;
        
        double sumaRespuesta = 0, sumaEspera = 0, sumaRetorno = 0;
        
        for (const auto& proceso : procesos) {
            cout << setw(3) << proceso.pid << " | "
                 << setw(7) << proceso.llegada << " | "
                 << setw(8) << proceso.servicio << " | "
                 << setw(6) << proceso.inicio << " | "
                 << setw(3) << proceso.fin << " | "
                 << setw(9) << proceso.respuesta() << " | "
                 << setw(6) << proceso.espera() << " | "
                 << setw(7) << proceso.retorno() << endl;
            
            sumaRespuesta += proceso.respuesta();
            sumaEspera += proceso.espera();
            sumaRetorno += proceso.retorno();
        }
        
        int n = procesos.size();
        int tiempoTotal = (*max_element(procesos.begin(), procesos.end(), 
                                        [](const Proceso& a, const Proceso& b) { return a.fin < b.fin; })).fin;
        
        cout << "\nMetricas Globales:" << endl;
        cout << "Tiempo promedio de respuesta: " << fixed << setprecision(2) << sumaRespuesta / n << endl;
        cout << "Tiempo promedio de espera: " << sumaEspera / n << endl;
        cout << "Tiempo promedio de retorno: " << sumaRetorno / n << endl;
        cout << "Throughput: " << fixed << setprecision(2) << (double)n / tiempoTotal << " procesos/tiempo" << endl;
    }
};

int main() {
    SimuladorSO simulador;
    
    cout << "=== SIMULADOR DE SISTEMA OPERATIVO ===" << endl;
    
    // Cargar procesos de ejemplo
    vector<Proceso> procesos = {
        Proceso(1, 0, 12),
        Proceso(2, 1, 5),
        Proceso(3, 2, 8)
    };
    
    simulador.cargarProcesos(procesos);
    
    // Menú principal
    int opcion;
    do {
        cout << "\n--- MENU PRINCIPAL ---" << endl;
        cout << "1. Ejecutar FCFS" << endl;
        cout << "2. Ejecutar SPN" << endl;
        cout << "3. Ejecutar Round Robin" << endl;
        cout << "4. Gestion de Memoria" << endl;
        cout << "5. Salir" << endl;
        cout << "Seleccione una opcion: ";
        cin >> opcion;
        
        switch(opcion) {
            case 1:
                simulador.ejecutarFCFS();
                break;
            case 2:
                simulador.ejecutarSPN();
                break;
            case 3: {
                int quantum;
                cout << "Ingrese el quantum (>=2): ";
                cin >> quantum;
                if (quantum >= 2) {
                    simulador.ejecutarRoundRobin(quantum);
                } else {
                    cout << "Quantum debe ser >= 2" << endl;
                }
                break;
            }
            case 4: {
                cout << "\n--- GESTION DE MEMORIA ---" << endl;
                int tamMemoria;
                string estrategia;
                cout << "Tamano de memoria (bytes): ";
                cin >> tamMemoria;
                cout << "Estrategia (first-fit / best-fit): ";
                cin >> estrategia;
                
                simulador.inicializarMemoria(tamMemoria, estrategia);
                
                int numSolicitudes;
                cout << "Numero de solicitudes de memoria: ";
                cin >> numSolicitudes;
                
                for (int i = 0; i < numSolicitudes; i++) {
                    int pid, tamano;
                    cout << "PID y tamano para solicitud " << (i+1) << ": ";
                    cin >> pid >> tamano;
                    
                    if (estrategia == "first-fit") {
                        simulador.asignarMemoriaFirstFit(pid, tamano);
                    } else if (estrategia == "best-fit") {
                        simulador.asignarMemoriaBestFit(pid, tamano);
                    }
                }
                break;
            }
            case 5:
                cout << "Saliendo del simulador..." << endl;
                break;
            default:
                cout << "Opcion no valida" << endl;
        }
    } while (opcion != 5);
    
    return 0;
}