#include <iostream>
#include <random>
#include <algorithm>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <fstream>
using namespace std;
class minTSP
{
public:
    int cost;
    long long id;
    vector<int> path;
    minTSP(int cost = 0, long long id = 0, vector<int> p = vector<int>())
    {
        this->cost = cost;
        this->id = id;
        for (int i = 0; i < p.size(); i++)
            path.push_back(p[i]);
    }
};

int s = 0;
vector<vector<int>> graph;
vector<minTSP> triedpaths;
vector<int> choosing, number;

int RandProMax(int min = 1, int max = 10)
{
    mt19937 gen((random_device())());
    uniform_int_distribution<> distr(min, max);
    return distr(gen);
}
int maxNumber()
{
    int max = number[0];
    for (int i = 1; i < number.size(); ++i)
        if (number[i] > max)
            max = number[i];

    return max;
}
void lock(int id)
{

    choosing[id] = 1;
    number[id] = 1 + maxNumber();
    choosing[id] = 0;

    for (int j = 0; j < choosing.size(); ++j)
    {
        while (choosing[j])
            ;
        while ((number[j] != 0) && (number[j] < number[id] || (number[j] == number[id] && j < id)))
            ;
    }
}
void *travllingSalesmanProblem(void *a)
{
    pair<int, vector<int>> param = *(*(pair<int, vector<int>> **)a);
    int current_pathweight = 0;

    int k = s;
    for (int i = 0; i < param.second.size(); i++)
    {
        current_pathweight += graph[k][param.second[i]];
        k = param.second[i];
    }
    current_pathweight += graph[k][s];

    param.second.insert(param.second.begin(), s);
    param.second.push_back(s);
    for (auto &i : param.second)
        i++;

    // insert lamport bakery algorithm here
    lock(param.first);
    triedpaths.push_back(minTSP(current_pathweight, (long long)pthread_self(), param.second));
    number[param.first] = 0; // unlocking the thread

    sleep(2);
    
    while (triedpaths.size() != number.size())
        ;

    minTSP min = triedpaths[0];
    int minidx = 0;
    for (int i = 1; i < triedpaths.size(); i++)
    {
        if (triedpaths[i].cost < min.cost)
        {
            min = triedpaths[i];
            minidx = i;
        }

        if (triedpaths[i].cost == min.cost) // tiebreaker
            if (triedpaths[i].id < min.id)
            {
                min = triedpaths[i];
                minidx = i;
            }
    }

    // now we have min value and min index
    // cancel all other threads
    // wait for the thread with minimum cost to reach this point

    while (pthread_self() != (pthread_t)min.id)
        ;

    for (int i = 0; i < triedpaths.size(); i++)
    {
        if (i != minidx)
        {
            pthread_cancel((pthread_t)triedpaths[i].id);
        }
    }
    printf("Minimum cost: %d\n", min.cost);
    printf("Minimum ID: %lld\n", min.id);
    printf("Minimum Path: ");
    for (int i = 0; i < min.path.size(); i++)
    {
        if (i == min.path.size() - 1)
            printf("%d\n", min.path[i]);
        else
            printf("%d -> ", min.path[i]);
    }
    printf("Thread %d is the winner\n", minidx);
    return NULL;
}
void printtriedpaths()
{
    for (int i = 0; i < triedpaths.size(); i++)
    {
        cout << "Cost: " << triedpaths[i].cost << endl;
        cout << "ThreadID: " << triedpaths[i].id << endl;
        cout << "Path: ";
        for (int j = 0; j < triedpaths[i].path.size(); j++)
        {
            if (j == triedpaths[i].path.size() - 1)
                cout << triedpaths[i].path[j];
            else
                cout << triedpaths[i].path[j] << " -> ";
        }
        cout << "\n\n";
    }
}
void TSPsetup(int size)
{
    vector<pthread_t> t;
    vector<int> vertex;
    vector<vector<int>> paths;
    for (int i = 0; i < size; i++)
        if (i != s)
            vertex.push_back(i);

    do
    {
        paths.push_back(vertex);
    } while (next_permutation(vertex.begin(), vertex.end()));

    cout << "Enter number of threads: ";
    int numthreads = 10;
    cin >> numthreads;

    bool less = false;

    if (numthreads > paths.size())
    {
        printf("Number of threads exceeds number of paths. Setting number of threads to number of paths.\n");
        printf("Num of possible paths: %d\n", (int)paths.size());
        numthreads = paths.size();
    }
    else if (numthreads < paths.size())
        less = true;

    vector<int> *visited = new vector<int>();
    int *ids = new int[numthreads];

    // Initialize choosing and number arrays

    for (int i = 0; i < numthreads; i++)
    {
        t.push_back(pthread_t());
        choosing.push_back(0);
        number.push_back(0);
        ids[i] = i;
    }
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    for (int i = 0; i < numthreads; i++)
    {

        if (!less)
        {
            pair<int, vector<int>> *a = new pair<int, vector<int>>();
            *a = make_pair(ids[i], paths[i]);
            pthread_create(&t[i], &thread_attr, travllingSalesmanProblem, (void *)&a);
        }
        else
        {
            int r;
            do
            {
                r = RandProMax(0, paths.size()-1);
            } while (find(visited->begin(), visited->end(), r) != visited->end());

            visited->push_back(r);
            pair<int, vector<int>> *a = new pair<int, vector<int>>();
            *a = make_pair(ids[i], paths[r]);
            pthread_create(&t[i], &thread_attr, travllingSalesmanProblem, (void *)&a);
        }
    }
    pthread_attr_destroy(&thread_attr);
    delete visited;
    delete[] ids;

    printtriedpaths();
}

int main()
{

    fstream fin("i22-0518_testFile.txt", ios::in);
    int u, v, w;
    string l;
    int numnodes;
    fin >> numnodes;
    for (int i = 0; i < numnodes; i++)
    {
        graph.push_back(vector<int>());
        for (int j = 0; j < numnodes; j++)
            graph[i].push_back(0);
    }
    while (!fin.eof())
    {
        fin >> u >> l >> v >> w;
        graph[u - 1][v - 1] = w;
        graph[v - 1][u - 1] = w;
    }
    fin.close();
    TSPsetup(numnodes);
    sleep(7);
    return 0;
}
