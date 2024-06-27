#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
using namespace std;
vector<int> queue;
int change_count = 0;
void debug_print(vector<vector<vector<int>>> &graph){
    int n = graph.size();
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            cout << graph[i][j][0] << " " << graph[i][j][1] << endl;
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
void dijkstra(int n, vector<vector<vector<int>>> &graph);
void find_route(ofstream & outputfile, int i, int j, vector<vector<vector<int>>> &graph, string &buf);
void change(ofstream & outputfile, ifstream & changesfile, ifstream & messagefile, vector<vector<vector<int>>> &graph);
void message(ofstream & outputfile, ifstream & messagefile, vector<vector<vector<int>>> &graph);
int main(int argc, char** argv){
    if(argc != 4){
        cout << "usage: linkstate topologyfile messagefile changesfile" << endl;
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
    outputfile.open("output_ls.txt");
    if(outputfile.fail()){
        cout << "Error: open output file." << endl;
        exit(0);
    }
    
    vector<vector<vector<int>>> graph(n, vector<vector<int>>(n, vector<int>(2)));
    fflush(stdout);

    initialize(topologyfile, graph);
    vector<vector<vector<int>>> before = graph;
    dijkstra(n, graph);
    print_graph_to_outputfile(outputfile, graph);
    message(outputfile, messagefile, graph);
    change(outputfile, changesfile, messagefile, before);
    
    topologyfile.close();
    messagefile.close();
    changesfile.close();
    outputfile.close();

    cout << "Complete. Output file written to output_ls.txt" << endl;
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
void dijkstra(int n, vector<vector<vector<int>>>& graph){
    vector<vector<vector<int>>> after = graph;
    for (int src = 0; src < n; src++) {
        vector<vector<vector<int>>> before = graph;
        vector<int> dist(n, -999);
        vector<bool> visited(n, false);
        visited[src] = true;
        int parent = src;
        dist[src] = 0;
        for (int count = 0; count < n; count++) {
            for (int child = 0; child < n - 1; child++) {
                //if(src == 5 && child == 0) printf("parent: %d, child: %d, before[child][parent][0]: %d, dist[parent]: %d, dist[child]: %d, before[child][parent][1] = %d, before[%d][%d][1] = %d, before[%d][%d][1] = %d\n", parent, child, before[child][parent][0], dist[parent], dist[child], before[child][parent][1], child, src, before[child][src][1], src, child, before[src][child][1]);
                //방문 안했고, 연결되어 있을 때, 거리가 더 짧거나 아직 연결 안된 상태라면
                if (!visited[child] && before[child][parent][0] != -999 && ((dist[parent] + before[child][parent][0] < dist[child]) || dist[child] == -999)) {
                    dist[child] = dist[parent] + before[child][parent][0];
                    before[child][src][0] = dist[child]; 
                    before[child][src][1] = parent;
                }
                else if(!visited[child] && before[child][parent][0] != -999 && dist[parent] + before[child][parent][0] == dist[child]){
                    int prev_p = child, cur_p = parent;
                    while(before[cur_p][src][1] != src) cur_p = before[cur_p][src][1];
                    while(before[prev_p][src][1] != src) prev_p = before[prev_p][src][1];
                    if(cur_p < prev_p) before[child][src][1] = parent;
                }
            }
            
            for(int i = 0; i < n; i++){
                for(int j = 0; j < n; j++){
                    if(j == src){
                        after[i][j][0] = before[i][j][0];
                        after[i][j][1] = before[i][j][1];
                    }
                }
            }

            int min_dist = INT32_MAX;
            for (int i = 0; i < n; i++){
                if(!visited[i] && dist[i] != -999 && dist[i] < min_dist){
                    min_dist = dist[i];
                    parent = i;
                }
                else if(!visited[i] && dist[i] != -999 && dist[i] == min_dist && i < parent){
                    parent = i;
                }
            }
            visited[parent] = true;

        }
    }
    graph = after;
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
        dijkstra(graph.size(), graph);
        print_graph_to_outputfile(outputfile, graph);
        messagefile.clear();
        messagefile.seekg(0, ios::beg);
        message(outputfile, messagefile, graph);
    }
}
