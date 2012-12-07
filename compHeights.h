#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <queue>
#include <algorithm>
#include <set>

using namespace std;

/* reads a line no matter how long*/
char* my_getline(FILE *fp);

struct pq_height {
  int height;
  int distance;
  int number;
  int from;

  bool operator<(const pq_height &right) const {
    if (height == right.height) {
      if (distance == right.distance) return number > right.number;
      else return distance > right.distance;
    } else return height > right.height;
  }

  pq_height(int h, int d, int n, int f) {
    height = h;
    distance = d;
    number = n;
    from = f;
  }
};

int en_fltoi(float en);

//edges in graph
struct edge {
  int i;
  int j;

  edge(int ii, int jj) {
    i = ii;
    j = jj;
  }

  bool operator<(const edge &second) const {
    if (i==second.i) {
      return j<second.j;
    } else return i<second.i;
  }

  int goesTo(int src) const { if (i==src) return j; else return i;}

};

struct edgeLL : public edge {
  int saddle;
  int en;

  edgeLL(int ii, int jj, int energy, int saddle):edge(ii,jj) {
    if (i > j) swap(i,j);
    this->saddle = saddle;
    this->en = energy;
  }
};

// energy comparator
struct edgeLL_compen {
  bool operator()(const edgeLL &first, const edgeLL &second) const {
    if (first.en==second.en) {
      if (first.i==second.i) {
        return first.j<second.j;
      } else return first.i<second.i;
    } else return first.en<second.en;
  }
};

// parse barr file!
int Parsefile (FILE *fp, vector< set<edgeLL> > &graph, vector<int> &energies);
void PrintDot(char *filename, char *file_print, vector< set<edgeLL> > &edgesV_l, vector<int> &energies);

vector<std::pair<int, int> > HeightSearch(int start, vector< set<edgeLL> > &edgesV_l, vector<int> &energies);

