// Ejercicio 4: Iterator - Recorrido de Grafo
#include <iostream>
#include <memory>
#include <queue>
#include <stack>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace std;
class Node {
public:
    explicit Node(int id) : id_(id) {}
    int getId() const { return id_; }
    void addNeighbor(int id) { neighbors_.push_back(id); }
    const vector<int>& getNeighbors() const { return neighbors_; }
private:
    int id_;
    vector<int> neighbors_;
};

class Graph {
public:

    void addEdge(int from, int to) {
        nodes_.try_emplace(from, from);
        nodes_.try_emplace(to, to);
        nodes_.at(from).addNeighbor(to);
    }

    bool hasNode(int id) const { return nodes_.count(id) > 0; }


    vector<int> neighborsOf(int id) const {
        auto it = nodes_.find(id);
        if (it == nodes_.end()) return {};
        return it->second.getNeighbors();
    }

private:
    unordered_map<int, Node> nodes_;
};


class GraphIterator {
public:
    virtual ~GraphIterator() = default;
    virtual bool hasNext() = 0;
    virtual int next() = 0;
};


class BFSIterator : public GraphIterator {
public:
    BFSIterator(const Graph& g, int start) : graph_(g) {
        if (!g.hasNode(start)) throw invalid_argument("Nodo inicial inexistente");
        queue_.push(start);
        visited_.insert(start);  // se marca al encolar: evita duplicados y ciclos
    }

    bool hasNext() override { return !queue_.empty(); }

    int next() override {
        if (queue_.empty()) throw out_of_range("No hay mas nodos");
        int current = queue_.front();
        queue_.pop();
        for (int n : graph_.neighborsOf(current)) {
            if (visited_.insert(n).second) queue_.push(n);
        }
        return current;
    }

private:
    const Graph& graph_;
    queue<int> queue_;
    unordered_set<int> visited_;
};


class DFSIterator : public GraphIterator {
public:
    DFSIterator(const Graph& g, int start) : graph_(g) {
        if (!g.hasNode(start)) throw invalid_argument("Nodo inicial inexistente");
        stack_.push(start);
    }

    bool hasNext() override {
        //Descarta nodos ya visitados que quedaron en la pila
        while (!stack_.empty() && visited_.count(stack_.top())) stack_.pop();
        return !stack_.empty();
    }

    int next() override {
        if (!hasNext()) throw out_of_range("No hay mas nodos");
        int current = stack_.top();
        stack_.pop();
        visited_.insert(current);
        // Se apilan al reves para visitar primero al primer vecino
        auto neighbors = graph_.neighborsOf(current);
        for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it) {
            if (!visited_.count(*it)) stack_.push(*it);
        }
        return current;
    }

private:
    const Graph& graph_;
    stack<int> stack_;
    unordered_set<int> visited_;
};

void traverse(GraphIterator& it) {
    while (it.hasNext()) cout << it.next() << " ";
    cout << endl;
}

int main() {
    Graph g;
    g.addEdge(1, 2);
    g.addEdge(1, 3);
    g.addEdge(2, 4);

    unique_ptr<GraphIterator> it = make_unique<BFSIterator>(g, 1);
    while (it->hasNext()) {
        cout << it->next() << " ";
    }
    cout << endl;

    it = make_unique<DFSIterator>(g, 1);
    cout << "DFS: ";
    traverse(*it);


    Graph c;
    c.addEdge(1, 2);
    c.addEdge(2, 3);
    c.addEdge(3, 1);
    c.addEdge(3, 4);
    c.addEdge(4, 2);
    cout << "Ciclico BFS: ";
    it = make_unique<BFSIterator>(c, 1);
    traverse(*it);
    cout << "Ciclico DFS: ";
    it = make_unique<DFSIterator>(c, 1);
    traverse(*it);
}
