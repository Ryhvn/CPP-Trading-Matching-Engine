#include "Logger.h"

namespace me {
    // On l’initialise à true pour garder les logs par défaut
    std::atomic<bool> g_loggingEnabled{true};
}
