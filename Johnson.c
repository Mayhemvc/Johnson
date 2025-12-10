#include <stdio.h>
#include <stdlib.h>

#define INF 9999999

//Bellman-Ford É o único que sabe lidar com os pesos negativos iniciais. Ele roda 1 vez para "limpar" o grafo. Complexidade O(V^2 vezes E)
//Dijkstra É o único rápido o suficiente para rodar V vezes (uma para cada vértice) sem travar o computador.

// Estrutura simples para o nó da lista
typedef struct Node {
    int v; // destino
    int peso;
    struct Node* prox;
} Node;

// Estrutura do Grafo
typedef struct {
    int V; // numero de vertices
    Node** adj;
} Grafo;

// Cria o grafo
Grafo* criarGrafo(int V) {
    Grafo* g = (Grafo*)malloc(sizeof(Grafo));
    g->V = V;
    g->adj = (Node**)malloc(V * sizeof(Node*));
    
    for (int i = 0; i < V; i++) {
        g->adj[i] = NULL;
    }
    return g;
}

// Adiciona aresta direcionada
void addAresta(Grafo* g, int u, int v, int peso) {
    Node* novo = (Node*)malloc(sizeof(Node));
    novo->v = v;
    novo->peso = peso;
    novo->prox = g->adj[u];
    g->adj[u] = novo;
}

// Encontrar o vertice com menor dist (versao linear simples)
int minDistance(int dist[], int processado[], int V) {
    int min = INF, min_index = -1; //O mínimo tem que ser infinito pois o resto sempre será menor. Começa com -1 pois caso continuar com esse quer dizer que não foi encontrado vértice.
    
    for (int v = 0; v < V; v++) {
        if (processado[v] == 0 && dist[v] <= min) {
            min = dist[v];
            min_index = v;
        }
    }
    return min_index;
}

// Bellman-Ford para achar h(v) e detectar ciclos negativos
int bellmanFord(Grafo* g, int src, int h[]) {
    int V = g->V;
    
    for (int i = 0; i < V; i++) h[i] = INF;
    h[src] = 0;
    
    // Relaxamento
    for (int i = 1; i <= V - 1; i++) {
        for (int u = 0; u < V; u++) {
            Node* temp = g->adj[u];
            while (temp != NULL) {
                int v = temp->v;
                int peso = temp->peso;
                if (h[u] != INF && h[u] + peso < h[v]) {
                    h[v] = h[u] + peso;
                }
                temp = temp->prox;
            }
        }
    }
    
    // Check de ciclo negativo
    for (int u = 0; u < V; u++) {
        Node* temp = g->adj[u];
        while (temp != NULL) {
            int v = temp->v;
            int peso = temp->peso;
            if (h[u] != INF && h[u] + peso < h[v]) {
                return 0; // Tem ciclo negativo
            }
            temp = temp->prox;
        }
    }
    return 1;
}

// Dijkstra modificado
void dijkstra(Grafo* g, int src, int dist[], int pred[], int h[]) {
    int V = g->V;
    int* processado = (int*)calloc(V, sizeof(int)); //Reserva a memória e zera tudo (preenche com 0).
    
    for (int i = 0; i < V; i++) {
        dist[i] = INF;
        pred[i] = -1;
    }
    dist[src] = 0;
    
    for (int count = 0; count < V - 1; count++) {
        int u = minDistance(dist, processado, V);
        if (u == -1) break;
        
        processado[u] = 1;
        
        Node* temp = g->adj[u];
        while (temp != NULL) {
            int v = temp->v;
            // Peso reponderado: w' = w + h[u] - h[v]
            int pesoNovo = temp->peso + h[u] - h[v];
            
            if (!processado[v] && dist[u] != INF && dist[u] + pesoNovo < dist[v]) {
                dist[v] = dist[u] + pesoNovo;
                pred[v] = u;
            }
            temp = temp->prox;
        }
    }
    free(processado);
}

void printCaminho(int pred[], int j) {
    if (pred[j] == -1) return;
    printCaminho(pred, pred[j]);
    printf(" -> %d", j);
}

// Algoritmo principal de Johnson
void johnson(Grafo* g) {
    int V = g->V;
    
    // 1. Grafo aumentado com nova fonte
    Grafo* gAux = criarGrafo(V + 1);
    for (int u = 0; u < V; u++) {
        Node* temp = g->adj[u];
        while (temp != NULL) {
            addAresta(gAux, u, temp->v, temp->peso);
            temp = temp->prox;
        }
    }
    // Arestas 0 da nova fonte (indice V) para todos
    for (int i = 0; i < V; i++) {
        addAresta(gAux, V, i, 0);
    }
    
    int* h = (int*)malloc((V + 1) * sizeof(int));
    
    printf("Rodando Bellman-Ford...\n");
    if (!bellmanFord(gAux, V, h)) {
        printf("Ciclo negativo detectado! Impossivel continuar.\n");
        free(h);
        return;
    }
    
    printf("Nao ha ciclos negativos. Rodando Dijkstra para cada vertice...\n\n");
    
    // Matrizes para resultado
    int** distMatrix = (int**)malloc(V * sizeof(int*));
    int** predMatrix = (int**)malloc(V * sizeof(int*));
    
    for (int u = 0; u < V; u++) {
        distMatrix[u] = (int*)malloc(V * sizeof(int));
        predMatrix[u] = (int*)malloc(V * sizeof(int));
        
        dijkstra(g, u, distMatrix[u], predMatrix[u], h);
    }
    
    // Exibir resultados (distancia real = dist_dijkstra - h[u] + h[v])
    printf("Matriz de Distancias Finais:\n");
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            if (distMatrix[i][j] == INF) {
                printf("%7s ", "INF");
            } else {
                // Ajuste final da distancia
                int realDist = distMatrix[i][j] - h[i] + h[j];
                printf("%7d ", realDist);
            }
        }
        printf("\n");
    }

    // Limpeza basica (estudante as vezes esquece de liberar tudo, mas vamos liberar o grosso)
    free(h);
    // (Poderia ter um liberarGrafo aqui, mas deixei de fora pra "simplificar")
}

int main() {
    int V, E;
    int u, v, w;
    
    printf("Algoritmo de Johnson\n");
    printf("Qtd Vertices: ");
    scanf("%d", &V);
    
    Grafo* g = criarGrafo(V);
    
    printf("Qtd Arestas: ");
    scanf("%d", &E);
    
    printf("Digite as arestas (origem destino peso):\n");
    for (int i = 0; i < E; i++) {
        scanf("%d %d %d", &u, &v, &w);
        if(u >= V || v >= V) {
            printf("Vertice invalido, ignorando...\n");
            continue;
        }
        addAresta(g, u, v, w);
    }
    
    johnson(g);
    
    return 0;
}