#ifndef PROVISIONING_H
#define PROVISIONING_H

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Start SoftAP and an HTTP server that serves a simple UI to scan and configure Wi‑Fi
esp_err_t provisioning_start_softap(const char *ap_ssid, const char *ap_pass, int channel);

// Stop HTTP server and SoftAP, if running
void provisioning_stop(void);

// Returns true if provisioning completed with received credentials
bool provisioning_was_configured(void);

#ifdef __cplusplus
}
#endif

#endif // PROVISIONING_H
