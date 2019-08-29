
typedef struct storable_options{
    char sta_name[64];
    char sta_passwd[64];
    char slack_path[64];
    char slack_message[64];        
} storable_options;

#define OPTIONS_EEPROM_ADDR 0