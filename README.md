# Herramienta de grafos y digrafos en C++

El proyecto usa la estructura de la imagen:

- `nodos`: vertices del grafo.
- `aristas`: nombres/codigos de las conexiones.
- `rutas`: conexiones con datos.
- `ni`: nodo inicial.
- `nf`: nodo final.
- `ac`: arista usada.
- `p`: peso o costo.
- `t`: tiempo.

Si `dirigido` es `1` o `true`, el grafo funciona como digrafo: una ruta `A -> B` no permite viajar automaticamente de `B -> A`.

## Archivos

- `graph.hpp`: contiene la clase `Graph`, Dijkstra, ruta larga simple, y lectura/escritura TXT, JSON, CSV y XML.
- `graph_exporter.cpp`: captura un grafo/digrafo y lo exporta segun la extension del archivo.
- `graph_router.cpp`: carga TXT, JSON, CSV o XML y calcula ruta corta o larga.

## Compilar

```bash
g++ -std=c++17 graph_exporter.cpp -o graph_exporter
g++ -std=c++17 graph_router.cpp -o graph_router
```

## Crear un archivo de grafo

```bash
./graph_exporter
```

El programa pregunta:

- si es digrafo/dirigido: `1` para si, `0` para no
- cantidad de nodos
- nombre de cada nodo
- cantidad de rutas/aristas
- cada ruta en formato `ni nf ac p t`
- archivo de salida: `.txt`, `.json`, `.csv` o `.xml`

Ejemplo de ruta:

```txt
F A a 5 5
```

Significa: de `F` a `A`, por la arista `a`, con peso `5` y tiempo `5`.

## Buscar una ruta

```bash
./graph_router
```

El programa pregunta:

- archivo de entrada: `.txt`, `.json`, `.csv` o `.xml`
- nodo inicial
- nodo final
- tipo de ruta: `corta` o `larga`
- criterio: `p` para peso/costo o `t` para tiempo
- nombre base para los resultados

El resultado se genera en:

- `.txt`
- `.json`
- `.csv`
- `.xml`
- `.dot`

El archivo `.dot` sirve para visualizar el grafo de la ruta en Graphviz.

## Formato TXT

```txt
dirigido 1
nodos 6 A B C D E F
aristas 7 a b c d e f g
rutas 7
F A a 5 5
A B b 2 3
B C c 4 2
C D d 1 4
D E e 3 6
E F f 2 1
A D g 10 2
```

## Formato JSON

```json
{
  "dirigido": true,
  "nodos": ["A", "B", "C", "D", "E", "F"],
  "aristas": ["a", "b", "c", "d", "e", "f", "g"],
  "rutas": [
    {"ni": "F", "nf": "A", "ac": "a", "p": 5, "t": 5},
    {"ni": "A", "nf": "B", "ac": "b", "p": 2, "t": 3}
  ]
}
```

## Formato CSV

```csv
tipo,valor
dirigido,1
nodo,A
nodo,B

ni,nf,ac,p,t
F,A,a,5,5
A,B,b,2,3
```

## Formato XML

```xml
<grafo dirigido="true">
  <nodos>
    <nodo id="A" />
    <nodo id="B" />
  </nodos>
  <aristas>
    <arista id="a" />
    <arista id="b" />
  </aristas>
  <rutas>
    <ruta ni="F" nf="A" ac="a" p="5" t="5" />
    <ruta ni="A" nf="B" ac="b" p="2" t="3" />
  </rutas>
</grafo>
```

## Algoritmos

La ruta corta usa Dijkstra, por lo que `p` y `t` deben ser valores no negativos.

La ruta larga se calcula como ruta simple mas larga, sin repetir nodos. Esto evita ciclos infinitos.
