// Wrap the house Wi-Fi PSK with AES-256-GCM.
// Key = HKDF-SHA256(flash_uid || MAC, info = OwlThree 05.01.01.01.A5 || MAC).
// mbedTLS is part of ESP-IDF (no extra crypto library).
// This is obfuscation bound to this module, not Flash Encryption.

#include "wifi_cred.h"

#include <string.h>

#include "esp_flash.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_random.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "mbedtls/gcm.h"
#include "mbedtls/hkdf.h"
#include "mbedtls/md.h"

#if defined(WIFI_SSID_FROM_ENV) && defined(WIFI_PASSWORD_FROM_ENV)
static const char *kProvSsid = WIFI_SSID_FROM_ENV;
static const char *kProvPsk = WIFI_PASSWORD_FROM_ENV;
#else
static const char *kProvSsid = CONFIG_NODE_WIFI_SSID;
static const char *kProvPsk = CONFIG_NODE_WIFI_PASSWORD;
#endif

static const char *TAG = "wifi_cred";

static const char *kNvsNs = "owl3wifi";
static const char *kNvsBlob = "psk_gcm";
static const char *kNvsSsid = "ssid";

// OwlThree assigned prefix (not a 6-byte node ID by itself).
static const uint8_t kOwlThreePrefix[] = {0x05, 0x01, 0x01, 0x01, 0xA5};

static const char *kHkdfSalt = "owlthree-d1r32-wifi-wrap-v1";

struct wrap_blob
{
    uint8_t ver;
    uint8_t nonce[12];
    uint8_t tag[16];
    uint8_t clen;
    uint8_t cipher[64];
};

static void get_mac(uint8_t mac[6])
{
    if (esp_read_mac(mac, ESP_MAC_WIFI_STA) != ESP_OK)
    {
        memset(mac, 0, 6);
    }
}

static void get_flash_uid(uint8_t uid[8])
{
    uint64_t id = 0;
    if (esp_flash_read_unique_chip_id(NULL, &id) != ESP_OK || id == 0)
    {
        ESP_LOGW(TAG, "flash unique id unavailable; wrap key uses MAC only");
        memset(uid, 0, 8);
        return;
    }
    for (int i = 7; i >= 0; --i)
    {
        uid[i] = (uint8_t)(id & 0xFF);
        id >>= 8;
    }
}

// IKM = flash_uid || MAC. Info = 05.01.01.01.A5 || MAC.
static esp_err_t derive_wrap_key(uint8_t key[32])
{
    uint8_t mac[6];
    uint8_t uid[8];
    uint8_t ikm[14];
    uint8_t info[11];

    get_mac(mac);
    get_flash_uid(uid);
    memcpy(ikm, uid, 8);
    memcpy(ikm + 8, mac, 6);
    memcpy(info, kOwlThreePrefix, 5);
    memcpy(info + 5, mac, 6);

    ESP_LOGI(TAG, "wrap bind MAC %02X:%02X:%02X:%02X:%02X:%02X (not the PSK)",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    const int rc = mbedtls_hkdf(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
                                reinterpret_cast<const unsigned char *>(kHkdfSalt),
                                strlen(kHkdfSalt),
                                ikm, sizeof(ikm),
                                info, sizeof(info),
                                key, 32);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "HKDF failed %d", rc);
        return ESP_FAIL;
    }
    return ESP_OK;
}

static esp_err_t gcm_encrypt(const uint8_t key[32], const char *psk, wrap_blob *out)
{
    memset(out, 0, sizeof(*out));
    out->ver = 1;
    const size_t n = strlen(psk);
    if (n == 0 || n > sizeof(out->cipher))
    {
        return ESP_ERR_INVALID_SIZE;
    }
    out->clen = static_cast<uint8_t>(n);
    esp_fill_random(out->nonce, sizeof(out->nonce));

    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);
    int rc = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, 256);
    if (rc == 0)
    {
        rc = mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT, n,
                                       out->nonce, sizeof(out->nonce),
                                       nullptr, 0,
                                       reinterpret_cast<const unsigned char *>(psk),
                                       out->cipher,
                                       sizeof(out->tag), out->tag);
    }
    mbedtls_gcm_free(&gcm);
    return rc == 0 ? ESP_OK : ESP_FAIL;
}

static esp_err_t gcm_decrypt(const uint8_t key[32], const wrap_blob *in, char *psk, size_t psk_len)
{
    if (in->ver != 1 || in->clen == 0 || in->clen >= psk_len || in->clen > sizeof(in->cipher))
    {
        return ESP_ERR_INVALID_SIZE;
    }
    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);
    int rc = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, 256);
    if (rc == 0)
    {
        rc = mbedtls_gcm_auth_decrypt(&gcm, in->clen,
                                      in->nonce, sizeof(in->nonce),
                                      nullptr, 0,
                                      in->tag, sizeof(in->tag),
                                      in->cipher,
                                      reinterpret_cast<unsigned char *>(psk));
    }
    mbedtls_gcm_free(&gcm);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "GCM unwrap failed (wrong chip or corrupt NVS)");
        return ESP_FAIL;
    }
    psk[in->clen] = '\0';
    return ESP_OK;
}

static esp_err_t nvs_save(const char *ssid, const wrap_blob *blob)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(kNvsNs, NVS_READWRITE, &h);
    if (err != ESP_OK)
    {
        return err;
    }
    err = nvs_set_str(h, kNvsSsid, ssid);
    if (err == ESP_OK)
    {
        err = nvs_set_blob(h, kNvsBlob, blob, sizeof(*blob));
    }
    if (err == ESP_OK)
    {
        err = nvs_commit(h);
    }
    nvs_close(h);
    return err;
}

static esp_err_t nvs_load(char *ssid, size_t ssid_len, wrap_blob *blob)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(kNvsNs, NVS_READONLY, &h);
    if (err != ESP_OK)
    {
        return err;
    }
    size_t slen = ssid_len;
    err = nvs_get_str(h, kNvsSsid, ssid, &slen);
    size_t blen = sizeof(*blob);
    if (err == ESP_OK)
    {
        err = nvs_get_blob(h, kNvsBlob, blob, &blen);
    }
    nvs_close(h);
    if (err == ESP_OK && blen != sizeof(*blob))
    {
        return ESP_ERR_NVS_INVALID_LENGTH;
    }
    return err;
}

static bool prov_ready(void)
{
    return kProvSsid != nullptr && kProvSsid[0] != '\0' &&
           kProvPsk != nullptr && kProvPsk[0] != '\0';
}

esp_err_t wifi_cred_load(char *ssid, size_t ssid_len, char *psk, size_t psk_len)
{
    if (ssid == nullptr || psk == nullptr || ssid_len < 2 || psk_len < 2)
    {
        return ESP_ERR_INVALID_ARG;
    }
    ssid[0] = '\0';
    psk[0] = '\0';

    uint8_t key[32];
    esp_err_t err = derive_wrap_key(key);
    if (err != ESP_OK)
    {
        return err;
    }

    wrap_blob blob;
    err = nvs_load(ssid, ssid_len, &blob);
    if (err == ESP_OK)
    {
        err = gcm_decrypt(key, &blob, psk, psk_len);
        memset(key, 0, sizeof(key));
        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "PSK unwrapped from NVS (SSID present, password not logged)");
        }
        return err;
    }

    if (!prov_ready())
    {
        memset(key, 0, sizeof(key));
        ESP_LOGW(TAG, "No NVS wrap yet. Build once with wifi_secrets.env, flash, then rebuild without the password.");
        return ESP_ERR_NOT_FOUND;
    }

    if (strlen(kProvSsid) >= ssid_len)
    {
        memset(key, 0, sizeof(key));
        return ESP_ERR_INVALID_SIZE;
    }
    strncpy(ssid, kProvSsid, ssid_len - 1);
    ssid[ssid_len - 1] = '\0';

    err = gcm_encrypt(key, kProvPsk, &blob);
    if (err == ESP_OK)
    {
        err = nvs_save(ssid, &blob);
    }
    if (err != ESP_OK)
    {
        memset(key, 0, sizeof(key));
        ESP_LOGE(TAG, "NVS provision failed (%s)", esp_err_to_name(err));
        return err;
    }
    err = gcm_decrypt(key, &blob, psk, psk_len);
    memset(key, 0, sizeof(key));
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "PSK wrapped into NVS. Reflash later without wifi_secrets.env; do not erase-flash.");
    }
    return err;
}
