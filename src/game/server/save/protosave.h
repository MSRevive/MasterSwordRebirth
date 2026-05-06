#pragma once

namespace MSRProto { class CharData; }
class CBasePlayer;
struct savedata_t;
struct chardata_t;

namespace msr
{
  class ProtoSave
  {
  public:
    /**
     * @brief ProtoSave::CreateSave
     * @param pPlayer
     * @param pData
     * @param out Writes to the CharData
     */
    static void CreateCharData(CBasePlayer *pPlayer, savedata_t *pData, MSRProto::CharData *out);
    static bool LoadCharProtobuf(void *pData, unsigned long Size, chardata_t *CharData);
    /**
     * @brief Save character to json for easier debugging
     */
    static void SaveCharJSON(CBasePlayer *pPlayer, savedata_t *pData);
    static void SaveCharProtobuf(CBasePlayer *pPlayer, savedata_t *pData);
  };

} // namespace msr

