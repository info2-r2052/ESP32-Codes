#pragma once

//#define WIFI_SSID "Cletosa&Emmosa"
//#define WIFI_PASS "Jul14nch0M3rl1n4&Ch0l4"


#define WIFI_SSID "info2"
#define WIFI_PASS "info2r2052"

void init_wifi();
void wifi_reset_credentials();
static void save_wifi_credentials(const char *ssid, const char *pass);