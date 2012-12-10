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
  vector<vector<int> > devs; // deviations on each distance
  vector<int> correct;
  vector<int> all;
  vector<int> sum;
  vector<double> stdev;
  while ((line = my_getline(stdin))) {
    int i, j, dist;
    float en;
    int ret = sscanf(line, "%d %d %f %d", &i, &j, &en, &dist);
    int ene = en_fltoi(en);
    i--;
    j--;
    //printf("%s\n", line);
    //stdout << ;

    // if we are outside of range, just skip it
    if (i>=(int)energies.size() || j>=(int)energies.size()) {
      free(line);
      continue;
    }

    // actual code:
      //resize arrays:
    if ((int)correct.size() <= dist) {
      correct.resize(dist+1, 0);
      all.resize(dist+1, 0);
      sum.resize(dist+1, 0);
      devs.resize(dist+1);
      stdev.resize(dist+1, 0);
    }

    if (res[i][j].first == INT_MAX) {
      fprintf(stderr, "nonergodic, skipping...\n");
      free(line);
      continue;
    }

    if (ene == res[i][j].first) correct[dist]++;
    else {
      //fprintf(stderr, "%s incorrect %d %d\n", (ene>res[i][j].first?"":"WROOOOOOOOONG"), ene, res[i][j].first);
      sum[dist] += abs(res[i][j].first-ene);
      devs[dist].push_back(abs(res[i][j].first-ene));
    }
    all[dist]++;
    free(line);
  }

  // do statistics
  for (unsigned int i=1; i<devs.size(); i++) {
    for (unsigned int j=0; j<devs[i].size(); j++) {
      stdev[i] += (sum[i]/(double)devs[i].size()-devs[i][j])*(sum[i]/(double)devs[i].size()-devs[i][j]);
    }
    stdev[i] = sqrt(stdev[i]/devs[i].size());
    printf("dist: %d %d/%d -- average: %.3f stdev: %.3f\n", i, correct[i], all[i], sum[i]/(double)devs[i].size(), stdev[i]);
    //printf("%d %.4f\n", i, sum[i]/(double)all[i]/100.0);
    //printf("%d %.2f\n", i, correct[i]/(double)all[i]*100);
  }


  cmdline_parser_free(&args_info);
  return 0;
}
