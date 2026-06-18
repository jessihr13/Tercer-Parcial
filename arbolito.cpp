#include <iostream>
#include <string>
#include <fstream> 

using namespace std;

struct ConexionNodo2Nodo {
    string nodoInicial;
    string nodoFinal;
    string aristaConexion;
};

struct Rutas {
    ConexionNodo2Nodo conjuntoConexiones[100];
    int cantidadConexiones; 
};

class HolaM {
public:
    string nodos[100];
    int cantNodos;
    string aristas[100];
    int cantAristas;
    Rutas rutas;

    HolaM() {
        cantNodos = 0;
        cantAristas = 0;
        rutas.cantidadConexiones = 0;
    }

    void mostrarMensaje() {
        cout << "=== GENERADOR DE ARBOL ===" << endl;
    }


    void agregarConexion(string ini, string fin, string arista) {
        
        int i;
        for (i = 0; i < rutas.cantidadConexiones; i++) {
            if (rutas.conjuntoConexiones[i].nodoFinal == fin) {
                cout << "Error: El nodo " << fin << " ya tiene padre. No es un arbol valido." << endl;
                return;
            }
        }

        int pos = rutas.cantidadConexiones;
        rutas.conjuntoConexiones[pos].nodoInicial = ini;
        rutas.conjuntoConexiones[pos].nodoFinal = fin;
        rutas.conjuntoConexiones[pos].aristaConexion = arista;
        rutas.cantidadConexiones++;

        
        bool existeIni = false, existeFin = false;
        for (i = 0; i < cantNodos; i++) {
            if (nodos[i] == ini) existeIni = true;
            if (nodos[i] == fin) existeFin = true;
        }
        if (!existeIni) { nodos[cantNodos] = ini; cantNodos++; }
        if (!existeFin) { nodos[cantNodos] = fin; cantNodos++; }

        bool existeArista = false;
        for (i = 0; i < cantAristas; i++) {
            if (aristas[i] == arista) existeArista = true;
        }
        if (!existeArista) { aristas[cantAristas] = arista; cantAristas++; }
    }

    
    void guardarTXT() {
        ofstream archivo("arbol.txt");
        int i;
        for (i = 0; i < rutas.cantidadConexiones; i++) {
            archivo << rutas.conjuntoConexiones[i].nodoInicial << " "
                    << rutas.conjuntoConexiones[i].nodoFinal << " "
                    << rutas.conjuntoConexiones[i].aristaConexion << "\n";
        }
        archivo.close();
        cout << "Arbol guardado en arbol.txt" << endl;
    }

    void guardarCSV() {
        ofstream archivo("arbol.csv");
        int i;
        archivo << "Origen,Destino,Arista\n";
        for (i = 0; i < rutas.cantidadConexiones; i++) {
            archivo << rutas.conjuntoConexiones[i].nodoInicial << ","
                    << rutas.conjuntoConexiones[i].nodoFinal << ","
                    << rutas.conjuntoConexiones[i].aristaConexion << "\n";
        }
        archivo.close();
        cout << "Arbol guardado en arbol.csv" << endl;
    }

    void guardarJSON() {
        ofstream archivo("arbol.json");
        int i;
        archivo << "{\n  \"conexiones\": [\n";
        for (i = 0; i < rutas.cantidadConexiones; i++) {
            archivo << "    {\n";
            archivo << "      \"nodoInicial\": \"" << rutas.conjuntoConexiones[i].nodoInicial << "\",\n";
            archivo << "      \"nodoFinal\": \"" << rutas.conjuntoConexiones[i].nodoFinal << "\",\n";
            archivo << "      \"aristaConexion\": \"" << rutas.conjuntoConexiones[i].aristaConexion << "\"\n";
            archivo << "    }";
            if (i < rutas.cantidadConexiones - 1) archivo << ",";
            archivo << "\n";
        }
        archivo << "  ]\n}";
        archivo.close();
        cout << "Arbol guardado en arbol.json" << endl;
    }

    void guardarXML() {
        ofstream archivo("arbol.xml");
        int i;
        archivo << "<arbol>\n";
        for (i = 0; i < rutas.cantidadConexiones; i++) {
            archivo << "  <conexionNodo2Nodo>\n";
            archivo << "    <nodoInicial>" << rutas.conjuntoConexiones[i].nodoInicial << "</nodoInicial>\n";
            archivo << "    <nodoFinal>" << rutas.conjuntoConexiones[i].nodoFinal << "</nodoFinal>\n";
            archivo << "    <aristaConexion>" << rutas.conjuntoConexiones[i].aristaConexion << "</aristaConexion>\n";
            archivo << "  </conexionNodo2Nodo>\n";
        }
        archivo << "</arbol>\n";
        archivo.close();
        cout << "Arbol guardado en arbol.xml" << endl;
    }
};

int main() {
    HolaM objeto;
    objeto.mostrarMensaje();

    
    objeto.agregarConexion("A", "B", "Arista1");
    objeto.agregarConexion("A", "C", "Arista2");
    objeto.agregarConexion("B", "D", "Arista3");
    objeto.agregarConexion("B", "E", "Arista4");

    
    objeto.guardarTXT();
    objeto.guardarCSV();
    objeto.guardarJSON();
    objeto.guardarXML();

    return 0;
}