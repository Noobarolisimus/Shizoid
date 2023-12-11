// lua generated
#include <map>
#include <string>
#include <vector>

extern std::map<std::string, std::vector<uint8_t>> asmTable;
extern std::map<std::string, uint8_t> regTable;
extern char const HELPTEXT[];

// Сколько занимает регион с регистрами в Байтах
constexpr int REGMEMAMOUNT = 64;
#define REG_null (*(int32_t*)(memory + 0))
#define REG_inn (*(int32_t*)(memory + 8))
#define REG_rinf (*(int32_t*)(memory + 12))
#define REG_sptr (*(int32_t*)(memory + 16))
#define REG_fndt (*(int32_t*)(memory + 20))
#define REG_fa (*(int32_t*)(memory + 24))
#define REG_fb (*(int32_t*)(memory + 32))
#define REG_fc (*(int32_t*)(memory + 40))
#define REG_ia (*(int32_t*)(memory + 48))
#define REG_ib (*(int32_t*)(memory + 52))
#define REG_ic (*(int32_t*)(memory + 56))
#define REG_ca (*(int32_t*)(memory + 60))
#define REG_cb (*(int32_t*)(memory + 61))
#define REG_cc (*(int32_t*)(memory + 62))
#define REG_cd (*(int32_t*)(memory + 63))
