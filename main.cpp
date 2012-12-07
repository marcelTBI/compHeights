#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <algorithm>
#include <vector>
#include <set>
#include <iostream>

#include "compHeights.h"

using namespace std;

extern "C" {
  #include "fold.h"

  #include "compHeights_cmdline.h"
}

int main(int argc, char **argv)
{
  // parse arguments
  gengetopt_args_info args_info;
  if (cmdline_parser(argc, argv, &args_info) != 0) {
    fprintf(stderr, "Argument parsing problem.\n");
    exit(EXIT_FAILURE);
  }

  FILE *barr;
  barr = fopen(args_info.barriers_arg, "r");
  if (!barr) {
    fprintf(stderr, "Cannot open file %s!!\n", args_info.barriers_arg);
    exit(EXIT_FAILURE);
  }

  // read barrier file
  vector< set<edgeLL> > graph;
  vector<int> energies;

  Parsefile(barr, graph, energies);
  fclose(barr);
  PrintDot("smth.dot", "smth.eps", graph, energies);

  // recompute all energy heights
  vector<vector<std::pair<int,int> > > res(energies.size());
  for (unsigned int i=0; i<energies.size(); i++) {
    res[i] = HeightSearch(i, graph, energies);
  }

  // now compare with DSUeval energies
  char *line;
  int correct = 0;
  int all = 0;
  int sum = 0;
  vector<int> devs;
  while ((line = my_getline(stdin))) {
    int i, j, dist;
    float en;
    int ret = sscanf(line, "%d %d %f %d", &i, &j, &en, &dist);
    int ene = en_fltoi(en);
    i--;
    j--;
    //printf("%s\n", line);
    //stdout << ;

    if (i>=(int)energies.size() || j>=(int)energies.size()) {
      free(line);
      continue;
    }

    if (ene == res[i][j].first) correct++;
    else {
      fprintf(stderr, "%s incorrect %d %d\n", (ene>res[i][j].first?"":"WROOOOOOOOONG"), ene, res[i][j].first);
      sum += abs(res[i][j].first-ene);
      devs.push_back(abs(res[i][j].first-ene));
    }
    all++;
    free(line);
  }

  double stdev = 0;
  for (unsigned int i=0; i<devs.size(); i++) {
    stdev += (sum/(double)devs.size()-devs[i])*(sum/(double)devs.size()-devs[i]);
  }
  stdev = sqrt(stdev/devs.size());
  printf("%d/%d -- average: %.3f stdev: %.3f\n", correct, all, sum/(double)devs.size(), stdev);

  cmdline_parser_free(&args_info);
  return 0;
}
