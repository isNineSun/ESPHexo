#ifndef WIFI_H
#define WIFI_H
    
#define NETWORK_NETIF_DESC_STA "network_netif_sta"

extern void wifi_init(char *ssid, char *password);

extern char* wifi_get_ipv4_address_str(void);

extern char* wifi_get_ipv6_address_str(void);

#endif