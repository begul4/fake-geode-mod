#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace {
    template <class T>
    T setting(char const* key) {
        return Mod::get()->getSettingValue<T>(key);
    }

    // Truncates (like the base game does) instead of rounding, so 99.999 never shows as 100.00
    std::string fmtPercent(float value, int decimals, bool symbol) {
        value = std::clamp(value, 0.f, 100.f);
        double scale = std::pow(10.0, decimals);
        double truncated = std::floor(static_cast<double>(value) * scale + 1e-4) / scale;
        return fmt::format("{:.{}f}{}", truncated, decimals, symbol ? "%" : "");
    }
}

class $modify(BPPlayLayer, PlayLayer) {
    struct Fields {
        float runFrom = 0.f;       // percent the current run started from
        float bestRun = 0.f;       // best percent reached from that same start point
        float bestRunFrom = -1.f;  // start point bestRun belongs to
    };

    std::string buildLabel() {
        auto f = m_fields.self();

        int decimals = static_cast<int>(std::clamp<int64_t>(setting<int64_t>("decimals"), 0, 10));

        // Percent symbol: global toggle, plus the "only from 0%" option.
        // From a startpos (m_startPosObject is set) the symbol is dropped when that option is on.
        bool symbol = setting<bool>("show-percent-symbol");
        if (symbol && setting<bool>("percent-symbol-only-from-zero") && m_startPosObject != nullptr) {
            symbol = false;
        }

        bool fromZero = f->runFrom < 0.01f;
        std::vector<std::string> parts;

        // Run from
        if (setting<bool>("show-run-from") && !(setting<bool>("hide-run-from-from-zero") && fromZero)) {
            parts.push_back(fmtPercent(f->runFrom, decimals, symbol));
        }

        // Current progress
        parts.push_back(fmtPercent(this->getCurrentPercent(), decimals, symbol));

        // Best run / level best
        if (setting<bool>("show-best-run")) {
            if (fromZero) {
                if (!setting<bool>("hide-best-run-from-zero")) {
                    if (setting<bool>("show-level-best-from-zero")) {
                        int levelBest = m_level->m_normalPercent.value();
                        parts.push_back(fmtPercent(static_cast<float>(levelBest), 0, symbol));
                    } else if (f->bestRun > 0.f) {
                        parts.push_back(fmtPercent(f->bestRun, decimals, symbol));
                    }
                }
            } else if (f->bestRun > 0.f) {
                parts.push_back(fmtPercent(f->bestRun, decimals, symbol));
            }
        }

        std::string sep = setting<bool>("seperators-with-spacing") ? " - " : "-";
        std::string out;
        for (size_t i = 0; i < parts.size(); i++) {
            if (i > 0) out += sep;
            out += parts[i];
        }
        return out;
    }

    void refreshLabel() {
        if (!m_percentageLabel || m_level->isPlatformer()) return;
        m_percentageLabel->setString(this->buildLabel().c_str());
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();
        this->refreshLabel();
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        if (m_level->isPlatformer()) return;

        auto f = m_fields.self();
        f->runFrom = this->getCurrentPercent();

        // Best run only counts runs from the same start point
        if (std::abs(f->runFrom - f->bestRunFrom) > 0.01f) {
            f->bestRun = 0.f;
            f->bestRunFrom = f->runFrom;
        }

        this->refreshLabel();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        float percent = this->getCurrentPercent();
        bool wasDead = m_player1 && m_player1->m_isDead;

        PlayLayer::destroyPlayer(player, object);

        if (m_level->isPlatformer()) return;

        // Only record when this call actually killed the player (not noclip / repeated calls)
        if (!wasDead && m_player1 && m_player1->m_isDead) {
            auto f = m_fields.self();
            f->bestRun = std::max(f->bestRun, percent);
            this->refreshLabel();
        }
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        if (m_level->isPlatformer()) return;

        auto f = m_fields.self();
        f->bestRun = 100.f;
        this->refreshLabel();
    }
};
