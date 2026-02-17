#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>

// --- Game Polyfills ---
typedef uint8_t byte;
typedef unsigned long ulong;
const int MSSTRING_SIZE = 255;

struct Vector {
    float x, y, z;
};

// --- Definitions from mscharacter.h ---
enum ChunkIDs {
    CHARDATA_HEADER1 = 0,
    CHARDATA_MAPSVISITED1,
    CHARDATA_SKILLS1,
    CHARDATA_SPELLS1,
    CHARDATA_ITEMS1,
    CHARDATA_STORAGE1,
    CHARDATA_COMPANIONS1,
    CHARDATA_HELPTIPS1,
    CHARDATA_QUESTS1,
    CHARDATA_QUICKSLOTS1,
    CHARDATA_ITEMS2,
    CHARDATA_UNKNOWN 
};

// --- Definitions from msitemdefs.h ---
#define ITEM_GROUPABLE  (1 << 4)
#define ITEM_CONTAINER  (1 << 5)
#define ITEM_DRINKABLE  (1 << 7)
#define ITEM_PERISHABLE (1 << 8)

// --- Definitions from mscharacterheader.h ---
// Enforce 4-byte packing exactly as <pshpack4.h> does in the game source
#pragma pack(push, 4)
struct savedata_t {
    int Version;
    char Name[32],
         Race[16],
         MapName[16], 
         NextMap[32], 
         OldTrans[32], 
         NewTrans[32],
         SteamID[32],
         Party[12];
    byte IsElite;
    int Gold;
    short MaxHP, MaxMP, HP, MP;
    Vector Origin, Angles;
    byte Gender;
    ulong PartyID; 
    short PlayerKills;
    float TimeWaitedToForgetKill;  
    float TimeWaitedToForgetSteal; 
};
#pragma pack(pop)

// --- Simplified Data Buffer for reading binary files ---
class CPlayer_DataReader {
private:
    std::ifstream m_File;

public:
    bool Open(const char* filename) {
        m_File.open(filename, std::ios::binary);
        return m_File.is_open();
    }

    bool ReadByte(byte& val) { return (bool)m_File.read((char*)&val, sizeof(byte)); }
    bool ReadShort(short& val) { return (bool)m_File.read((char*)&val, sizeof(short)); }
    bool ReadInt(int& val) { return (bool)m_File.read((char*)&val, sizeof(int)); }
    
    // Mimics CGameFile::ReadString logic
    void ReadString(char* data, size_t length) {
        byte readIn = 0;
        data[0] = 0;
        int i = 0;
        do {
            if (!ReadByte(readIn)) break;
            data[i++] = readIn;
        } while (readIn && (i < length));
    }

    bool ReadRaw(void* dest, size_t size) {
        return (bool)m_File.read((char*)dest, size);
    }
};

static const char* GetStatName(int stat) {
    switch(stat)
    {
    // Stats
    case 0:
        return "Strength";
    case 1:
        return "Agility";
    case 2:
        return "Concentration";
    case 3:
        return "Awareness";
    case 4:
        return "Fitness";
    case 5:
        return "Wisdom";
    // Skills
    case 6:
        return "Swordsmanship";
    case 7:
        return "Martial Arts";
    case 8:
        return "Small Arms";
    case 9:
        return "Axe Handling";
    case 10:
        return "Blunt Arms";
    case 11:
        return "Archery";
    case 12:
        return "Spell Casting";
    case 13:
        return "Parry";
    case 14:
        return "Pole Arms";
    default:
        return "UNKNOWN";
    }
}

// Forward declaration for recursive item reading
void PrintItem(CPlayer_DataReader& file, byte DataID, std::ofstream& out, std::string indent = "\t");

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: chardecoder.exe <input_save.dat> <output_text.txt>\n";
        return 1;
    }

    CPlayer_DataReader file;
    if (!file.Open(argv[1])) {
        std::cout << "Failed to open input file: " << argv[1] << "\n";
        return 1;
    }

    std::ofstream out(argv[2]);
    if (!out.is_open()) {
        std::cout << "Failed to open output file: " << argv[2] << "\n";
        return 1;
    }

    out << "--- MASTER-SWORD SAVE FILE DUMP ---\n\n";

    byte chunkID = 0;
    char cTemp[MSSTRING_SIZE];

    // Read chunks sequentially until EOF
    while (file.ReadByte(chunkID)) {
        switch (chunkID) {
            case CHARDATA_HEADER1: {
                out << "[HEADER CHUNK]\n";
                savedata_t Data;
                if (file.ReadRaw(&Data, sizeof(savedata_t))) {
                    out << "\tVersion: " << Data.Version << "\n";
                    out << "\tName: " << Data.Name << "\n";
                    out << "\tSteamID: " << Data.SteamID << "\n";
                    out << "\tMap: " << Data.MapName << "\n";
                    out << "\tHP: " << Data.HP << " / " << Data.MaxHP << "\n";
                    out << "\tMP: " << Data.MP << " / " << Data.MaxMP << "\n";
                    out << "\tGold: " << Data.Gold << "\n";
                    out << "\tOrigin: " << Data.Origin.x << ", " << Data.Origin.y << ", " << Data.Origin.z << "\n";
                }
                break;
            }
            case CHARDATA_MAPSVISITED1: {
                out << "[MAPS VISITED]\n";
                int maps = 0;
                file.ReadInt(maps);
                out << "\tTotal Maps: " << maps << "\n";
                for (int m = 0; m < maps; m++) {
                    file.ReadString(cTemp, MSSTRING_SIZE);
                    out << "\t- " << cTemp << "\n";
                }
                break;
            }
            case CHARDATA_SKILLS1: {
                out << "[SKILLS]\n";
                byte stats = 0;
                file.ReadByte(stats);
                for (int i = 0; i < stats; i++) {
                    byte subStats = 0;
                    file.ReadByte(subStats);
                    out << "\tStat " << i << ": " << GetStatName(i) << " (" << (int)subStats << " SubStats):\n";
                    for (int r = 0; r < subStats; r++) {
                        short val = 0;
                        int exp = 0;
                        file.ReadShort(val);
                        file.ReadInt(exp);
                        out << "\t\tSubStat " << r << " -> Value: " << val << ", Exp: " << exp << "\n";
                    }
                }
                break;
            }
            case CHARDATA_SPELLS1: {
                out << "[SPELLS]\n";
                byte spells = 0;
                file.ReadByte(spells);
                for (int s = 0; s < spells; s++) {
                    file.ReadString(cTemp, MSSTRING_SIZE);
                    out << "\t- " << cTemp << "\n";
                }
                break;
            }
            case CHARDATA_ITEMS1:
            case CHARDATA_ITEMS2: {
                out << "[GEAR ITEMS]\n";
                byte gearItems = 0;
                file.ReadByte(gearItems);
                out << "\tTotal Items: " << (int)gearItems << "\n";
                for (int i = 0; i < gearItems; i++) {
                    PrintItem(file, chunkID, out);
                }
                break;
            }
            case CHARDATA_STORAGE1: {
                out << "[STORAGE]\n";
                short storages = 0;
                file.ReadShort(storages);
                for (int i = 0; i < storages; i++) {
                    file.ReadString(cTemp, MSSTRING_SIZE);
                    out << "\tStorage Name: " << cTemp << "\n";
                    short items = 0;
                    file.ReadShort(items);
                    for (int j = 0; j < items; j++) {
                        PrintItem(file, chunkID, out, "\t\t");
                    }
                }
                break;
            }
            case CHARDATA_COMPANIONS1: {
                out << "[COMPANIONS]\n";
                short companions = 0;
                file.ReadShort(companions);
                for (int c = 0; c < companions; c++) {
                    file.ReadString(cTemp, MSSTRING_SIZE);
                    out << "\tScript: " << cTemp << "\n";
                    short vars = 0;
                    file.ReadShort(vars);
                    for (int var = 0; var < vars; var++) {
                        char vName[MSSTRING_SIZE], vVal[MSSTRING_SIZE];
                        file.ReadString(vName, MSSTRING_SIZE);
                        file.ReadString(vVal, MSSTRING_SIZE);
                        out << "\t\t" << vName << " = " << vVal << "\n";
                    }
                }
                break;
            }
            case CHARDATA_HELPTIPS1: {
                out << "[HELP TIPS]\n";
                short tips = 0;
                file.ReadShort(tips);
                for (int t = 0; t < tips; t++) {
                    file.ReadString(cTemp, MSSTRING_SIZE);
                    out << "\t- " << cTemp << "\n";
                }
                break;
            }
            case CHARDATA_QUESTS1: {
                out << "[QUESTS]\n";
                int quests = 0;
                file.ReadInt(quests);
                for (int q = 0; q < quests; q++) {
                    char qName[MSSTRING_SIZE], qData[MSSTRING_SIZE];
                    file.ReadString(qName, MSSTRING_SIZE);
                    file.ReadString(qData, MSSTRING_SIZE);
                    out << "\tName: " << qName << " | Data: " << qData << "\n";
                }
                break;
            }
            case CHARDATA_QUICKSLOTS1: {
                out << "[QUICKSLOTS]\n";
                byte slots = 0;
                file.ReadByte(slots);
                for (int q = 0; q < slots; q++) {
                    byte type = 0;
                    file.ReadByte(type);
                    if (type) {
                        int oldID = -1;
                        file.ReadInt(oldID);
                        out << "\tSlot " << q << ": Type " << (int)(type - 1) << ", ID " << oldID << "\n";
                    } else {
                        out << "\tSlot " << q << ": Empty\n";
                    }
                }
                break;
            }
            default:
                out << "[UNKNOWN CHUNK ID: " << (int)chunkID << "]\n";
                out << "Warning: Cannot safely proceed past unknown chunk.\n";
                return 1; 
        }
        out << "\n";
    }

    std::cout << "Successfully parsed save file to text.\n";
    return 0;
}

// Mimics ReadItem1 from sv_character.cpp
void PrintItem(CPlayer_DataReader& file, byte DataID, std::ofstream& out, std::string indent) {
    char name[128];
    file.ReadString(name, sizeof(name));
    if (!name || !name[0]) return;

    short props = 0, loc = 0;
    byte hand = 0;
    
    file.ReadShort(props);
    file.ReadShort(loc);
    file.ReadByte(hand);

    out << indent << "Item: " << name << " [Props: " << props << ", Loc: " << loc << ", Hand: " << (int)hand << "]\n";

    if (DataID == CHARDATA_ITEMS2) {
        int lastID = 0;
        file.ReadInt(lastID);
        out << indent << "  ID: " << lastID << "\n";
    }

    // Properties check based on msitemdefs.h
    if ((props & ITEM_PERISHABLE) || (props & ITEM_DRINKABLE)) {
        short qual = 0, maxQual = 0;
        file.ReadShort(qual);
        file.ReadShort(maxQual);
        out << indent << "  Quality: " << qual << " / " << maxQual << "\n";
    }

    if (props & ITEM_GROUPABLE) {
        short amt = 0;
        file.ReadShort(amt);
        out << indent << "  Quantity: " << amt << "\n";
    }

    if (props & ITEM_CONTAINER) {
        short itemCount = 0;
        file.ReadShort(itemCount);
        out << indent << "  Container (" << itemCount << " items):\n";
        for (int i = 0; i < itemCount; i++) {
            PrintItem(file, DataID, out, indent + "\t");
        }
    }
}