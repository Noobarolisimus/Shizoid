// Коды возврата из виртуальной машины.

#pragma once


enum ReturnCodes {
    OK = 0,

    MEM_GENERAL = 1000,
    MEM_OUT_OF_PAGES,
};