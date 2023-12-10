// lua generated
#include <map>
#include <string>

extern std::map<std::string, uint8_t> asmTable;
extern std::map<std::string, uint8_t> regTable;

// Сколько занимает регион с регистрами в Байтах
constexpr int REGMEMAMOUNT = 64;
#define REG_null memory[0]
#define REG_inn memory[8]
#define REG_rinf memory[12]
#define REG_sptr memory[16]
#define REG_fndt memory[20]
#define REG_fa memory[24]
#define REG_fb memory[32]
#define REG_fc memory[40]
#define REG_ia memory[48]
#define REG_ib memory[52]
#define REG_ic memory[56]
#define REG_ca memory[60]
#define REG_cb memory[61]
#define REG_cc memory[62]
#define REG_cd memory[63]
