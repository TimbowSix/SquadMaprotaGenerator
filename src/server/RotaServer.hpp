#pragma once

#include "Generator.hpp"

#define CERT_PATH "/etc/maprota/maprotaServer.crt"
#define PRIVATE_KEY_PATH "/etc/maprota/maprotaServer.key"

/**
 * @brief re/initialized generator and runs optimizer
 */
rota::Generator *initialize();