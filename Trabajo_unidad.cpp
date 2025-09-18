#include <iostream> 
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <string>

using namespace std;

// Estructura para representar un proceso (PCB)
struct Proceso {
    int pid;   //Indetificador unico del Proceso 
    int llegada; // TImepo de llegado del sistema 
    int servicio; //CPU burts total Requerido 
    int inicio; //Tiempo de la primera Ejecucion y terminacion 
    int fin; 
    int tiempoRestante;//Tiempo restante RR
    bool iniciado;  // Control del primer despacho
    
    Proceso(int p, int ll, int s) : pid(p), llegada(ll), servicio(s), 
                                    inicio(-1), fin(-1), tiempoRestante(s), iniciado(false) {}
    
    int respuesta() const { return inicio - llegada; }
    int espera() const { return fin - llegada - servicio; }
    int retorno() const { return fin - llegada; }
};

struct BloqueMemoria {
    int id; // Indetificador de bloques
    int tamano;// Tamaño de bytes 
    bool ocupado; // Estado del bloque 
    int pidAsignado; // Pid  del proceso asignado 
    
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
    
   //Ingresar procesos manualmente
    void ingresarProcesos() {
        procesos.clear();
        int numProcesos;
        
        cout << "\n=== INGRESO DE PROCESOS ===" << endl;
        cout << "Numero de procesos a ingresar: ";
        cin >> numProcesos;
        
        for (int i = 0; i < numProcesos; i++) {
            int pid, llegada, servicio;
            cout << "\n--- Proceso " << (i+1) << " ---" << endl;
            cout << "PID: ";
            cin >> pid;
            cout << "Tiempo de llegada: ";
            cin >> llegada;
            cout << "Tiempo de servicio (CPU burst): ";
            cin >> servicio;
            
            procesos.push_back(Proceso(pid, llegada, servicio));
        }
        
        cout << "\nProcesos ingresados exitosamente!" << endl;
        mostrarProcesosIngresados();
    }
    
   // Mostrar procesos actuales
    void mostrarProcesosIngresados() {
        cout << "\n=== PROCESOS ACTUALES ===" << endl;
        cout << "PID | Llegada | Servicio" << endl;
        cout << "--- | ------- | --------" << endl;
        for (const auto& p : procesos) {
            cout << setw(3) << p.pid << " | "
                 << setw(7) << p.llegada << " | "
                 << setw(8) << p.servicio << endl;
        }
    }
    
   // Cargar datos de ejemplo rápido
    void cargarDatosEjemplo() {
        procesos.clear();
        procesos = {
            Proceso(1, 0, 12),
            Proceso(2, 1, 5),
            Proceso(3, 2, 8)
        };
        cout << "Datos de ejemplo cargados!" << endl;
        mostrarProcesosIngresados();
    }
    
    void cargarProcesos(const vector<Proceso>& procs) {
        procesos = procs;
    }
    
    void inicializarMemoria(int tamano, const string& estrategia) {
        tamanoMemoria = tamano;
        algoritmoMemoria = estrategia;
        memoria.clear();
        memoria.push_back(BloqueMemoria(0, tamano));
    }
    
    void ejecutarFCFS() {
        if (procesos.empty()) {
            cout << "No hay procesos cargados. Ingrese procesos primero." << endl;
            return;
        }
        
        cout << "\n=== ALGORITMO FCFS ===" << endl;
        
        // Crear copia para no modificar los originales
        vector<Proceso> procesosTemp = procesos;
        
        // 1. O
        sort(procesosTemp.begin(), procesosTemp.end(), 
             [](const Proceso& a, const Proceso& b) { return a.llegada < b.llegada; });
        
        int tiempoActual = 0;
        
        cout << "\nSimulacion paso a paso:" << endl;
        for (auto& proceso : procesosTemp) {
            if (tiempoActual < proceso.llegada) {
                cout << "Tiempo " << tiempoActual << "-" << proceso.llegada << ": CPU inactiva" << endl;
                tiempoActual = proceso.llegada;
            }
            
            proceso.inicio = tiempoActual;
            cout << "Tiempo " << tiempoActual << ": Inicia proceso PID " << proceso.pid << endl;
            tiempoActual += proceso.servicio;
            proceso.fin = tiempoActual;
            cout << "Tiempo " << tiempoActual << ": Termina proceso PID " << proceso.pid << endl;
        }
        
        procesos = procesosTemp;
        mostrarResultados();
    }
    
    void ejecutarSPN() {
        if (procesos.empty()) {
            cout << "No hay procesos cargados. Ingrese procesos primero." << endl;
            return;
        }
        
        cout << "\n=== ALGORITMO SPN ===" << endl;
        
        vector<Proceso> cola;
        vector<Proceso> ejecutados;
        int tiempoActual = 0;
        
        cout << "\nSimulacion paso a paso:" << endl;
        
        while (ejecutados.size() < procesos.size()) {
            // Agregar procesos que han llegado
            for (auto& proceso : procesos) {
                if (proceso.llegada <= tiempoActual && 
                    find_if(cola.begin(), cola.end(), 
                           [&proceso](const Proceso& p) { return p.pid == proceso.pid; }) == cola.end() &&
                    find_if(ejecutados.begin(), ejecutados.end(),
                           [&proceso](const Proceso& p) { return p.pid == proceso.pid; }) == ejecutados.end()) {
                    cola.push_back(proceso);
                    cout << "Tiempo " << tiempoActual << ": Llega proceso PID " << proceso.pid 
                         << " (servicio=" << proceso.servicio << ")" << endl;
                }
            }
            
            if (cola.empty()) {
                tiempoActual++;
                continue;
            }
            
            // Ordenar por servicio
            sort(cola.begin(), cola.end(), [](const Proceso& a, const Proceso& b) {
                if (a.servicio == b.servicio) return a.llegada < b.llegada;
                return a.servicio < b.servicio;
            });
            
            // Ejecutar el más corto
            Proceso& procesoActual = cola.front();
            procesoActual.inicio = tiempoActual;
            cout << "Tiempo " << tiempoActual << ": Ejecuta proceso PID " << procesoActual.pid 
                 << " (el mas corto con servicio=" << procesoActual.servicio << ")" << endl;
            tiempoActual += procesoActual.servicio;
            procesoActual.fin = tiempoActual;
            cout << "Tiempo " << tiempoActual << ": Termina proceso PID " << procesoActual.pid << endl;
            
            ejecutados.push_back(procesoActual);
            cola.erase(cola.begin());
        }
        
        procesos = ejecutados;
        mostrarResultados();
    }
    
    void ejecutarRoundRobin(int quantum) {
        if (procesos.empty()) {
            cout << "No hay procesos cargados. Ingrese procesos primero." << endl;
            return;
        }
        
        cout << "\n=== ALGORITMO ROUND ROBIN (Quantum=" << quantum << ") ===" << endl;
        
        vector<Proceso> procesosRR = procesos;
        queue<int> colaListos;
        vector<bool> enCola(procesosRR.size(), false);
        int tiempoActual = 0;
        int procesosTerminados = 0;
        
        cout << "\nSimulacion paso a paso:" << endl;
        
        while (procesosTerminados < procesosRR.size()) {
            // Agregar procesos que llegaron
            for (int i = 0; i < procesosRR.size(); i++) {
                if (procesosRR[i].llegada <= tiempoActual && 
                    procesosRR[i].tiempoRestante > 0 && 
                    !enCola[i]) {
                    colaListos.push(i);
                    enCola[i] = true;
                    cout << "Tiempo " << tiempoActual << ": PID " << procesosRR[i].pid 
                         << " entra a cola de listos" << endl;
                }
            }
            
            if (colaListos.empty()) {
                tiempoActual++;
                continue;
            }
            
            int indice = colaListos.front();
            colaListos.pop();
            enCola[indice] = false;
            
            Proceso& procesoActual = procesosRR[indice];
            
            if (!procesoActual.iniciado) {
                procesoActual.inicio = tiempoActual;
                procesoActual.iniciado = true;
                cout << "Tiempo " << tiempoActual << ": PID " << procesoActual.pid 
                     << " inicia por primera vez" << endl;
            }
            
            int tiempoEjecucion = min(quantum, procesoActual.tiempoRestante);
            cout << "Tiempo " << tiempoActual << "-" << (tiempoActual + tiempoEjecucion) 
                 << ": Ejecutando PID " << procesoActual.pid 
                 << " (restante=" << procesoActual.tiempoRestante << ")" << endl;
            
            tiempoActual += tiempoEjecucion;
            procesoActual.tiempoRestante -= tiempoEjecucion;
            
            if (procesoActual.tiempoRestante == 0) {
                procesoActual.fin = tiempoActual;
                procesosTerminados++;
                cout << "Tiempo " << tiempoActual << ": PID " << procesoActual.pid << " TERMINA" << endl;
            } else {
                colaListos.push(indice);
                enCola[indice] = true;
                cout << "Tiempo " << tiempoActual << ": PID " << procesoActual.pid 
                     << " vuelve a cola (quantum agotado)" << endl;
            }
        }
        
        procesos = procesosRR;
        mostrarResultados();
    }
    
    bool asignarMemoriaFirstFit(int pid, int tamano) {
        for (auto& bloque : memoria) {
            if (!bloque.ocupado && bloque.tamano >= tamano) {
                cout << "Memoria asignada: PID " << pid << " -> Bloque ID " << bloque.id 
                     << " (Tamano: " << bloque.tamano << ")" << endl;
                
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
        cout << "No se pudo asignar memoria para PID " << pid << " (Tamano: " << tamano << ")" << endl;
        return false;
    }
    
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
            
            if (bloque.tamano > tamano) {
                memoria.insert(memoria.begin() + mejorBloque + 1,
                               BloqueMemoria(memoria.size(), bloque.tamano - tamano));
            }
            
            bloque.ocupado = true;
            bloque.pidAsignado = pid;
            bloque.tamano = tamano;
            return true;
        }
        
        cout << "No se pudo asignar memoria para PID " << pid << " (Tamano: " << tamano << ")" << endl;
        return false;
    }
    
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
    
    int opcion;
    do {
        cout << "\n--- MENU PRINCIPAL ---" << endl;
        cout << "0. Ingresar procesos manualmente" << endl;
        cout << "1. Cargar datos de ejemplo" << endl;
        cout << "2. Ver procesos actuales" << endl;
        cout << "3. Ejecutar FCFS" << endl;
        cout << "4. Ejecutar SPN" << endl;
        cout << "5. Ejecutar Round Robin" << endl;
        cout << "6. Gestion de Memoria" << endl;
        cout << "7. Salir" << endl;
        cout << "Seleccione una opcion: ";
        cin >> opcion;
        
        switch(opcion) {
            case 0:
                simulador.ingresarProcesos();
                break;
            case 1:
                simulador.cargarDatosEjemplo();
                break;
            case 2:
                simulador.mostrarProcesosIngresados();
                break;
            case 3:
                simulador.ejecutarFCFS();
                break;
            case 4:
                simulador.ejecutarSPN();
                break;
            case 5: {
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
            case 6: {
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
            case 7:
                cout << "Saliendo del simulador..." << endl;
                break;
            default:
                cout << "Opcion no valida" << endl;
        }
    } while (opcion != 7);
    
    return 0;
}