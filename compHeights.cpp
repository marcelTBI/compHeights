#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>

#include "compHeights.h"

using namespace std;

/* reads a line no matter how long*/
char* my_getline(FILE *fp)
{
  char s[512], *line, *cp;
  line = NULL;
  do {
    if(fgets(s, 512, fp) == NULL) break;
    cp = strchr(s, '\n');
    if(cp != NULL) *cp = '\0';
    if(line == NULL) line = (char *) calloc(strlen(s) + 1, sizeof(char));
    else line = (char *) realloc(line, strlen(s) + strlen(line) + 1);
    strcat (line, s);
  } while (cp == NULL);
  return (line);
}

int en_fltoi(float en)
{
  if (en < 0.0) return (int)(en*100 - 0.5);
  else return (int)(en*100 + 0.5);
}

// parse barr file!
int Parsefile (FILE *fp, vector< set<edgeLL> > &graph, vector<int> &energies)
{
  char *line = NULL, *p, sep[] = " \t";
  int count = 0;

  line = my_getline(fp);
  free(line); line=NULL;

  int num, father;
  float energy, e_diff;

  for (count = 0, line = my_getline(fp); line != NULL; count++, line = my_getline(fp)) {
    if (strlen(line) < 10) break;
    p = strtok(line, sep);
    sscanf(p, "%d",  &num);
    p = strtok(NULL, sep);
    string str = p;
    p = strtok(NULL, sep);
    sscanf(p, "%f",  &energy);
    p = strtok(NULL, sep);
    if (p!=NULL) sscanf(p, "%d",  &father);
    p = strtok(NULL, sep);
    if (p!=NULL) sscanf(p, "%f",  &e_diff);

    num--;
    father--;

    // insert new edge and energy
    if ((int)graph.size() < num+1) graph.resize(num+1);
    if (father>=0) if ((int)graph.size() < father+1) graph.resize(father+1);
    if ((int)energies.size() < num+1) energies.resize(num+1);
    energies[num] = en_fltoi(energy);
    if (father>=0) {
      graph[num].insert(edgeLL(num, father, en_fltoi(energy + e_diff), -1));
      graph[father].insert(edgeLL(num, father, en_fltoi(energy + e_diff), -1));
    }

    // prinf
    fprintf(stderr, "%d %d %d\n", num+1, father+1, en_fltoi(energy + e_diff));

    if (line != NULL) free(line);
  }
  if (line != NULL) free(line);

  return count;
}


vector<std::pair<int, int> > HeightSearch(int start, vector< set<edgeLL> > &edgesV_l, vector<int> &energies)
{
  // define + init
  vector<int> heights(energies.size(), INT_MAX);
  vector<int> distance(energies.size(), INT_MAX);
  vector<int> previous(energies.size(), -1);
  vector<bool> done(energies.size(), false);

  priority_queue<pq_height> que;

  // starting point
  done[start] = true;
  distance[start] = 0;
  heights[start] = energies[start];
  for (set<edgeLL>::iterator it=edgesV_l[start].begin(); it!=edgesV_l[start].end(); it++) {
    que.push(pq_height(it->en, 1, it->goesTo(start), start));
  }

  // main loop
  while (!que.empty()) {
    // get next one to do
    pq_height pq = que.top(); que.pop();
    if (done[pq.number]) continue;

    // write him:
    done[pq.number] = true;
    distance[pq.number] = pq.distance;
    previous[pq.number] = pq.from;
    heights[pq.number] = pq.height;

    // push next ones:
    for (set<edgeLL>::iterator it=edgesV_l[pq.number].begin(); it!=edgesV_l[pq.number].end(); it++) {
      int to = it->goesTo(pq.number);
      // if we already had him
      if (done[to]) {
        if (pq.height < heights[to]) fprintf(stderr, "WRONG from: %d to: %d enLM %d enHeight %d\n", pq.number, to, pq.height, heights[to]);
        continue;
      }
      // push next ones
      if (!done[to]) {
        que.push(pq_height(max(it->en, pq.height), pq.distance+1, to, pq.number));
      }
    }
  }

  vector<std::pair<int, int> > res(energies.size());
  for (unsigned int i=0; i<energies.size(); i++) {
    //fprintf(stderr, "%d %d %d %d %d\n", start+1, i+1, heights[i], distance[i], previous[i]+1);
    res[i] = make_pair(heights[i], distance[i]);
  }

  return res;
}

void PrintDot(char *filename, char *file_print, vector< set<edgeLL> > &edgesV_l, vector<int> &energies)
{
  //open file
  FILE *dot;
  dot = fopen(filename, "w");
  if (dot) {
    fprintf(dot, "Graph G {\n\tnode [width=0.1, height=0.1, shape=circle];\n");
    //nodes LM:
    for (unsigned int i=0; i<energies.size(); i++) {
      fprintf(dot, "\"%d\" [label=\"%d\"]\n", i+1, i+1); break;
    }
    fprintf(dot, "\n");
    // edges l-l
    for (unsigned int i=0; i<edgesV_l.size(); i++) {
      for (set<edgeLL>::iterator it=edgesV_l[i].begin(); it!=edgesV_l[i].end(); it++) {
        if (it->goesTo(i)<(int)i) continue;
        fprintf(dot, "\"%d\" -- \"%d\" [label=\"%.2f\"]\n", (it->i)+1, (it->j)+1, it->en/100.0);
      }
    }
  }
  fprintf(dot, "\n}\n");

  fclose(dot);

  // start neato/dot:
  if (file_print) {
    char syst[200];
    sprintf(syst, "%s -Tps < %s > %s", "dot", filename, file_print);
    system(syst);
    //printf("%s returned %d\n", syst, res);
  }
}
