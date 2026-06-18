#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

struct Route {
    std::string ni, nf, ac;
    double p = 0, t = 0;
};

struct RouteResult {
    bool found = false;
    double cost = 0;
    std::vector<std::string> nodes;
    std::vector<Route> routes;
};

inline std::string lowerText(std::string value) {
    for (char& c : value) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return value;
}

inline std::string extensionOf(const std::string& name) {
    auto pos = name.find_last_of('.');
    return pos == std::string::npos ? "" : lowerText(name.substr(pos + 1));
}

class Graph {
public:
    bool directed = true;
    std::set<std::string> nodes, edgeNames;
    std::vector<Route> routes;

    void addNode(const std::string& node) { nodes.insert(node); }

    void addRoute(const std::string& ni, const std::string& nf, const std::string& ac, double p, double t) {
        if (p < 0 || t < 0) throw std::runtime_error("Dijkstra requiere costos no negativos.");
        nodes.insert(ni); nodes.insert(nf); edgeNames.insert(ac);
        routes.push_back({ni, nf, ac, p, t});
    }

    RouteResult shortest(const std::string& start, const std::string& goal, const std::string& metric) const {
        const double inf = std::numeric_limits<double>::infinity();
        std::map<std::string, double> dist;
        std::map<std::string, Route> parent;
        for (const auto& node : nodes) dist[node] = inf;
        if (!nodes.count(start) || !nodes.count(goal)) return {};

        using Item = std::pair<double, std::string>;
        std::priority_queue<Item, std::vector<Item>, std::greater<Item>> pq;
        dist[start] = 0; pq.push({0, start});
        while (!pq.empty()) {
            auto [cost, current] = pq.top(); pq.pop();
            if (cost != dist[current]) continue;
            if (current == goal) break;
            for (const auto& edge : outgoing(current)) {
                double candidate = cost + value(edge, metric);
                if (candidate < dist[edge.nf]) {
                    dist[edge.nf] = candidate;
                    parent[edge.nf] = edge;
                    pq.push({candidate, edge.nf});
                }
            }
        }
        if (dist[goal] == inf) return {};
        return buildResult(start, goal, dist[goal], parent);
    }

    RouteResult longest(const std::string& start, const std::string& goal, const std::string& metric) const {
        RouteResult best;
        if (!nodes.count(start) || !nodes.count(goal)) return best;
        std::set<std::string> visited;
        std::vector<std::string> nodePath;
        std::vector<Route> routePath;
        dfsLongest(start, goal, metric, 0, visited, nodePath, routePath, best);
        return best;
    }

    RouteResult dijkstraShortest(const std::string& start, const std::string& goal,
                                 const std::string& metric) const {
        return shortest(start, goal, metric);
    }

    RouteResult longestSimplePath(const std::string& start, const std::string& goal,
                                  const std::string& metric) const {
        return longest(start, goal, metric);
    }

    std::vector<RouteResult> allSimplePaths(const std::string& start, const std::string& goal,
                                            const std::string& metric) const {
        std::vector<RouteResult> results;
        if (!nodes.count(start) || !nodes.count(goal)) return results;
        std::set<std::string> visited;
        std::vector<std::string> nodePath;
        std::vector<Route> routePath;
        collectPaths(start, goal, metric, 0, visited, nodePath, routePath, results);
        return results;
    }

    void save(const std::string& fileName) const {
        std::string ext = extensionOf(fileName);
        if (ext == "json") saveJson(fileName);
        else if (ext == "csv") saveCsv(fileName);
        else if (ext == "xml") saveXml(fileName);
        else saveTxt(fileName);
    }

    static Graph load(const std::string& fileName) {
        std::string ext = extensionOf(fileName);
        if (ext == "json") return loadJson(fileName);
        if (ext == "csv") return loadCsv(fileName);
        if (ext == "xml") return loadXml(fileName);
        return loadTxt(fileName);
    }

    void saveResults(const std::string& base, const RouteResult& result, const std::string& metric) const {
        saveResultTxt(base + ".txt", result, metric);
        saveResultJson(base + ".json", result, metric);
        saveResultCsv(base + ".csv", result, metric);
        saveResultXml(base + ".xml", result, metric);
        saveResultDot(base + ".dot", result, metric);
    }

    void saveFullGraphDot(const std::string& fileName) const {
        std::ofstream out(fileName);
        if (!out) throw std::runtime_error("No se pudo escribir " + fileName);
        const std::string arrow = directed ? " -> " : " -- ";
        out << (directed ? "digraph" : "graph") << " completo {\n  rankdir=LR;\n";
        for (const auto& node : nodes)
            out << "  \"" << escape(node) << "\" [shape=circle];\n";
        for (const auto& route : routes)
            out << "  \"" << escape(route.ni) << "\"" << arrow << "\"" << escape(route.nf)
                << "\" [label=\"" << escape(route.ac) << " | p=" << route.p << " | t=" << route.t << "\"];\n";
        out << "}\n";
    }

private:
    static double value(const Route& route, const std::string& metric) {
        return metric == "t" || metric == "tiempo" ? route.t : route.p;
    }

    std::vector<Route> outgoing(const std::string& node) const {
        std::vector<Route> result;
        for (const auto& route : routes) {
            if (route.ni == node) result.push_back(route);
            if (!directed && route.nf == node) result.push_back({route.nf, route.ni, route.ac, route.p, route.t});
        }
        return result;
    }

    static RouteResult buildResult(const std::string& start, const std::string& goal, double cost,
                                   const std::map<std::string, Route>& parent) {
        RouteResult result; result.found = true; result.cost = cost;
        std::string current = goal;
        while (current != start) {
            auto it = parent.find(current);
            if (it == parent.end()) return {};
            result.routes.push_back(it->second);
            result.nodes.push_back(current);
            current = it->second.ni;
        }
        result.nodes.push_back(start);
        std::reverse(result.nodes.begin(), result.nodes.end());
        std::reverse(result.routes.begin(), result.routes.end());
        return result;
    }

    void dfsLongest(const std::string& current, const std::string& goal, const std::string& metric,
                    double cost, std::set<std::string>& visited, std::vector<std::string>& nodePath,
                    std::vector<Route>& routePath, RouteResult& best) const {
        visited.insert(current); nodePath.push_back(current);
        if (current == goal) {
            if (!best.found || cost > best.cost) {
                best = {true, cost, nodePath, routePath};
            }
        } else {
            for (const auto& route : outgoing(current)) {
                if (!visited.count(route.nf)) {
                    routePath.push_back(route);
                    dfsLongest(route.nf, goal, metric, cost + value(route, metric), visited, nodePath, routePath, best);
                    routePath.pop_back();
                }
            }
        }
        nodePath.pop_back(); visited.erase(current);
    }

    void collectPaths(const std::string& current, const std::string& goal, const std::string& metric,
                      double cost, std::set<std::string>& visited, std::vector<std::string>& nodePath,
                      std::vector<Route>& routePath, std::vector<RouteResult>& results) const {
        visited.insert(current);
        nodePath.push_back(current);
        if (current == goal) {
            results.push_back({true, cost, nodePath, routePath});
        } else {
            for (const auto& route : outgoing(current)) {
                if (!visited.count(route.nf)) {
                    routePath.push_back(route);
                    collectPaths(route.nf, goal, metric, cost + value(route, metric),
                                 visited, nodePath, routePath, results);
                    routePath.pop_back();
                }
            }
        }
        nodePath.pop_back();
        visited.erase(current);
    }

    static std::string readFile(const std::string& name) {
        std::ifstream in(name);
        if (!in) throw std::runtime_error("No se pudo abrir " + name);
        std::ostringstream out; out << in.rdbuf(); return out.str();
    }

    static std::vector<std::string> split(const std::string& text, char delimiter) {
        std::vector<std::string> parts; std::stringstream ss(text); std::string part;
        while (std::getline(ss, part, delimiter)) parts.push_back(part);
        return parts;
    }

    static std::string escape(const std::string& text) {
        std::string result;
        for (char c : text) { if (c == '"' || c == '\\') result += '\\'; result += c; }
        return result;
    }

    void saveTxt(const std::string& name) const {
        std::ofstream out(name); if (!out) throw std::runtime_error("No se pudo escribir " + name);
        out << "dirigido " << directed << "\nnodos " << nodes.size();
        for (const auto& n : nodes) out << " " << n;
        out << "\naristas " << edgeNames.size(); for (const auto& e : edgeNames) out << " " << e;
        out << "\nrutas " << routes.size() << "\n";
        for (const auto& r : routes) out << r.ni << " " << r.nf << " " << r.ac << " " << r.p << " " << r.t << "\n";
    }

    void saveJson(const std::string& name) const {
        std::ofstream out(name); if (!out) throw std::runtime_error("No se pudo escribir " + name);
        out << "{\n  \"dirigido\": " << (directed ? "true" : "false") << ",\n  \"nodos\": [";
        std::size_t i = 0; for (const auto& n : nodes) out << "\"" << escape(n) << "\"" << (++i == nodes.size() ? "" : ", ");
        out << "],\n  \"rutas\": [\n";
        for (i = 0; i < routes.size(); ++i) {
            const auto& r = routes[i];
            out << "    {\"ni\":\"" << escape(r.ni) << "\",\"nf\":\"" << escape(r.nf)
                << "\",\"ac\":\"" << escape(r.ac) << "\",\"p\":" << r.p << ",\"t\":" << r.t << "}"
                << (i + 1 == routes.size() ? "\n" : ",\n");
        }
        out << "  ]\n}\n";
    }

    void saveCsv(const std::string& name) const {
        std::ofstream out(name); if (!out) throw std::runtime_error("No se pudo escribir " + name);
        out << "tipo,valor\ndirigido," << directed << "\n";
        for (const auto& n : nodes) out << "nodo," << n << "\n";
        out << "\nni,nf,ac,p,t\n";
        for (const auto& r : routes) out << r.ni << "," << r.nf << "," << r.ac << "," << r.p << "," << r.t << "\n";
    }

    void saveXml(const std::string& name) const {
        std::ofstream out(name); if (!out) throw std::runtime_error("No se pudo escribir " + name);
        out << "<grafo dirigido=\"" << (directed ? "true" : "false") << "\">\n  <nodos>\n";
        for (const auto& n : nodes) out << "    <nodo id=\"" << n << "\" />\n";
        out << "  </nodos>\n  <rutas>\n";
        for (const auto& r : routes) out << "    <ruta ni=\"" << r.ni << "\" nf=\"" << r.nf << "\" ac=\"" << r.ac
                                         << "\" p=\"" << r.p << "\" t=\"" << r.t << "\" />\n";
        out << "  </rutas>\n</grafo>\n";
    }

    static Graph loadTxt(const std::string& name) {
        std::ifstream in(name); if (!in) throw std::runtime_error("No se pudo abrir " + name);
        Graph g; std::string label; std::size_t count;
        in >> label >> g.directed >> label >> count;
        while (count--) { std::string n; in >> n; g.addNode(n); }
        in >> label >> count; while (count--) { std::string e; in >> e; g.edgeNames.insert(e); }
        in >> label >> count;
        while (count--) { Route r; in >> r.ni >> r.nf >> r.ac >> r.p >> r.t; g.addRoute(r.ni, r.nf, r.ac, r.p, r.t); }

        Route extra;
        while (in >> extra.ni >> extra.nf >> extra.ac >> extra.p >> extra.t)
            g.addRoute(extra.ni, extra.nf, extra.ac, extra.p, extra.t);
        return g;
    }

    static Graph loadJson(const std::string& name) {
        std::string text = readFile(name); Graph g;
        g.directed = text.find("\"dirigido\": false") == std::string::npos;
        std::regex routeRe(R"x(\{\s*"ni"\s*:\s*"([^"]+)"\s*,\s*"nf"\s*:\s*"([^"]+)"\s*,\s*"ac"\s*:\s*"([^"]+)"\s*,\s*"p"\s*:\s*([0-9.]+)\s*,\s*"t"\s*:\s*([0-9.]+)\s*\})x");
        for (auto it = std::sregex_iterator(text.begin(), text.end(), routeRe); it != std::sregex_iterator(); ++it)
            g.addRoute((*it)[1], (*it)[2], (*it)[3], std::stod((*it)[4]), std::stod((*it)[5]));
        return g;
    }

    static Graph loadCsv(const std::string& name) {
        std::ifstream in(name); if (!in) throw std::runtime_error("No se pudo abrir " + name);
        Graph g; std::string line; bool routeSection = false;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto c = split(line, ',');
            if (c[0] == "tipo") continue;
            if (c[0] == "ni") { routeSection = true; continue; }
            if (!routeSection && c.size() >= 2) {
                if (c[0] == "dirigido") g.directed = c[1] == "1" || lowerText(c[1]) == "true";
                else if (c[0] == "nodo") g.addNode(c[1]);
            } else if (routeSection && c.size() >= 5) g.addRoute(c[0], c[1], c[2], std::stod(c[3]), std::stod(c[4]));
        }
        return g;
    }

    static Graph loadXml(const std::string& name) {
        std::string text = readFile(name); Graph g;
        g.directed = text.find("dirigido=\"false\"") == std::string::npos;
        std::regex routeRe(R"x(<ruta\s+ni="([^"]+)"\s+nf="([^"]+)"\s+ac="([^"]+)"\s+p="([0-9.]+)"\s+t="([0-9.]+)"\s*/>)x");
        for (auto it = std::sregex_iterator(text.begin(), text.end(), routeRe); it != std::sregex_iterator(); ++it)
            g.addRoute((*it)[1], (*it)[2], (*it)[3], std::stod((*it)[4]), std::stod((*it)[5]));
        return g;
    }

    void saveResultTxt(const std::string& name, const RouteResult& r, const std::string& metric) const {
        std::ofstream out(name); out << "encontrada " << r.found << "\ncosto_" << metric << " " << r.cost << "\n";
        for (const auto& e : r.routes) out << e.ni << " " << e.nf << " " << e.ac << " " << e.p << " " << e.t << "\n";
    }
    void saveResultJson(const std::string& name, const RouteResult& r, const std::string& metric) const {
        std::ofstream out(name); out << "{\"found\":" << (r.found ? "true" : "false") << ",\"metric\":\"" << metric << "\",\"cost\":" << r.cost << ",\"rutas\":[";
        for (std::size_t i = 0; i < r.routes.size(); ++i) { const auto& e = r.routes[i]; out << "{\"ni\":\"" << e.ni << "\",\"nf\":\"" << e.nf << "\",\"ac\":\"" << e.ac << "\",\"p\":" << e.p << ",\"t\":" << e.t << "}" << (i + 1 == r.routes.size() ? "" : ","); }
        out << "]}\n";
    }
    void saveResultCsv(const std::string& name, const RouteResult& r, const std::string& metric) const {
        std::ofstream out(name); out << "found," << r.found << "\nmetric," << metric << "\ncost," << r.cost << "\nni,nf,ac,p,t\n";
        for (const auto& e : r.routes) out << e.ni << "," << e.nf << "," << e.ac << "," << e.p << "," << e.t << "\n";
    }
    void saveResultXml(const std::string& name, const RouteResult& r, const std::string& metric) const {
        std::ofstream out(name); out << "<resultado found=\"" << (r.found ? "true" : "false") << "\" metric=\"" << metric << "\" cost=\"" << r.cost << "\">\n";
        for (const auto& e : r.routes) out << "  <ruta ni=\"" << e.ni << "\" nf=\"" << e.nf << "\" ac=\"" << e.ac << "\" p=\"" << e.p << "\" t=\"" << e.t << "\" />\n";
        out << "</resultado>\n";
    }
    void saveResultDot(const std::string& name, const RouteResult& r, const std::string& metric) const {
        std::ofstream out(name); std::string arrow = directed ? " -> " : " -- ";
        out << (directed ? "digraph" : "graph") << " ruta {\n  rankdir=LR;\n";
        for (const auto& e : r.routes) out << "  \"" << e.ni << "\"" << arrow << "\"" << e.nf << "\" [label=\"" << e.ac << " | " << metric << "=" << value(e, metric) << "\"];\n";
        out << "}\n";
    }
};

#endif

static std::string readText(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string value;
        if (!std::getline(std::cin, value)) throw std::runtime_error("Entrada finalizada.");
        if (!value.empty()) return value;
        std::cout << "La entrada no puede estar vacia. Intente nuevamente.\n";
    }
}

static std::string normalizePath(std::string value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) value.erase(value.begin());
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) value.pop_back();

    if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                              (value.front() == '\'' && value.back() == '\''))) {
        value = value.substr(1, value.size() - 2);
    }

    std::string normalized;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size() &&
            (std::isspace(static_cast<unsigned char>(value[i + 1])) ||
             value[i + 1] == '(' || value[i + 1] == ')')) {
            continue;
        }
        normalized.push_back(value[i]);
    }
    return normalized;
}

static std::string readPath(const std::string& prompt) {
    return normalizePath(readText(prompt));
}

static int readInteger(const std::string& prompt, int minimum, int maximum) {
    while (true) {
        std::string value = readText(prompt);
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) value.pop_back();
        if (!value.empty() && value.back() == '.') value.pop_back();

        std::stringstream parser(value);
        int number = 0;
        char extra = 0;
        if ((parser >> number) && !(parser >> extra) && number >= minimum && number <= maximum)
            return number;

        std::cout << "Opcion no valida. Escriba un numero entre " << minimum
                  << " y " << maximum << ".\n";
    }
}

static bool readYesNo(const std::string& prompt) {
    while (true) {
        std::string answer = lowerText(readText(prompt));
        if (answer == "s" || answer == "si") return true;
        if (answer == "n" || answer == "no") return false;
        std::cout << "Escriba s o n.\n";
    }
}

static void useUnitWeights(Graph& graph) {
    for (auto& route : graph.routes) {
        route.p = 1;
        route.t = 1;
    }
}

static std::string fileBase(const std::string& fileName) {
    std::size_t position = fileName.find_last_of('.');
    return position == std::string::npos ? fileName : fileName.substr(0, position);
}

static void saveGraphSvg(const Graph& graph, const std::string& fileName) {
    std::ofstream out(fileName);
    if (!out) throw std::runtime_error("No se pudo escribir " + fileName);

    const double pi = 3.14159265358979323846;
    const double centerX = 400, centerY = 300, radius = 220;
    std::map<std::string, std::pair<double, double>> positions;
    std::size_t index = 0;
    for (const auto& node : graph.nodes) {
        double angle = graph.nodes.size() <= 1 ? 0 : 2 * pi * index / graph.nodes.size() - pi / 2;
        positions[node] = {centerX + radius * std::cos(angle), centerY + radius * std::sin(angle)};
        ++index;
    }

    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"800\" height=\"600\">\n";
    out << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";
    if (graph.directed)
        out << "<defs><marker id=\"arrow\" markerWidth=\"10\" markerHeight=\"10\" refX=\"9\" refY=\"3\" orient=\"auto\"><path d=\"M0,0 L0,6 L9,3 z\" fill=\"#333\"/></marker></defs>\n";

    for (const auto& route : graph.routes) {
        auto from = positions.at(route.ni), to = positions.at(route.nf);
        double dx = to.first - from.first, dy = to.second - from.second;
        double length = std::sqrt(dx * dx + dy * dy);
        double ux = length == 0 ? 0 : dx / length, uy = length == 0 ? 0 : dy / length;
        out << "<line x1=\"" << from.first + ux * 28 << "\" y1=\"" << from.second + uy * 28
            << "\" x2=\"" << to.first - ux * 32 << "\" y2=\"" << to.second - uy * 32
            << "\" stroke=\"#333\" stroke-width=\"2\""
            << (graph.directed ? " marker-end=\"url(#arrow)\"" : "") << "/>\n";
        out << "<text x=\"" << (from.first + to.first) / 2 << "\" y=\""
            << (from.second + to.second) / 2 - 7
            << "\" text-anchor=\"middle\" font-family=\"Arial\" fill=\"#b42318\">"
            << route.ac << "</text>\n";
    }

    for (const auto& node : graph.nodes) {
        auto point = positions.at(node);
        out << "<circle cx=\"" << point.first << "\" cy=\"" << point.second
            << "\" r=\"27\" fill=\"#e8f3ff\" stroke=\"#175cd3\" stroke-width=\"3\"/>\n";
        out << "<text x=\"" << point.first << "\" y=\"" << point.second + 6
            << "\" text-anchor=\"middle\" font-family=\"Arial\" font-size=\"17\" font-weight=\"bold\">"
            << node << "</text>\n";
    }
    out << "</svg>\n";
}

static void showGraph(const Graph& graph) {
    std::cout << "\nGrafo completo:\n";
    for (std::size_t i = 0; i < graph.routes.size(); ++i) {
        const Route& route = graph.routes[i];
        std::cout << i + 1 << ". " << route.ni
                  << (graph.directed ? " -> " : " -- ") << route.nf
                  << "  arista=" << route.ac << "\n";
    }
}

static void showPath(const RouteResult& path) {
    for (std::size_t i = 0; i < path.nodes.size(); ++i)
        std::cout << path.nodes[i] << (i + 1 == path.nodes.size() ? "\n" : " -> ");
}

static void showAllPaths(const std::vector<RouteResult>& paths) {
    std::cout << "\nTodas las rutas posibles:\n";
    if (paths.empty()) {
        std::cout << "No existe una ruta entre los nodos seleccionados.\n";
        return;
    }
    for (std::size_t i = 0; i < paths.size(); ++i) {
        std::cout << "Ruta " << i + 1 << ":\n";
        showPath(paths[i]);
        std::cout << "Numero de conexiones: " << paths[i].cost << "\n";
    }
}

static void addRoutes(Graph& graph) {
    while (readYesNo("Desea agregar una ruta adicional? (s/n): ")) {
        std::string ni = readText("ni: ");
        std::string nf = readText("nf: ");
        std::string ac = readText("ac: ");
        graph.addRoute(ni, nf, ac, 1, 1);
        std::cout << "Ruta agregada correctamente.\n";
    }
}

static void createGraph() {
    Graph graph;
    graph.directed = readInteger("Es digrafo/dirigido? (1=si, 0=no): ", 0, 1) == 1;
    int nodeCount = readInteger("Cantidad de nodos: ", 1, 100);
    for (int i = 0; i < nodeCount; ++i)
        graph.addNode(readText("Nodo " + std::to_string(i + 1) + ": "));

    int routeCount = readInteger("Cantidad de rutas: ", 0, 1000);
    for (int i = 0; i < routeCount; ++i) {
        std::cout << "Ruta " << i + 1 << ":\n";
        graph.addRoute(readText("  ni: "), readText("  nf: "), readText("  ac: "), 1, 1);
    }

    std::string output = readPath("Archivo de salida (.txt, .json, .csv o .xml): ");
    graph.save(output);
    graph.saveFullGraphDot(fileBase(output) + "_grafo.dot");
    saveGraphSvg(graph, fileBase(output) + "_grafo.svg");
    std::cout << "Grafo guardado. Imagen: " << fileBase(output) << "_grafo.svg\n";
}

static void queryGraph() {
    std::string inputFile = readPath("Archivo del grafo: ");
    Graph graph = Graph::load(inputFile);
    useUnitWeights(graph);

    bool repeat = true;
    while (repeat) {
        if (readYesNo("Desea recargar cambios del archivo? (s/n): ")) {
            graph = Graph::load(inputFile);
            useUnitWeights(graph);
        }
        addRoutes(graph);
        if (readYesNo("Desea guardar las rutas nuevas en el archivo? (s/n): ")) graph.save(inputFile);

        showGraph(graph);
        std::string start = readText("Nodo origen: ");
        std::string goal = readText("Nodo destino: ");
        const std::string metric = "p";
        std::vector<RouteResult> paths = graph.allSimplePaths(start, goal, metric);
        showAllPaths(paths);

        std::string type = lowerText(readText("Tipo de ruta (todas/corta/larga): "));
        std::string output = readPath("Nombre base de resultados: ");
        graph.saveFullGraphDot(output + "_grafo_completo.dot");
        saveGraphSvg(graph, output + "_grafo_completo.svg");

        if (type == "corta") {
            RouteResult result = graph.dijkstraShortest(start, goal, metric);
            std::cout << "Ruta corta:\n"; if (result.found) showPath(result);
            graph.saveResults(output, result, metric);
        } else if (type == "larga") {
            RouteResult result = graph.longestSimplePath(start, goal, metric);
            std::cout << "Ruta larga:\n"; if (result.found) showPath(result);
            graph.saveResults(output, result, metric);
        } else if (type == "todas") {
            for (std::size_t i = 0; i < paths.size(); ++i)
                graph.saveResults(output + "_ruta_" + std::to_string(i + 1), paths[i], metric);
        } else {
            std::cout << "Tipo de ruta no valido.\n";
        }
        std::cout << "Imagen generada: " << output << "_grafo_completo.svg\n";
        repeat = readYesNo("Desea realizar otra consulta? (s/n): ");
    }
}

int main() {
    try {
        while (true) {
            std::cout << "\n1. Crear y exportar grafo\n2. Cargar grafo y consultar rutas\n3. Salir\n";
            int option = readInteger("Opcion: ", 1, 3);
            if (option == 1) createGraph();
            else if (option == 2) queryGraph();
            else break;
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return 1;
    }
    return 0;
}
