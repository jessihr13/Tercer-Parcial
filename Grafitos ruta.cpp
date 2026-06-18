#include "grafos.hpp"

static void printRoute(const RouteResult& route) {
    if (!route.found) {
        std::cout << "No existe una ruta entre esos nodos.\n";
        return;
    }

    std::cout << "Costo total: " << route.cost << "\n";
    std::cout << "Ruta: ";
    for (std::size_t i = 0; i < route.nodes.size(); ++i) {
        std::cout << route.nodes[i] << (i + 1 == route.nodes.size() ? "\n" : " -> ");
    }

    std::cout << "Grafo de la ruta:\n";
    for (const auto& step : route.routes) {
        std::cout << "  " << step.ni << " -> " << step.nf
                  << "  arista=" << step.ac
                  << "  p=" << step.p
                  << "  t=" << step.t << "\n";
    }
}

int main() {
    try {
        std::string inputFile;
        std::string start;
        std::string goal;
        std::string routeType;
        std::string metric;
        std::string outputBase;

        std::cout << "Archivo del grafo (.txt, .json, .csv o .xml): ";
        std::cin >> inputFile;
        Graph graph = Graph::loadByExtension(inputFile);

        std::cout << "Nodo inicial: ";
        std::cin >> start;
        std::cout << "Nodo final: ";
        std::cin >> goal;
        std::cout << "Tipo de ruta (corta/larga): ";
        std::cin >> routeType;
        std::cout << "Criterio de costo (p = peso, t = tiempo): ";
        std::cin >> metric;
        std::cout << "Nombre base para archivos de resultado: ";
        std::cin >> outputBase;

        routeType = lowerText(routeType);
        metric = lowerText(metric);

        RouteResult route;
        if (routeType == "larga" || routeType == "longest") {
            route = graph.longestSimplePath(start, goal, metric);
        } else {
            route = graph.dijkstraShortest(start, goal, metric);
        }

        printRoute(route);

        graph.saveRouteGraphTxt(outputBase + ".txt", route, metric);
        graph.saveRouteGraphJson(outputBase + ".json", route, metric);
        graph.saveRouteGraphCsv(outputBase + ".csv", route, metric);
        graph.saveRouteGraphXml(outputBase + ".xml", route, metric);
        graph.saveRouteGraphDot(outputBase + ".dot", route, metric);

        std::cout << "Archivos generados:\n";
        std::cout << "  " << outputBase << ".txt\n";
        std::cout << "  " << outputBase << ".json\n";
        std::cout << "  " << outputBase << ".csv\n";
        std::cout << "  " << outputBase << ".xml\n";
        std::cout << "  " << outputBase << ".dot\n";
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return 1;
    }
    return 0;
}
