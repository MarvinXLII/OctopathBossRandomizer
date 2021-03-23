#include "Octopath.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <ios>
// prevents issues with std::max
#define NOMINMAX
#include <Windows.h>

#include "enemy.h"
#include "pak.h"
#include <string>

// Get high value for a two byte value
char high(int input) {
	return (input >> 8) & 0xFF;
}
// Get low value for two byte value
char low(int input) {
	return input & 0xFF;
}
// Combine two values in little endian order
int value(char low, char high) {
	return (high << 8) | low;
}


// Binary copy for regular enemies
//void hexCopy(std::wstring inputFile, std::wstring outputFile) {
void hexCopy(Enemy *e, unsigned char *data) {
	std::memcpy(&data[e->pointer], e->data, e->size);
}

// Binary copy with music changing options
void bossHexCopy(Enemy *e_old, Enemy *e_new, unsigned char *data, bool useBattleBGM, bool useVictoryBGM, bool viewResults, int musicType, int endrollType) {
	// Map for the music types
	std::map<int, int> music;
	music[0] = 2034; // No music, for story bosses
	music[1] = 5; // Battle 1
	music[2] = 6; // Battle 2
	music[3] = 7; // Battle 3
	music[4] = 8; // Galdera phase 1
	music[5] = 9; // MBoss 1
	music[6] = 10; // MBoss 2
	music[7] = 11; // Shrine boss

	// Map for Endroll segment, for first byte of four bytes
	std::map<int, int> endrollhigh;
	endrollhigh[0] = 694; // Do not add to endroll
	endrollhigh[1] = 684; // Olberic chapter 1
	endrollhigh[2] = 685; // Olberic chapter 2
	endrollhigh[3] = 686; // Olberic chapter 3
	endrollhigh[4] = 687; // Olberic chapter 4
	endrollhigh[5] = 688; // Tressa chapter 1
	endrollhigh[6] = 689; // Tressa chapter 2
	endrollhigh[7] = 690; // Tressa chapter 3
	endrollhigh[8] = 691; // Tressa chapter 4
	endrollhigh[9] = 692; // Cyrus chapter 1
	endrollhigh[10] = 693; // Cyrus chapter 2
	endrollhigh[11] = 683; // all the others are this for high byte
	endrollhigh[12] = 683;
	endrollhigh[13] = 683;
	endrollhigh[14] = 683;
	endrollhigh[15] = 683;
	endrollhigh[16] = 683;
	endrollhigh[17] = 683;
	endrollhigh[18] = 683;
	endrollhigh[19] = 683;
	endrollhigh[20] = 683;
	endrollhigh[21] = 683;
	endrollhigh[22] = 683;
	endrollhigh[23] = 683;
	endrollhigh[24] = 683;
	endrollhigh[25] = 683;
	endrollhigh[26] = 683; 
	endrollhigh[27] = 683;
	endrollhigh[28] = 683;
	endrollhigh[29] = 683;
	endrollhigh[30] = 683;
	endrollhigh[31] = 683;
	endrollhigh[32] = 683;
	// Map for Endroll Segment, for second of four bytes
	std::map<int, uint8_t> endrolllow;
	endrolllow[0] = 0; // first 10 are 0
	endrolllow[1] = 0;
	endrolllow[2] = 0;
	endrolllow[3] = 0;
	endrolllow[4] = 0;
	endrolllow[5] = 0;
	endrolllow[6] = 0;
	endrolllow[7] = 0;
	endrolllow[8] = 0;
	endrolllow[9] = 0;
	endrolllow[10] = 0;
	endrolllow[11] = 11; // Cyrus chapter 3
	endrolllow[12] = 12; // Cyrus chapter 4
	endrolllow[13] = 13; // Primrose chapter 1
	endrolllow[14] = 14; // Primrose chapter 2
	endrolllow[15] = 15; // Primrose chapter 3
	endrolllow[16] = 16; // Primrose chapter 4
	endrolllow[17] = 17; // Hannit chapter 1
	endrolllow[18] = 18; // Hannit chapter 2
	endrolllow[19] = 19; // Hannit chapter 3
	endrolllow[20] = 20; // Hannit chapter 4
	endrolllow[21] = 21; // Therion chapter 1
	endrolllow[22] = 22; // Therion chapter 2
	endrolllow[23] = 23; // Therion chapter 3
	endrolllow[24] = 24; // Therion chapter 4
	endrolllow[25] = 25; // Ophilia chapter 1
	endrolllow[26] = 26; // Ophilia chapter 2
	endrolllow[27] = 27; // Ophilia chapter 3
	endrolllow[28] = 28; // Ophilia chapter 4
	endrolllow[29] = 29; // Alfyn chapter 1
	endrolllow[30] = 30; // Alfyn chapter 2
	endrolllow[31] = 31; // Alfyn chapter 3
	endrolllow[32] = 32; // Alfyn chapter 4

	// Copy the new data to uexp
	std::memcpy(&data[e_old->pointer], e_new->data, e_new->size);

	// Copy the boss name from first file, since it has to be the same
	std::memcpy(&data[e_old->pointer], e_old->data, 8);

	// Music
	char lowm = low(music[musicType]);
	char highm = high(music[musicType]);
	data[e_old->pointer + 140] = lowm;
	data[e_old->pointer + 141] = highm;

	// Various bools
	uint8_t battle = useBattleBGM ? 1 : 0;
	uint8_t victory = useVictoryBGM ? 1 : 0;
	uint8_t result = 1; // same for both viewResults true and false!?

	data[e_old->pointer + 172] = battle;
	data[e_old->pointer + 198] = victory;
	data[e_old->pointer + 224] = result;
	data[e_old->pointer + 250] = battle;

	uint8_t zero = 0;
	char lowe = low(endrollhigh[endrollType]);
	char highe = high(endrollhigh[endrollType]);
	data[e_old->pointer + 487] = lowe;
	data[e_old->pointer + 488] = highe;
	data[e_old->pointer + 489] = 0;
	data[e_old->pointer + 490] = 0;
	data[e_old->pointer + 491] = endrolllow[endrollType];
	data[e_old->pointer + 492] = 0;
	data[e_old->pointer + 493] = 0;
	data[e_old->pointer + 494] = 0;

}

std::map<int, Enemy> loadEnemies(unsigned char*enemy_data, std::wstring filename) {
	std::map<int, Enemy> m;
	std::ifstream file;
	file.open(filename, std::ios::in);
	if (file.is_open()) {
		std::string delimiter = ",";
		std::string line;
		std::string token;
		size_t pos;
		int i = 0;
		std::vector<std::string> row;
		while (getline(file, line)) {
			pos = 0;
			row.clear();
			while ((pos = line.find(delimiter)) != std::string::npos) {
				token = line.substr(0, pos);
				row.push_back(token);
				line.erase(0, pos + delimiter.length());
			}
			row.push_back(line);
			Enemy e;
			e.name = row[0];
			e.pointer = std::stoi(row[1]);
			e.size = std::stoi(row[2]);
			e.data = new unsigned char[e.size];
			std::memcpy(e.data, &enemy_data[e.pointer], e.size);
			m[i] = e;
			i += 1;
		}
	}
	return m;
}


// Main randomization function
bool randomToHexFile(std::mt19937 rng, vectorvector inputVector, unsigned char *enemy_data) {
	// Load enemy data from "EnemyGroupData.uexp"
	std::map <int, Enemy> hexFiles = loadEnemies(enemy_data, L".\\working\\enemy_data.csv");

	// map for storing music of normal boss
	std::map <int, int> music;
	music[0] = 0; // Frist chunk of files is enemies, enemy rando comming at some point
	music[1] = 0; // solo Tressa Mikk and Makk
	music[2] = 0; // duo Tressa Mikk and Makk
	music[3] = 0; // tri Tressa Mikk and Makk
	music[4] = 0; // normal Tressa Mikk and Makk
	music[5] = 0; // Tressa chapter 2
	music[6] = 0; // Tressa chapter 3
	music[7] = 0; // Tressa chapter 4
	music[8] = 0; // solo Therion chapter 1
	music[9] = 0; // duo Therion chapter 1
	music[10] = 0; // tri Therion chapter 1
	music[11] = 0; // normal Therion chapter 1
	music[12] = 0; // Therion chapter 2
	music[13] = 0; // Therion chapter 3
	music[14] = 0; // Therion chapter 4
	music[15] = 0; // solo Olberic chapter 1
	music[16] = 0; // duo Olberic chapter 1
	music[17] = 0; // tri Olberic chapter 1
	music[18] = 0; // normal Olberic chapter 1
	music[19] = 0; // Olberic chapter 2
	music[20] = 0; // Olberic chapter 3
	music[21] = 0; // Olberic chapter 4
	music[22] = 0; // solo Hannit chapter 1
	music[23] = 0; // duo Hannit chapter 1
	music[24] = 0; // tri Hannit chapter 1
	music[25] = 0; // normal Hannit chapter 1
	music[26] = 0; // Hannit chapter 2
	music[27] = 0; // Hannit Blocking tree enemy
	music[28] = 0; // Hannit chapter 3
	music[29] = 0; // Hannit chapter 4
	music[30] = 0; // solo Ophilia chapter 1
	music[31] = 0; // duo Ophilia chapter 1
	music[32] = 0; // tri Ophilia chapter 1
	music[33] = 0; // normal Ophilia chapter 1
	music[34] = 0; // Ophilia chapter 2
	music[35] = 0; // Ophilia chapter 3
	music[36] = 0; // Ophilia chapter 4
	music[37] = 0; // solo Primrose chapter 1
	music[38] = 0; // duo Primrose chapter 1
	music[39] = 0; // tri Primrose chapter 1
	music[40] = 0; // normal Primrose chapter 1
	music[41] = 0; // Primrose chapter 2
	music[42] = 0; // Primrose chapter 3
	music[43] = 0; // Primrose chapter 4 phase 1
	music[44] = 0; // Primrose chapter 4 phase 2
	music[45] = 0; // solo Cyrus chapter 1
	music[46] = 0; // duo Cyrus chapter 1
	music[47] = 0; // tri Cyrus chapter 1
	music[48] = 0; // normal Cyrus chapter 1
	music[49] = 0; // Cyrus chapter 2
	music[50] = 0; // Cyrus chapter 3
	music[51] = 0; // Cyrus chapter 4
	music[52] = 0; // solo Alfyn chapter 1
	music[53] = 0; // duo Alfyn chapter 1
	music[54] = 0; // tri Alfyn chapter 1
	music[55] = 0; // normal Alfyn chapter 1
	music[56] = 0; // Alfyn chapter 2
	music[57] = 0; // Alfyn chapter 3
	music[58] = 0; // Alfyn chapter 4
	music[59] = 7; // Warmaster
	music[60] = 7; // Sorceror
	music[61] = 7; // Runelord
	music[62] = 7; // Starseer
	music[63] = 0; // Sidestory bosses, randomizer comming at some point
	music[64] = 0; // Galdera phase 2
	music[65] = 4; // Galdera phase 1
	music[66] = 6; // Tressa Gate
	music[67] = 6; // Therion Gate
	music[68] = 5; // Olberic Gate
	music[69] = 5; // Hannit Gate
	music[70] = 5; // Ophilia Gate
	music[71] = 5; // Primrose Gate
	music[72] = 6; // Cyrus Gate
	music[73] = 6; // Alfyn Gate
	music[74] = 0; // NPCs, probably will not randomize except for items.

	// map for storing endroll segment
	std::map <int, int> endroll;
	endroll[0] = 0; // Frist chunk of files is enemies, enemy rando comming at some point
	endroll[1] = 5; // solo Tressa Mikk and Makk
	endroll[2] = 5; // duo Tressa Mikk and Makk
	endroll[3] = 5; // tri Tressa Mikk and Makk
	endroll[4] = 5; // normal Tressa Mikk and Makk
	endroll[5] = 6; // Tressa chapter 2
	endroll[6] = 7; // Tressa chapter 3
	endroll[7] = 8; // Tressa chapter 4
	endroll[8] = 21; // solo Therion chapter 1
	endroll[9] = 21; // duo Therion chapter 1
	endroll[10] = 21; // tri Therion chapter 1
	endroll[11] = 21; // normal Therion chapter 1
	endroll[12] = 22; // Therion chapter 2
	endroll[13] = 23; // Therion chapter 3
	endroll[14] = 24; // Therion chapter 4
	endroll[15] = 1; // solo Olberic chapter 1
	endroll[16] = 1; // duo Olberic chapter 1
	endroll[17] = 1; // tri Olberic chapter 1
	endroll[18] = 1; // normal Olberic chapter 1
	endroll[19] = 2; // Olberic chapter 2
	endroll[20] = 3; // Olberic chapter 3
	endroll[21] = 4; // Olberic chapter 4
	endroll[22] = 17; // solo Hannit chapter 1
	endroll[23] = 17; // duo Hannit chapter 1
	endroll[24] = 17; // tri Hannit chapter 1
	endroll[25] = 17; // normal Hannit chapter 1
	endroll[26] = 18; // Hannit chapter 2
	endroll[27] = 0; // Hannit Blocking tree enemy
	endroll[28] = 19; // Hannit chapter 3
	endroll[29] = 20; // Hannit chapter 4
	endroll[30] = 25; // solo Ophilia chapter 1
	endroll[31] = 25; // duo Ophilia chapter 1
	endroll[32] = 25; // tri Ophilia chapter 1
	endroll[33] = 25; // normal Ophilia chapter 1
	endroll[34] = 26; // Ophilia chapter 2
	endroll[35] = 27; // Ophilia chapter 3
	endroll[36] = 28; // Ophilia chapter 4
	endroll[37] = 13; // solo Primrose chapter 1
	endroll[38] = 13; // duo Primrose chapter 1
	endroll[39] = 13; // tri Primrose chapter 1
	endroll[40] = 13; // normal Primrose chapter 1
	endroll[41] = 14; // Primrose chapter 2
	endroll[42] = 15; // Primrose chapter 3
	endroll[43] = 0; // Primrose chapter 4 phase 1
	endroll[44] = 16; // Primrose chapter 4 phase 2
	endroll[45] = 9; // solo Cyrus chapter 1
	endroll[46] = 9; // duo Cyrus chapter 1
	endroll[47] = 9; // tri Cyrus chapter 1
	endroll[48] = 9; // normal Cyrus chapter 1
	endroll[49] = 10; // Cyrus chapter 2
	endroll[50] = 11; // Cyrus chapter 3
	endroll[51] = 12; // Cyrus chapter 4
	endroll[52] = 29; // solo Alfyn chapter 1
	endroll[53] = 29; // duo Alfyn chapter 1
	endroll[54] = 29; // tri Alfyn chapter 1
	endroll[55] = 29; // normal Alfyn chapter 1
	endroll[56] = 30; // Alfyn chapter 2
	endroll[57] = 31; // Alfyn chapter 3
	endroll[58] = 32; // Alfyn chapter 4
	endroll[59] = 0; // Warmaster
	endroll[60] = 0; // Sorceror
	endroll[61] = 0; // Runelord
	endroll[62] = 0; // Starseer
	endroll[63] = 0; // Sidestory bosses, randomizer comming at some point
	endroll[64] = 0; // Galdera phase 2
	endroll[65] = 0; // Galdera phase 1
	endroll[66] = 0; // Tressa Gate
	endroll[67] = 0; // Therion Gate
	endroll[68] = 0; // Olberic Gate
	endroll[69] = 0; // Hannit Gate
	endroll[70] = 0; // Ophilia Gate
	endroll[71] = 0; // Primrose Gate
	endroll[72] = 0; // Cyrus Gate
	endroll[73] = 0; // Alfyn Gate
	endroll[74] = 0; // NPCs, probably will not randomize except for items.

	// map for useBattleBGM
	std::map <int, bool> battleBGM;
	battleBGM[0] = false; // Frist chunk of files is enemies, enemy rando comming at some point
	battleBGM[1] = false; // solo Tressa Mikk and Makk
	battleBGM[2] = false; // duo Tressa Mikk and Makk
	battleBGM[3] = false; // tri Tressa Mikk and Makk
	battleBGM[4] = false; // normal Tressa Mikk and Makk
	battleBGM[5] = false; // Tressa chapter 2
	battleBGM[6] = false; // Tressa chapter 3
	battleBGM[7] = false; // Tressa chapter 4
	battleBGM[8] = false; // solo Therion chapter 1
	battleBGM[9] = false; // duo Therion chapter 1
	battleBGM[10] = false; // tri Therion chapter 1
	battleBGM[11] = false; // normal Therion chapter 1
	battleBGM[12] = false; // Therion chapter 2
	battleBGM[13] = false; // Therion chapter 3
	battleBGM[14] = false; // Therion chapter 4
	battleBGM[15] = false; // solo Olberic chapter 1
	battleBGM[16] = false; // duo Olberic chapter 1
	battleBGM[17] = false; // tri Olberic chapter 1
	battleBGM[18] = false; // normal Olberic chapter 1
	battleBGM[19] = false; // Olberic chapter 2
	battleBGM[20] = false; // Olberic chapter 3
	battleBGM[21] = false; // Olberic chapter 4
	battleBGM[22] = false; // solo Hannit chapter 1
	battleBGM[23] = false; // duo Hannit chapter 1
	battleBGM[24] = false; // tri Hannit chapter 1
	battleBGM[25] = false; // normal Hannit chapter 1
	battleBGM[26] = false; // Hannit chapter 2
	battleBGM[27] = false; // Hannit Blocking tree enemy
	battleBGM[28] = false; // Hannit chapter 3
	battleBGM[29] = false; // Hannit chapter 4
	battleBGM[30] = false; // solo Ophilia chapter 1
	battleBGM[31] = false; // duo Ophilia chapter 1
	battleBGM[32] = false; // tri Ophilia chapter 1
	battleBGM[33] = false; // normal Ophilia chapter 1
	battleBGM[34] = false; // Ophilia chapter 2
	battleBGM[35] = false; // Ophilia chapter 3
	battleBGM[36] = false; // Ophilia chapter 4
	battleBGM[37] = false; // solo Primrose chapter 1
	battleBGM[38] = false; // duo Primrose chapter 1
	battleBGM[39] = false; // tri Primrose chapter 1
	battleBGM[40] = false; // normal Primrose chapter 1
	battleBGM[41] = false; // Primrose chapter 2
	battleBGM[42] = false; // Primrose chapter 3
	battleBGM[43] = false; // Primrose chapter 4 phase 1
	battleBGM[44] = false; // Primrose chapter 4 phase 2
	battleBGM[45] = false; // solo Cyrus chapter 1
	battleBGM[46] = false; // duo Cyrus chapter 1
	battleBGM[47] = false; // tri Cyrus chapter 1
	battleBGM[48] = false; // normal Cyrus chapter 1
	battleBGM[49] = false; // Cyrus chapter 2
	battleBGM[50] = false; // Cyrus chapter 3
	battleBGM[51] = false; // Cyrus chapter 4
	battleBGM[52] = false; // solo Alfyn chapter 1
	battleBGM[53] = false; // duo Alfyn chapter 1
	battleBGM[54] = false; // tri Alfyn chapter 1
	battleBGM[55] = false; // normal Alfyn chapter 1
	battleBGM[56] = false; // Alfyn chapter 2
	battleBGM[57] = false; // Alfyn chapter 3
	battleBGM[58] = false; // Alfyn chapter 4
	battleBGM[59] = true; // Warmaster
	battleBGM[60] = true; // Sorceror
	battleBGM[61] = true; // Runelord
	battleBGM[62] = true; // Starseer
	battleBGM[63] = false; // Sidestory bosses, randomizer comming at some point
	battleBGM[64] = false; // Galdera phase 2
	battleBGM[65] = true; // Galdera phase 1
	battleBGM[66] = true; // Tressa Gate
	battleBGM[67] = true; // Therion Gate
	battleBGM[68] = true; // Olberic Gate
	battleBGM[69] = true; // Hannit Gate
	battleBGM[70] = true; // Ophilia Gate
	battleBGM[71] = true; // Primrose Gate
	battleBGM[72] = true; // Cyrus Gate
	battleBGM[73] = true; // Alfyn Gate
	battleBGM[74] = false; // NPCs, probably will not randomize except for items.

	// map for use victoryBGM
	std::map <int, int> victoryBGM;
	victoryBGM[0] = false; // Frist chunk of files is enemies, enemy rando comming at some point
	victoryBGM[1] = false; // solo Tressa Mikk and Makk
	victoryBGM[2] = false; // duo Tressa Mikk and Makk
	victoryBGM[3] = false; // tri Tressa Mikk and Makk
	victoryBGM[4] = false; // normal Tressa Mikk and Makk
	victoryBGM[5] = true; // Tressa chapter 2
	victoryBGM[6] = true; // Tressa chapter 3
	victoryBGM[7] = true; // Tressa chapter 4
	victoryBGM[8] = true; // solo Therion chapter 1
	victoryBGM[9] = true; // duo Therion chapter 1
	victoryBGM[10] = true; // tri Therion chapter 1
	victoryBGM[11] = true; // normal Therion chapter 1
	victoryBGM[12] = true; // Therion chapter 2
	victoryBGM[13] = true; // Therion chapter 3
	victoryBGM[14] = true; // Therion chapter 4
	victoryBGM[15] = true; // solo Olberic chapter 1
	victoryBGM[16] = true; // duo Olberic chapter 1
	victoryBGM[17] = true; // tri Olberic chapter 1
	victoryBGM[18] = true; // normal Olberic chapter 1
	victoryBGM[19] = true; // Olberic chapter 2
	victoryBGM[20] = true; // Olberic chapter 3
	victoryBGM[21] = true; // Olberic chapter 4
	victoryBGM[22] = true; // solo Hannit chapter 1
	victoryBGM[23] = true; // duo Hannit chapter 1
	victoryBGM[24] = true; // tri Hannit chapter 1
	victoryBGM[25] = true; // normal Hannit chapter 1
	victoryBGM[26] = true; // Hannit chapter 2
	victoryBGM[27] = false; // Hannit Blocking tree enemy
	victoryBGM[28] = true; // Hannit chapter 3
	victoryBGM[29] = true; // Hannit chapter 4
	victoryBGM[30] = true; // solo Ophilia chapter 1
	victoryBGM[31] = true; // duo Ophilia chapter 1
	victoryBGM[32] = true; // tri Ophilia chapter 1
	victoryBGM[33] = true; // normal Ophilia chapter 1
	victoryBGM[34] = true; // Ophilia chapter 2
	victoryBGM[35] = true; // Ophilia chapter 3
	victoryBGM[36] = true; // Ophilia chapter 4
	victoryBGM[37] = true; // solo Primrose chapter 1
	victoryBGM[38] = true; // duo Primrose chapter 1
	victoryBGM[39] = true; // tri Primrose chapter 1
	victoryBGM[40] = true; // normal Primrose chapter 1
	victoryBGM[41] = true; // Primrose chapter 2
	victoryBGM[42] = true; // Primrose chapter 3
	victoryBGM[43] = true; // Primrose chapter 4 phase 1
	victoryBGM[44] = true; // Primrose chapter 4 phase 2
	victoryBGM[45] = true; // solo Cyrus chapter 1
	victoryBGM[46] = true; // duo Cyrus chapter 1
	victoryBGM[47] = true; // tri Cyrus chapter 1
	victoryBGM[48] = true; // normal Cyrus chapter 1
	victoryBGM[49] = true; // Cyrus chapter 2
	victoryBGM[50] = true; // Cyrus chapter 3
	victoryBGM[51] = true; // Cyrus chapter 4
	victoryBGM[52] = true; // solo Alfyn chapter 1
	victoryBGM[53] = true; // duo Alfyn chapter 1
	victoryBGM[54] = true; // tri Alfyn chapter 1
	victoryBGM[55] = true; // normal Alfyn chapter 1
	victoryBGM[56] = true; // Alfyn chapter 2
	victoryBGM[57] = true; // Alfyn chapter 3
	victoryBGM[58] = true; // Alfyn chapter 4
	victoryBGM[59] = true; // Warmaster
	victoryBGM[60] = true; // Sorceror
	victoryBGM[61] = true; // Runelord
	victoryBGM[62] = true; // Starseer
	victoryBGM[63] = false; // Sidestory bosses, randomizer comming at some point
	victoryBGM[64] = true; // Galdera phase 2
	victoryBGM[65] = false; // Galdera phase 1
	victoryBGM[66] = true; // Tressa Gate
	victoryBGM[67] = true; // Therion Gate
	victoryBGM[68] = true; // Olberic Gate
	victoryBGM[69] = true; // Hannit Gate
	victoryBGM[70] = true; // Ophilia Gate
	victoryBGM[71] = true; // Primrose Gate
	victoryBGM[72] = true; // Cyrus Gate
	victoryBGM[73] = true; // Alfyn Gate
	victoryBGM[74] = false; // NPCs, probably will not randomize except for items.

	// map for tabulating boss to file
	// chapter 1 bosses are special
	std::map <int, int> bossToFile;
	bossToFile[0] = 4; // Tressa Chapter 1 Boss
	bossToFile[1] = 55; // Alfyn Chapter 1 Boss
	bossToFile[2] = 11; // Therion Chapter 1 Boss
	bossToFile[3] = 25; // H'annit Chapter 1 Boss
	bossToFile[4] = 18; // Olberic Chapter 1 Boss
	bossToFile[5] = 40; // Primrose Chapter 1 Boss
	bossToFile[6] = 48; // Cyrus Chapter 1 Boss
	bossToFile[7] = 33; // Ophilia Chapter 1 Boss
	bossToFile[8] = 5; // Tressa Chapter 2 Boss
	bossToFile[9] = 56; // Alfyn Chapter 2 Boss
	bossToFile[10] = 12; // Therion Chapter 2 Boss
	bossToFile[11] = 26; // H'annit Chapter 2 Boss
	bossToFile[12] = 19; // Olberic Chapter 2 Boss
	bossToFile[13] = 41; // Primrose Chapter 2 Boss
	bossToFile[14] = 49; // Cyrus Chapter 2 Boss
	bossToFile[15] = 34; // Ophilia Chapter 2 Boss
	bossToFile[16] = 6; // Tressa Chapter 3 Boss
	bossToFile[17] = 57; // Alfyn Chapter 3 Boss
	bossToFile[18] = 13; // Therion Chapter 3 Boss
	bossToFile[19] = 28; // H'annit Chapter 3 Boss
	bossToFile[20] = 20; // Olberic Chapter 3 Boss
	bossToFile[21] = 42; // Primrose Chapter 3 Boss
	bossToFile[22] = 50; // Cyrus Chapter 3 Boss
	bossToFile[23] = 35; // Ophilia Chapter 3 Boss
	bossToFile[24] = 7; // Tressa Chapter 4 Boss
	bossToFile[25] = 58; // Alfyn Chapter 4 Boss
	bossToFile[26] = 14; // Therion Chapter 4 Boss
	bossToFile[27] = 29; // H'annit Chapter 4 Boss
	bossToFile[28] = 21; // Olberic Chapter 4 Boss
	bossToFile[29] = 43; // Primrose Chapter 4 Boss Phase 1
	bossToFile[30] = 44; // Primrose Chapter 4 Boss Phase 2
	bossToFile[31] = 51; // Cyrus Chapter 4 Boss
	bossToFile[32] = 36; // Ophilia Chapter 4 Boss
	bossToFile[33] = 59; // Winnehild
	bossToFile[34] = 60; // Dreisang 
	bossToFile[35] = 61; // Balogar
	bossToFile[36] = 62; // Steorra
	bossToFile[37] = 66; // Tressa's Gate Boss
	bossToFile[38] = 67; // Therion's Gate Boss
	bossToFile[39] = 68; // Olberic's Gate Boss
	bossToFile[40] = 69; // H'annit's Gate Boss
	bossToFile[41] = 70; // Ophilia's Gate Boss
	bossToFile[42] = 71; // Primrose's Gate Boss
	bossToFile[43] = 72; // Cyrus' Gate Boss
	bossToFile[44] = 73; // Alfyn's Gate Boss
	bossToFile[45] = 65; // Galdera Phase 1
	bossToFile[46] = 64; // Galdera Phase 2


	// begin the randomization for the bosses
	// put bosses into deque for easy iteration
	std::deque<int> randomBosses;
	// deque has to be in specific order, do it manually
	randomBosses.push_back(inputVector[0][0]); // Tressa chapter 1
	randomBosses.push_back(inputVector[1][0]); // Tressa chapter 2
	randomBosses.push_back(inputVector[2][0]); // Tressa chapter 3
	randomBosses.push_back(inputVector[3][0]); // Tressa chapter 4
	randomBosses.push_back(inputVector[0][2]); // Therion chapter 1
	randomBosses.push_back(inputVector[1][2]); // Therion chapter 2
	randomBosses.push_back(inputVector[2][2]); // Therion chapter 3
	randomBosses.push_back(inputVector[3][2]); // Therion chapter 4
	randomBosses.push_back(inputVector[0][4]); // Olberic chapter 1
	randomBosses.push_back(inputVector[1][4]); // Olberic chapter 2
	randomBosses.push_back(inputVector[2][4]); // Olberic chapter 3
	randomBosses.push_back(inputVector[3][4]); // Olberic chapter 4
	randomBosses.push_back(inputVector[0][3]); // Hannit chapter 1
	randomBosses.push_back(inputVector[1][3]); // Hannit chapter 2
	randomBosses.push_back(inputVector[2][3]); // Hannit chapter 3
	randomBosses.push_back(inputVector[3][3]); // Hannit chapter 4
	randomBosses.push_back(inputVector[0][7]); // Ophilia chapter 1
	randomBosses.push_back(inputVector[1][7]); // Ophilia chapter 2
	randomBosses.push_back(inputVector[2][7]); // Ophilia chapter 3
	randomBosses.push_back(inputVector[3][8]); // Ophilia chapter 4
	randomBosses.push_back(inputVector[0][5]); // Primrose chapter 1
	randomBosses.push_back(inputVector[1][5]); // Primrose chapter 2
	randomBosses.push_back(inputVector[2][5]); // Primrose chapter 3
	randomBosses.push_back(inputVector[3][5]); // Primrose chapter 4 phase 1
	randomBosses.push_back(inputVector[3][6]); // Primrose chapter 4 phase 2
	randomBosses.push_back(inputVector[0][6]); // Cyrus chapter 1
	randomBosses.push_back(inputVector[1][6]); // Cyrus chapter 2
	randomBosses.push_back(inputVector[2][6]); // Cyrus chapter 3
	randomBosses.push_back(inputVector[3][7]); // Cyrus chapter 4
	randomBosses.push_back(inputVector[0][1]); // Alfyn chapter 1
	randomBosses.push_back(inputVector[1][1]); // Alfyn chapter 2
	randomBosses.push_back(inputVector[2][1]); // Alfyn chapter 3
	randomBosses.push_back(inputVector[3][1]); // Alfyn chapter 4
	randomBosses.push_back(inputVector[4][0]); // Warmaster shrine
	randomBosses.push_back(inputVector[4][1]); // Sorceror shrine
	randomBosses.push_back(inputVector[4][2]); // Runelord shrine
	randomBosses.push_back(inputVector[4][3]); // Starseer shrine
	randomBosses.push_back(inputVector[6][1]); // Galdera phase 2
	randomBosses.push_back(inputVector[6][0]); // Galdera phase 1
	randomBosses.push_back(inputVector[5][0]); // Tressa's gate
	randomBosses.push_back(inputVector[5][1]); // Therion's gate
	randomBosses.push_back(inputVector[5][2]); // Olberic's gate
	randomBosses.push_back(inputVector[5][3]); // Hannit's gate
	randomBosses.push_back(inputVector[5][4]); // Ophilia's gate
	randomBosses.push_back(inputVector[5][5]); // Primrose's gate
	randomBosses.push_back(inputVector[5][6]); // Cyrus' gate
	randomBosses.push_back(inputVector[5][7]); // Alfyn's gate


	// Ensure that the size of the deque matches that of the map
	bool errorCheck = true;
	if (randomBosses.size() != 47) {
		std::cout << "Internal Error: RandomBosses does not equal 47" << std::endl;
		errorCheck = false;
	}

	// prevent any randomization if error checks fail
	if (errorCheck == false) {
		return false;
	}
	else {
		// now, map deque to map for easy iteration with vector
		std::map <int, int> randomBossMap;
		randomBossMap[0] = 0; // For first
		for (int i = 1; i <= 74; i++) {
			switch (i) {
				// first tier chapter 1 bosses
			case 1:
			case 8:
			case 15:
			case 22:
			case 30:
			case 37:
			case 45:
			case 52:
				if (randomBosses.front() < 8) {
					// Take the boss input and subtract 3 to get the first tier
					randomBossMap[i] = (bossToFile[randomBosses.front()] - 3);
				}
				else {
					// just write normal boss
					randomBossMap[i] = bossToFile[randomBosses.front()];
				}
				break;
				// Second tier chapter 1 bosses
			case 2:
			case 9:
			case 16:
			case 23:
			case 31:
			case 38:
			case 46:
			case 53:
				if (randomBosses.front() < 8) {
					// Take the boss input and subtract 2 to get the second tier
					randomBossMap[i] = (bossToFile[randomBosses.front()] - 2);
				}
				else {
					// just write normal boss
					randomBossMap[i] = bossToFile[randomBosses.front()];
				}
				break;
				// Third tier chapter 1 bosses
			case 3:
			case 10:
			case 17:
			case 24:
			case 32:
			case 39:
			case 47:
			case 54:
				if (randomBosses.front() < 8) {
					// Take the boss input and subtract 1 to get the third tier
					randomBossMap[i] = (bossToFile[randomBosses.front()] - 1);
				}
				else {
					// just write normal boss
					randomBossMap[i] = bossToFile[randomBosses.front()];
				}
				break;
				// Fourth tier is normal, use to pop the front of the deque
			case 4:
			case 11:
			case 18:
			case 25:
			case 33:
			case 40:
			case 48:
			case 55:
				randomBossMap[i] = bossToFile[randomBosses.front()];
				randomBosses.pop_front();
				break;
				// for inbetween files
			case 27:
				randomBossMap[27] = 27;
				break;
			case 63:
				randomBossMap[63] = 63;
				break;
			case 74:
				randomBossMap[74] = 74;
				break;
				// Default case for all other bosses
			default:
				randomBossMap[i] = bossToFile[randomBosses.front()];
				randomBosses.pop_front();
				break;
			}
		}

		// Now, write to the file, using the various maps and files
		for (int i = 0; i <= 74; i++) {
			switch (i) {
			case 0:
			case 27:
			case 63:
			case 74:
				// TODO: update hexCopy inputs to just char array
				// do a straight copy for the files
				hexCopy(&hexFiles[randomBossMap[i]], enemy_data);
				break;
			default:
				// TODO: update hexCopy input by replacing hex files with vanilla and swap char arrays. Everything else should remain the same!
				// Copy with random bosses
				bossHexCopy(&hexFiles[i], &hexFiles[randomBossMap[i]], enemy_data, battleBGM[i], victoryBGM[i], i == 65 ? false : true, music[i], endroll[i]);
				break;
			}
		}

		return true;
	}
}