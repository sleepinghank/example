#ifndef _OTA_H_
#define _OTA_H_
#include <stdint.h>
#pragma pack(1)
#define CMD_FW_OTA_INIT 0x10
#define CMD_FW_SET_ADDRESS  0x11
#define CMD_FW_ERASE        0x16
#define CMD_FW_WRITE        0x17
#define CMD_FW_UPGRADE  0x18
#define CMD_FW_SECTOR_ERASE 0x19
#define CMD_FW_FAST_WRITE_SET   0x20
#define CMD_FW_UPDATE_CONN_PARAM    0x21
#define CMD_FW_MCU_RESET    0x22
#define CMD_FW_GET_INFO     0x23
#define CMD_FW_GET_ATT_MTU     0x24


#define ERR_COMMAND_SUCCESS           0x00
#define ERR_COMMAND_UPDATE_FAIL       0xFF
#define EVT_COMMAND_COMPLETE        (0x0E)
#define CMD_COMPLETE_HDR_SZ     1
#define MAX_FW_WRITE_SZ         20

struct ret_fw_erase_cmd
{
    uint8_t status;
};

struct cmd_fw_sector_erase
{
    uint32_t sector_addr;
    uint16_t sector_sz;
};

struct ret_fw_sector_erase_cmd
{
    uint8_t status;
};

struct ret_fw_set_address_cmd
{
    uint8_t status;
};

struct ret_fw_ota_init_cmd
{
    uint8_t status;
};

struct cmd_fw_fast_write_set
{
    uint32_t sz;
};

struct ret_fw_fast_write_set_cmd
{
    uint8_t status;
};


struct cmd_fw_set_address
{
    uint8_t sz;
    uint32_t addr;
};

struct cmd_fw_write
{
    uint8_t sz;
    uint8_t data[MAX_FW_WRITE_SZ];
};

struct ret_fw_write_cmd
{
    uint8_t status;
};

struct cmd_fw_upgrade
{
    uint32_t sz;
    uint16_t checksum;
    uint8_t version[5];
};

struct ret_fw_upgrade_cmd
{
    uint8_t status;
};

struct ret_fw_info_get
{
    uint8_t status;
    uint8_t version[5];
    uint16_t checksum;
};

struct ret_fw_conn_para_set
{
    uint8_t status;
};

struct ret_get_att_mtu_cmd
{
    uint16_t    att_mtu;
    uint8_t     status;
};

struct cmd_fw_conn_para_set
{
    uint16_t updateitv_max;
    uint16_t updateitv_min;
    uint16_t updatelatency;
    uint16_t updatesvto;
};

union cmd_parm
{
    struct cmd_fw_set_address fw_set_address;
    struct cmd_fw_write fw_write;
    struct cmd_fw_upgrade fw_upgrade;
    struct cmd_fw_sector_erase fw_sector_erase;
    struct cmd_fw_fast_write_set fw_fast_write_set;
    struct cmd_fw_conn_para_set fw_conn_para_set;
};

struct hci_cmd
{
    uint8_t opcode;
    union cmd_parm cmdparm;
};

union ret_parm
{
    struct ret_fw_erase_cmd fw_erase;
    struct ret_fw_write_cmd fw_write;
    struct ret_fw_upgrade_cmd fw_upgrade;
    struct ret_fw_info_get fw_info_get;
};

struct evt_command_complete
{
    uint8_t opcode;
    union ret_parm ret_param;
};



union  evt_parm
{
    struct evt_command_complete evt_command_complete;
};

struct hci_evt
{
    uint8_t evtcode;
    uint8_t evtlen;
    union evt_parm evtparm;
};

enum ota_schedule_mode
{
    OTA_MODE_INIT = 1,
    OTA_MODE_ERASE = 2,
    OTA_MODE_WRITE = 3,
    OTA_MODE_UPDATE = 4,
    OTA_MODE_SETADDR = 5,
    OTA_MODE_FINAL_WRITE = 6,
};

struct ota_fw_info
{
    uint8_t     fw_desc[32];
    uint8_t     fw_version[8];
    uint32_t    fw_size;
    uint16_t    fw_checksum;
};

enum ota_fw_upgrade_option
{
    OTA_FW_UPGRADE_SWITCH_BANK,
    OTA_FW_UPGRADE_WITHOUT_SWITCH,
};


struct flash_header_struct {
	uint16_t tag;	//09 3a
	uint32_t flash_header_address;	//00 00 00 00
	uint32_t register_setting_address;	//00 00 01 80
	uint32_t fw_code_header_address;	//00 00 00 40
	uint16_t fw_sector_size;			//--add
	uint32_t fw_code_0_address;		//00 00 20 00
	uint32_t fw_code_1_address;		//00 00 e0 00
	uint32_t bond_manager_address;	//00 00 10 00
	uint32_t protocol_setting_address;	//00 00 00 c0
	uint32_t gatt_profile_address;		//00 00 05 00
	uint32_t profile_parameters_address;	//00 00 14 00	
	uint32_t profile_parameters_address2;	//00 01 a0 00
	uint8_t sub_dump_key[4];	//--add
	uint8_t cache_line_size;					//--add
};

struct fw_code_header_struct {
	uint8_t code_idx;
	uint8_t  code_0_desc[0x20];
	uint8_t code_0_version[0x05];
	uint32_t code_0_size;	//--up
	uint16_t code_0_checksum;
	uint8_t  code_1_desc[0x20];
	uint8_t code_1_version[0x05];
	uint32_t code_1_size;	//--up
	uint16_t code_1_checksum;
};

#pragma pack()
void ota_cmd(uint8_t* p_cmd, uint8_t sz);
void ota_rsp(uint8_t* p_rsp, uint8_t* p_sz);
uint8_t ota_init_check(void);
void ota_init_clear(void);
uint8_t ota_check(void);
void ota_code_jump(void);
void ota_fw_upgrade_selection(enum ota_fw_upgrade_option option);
void ota_scheduled_task(void);
void ota_reset(void);

#ifdef _OTA_C
#else
    extern void ota_cmd(uint8_t* p_cmd, uint8_t sz);
    extern void ota_rsp(uint8_t* p_rsp, uint8_t* p_sz);
    extern uint8_t ota_init_check(void);
    extern void ota_init_clear(void);
    extern uint8_t ota_check(void);
    extern void ota_code_jump(void);
    extern void ota_fw_upgrade_selection(enum ota_fw_upgrade_option option);
    extern void ota_scheduled_task(void);
    extern void ota_reset(void);
#endif
#endif

