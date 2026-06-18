#include "graph.hpp"

int main() {
    try {
        Graph graph;
        int directedOption = 1;
        int nodeCount = 0;
        int routeCount = 0;

        std::cout << "Crear grafo/digrafo con nodos, aristas y rutas\n";
        std::cout << "Es digrafo/dirigido? (1 = si, 0 = no): ";
        std::cin >> directedOption;
        graph.directed = directedOption == 1;

        std::cout << "Cantidad de nodos: ";
        std::cin >> nodeCount;
        for (int i = 0; i < nodeCount; ++i) {
            std::string nodo;
            std::cout << "Nodo " << (i + 1) << ": ";
            std::cin >> nodo;
            graph.addNode(nodo);
        }

        std::cout << "Cantidad de rutas/aristas: ";
        std::cin >> routeCount;
        for (int i = 0; i < routeCount; ++i) {
            std::string ni;
            std::string nf;
            std::string ac;
            double p = 0.0;
            double t = 0.0;

            std::cout << "Ruta " << (i + 1) << " - ni nf ac p t: ";
            std::cin >> ni >> nf >> ac >> p >> t;
            graph.addRoute(ni, nf, ac, p, t);
        }

        std::string fileName;
        std::cout << "Archivo de salida (.txt, .json, .csv o .xml): ";
        std::cin >> fileName;

        graph.saveByExtension(fileName);
        std::cout << "Grafo guardado en: " << fileName << "\n";
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return 1;
    }
    return 0;
}
