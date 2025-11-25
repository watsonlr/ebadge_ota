/* Simple SoftAP + HTTP provisioning module
 * Provides:
 *   /           -> HTML page with scan & credential form
 *   /scan       -> JSON list of nearby SSIDs
 *   /configure  -> Accepts POST (ssid, password) saves to NVS and sets flag
 */

#include "provisioning.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define PROV_TAG "provisioning"
#define NVS_NAMESPACE "wifi"
#define NVS_KEY_SSID  "ssid"
#define NVS_KEY_PASS  "pass"

static httpd_handle_t s_server = NULL;
static bool s_configured = false;

static const char *HTML_PAGE =
    "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Provision</title></head><body>"
    "<h2>ESP32 Provisioning</h2>"
    "<button onclick=\"scan()\">Scan Networks</button><div id=results></div>"
    "<form onsubmit=\"return sendCreds(event)\">SSID:<br><select id=ssid name=ssid required><option value=''>-- Select Network --</option></select><br>Password:<br><input id=password name=password><br><button type=submit>Save</button></form>"
    "<script>function scan(){fetch('/scan').then(function(r){return r.json()}).then(function(j){var o='<ul>';var sel=document.getElementById('ssid');sel.innerHTML='<option value=\"\">-- Select Network --</option>';for(var i=0;i<j.networks.length;i++){var n=j.networks[i];o+='<li onclick=\"selectSSID(\\''+n.ssid+'\\','+n.auth+')\">'+n.ssid+' (RSSI '+n.rssi+')'+(n.auth!=0?' SECURED':'')+'</li>';var opt=document.createElement('option');opt.value=n.ssid;opt.text=n.ssid+(n.auth!=0?' (secured)':'');sel.appendChild(opt);} o+='</ul>'; document.getElementById('results').innerHTML=o;});}"
    "function selectSSID(s,a){document.getElementById('ssid').value=s;if(a==0){document.getElementById('password').value='';document.getElementById('password').disabled=true;}else{document.getElementById('password').disabled=false;document.getElementById('password').focus();}}"
    "function sendCreds(e){e.preventDefault();var f=new FormData(e.target);fetch('/configure',{method:'POST',body:new URLSearchParams(f)}).then(function(r){return r.text()}).then(function(t){alert(t);});return false;}" 
    "</script></body></html>";

static void url_decode(char *dst, const char *src, size_t max_out) {
    size_t di = 0;
    for (size_t si = 0; src[si] && di + 1 < max_out; ++si) {
        char c = src[si];
        if (c == '+') {
            dst[di++] = ' ';
        } else if (c == '%' && isxdigit((unsigned char)src[si+1]) && isxdigit((unsigned char)src[si+2])) {
            char h1 = src[++si];
            char h2 = src[++si];
            int val = (isdigit((unsigned char)h1) ? h1 - '0' : (toupper((unsigned char)h1) - 'A' + 10)) * 16 + (isdigit((unsigned char)h2) ? h2 - '0' : (toupper((unsigned char)h2) - 'A' + 10));
            dst[di++] = (char)val;
        } else {
            dst[di++] = c;
        }
    }
    dst[di] = '\0';
}

static esp_err_t root_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, HTML_PAGE, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t scan_get_handler(httpd_req_t *req) {
    // Perform a fresh blocking scan
    wifi_scan_config_t scan_cfg = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true,
    };
    esp_wifi_scan_start(&scan_cfg, true);

    uint16_t num = 0;
    esp_wifi_scan_get_ap_num(&num);
    wifi_ap_record_t *ap_records = calloc(num, sizeof(wifi_ap_record_t));
    if (!ap_records) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OOM");
        return ESP_FAIL;
    }
    uint16_t max = num;
    if (esp_wifi_scan_get_ap_records(&max, ap_records) != ESP_OK) {
        free(ap_records);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "SCANERR");
        return ESP_FAIL;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON *arr = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "networks", arr);
    for (int i = 0; i < max; ++i) {
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "ssid", (char*)ap_records[i].ssid);
        cJSON_AddNumberToObject(obj, "rssi", ap_records[i].rssi);
        cJSON_AddNumberToObject(obj, "auth", ap_records[i].authmode);
        cJSON_AddItemToArray(arr, obj);
    }
    free(ap_records);
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    free(json);
    return ESP_OK;
}

static esp_err_t configure_post_handler(httpd_req_t *req) {
    char buf[256];
    int len = httpd_req_recv(req, buf, sizeof(buf)-1);
    if (len <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "NO DATA");
        return ESP_FAIL;
    }
    buf[len] = '\0';

    // Parse form encoded: ssid=...&password=...
    char ssid[33] = {0};
    char pass[65] = {0};
    char *p_ssid = strstr(buf, "ssid=");
    char *p_pass = strstr(buf, "password=");
    if (!p_ssid) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "NO SSID");
        return ESP_FAIL;
    }
    p_ssid += 5;
    char *amp = strchr(p_ssid, '&');
    int ssid_len = amp ? (int)(amp - p_ssid) : (int)strlen(p_ssid);
    if (ssid_len > 32) ssid_len = 32;
    memcpy(ssid, p_ssid, ssid_len);
    ssid[ssid_len] = '\0';
    char decoded_ssid[33];
    url_decode(decoded_ssid, ssid, sizeof(decoded_ssid));
    strlcpy(ssid, decoded_ssid, sizeof(ssid));

    if (p_pass) {
        p_pass += 9;
        char *amp2 = strchr(p_pass, '&');
        int pass_len = amp2 ? (int)(amp2 - p_pass) : (int)strlen(p_pass);
        if (pass_len > 64) pass_len = 64;
    memcpy(pass, p_pass, pass_len);
    pass[pass_len] = '\0';
    char decoded_pass[65];
    url_decode(decoded_pass, pass, sizeof(decoded_pass));
    strlcpy(pass, decoded_pass, sizeof(pass));
    }

    ESP_LOGI(PROV_TAG, "Saving credentials SSID='%s' PASS len=%d", ssid, (int)strlen(pass));
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_str(h, NVS_KEY_SSID, ssid);
        nvs_set_str(h, NVS_KEY_PASS, pass);
        nvs_commit(h);
        nvs_close(h);
        s_configured = true;
        httpd_resp_sendstr(req, "Credentials saved. Reboot device to connect.");
    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "NVS ERR");
    }
    return ESP_OK;
}

static httpd_uri_t uri_root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler,
    .user_ctx = NULL
};
static httpd_uri_t uri_scan = {
    .uri = "/scan",
    .method = HTTP_GET,
    .handler = scan_get_handler,
    .user_ctx = NULL
};
static httpd_uri_t uri_configure = {
    .uri = "/configure",
    .method = HTTP_POST,
    .handler = configure_post_handler,
    .user_ctx = NULL
};

esp_err_t provisioning_start_softap(const char *ap_ssid, const char *ap_pass, int channel) {
    ESP_LOGI(PROV_TAG, "Starting SoftAP SSID='%s'", ap_ssid);

    esp_err_t err;
    
    // Create both STA and AP netif for APSTA mode (allows scanning while serving AP)
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    if (!sta_netif) {
        ESP_LOGE(PROV_TAG, "Failed to create STA netif");
    }
    
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
    if (!ap_netif) {
        ESP_LOGE(PROV_TAG, "Failed to create AP netif");
        return ESP_FAIL;
    }

    // Initialize Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(PROV_TAG, "esp_wifi_init failed: %s", esp_err_to_name(err));
        return err;
    }

    wifi_config_t ap_config = { 0 };
    strlcpy((char*)ap_config.ap.ssid, ap_ssid, sizeof(ap_config.ap.ssid));
    ap_config.ap.ssid_len = strnlen((const char*)ap_config.ap.ssid, sizeof(ap_config.ap.ssid));
    ap_config.ap.channel = channel;
    ap_config.ap.max_connection = 4;
    ap_config.ap.authmode = (ap_pass && strlen(ap_pass) > 0) ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
    if (ap_config.ap.authmode != WIFI_AUTH_OPEN) {
        strlcpy((char*)ap_config.ap.password, ap_pass, sizeof(ap_config.ap.password));
    }

    // Use APSTA mode so we can scan for networks while also serving as AP
    err = esp_wifi_set_mode(WIFI_MODE_APSTA);
    if (err != ESP_OK) {
        ESP_LOGE(PROV_TAG, "esp_wifi_set_mode failed: %s", esp_err_to_name(err));
        return err;
    }
    err = esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    if (err != ESP_OK) {
        ESP_LOGE(PROV_TAG, "esp_wifi_set_config failed: %s", esp_err_to_name(err));
        return err;
    }
    err = esp_wifi_start();
    if (err != ESP_OK) {
        ESP_LOGE(PROV_TAG, "esp_wifi_start failed: %s", esp_err_to_name(err));
        return err;
    }
    
    ESP_LOGI(PROV_TAG, "SoftAP started successfully");

    // No initial scan; /scan handler performs a blocking scan on request

    httpd_config_t conf = HTTPD_DEFAULT_CONFIG();
    conf.stack_size = 8192;
    if (httpd_start(&s_server, &conf) != ESP_OK) {
        ESP_LOGE(PROV_TAG, "Failed to start HTTP server");
        return ESP_FAIL;
    }
    httpd_register_uri_handler(s_server, &uri_root);
    httpd_register_uri_handler(s_server, &uri_scan);
    httpd_register_uri_handler(s_server, &uri_configure);

    ESP_LOGI(PROV_TAG, "Provisioning portal started. Connect and visit http://192.168.4.1/");
    return ESP_OK;
}

void provisioning_stop(void) {
    if (s_server) {
        httpd_stop(s_server);
        s_server = NULL;
    }
    esp_wifi_stop();
}

bool provisioning_was_configured(void) {
    return s_configured;
}
