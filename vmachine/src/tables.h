// lua generated
#pragma once
#include <map>
#include <string>
#include <vector>

extern std::map<std::string, std::vector<uint8_t>> asmTable;
extern std::map<std::string, uint8_t> regTable;
extern char const HELPTEXT[];

// Сколько занимает регион с регистрами в Байтах
constexpr int REGMEMAMOUNT = 40;
#define REG_null (*(int64_t*)(memory + 0))
#define REG_inn (*(int32_t*)(memory + 8))
#define REG_sptr (*(int32_t*)(memory + 12))
#define REG_fndt (*(int32_t*)(memory + 16))
#define REG_ca (*(int8_t*)(memory + 20))
#define REG_cb (*(int8_t*)(memory + 21))
#define REG_cc (*(int8_t*)(memory + 22))
#define REG_cd (*(int8_t*)(memory + 23))
#define REG_ia (*(int32_t*)(memory + 24))
#define REG_ib (*(int32_t*)(memory + 28))
#define REG_ic (*(int32_t*)(memory + 32))
#define REG_id (*(int32_t*)(memory + 36))
