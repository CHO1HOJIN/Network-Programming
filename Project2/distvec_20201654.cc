#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

using namespace std;
vector<int> queue;
void debug_print(vector<vector<vector<int>>> &graph){
    int n = graph.size();
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            if(graph[i][j][0] != -999) cout << graph[i][j][0] << " " << graph[i][j][1] << endl;
        }
        cout << endl;
    }
}
void print_graph_to_outputfile(ofstream & outputfile, vector<vector<vector<int>>> &graph){
    int n = graph.size();
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            if(graph[i][j][0] != -999) outputfile << j << " " << graph[i][j][1] << " " << graph[i][j][0] << endl;
        }
        outputfile << endl;
    }
}
void initialize(ifstream &topologyfile, vector<vector<vector<int>>> &graph);
void real_routing(int i, int j, int distance_from_to, vector<vector<vector<int>>> &update);
void routing(int n, vector<vector<vector<int>>> &graph);
void find_route(ofstream & outputfile, int i, int j, vector<vector<vector<int>>> &graph, string &buf);
void change(ofstream & outputfile, ifstream & changesfile, ifstream & messagefile, vector<vector<vector<int>>> &graph);
void message(ofstream & outputfile, ifstream & messagefile, vector<vector<vector<int>>> &graph);
int main(int argc, char** argv){
    if(argc != 4){
        cout << "usage: distvec topologyfile messagefile changesfile" << endl;
        exit(0);
    }
    ifstream topologyfile, messagefile, changesfile;
    topologyfile.open(argv[1]);
    messagefile.open(argv[2]);
    changesfile.open(argv[3]);
    if(topologyfile.fail() || messagefile.fail() || changesfile.fail()){
        cout << "Error: open input file." << endl;
        exit(0);
    }
    int n;
    topologyfile >> n;
    ofstream outputfile;
    outputfile.open("output_dv.txt");
    if(outputfile.fail()){
        cout << "Error: open output file." << endl;
        exit(0);
    }
    
    vector<vector<vector<int>>> graph(n, vector<vector<int>>(n, vector<int>(2)));
    fflush(stdout);

    initialize(topologyfile, graph);
    vector<vector<vector<int>>> before = graph;
    routing(n, graph);
    print_graph_to_outputfile(outputfile, graph);
    message(outputfile, messagefile, graph);
    change(outputfile, changesfile, messagefile, before);
    
    topologyfile.close();
    messagefile.close();
    changesfile.close();
    outputfile.close();

    cout << "Complete. Output file written to output_dv.txt" << endl;
    return 0;
}

void initialize(ifstream & topologyfile, vector<vector<vector<int>>> &graph){
    string buf;
    while(getline(topologyfile, buf)){
        istringstream line(buf);
        int src, dest, cost;
        line >> src >> dest >> cost;
        graph[src][dest][0] = cost;
        graph[src][dest][1] = dest;
        graph[dest][src][0] = cost;
        graph[dest][src][1] = src;
    }
    int n = graph.size();
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            if(i == j) {
                graph[i][j][0] = 0;
                graph[i][j][1] = i;
            }
            else if(graph[i][j][0] == 0) graph[i][j][0] = -999;
        }
    
    }
}
void real_routing(int i, int j, int distance_from_to, vector<vector<vector<int>>> &graph){
    int n = graph.size();
    for(int k = 0; k < n; k++){ //node i와 연결된 노드 확인
        if(k!= j && graph[i][k][0] != 0 && graph[i][k][0] != -999){ // if i and k are connected
            if((graph[i][k][0] + distance_from_to < graph[j][k][0]) || graph[j][k][0] == -999){
                graph[j][k][0] = graph[i][k][0] + distance_from_to;
                graph[j][k][1] = i;
                if (find(queue.begin(), queue.end(), j) == queue.end()) {
                    queue.push_back(j);
                }
            }
            else if(graph[i][k][0] + distance_from_to == graph[j][k][0]){ 
                if(i < graph[j][k][1]){
                    graph[j][k][1] = i;
                    if (find(queue.begin(), queue.end(), j) == queue.end()) {
                    queue.push_back(j);
                    }
                }
            }
        }
    }

}
void routing(int n, vector<vector<vector<int>>>& graph){
    vector<vector<vector<int>>> before = graph;
    for(int i = 0; i < n; i++) queue.push_back(i);
    while(!queue.empty()){
        int i = queue[0];
        for(int j = 0; j < n; j++){
            if(before[i][j][0] != 0 && before[i][j][0] != -999){  // if i and j are connected
                real_routing(i, j, before[i][j][0], graph);
            }
        }
        queue.erase(queue.begin());
    }
    
}

void find_route(ofstream & outputfile, int src, int dst, vector<vector<vector<int>>> &graph ,string &buf){
    string print_buf;
    print_buf = "from " + to_string(src) + " to " + to_string(dst) + " cost ";
    if(graph[src][dst][0] == -999){
        print_buf += "infinite hops unreachable message";
        print_buf += buf;
        outputfile << print_buf << endl;
        return;
    }
    else print_buf += to_string(graph[src][dst][0]) + " hops ";
    int k = src;
    while(k!=graph[k][dst][1]){
        print_buf += to_string(k) + " ";
        k = graph[k][dst][1];
    }
    outputfile << print_buf << "message" << buf << endl;
}
void message(ofstream & outputfile, ifstream & messagefile, vector<vector<vector<int>>> &graph){
    string buf, buf2;
    while(getline(messagefile, buf)){
        istringstream line(buf);
        int src, dest;
        line >> src >> dest;
        getline(line, buf2);
        //printf("message %d %d %s\n", src, dest, buf2.c_str());
        find_route(outputfile, src, dest, graph, buf2);
    }
    outputfile << endl;
}
void change(ofstream & outputfile, ifstream & changesfile, ifstream & messagefile, vector<vector<vector<int>>> &graph){
    string buf;
    vector<vector<vector<int>>> before = graph;
    while(getline(changesfile, buf)){
        istringstream line(buf);
        int src, dest, cost;
        line >> src >> dest >> cost;
        //printf("change from %d to %d distance: %d\n", src, dest, cost);
        before[src][dest][0] = cost;
        before[dest][src][0] = cost;
        before[src][dest][1] = dest;
        before[dest][src][1] = src;

        graph = before;
        routing(graph.size(), graph);
        print_graph_to_outputfile(outputfile, graph);
        messagefile.clear();
        messagefile.seekg(0, ios::beg);
        message(outputfile, messagefile, graph);
    }
}
