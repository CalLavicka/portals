#pragma once

#include <stdint.h>
#include <vector>


struct Save {
    struct SaveData {
        uint32_t currentLevel = 0;
        std::vector <uint32_t> personalBests;
	};

	void save(uint32_t level, std::vector <uint32_t> const &scores) const;

    SaveData LoadSave(uint32_t saveNumber) const;
};
