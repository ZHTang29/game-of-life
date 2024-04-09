#include "iom.h"
#include <algorithm>
#include <iostream>

int iom(int nThreads, int nGenerations, std::vector<std::vector<int>>& startWorld, int nRows, int nCols, int nInvasions, std::vector<int> invasionTimes, std::vector<std::vector<std::vector<int>>> invasionPlans) {
    int generation = 0;
    int deathFighting = 0;
    int deathInvader = 0;
    int numInvasions = 0;
    std::vector<std::vector<int>> currWorld = startWorld;

    while(generation < nGenerations) {
        std::vector<std::vector<int>> nextWorld(nRows, std::vector<int>(nCols, 0));
        bool invasion = std::find(invasionTimes.begin(), invasionTimes.end(), generation + 1) != invasionTimes.end();
        
        #pragma omp parallel for num_threads(nThreads)
        for (int i = 0; i < nRows; i++) {
            for (int j = 0; j < nCols; j++) {
                int selfType = currWorld[i][j];
                if (selfType == 0){
                    if (invasion) {
                        int invadingNum = invasionPlans[numInvasions][i][j];

                        if(invadingNum > 0){
                            nextWorld[i][j] = invadingNum;
                            continue;
                        }
                    }
                    int nextState = zero_cell_next_state(currWorld, nRows, nCols, i, j);
                    if (nextState > 0) {
                        nextWorld[i][j] = nextState;
                    }
                } else {
                    if (invasion) {
                       int invadingNum = invasionPlans[numInvasions][i][j];

                        if(invadingNum > 0){
                            nextWorld[i][j] = invadingNum;
                            #pragma omp critical
                            deathInvader++;
                            continue;
                        } 
                    }
                    int neighbourCount = check_neighbour(currWorld, nRows, nCols, i, j);

                    //-1 is used to signal death due to fighting
                    if (neighbourCount < 0) {
                        nextWorld[i][j] = 0;
                        #pragma omp critical
                        deathFighting++;
                    } else if (neighbourCount < 2 || neighbourCount >= 4) {
                        if (generation == 18) {
                            std::cout << "die " << std::endl;
                        }
                        nextWorld[i][j] = 0;
                    } else {
                        nextWorld[i][j] = selfType;
                    }
                }
            }
        }

        if (invasion) {
            numInvasions++;
        }
        currWorld = nextWorld;
        generation++;
    }

    return deathFighting + deathInvader;


}

int zero_cell_next_state(std::vector<std::vector<int>>& startWorld, int nRows, int nCols, int curr_i, int curr_j){
    int dx[] = {-1,0,1};
    int dy[] = {-1,0,1}; 

    std::vector<int> result(10, 0);
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){       
            //Dont count itself     
            if(i == 1 && j == 1) {
                continue;
            }
            int neighbourX = curr_i + dx[i];
            int neighbourY = curr_j + dy[j];

            // code to factor in the wraparound
            if (neighbourX < 0) {
                neighbourX = nRows - 1;
            }

            if (neighbourX == nRows) {
                neighbourX = 0;
            }

            if (neighbourY < 0) {
                neighbourY = nCols - 1;
            }

            if (neighbourY == nCols) {
                neighbourY = 0;
            }
            
            int neighbourType = startWorld[neighbourX][neighbourY];   
            result[neighbourType]++;
            
        }
    }

    int final = -1;
    for (int i = 0; i < 10; i++) {
        if (result[i] == 3) {
            final = i;
        }
    }
    return final;
}

int check_neighbour(std::vector<std::vector<int>>& startWorld, int nRows, int nCols, int curr_i, int curr_j) {
    int selfType = startWorld[curr_i][curr_j];
    int dx[] = {-1,0,1};
    int dy[] = {-1,0,1};

    int result = 0;

    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            //Dont count itself
            if(i == 1 && j == 1) {
                continue;
            }
            int neighbourX = curr_i + dx[i];
            int neighbourY = curr_j + dy[j];

            // code to factor in the wraparound
            if (neighbourX < 0) {
                neighbourX = nRows - 1;
            }

            if (neighbourX == nRows) {
                neighbourX = 0;
            }

            if (neighbourY < 0) {
                neighbourY = nCols - 1;
            }

            if (neighbourY == nCols) {
                neighbourY = 0;
            }
            
            int neighbourType = startWorld[neighbourX][neighbourY];

            if (neighbourType > 0 && neighbourType != selfType) {
                //-1 is used to signal death due to fighting
                return -1;
            } else if (neighbourType == 0) {
                continue;
            }

            result += (selfType == neighbourType) ? 1 : 0;    
            
        }
    }

    return result;
}
