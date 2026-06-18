#ifndef GRAPH_TOOL_GRAPH_HPP
#define GRAPH_TOOL_GRAPH_HPP

#include <algorithm>
#include <cctype>
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
    std::string ni;
    std::string nf;
    std::string ac;
    double p = 0.0;
    double t = 0.0;
};

struct RouteResult {
    bool found = false;
    double cost = 0.0;
    std::vector<std::string> nodes;
    std::vector<Route> routes;
};

inline std::string lowerText(std::string value) {
    for (char& c : value) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return value;
}

inline std::string fileExtension(const std::string& fileName) {
    std::size_t pos = fileName.find_last_of('.');
    if (pos == std::string::npos) {
        return "";
    }
    return lowerText(fileName.substr(pos + 1));
}

class Graph {
public:
    bool directed = true;
    std::set<std::string> nodos;
    std::set<std::string> aristas;
    std::vector<Route> rutas;

    void addNode(const std::string& nodo) {
        nodos.insert(nodo);
    }

    void addRoute(const std::string& ni, const std::string& nf, const std::string& ac, double p, double t) {
        if (p < 0 || t < 0) {
            throw std::runtime_error("Dijkstra requiere pesos y tiempos no negativos.");
        }
        nodos.insert(ni);
        nodos.insert(nf);
        aristas.insert(ac);
        rutas.push_back({ni, nf, ac, p, t});
    }

    RouteResult dijkstraShortest(const std::string& start, const std::string& goal, const std::string& metric) const {
        const double inf = std::numeric_limits<double>::infinity();
        std::map<std::string, double> dist;
        std::map<std::string, Route> parent;

        for (const auto& nodo : nodos) {
            dist[nodo] = inf;
        }
        if (!nodos.count(start) || !nodos.count(goal)) {
            return {};
        }

        using Item = std::pair<double, std::string>;
        std::priority_queue<Item, std::vector<Item>, std::greater<Item>> pq;
        dist[start] = 0.0;
        pq.push({0.0, start});

        while (!pq.empty()) {
            auto [currentCost, current] = pq.top();
            pq.pop();
            if (currentCost != dist[current]) {
                continue;
            }
            if (current == goal) {
                break;
            }
            for (const auto& route : outgoing(current)) {
                double candidate = currentCost + metricValue(route, metric);
                if (candidate < dist[route.nf]) {
                    dist[route.nf] = candidate;
                    parent[route.nf] = route;
                    pq.push({candidate, route.nf});
                }
            }
        }

        if (dist[goal] == inf) {
            return {};
        }
        return buildRoute(start, goal, dist[goal], parent);
    }

    RouteResult longestSimplePath(const std::string& start, const std::string& goal, const std::string& metric) const {
        RouteResult best;
        if (!nodos.count(start) || !nodos.count(goal)) {
            return best;
        }
        std::set<std::string> visited;
        std::vector<std::string> nodePath;
        std::vector<Route> routePath;
        dfsLongest(start, goal, metric, 0.0, visited, nodePath, routePath, best);
        return best;
    }

    void saveByExtension(const std::string& fileName) const {
        std::string ext = fileExtension(fileName);
        if (ext == "json") {
            saveJson(fileName);
        } else if (ext == "csv") {
            saveCsv(fileName);
        } else if (ext == "xml") {
            saveXml(fileName);
        } else {
            saveTxt(fileName);
        }
    }

    void saveTxt(const std::string& fileName) const {
        std::ofstream out(fileName);
        requireOutput(out, "TXT");
        out << "dirigido " << (directed ? 1 : 0) << "\n";
        out << "nodos " << nodos.size();
        for (const auto& nodo : nodos) {
            out << " " << nodo;
        }
        out << "\n";
        out << "aristas " << aristas.size();
        for (const auto& arista : aristas) {
            out << " " << arista;
        }
        out << "\n";
        out << "rutas " << rutas.size() << "\n";
        for (const auto& route : rutas) {
            out << route.ni << " " << route.nf << " " << route.ac << " " << route.p << " " << route.t << "\n";
        }
    }

    void saveJson(const std::string& fileName) const {
        std::ofstream out(fileName);
        requireOutput(out, "JSON");
        out << "{\n";
        out << "  \"dirigido\": " << (directed ? "true" : "false") << ",\n";
        out << "  \"nodos\": [";
        writeStringSet(out, nodos);
        out << "],\n";
        out << "  \"aristas\": [";
        writeStringSet(out, aristas);
        out << "],\n";
        out << "  \"rutas\": [\n";
        for (std::size_t i = 0; i < rutas.size(); ++i) {
            writeRouteJson(out, rutas[i], "    ");
            out << (i + 1 == rutas.size() ? "\n" : ",\n");
        }
        out << "  ]\n";
        out << "}\n";
    }

    void saveCsv(const std::string& fileName) const {
        std::ofstream out(fileName);
        requireOutput(out, "CSV");
        out << "tipo,valor\n";
        out << "dirigido," << (directed ? 1 : 0) << "\n";
        for (const auto& nodo : nodos) {
            out << "nodo," << nodo << "\n";
        }
        out << "\n";
        out << "ni,nf,ac,p,t\n";
        for (const auto& route : rutas) {
            out << route.ni << "," << route.nf << "," << route.ac << "," << route.p << "," << route.t << "\n";
        }
    }

    void saveXml(const std::string& fileName) const {
        std::ofstream out(fileName);
        requireOutput(out, "XML");
        out << "<grafo dirigido=\"" << (directed ? "true" : "false") << "\">\n";
        out << "  <nodos>\n";
        for (const auto& nodo : nodos) {
            out << "    <nodo id=\"" << escapeXml(nodo) << "\" />\n";
        }
        out << "  </nodos>\n";
        out << "  <aristas>\n";
        for (const auto& arista : aristas) {
            out << "    <arista id=\"" << escapeXml(arista) << "\" />\n";
        }
        out << "  </aristas>\n";
        out << "  <rutas>\n";
        for (const auto& route : rutas) {
            out << "    <ruta ni=\"" << escapeXml(route.ni) << "\" nf=\"" << escapeXml(route.nf)
                << "\" ac=\"" << escapeXml(route.ac) << "\" p=\"" << route.p
                << "\" t=\"" << route.t << "\" />\n";
        }
        out << "  </rutas>\n";
        out << "</grafo>\n";
    }

    void saveRouteGraphTxt(const std::string& fileName, const RouteResult& result, const std::string& metric) const {
        std::ofstream out(fileName);
        requireOutput(out, "TXT");
        if (!result.found) {
            out << "No existe ruta.\n";
            return;
        }
        out << "costo_" << metric << " " << result.cost << "\n";
        out << "nodos";
        for (const auto& nodo : result.nodes) {
            out << " " << nodo;
        }
        out << "\n";
        out << "rutas " << result.routes.size() << "\n";
        for (const auto& route : result.routes) {
            out << route.ni << " " << route.nf << " " << route.ac << " " << route.p << " " << route.t << "\n";
        }
    }

    void saveRouteGraphJson(const std::string& fileName, const RouteResult& result, const std::string& metric) const {
        std::ofstream out(fileName);
        requireOutput(out, "JSON");
        out << "{\n";
        out << "  \"found\": " << (result.found ? "true" : "false") << ",\n";
        out << "  \"metric\": \"" << escapeJson(metric) << "\",\n";
        out << "  \"cost\": " << result.cost << ",\n";
        out << "  \"nodos\": [";
        for (std::size_t i = 0; i < result.nodes.size(); ++i) {
            out << "\"" << escapeJson(result.nodes[i]) << "\"" << (i + 1 == result.nodes.size() ? "" : ", ");
        }
        out << "],\n";
        out << "  \"rutas\": [\n";
        for (std::size_t i = 0; i < result.routes.size(); ++i) {
            writeRouteJson(out, result.routes[i], "    ");
            out << (i + 1 == result.routes.size() ? "\n" : ",\n");
        }
        out << "  ]\n";
        out << "}\n";
    }

    void saveRouteGraphCsv(const std::string& fileName, const RouteResult& result, const std::string& metric) const {
        std::ofstream out(fileName);
        requireOutput(out, "CSV");
        out << "found," << (result.found ? 1 : 0) << "\n";
        out << "metric," << metric << "\n";
        out << "cost," << result.cost << "\n";
        out << "ni,nf,ac,p,t\n";
        for (const auto& route : result.routes) {
            out << route.ni << "," << route.nf << "," << route.ac << "," << route.p << "," << route.t << "\n";
        }
    }

    void saveRouteGraphXml(const std::string& fileName, const RouteResult& result, const std::string& metric) const {
        std::ofstream out(fileName);
        requireOutput(out, "XML");
        out << "<resultado found=\"" << (result.found ? "true" : "false") << "\" metric=\""
            << escapeXml(metric) << "\" cost=\"" << result.cost << "\">\n";
        out << "  <nodos>\n";
        for (const auto& nodo : result.nodes) {
            out << "    <nodo id=\"" << escapeXml(nodo) << "\" />\n";
        }
        out << "  </nodos>\n";
        out << "  <rutas>\n";
        for (const auto& route : result.routes) {
            out << "    <ruta ni=\"" << escapeXml(route.ni) << "\" nf=\"" << escapeXml(route.nf)
                << "\" ac=\"" << escapeXml(route.ac) << "\" p=\"" << route.p
                << "\" t=\"" << route.t << "\" />\n";
        }
        out << "  </rutas>\n";
        out << "</resultado>\n";
    }

    void saveRouteGraphDot(const std::string& fileName, const RouteResult& result, const std::string& metric) const {
        std::ofstream out(fileName);
        requireOutput(out, "DOT");
        std::string arrow = directed ? " -> " : " -- ";
        out << (directed ? "digraph" : "graph") << " ruta {\n";
        out << "  rankdir=LR;\n";
        if (!result.found) {
            out << "  \"sin_ruta\";\n";
        } else {
            for (const auto& nodo : result.nodes) {
                out << "  \"" << escapeJson(nodo) << "\" [shape=circle, style=filled, fillcolor=\"#d6f5d6\"];\n";
            }
            for (const auto& route : result.routes) {
                out << "  \"" << escapeJson(route.ni) << "\"" << arrow << "\"" << escapeJson(route.nf)
                    << "\" [label=\"" << escapeJson(route.ac) << " | " << metric
                    << "=" << metricValue(route, metric) << "\"];\n";
            }
        }
        out << "}\n";
    }

    static Graph loadByExtension(const std::string& fileName) {
        std::string ext = fileExtension(fileName);
        if (ext == "json") {
            return loadJson(fileName);
        }
        if (ext == "csv") {
            return loadCsv(fileName);
        }
        if (ext == "xml") {
            return loadXml(fileName);
        }
        return loadTxt(fileName);
    }

    static Graph loadTxt(const std::string& fileName) {
        std::ifstream in(fileName);
        requireInput(in, "TXT");
        Graph graph;
        std::string label;
        in >> label >> graph.directed;

        std::size_t count = 0;
        in >> label >> count;
        for (std::size_t i = 0; i < count; ++i) {
            std::string nodo;
            in >> nodo;
            graph.addNode(nodo);
        }

        in >> label >> count;
        for (std::size_t i = 0; i < count; ++i) {
            std::string arista;
            in >> arista;
            graph.aristas.insert(arista);
        }

        in >> label >> count;
        for (std::size_t i = 0; i < count; ++i) {
            std::string ni;
            std::string nf;
            std::string ac;
            double p = 0.0;
            double t = 0.0;
            in >> ni >> nf >> ac >> p >> t;
            graph.addRoute(ni, nf, ac, p, t);
        }
        return graph;
    }

    static Graph loadJson(const std::string& fileName) {
        std::ifstream in(fileName);
        requireInput(in, "JSON");
        std::ostringstream buffer;
        buffer << in.rdbuf();
        std::string text = buffer.str();

        Graph graph;
        graph.directed = readBool(text, "dirigido", true);
        for (const auto& nodo : readStringArray(text, "nodos")) {
            graph.addNode(nodo);
        }
        for (const auto& arista : readStringArray(text, "aristas")) {
            graph.aristas.insert(arista);
        }

        std::regex routeRegex(R"json(\{\s*"ni"\s*:\s*"([^"]+)"\s*,\s*"nf"\s*:\s*"([^"]+)"\s*,\s*"ac"\s*:\s*"([^"]+)"\s*,\s*"p"\s*:\s*(-?[0-9]+(?:\.[0-9]+)?)\s*,\s*"t"\s*:\s*(-?[0-9]+(?:\.[0-9]+)?)\s*\})json");
        for (auto it = std::sregex_iterator(text.begin(), text.end(), routeRegex); it != std::sregex_iterator(); ++it) {
            graph.addRoute((*it)[1].str(), (*it)[2].str(), (*it)[3].str(), std::stod((*it)[4].str()), std::stod((*it)[5].str()));
        }
        return graph;
    }

    static Graph loadCsv(const std::string& fileName) {
        std::ifstream in(fileName);
        requireInput(in, "CSV");
        Graph graph;
        std::string line;
        bool routeSection = false;

        while (std::getline(in, line)) {
            if (line.empty()) {
                continue;
            }
            std::vector<std::string> cols = split(line, ',');
            if (cols.empty()) {
                continue;
            }
            if (cols[0] == "tipo" || cols[0] == "ni") {
                routeSection = cols[0] == "ni";
                continue;
            }
            if (!routeSection && cols.size() >= 2) {
                if (cols[0] == "dirigido") {
                    graph.directed = cols[1] == "1" || lowerText(cols[1]) == "true";
                } else if (cols[0] == "nodo") {
                    graph.addNode(cols[1]);
                }
            } else if (routeSection && cols.size() >= 5) {
                graph.addRoute(cols[0], cols[1], cols[2], std::stod(cols[3]), std::stod(cols[4]));
            }
        }
        return graph;
    }

    static Graph loadXml(const std::string& fileName) {
        std::ifstream in(fileName);
        requireInput(in, "XML");
        std::ostringstream buffer;
        buffer << in.rdbuf();
        std::string text = buffer.str();

        Graph graph;
        graph.directed = text.find("dirigido=\"false\"") == std::string::npos;

        std::regex nodeRegex(R"xml(<nodo\s+id="([^"]+)"\s*/>)xml");
        for (auto it = std::sregex_iterator(text.begin(), text.end(), nodeRegex); it != std::sregex_iterator(); ++it) {
            graph.addNode((*it)[1].str());
        }

        std::regex edgeRegex(R"xml(<arista\s+id="([^"]+)"\s*/>)xml");
        for (auto it = std::sregex_iterator(text.begin(), text.end(), edgeRegex); it != std::sregex_iterator(); ++it) {
            graph.aristas.insert((*it)[1].str());
        }

        std::regex routeRegex(R"xml(<ruta\s+ni="([^"]+)"\s+nf="([^"]+)"\s+ac="([^"]+)"\s+p="(-?[0-9]+(?:\.[0-9]+)?)"\s+t="(-?[0-9]+(?:\.[0-9]+)?)"\s*/>)xml");
        for (auto it = std::sregex_iterator(text.begin(), text.end(), routeRegex); it != std::sregex_iterator(); ++it) {
            graph.addRoute((*it)[1].str(), (*it)[2].str(), (*it)[3].str(), std::stod((*it)[4].str()), std::stod((*it)[5].str()));
        }
        return graph;
    }

private:
    std::vector<Route> outgoing(const std::string& nodo) const {
        std::vector<Route> result;
        for (const auto& route : rutas) {
            if (route.ni == nodo) {
                result.push_back(route);
            }
            if (!directed && route.nf == nodo) {
                result.push_back({route.nf, route.ni, route.ac, route.p, route.t});
            }
        }
        return result;
    }

    static RouteResult buildRoute(const std::string& start, const std::string& goal, double cost, const std::map<std::string, Route>& parent) {
        RouteResult result;
        result.found = true;
        result.cost = cost;
        std::string current = goal;
        while (current != start) {
            auto it = parent.find(current);
            if (it == parent.end()) {
                return {};
            }
            result.routes.push_back(it->second);
            result.nodes.push_back(current);
            current = it->second.ni;
        }
        result.nodes.push_back(start);
        std::reverse(result.nodes.begin(), result.nodes.end());
        std::reverse(result.routes.begin(), result.routes.end());
        return result;
    }

    void dfsLongest(
        const std::string& current,
        const std::string& goal,
        const std::string& metric,
        double cost,
        std::set<std::string>& visited,
        std::vector<std::string>& nodePath,
        std::vector<Route>& routePath,
        RouteResult& best
    ) const {
        visited.insert(current);
        nodePath.push_back(current);
        if (current == goal) {
            if (!best.found || cost > best.cost) {
                best.found = true;
                best.cost = cost;
                best.nodes = nodePath;
                best.routes = routePath;
            }
        } else {
            for (const auto& route : outgoing(current)) {
                if (!visited.count(route.nf)) {
                    routePath.push_back(route);
                    dfsLongest(route.nf, goal, metric, cost + metricValue(route, metric), visited, nodePath, routePath, best);
                    routePath.pop_back();
                }
            }
        }
        nodePath.pop_back();
        visited.erase(current);
    }

    static double metricValue(const Route& route, const std::string& metric) {
        return metric == "t" || metric == "tiempo" ? route.t : route.p;
    }

    static void requireOutput(const std::ofstream& out, const std::string& type) {
        if (!out) {
            throw std::runtime_error("No se pudo escribir el archivo " + type + ".");
        }
    }

    static void requireInput(const std::ifstream& in, const std::string& type) {
        if (!in) {
            throw std::runtime_error("No se pudo abrir el archivo " + type + ".");
        }
    }

    static std::vector<std::string> split(const std::string& line, char delimiter) {
        std::vector<std::string> result;
        std::stringstream ss(line);
        std::string item;
        while (std::getline(ss, item, delimiter)) {
            result.push_back(item);
        }
        return result;
    }

    static void writeStringSet(std::ofstream& out, const std::set<std::string>& values) {
        std::size_t i = 0;
        for (const auto& value : values) {
            out << "\"" << escapeJson(value) << "\"" << (++i == values.size() ? "" : ", ");
        }
    }

    static void writeRouteJson(std::ofstream& out, const Route& route, const std::string& indent) {
        out << indent << "{\"ni\": \"" << escapeJson(route.ni) << "\", "
            << "\"nf\": \"" << escapeJson(route.nf) << "\", "
            << "\"ac\": \"" << escapeJson(route.ac) << "\", "
            << "\"p\": " << route.p << ", "
            << "\"t\": " << route.t << "}";
    }

    static bool readBool(const std::string& text, const std::string& key, bool fallback) {
        std::size_t pos = text.find("\"" + key + "\"");
        if (pos == std::string::npos) {
            return fallback;
        }
        std::size_t truePos = text.find("true", pos);
        std::size_t falsePos = text.find("false", pos);
        if (truePos == std::string::npos) {
            return false;
        }
        if (falsePos == std::string::npos) {
            return true;
        }
        return truePos < falsePos;
    }

    static std::vector<std::string> readStringArray(const std::string& text, const std::string& key) {
        std::vector<std::string> values;
        std::size_t keyPos = text.find("\"" + key + "\"");
        if (keyPos == std::string::npos) {
            return values;
        }
        std::size_t open = text.find("[", keyPos);
        std::size_t close = text.find("]", open);
        if (open == std::string::npos || close == std::string::npos) {
            return values;
        }
        std::string arrayText = text.substr(open + 1, close - open - 1);
        std::regex valueRegex(R"json("([^"]+)")json");
        for (auto it = std::sregex_iterator(arrayText.begin(), arrayText.end(), valueRegex); it != std::sregex_iterator(); ++it) {
            values.push_back((*it)[1].str());
        }
        return values;
    }

    static std::string escapeJson(const std::string& value) {
        std::string escaped;
        for (char c : value) {
            if (c == '"' || c == '\\') {
                escaped.push_back('\\');
            }
            escaped.push_back(c);
        }
        return escaped;
    }

    static std::string escapeXml(const std::string& value) {
        std::string escaped;
        for (char c : value) {
            if (c == '&') {
                escaped += "&amp;";
            } else if (c == '"') {
                escaped += "&quot;";
            } else if (c == '<') {
                escaped += "&lt;";
            } else if (c == '>') {
                escaped += "&gt;";
            } else {
                escaped.push_back(c);
            }
        }
        return escaped;
    }
};

#endif
