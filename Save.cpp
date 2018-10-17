#include "Save.hpp"

#include "read_chunk.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>

using namespace std;
void save(uint32_t level, vector <uint32_t> const &scores){
    cout<<"Saving to save "<<level<<endl;
    ofstream saveFile;
    saveFile.open("saves/save"+to_string(level)+".txt");
    saveFile<<to_string(level)<<"\n";
    for(uint32_t i = 0; i<scores.size(); i++){
        saveFile<<to_string(scores[i])<<"\n";
    }
    saveFile.close();
}

/*
 * struct SaveData {
 *     uint32_t currentLevel = 0;
 *     std::vector <uint32_t> personalBests;
 * };
*/

//referenced this https://www.geeksforgeeks.org/cpp-program-read-file-word-word/
SaveData LoadSave(uint32_t saveNumber){
    SaveData loaded;
    string line;
    string filename = "saves/save"+to_string(saveNumber)+".txt";

    fstream file;
    file.open(filename.c_str());

    //uint32_t level;
    file>>loaded.currentLevel;

    uint32_t score;
    while(file>>score){
        loaded.personalBests.emplace_back(score);
    }
    return loaded;
}

