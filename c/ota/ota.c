#define  _OTA_C

#include "ota.h"
//#include "pxi_par2860_ble_lib.h"
//#include "debug.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
/*
typedef uint8_t (*ota_upgrade_callback_t)(uint8_t* p_desc, uint8_t* p_ver, \
        uint32_t sz, uint16_t checksum);
*/
static uint8_t _evt_buf[sizeof(struct hci_evt)];
struct hci_evt* pevt = (struct hci_evt*)_evt_buf;
static uint32_t _code_write_offset = 0;
uint8_t ota_init = 0;
uint8_t ota_start = 0;
static uint32_t _fw_fast_write_size = 0;
//static ota_upgrade_callback_t _ota_upgrade_cb = pxi_ota_code_update;
static uint8_t _ota_erase_request;
static uint8_t _ota_erase_sector;
//
//static void _evt_command_complete(uint8_t opcode, uint8_t* ret_parm, uint8_t ret_len)
//{
//    pevt->evtcode = EVT_COMMAND_COMPLETE;
//    pevt->evtlen = ret_len + CMD_COMPLETE_HDR_SZ;
//    pevt->evtparm.evt_command_complete.opcode = opcode;
//
//    memcpy((uint8_t*)&pevt->evtparm.evt_command_complete.ret_param, ret_parm, ret_len);
//}

//void cmd_fw_ota_init(struct cmd_fw_set_address* p_cmd)
//{
//    struct ret_fw_ota_init_cmd ret;
//
//    ota_start = 1;
//    _fw_fast_write_size = 0;
//    ota_init = 1;
//    ret.status = ERR_COMMAND_SUCCESS;
//    _evt_command_complete(CMD_FW_OTA_INIT, (uint8_t*)&ret, sizeof (ret));
//
//    printf(("cmd_fw_ota_init\r\n"));
//}
//
//void cmd_fw_set_address(struct cmd_fw_set_address* p_cmd)
//{
//    struct ret_fw_set_address_cmd ret;
//
//    _code_write_offset = p_cmd->addr;
//
//    ret.status = ERR_COMMAND_SUCCESS;
//
//    _evt_command_complete(CMD_FW_SET_ADDRESS, (uint8_t*)&ret, sizeof (ret));
//
//    printf(("cmd_fw_set_address\r\n"));
//}
//
//void cmd_fw_erase(void)
//{
//    struct ret_fw_erase_cmd ret;
//
//
//
//    _code_write_offset = 0;
//
//    //pxi_code_erase();
//
//    ret.status = ERR_COMMAND_SUCCESS;
//
//    _evt_command_complete(CMD_FW_ERASE, (uint8_t*)&ret, sizeof (ret));
//}
//
//void cmd_fw_write(uint8_t* p_cmd, uint8_t sz)
//{
//    struct ret_fw_write_cmd ret;
//    static uint32_t sector_check = 0;
//
//    if (sz > _fw_fast_write_size)
//        sz = _fw_fast_write_size;
//
//    pxi_code_write(_code_write_offset, sz, p_cmd);
//    printf("code %d sz %d\r\n",_fw_fast_write_size,sz);
//    _code_write_offset += sz;
//
//    _fw_fast_write_size -= sz;
//
//    sector_check += sz;
//
//    if (_fw_fast_write_size == 0)
//    {
//        ret.status = ERR_COMMAND_SUCCESS;
//        _evt_command_complete(CMD_FW_WRITE, (uint8_t*)&ret, sizeof (ret));
//    }
//    else
//        _evt_command_complete(0, (uint8_t*)&ret, sizeof (ret));
//}
//
//void cmd_fw_sector_erase(struct cmd_fw_sector_erase* p_cmd)
//{
//    struct ret_fw_sector_erase_cmd ret;
//
//    printf(("cmd_fw_sector_erase\r\n"));
//
//    ret.status = ERR_COMMAND_SUCCESS;
//
//    _evt_command_complete(CMD_FW_SECTOR_ERASE, (uint8_t*)&ret, sizeof (ret));
//}
//
//
//void cmd_fw_upgrade(struct cmd_fw_upgrade* p_cmd)
//{
//    struct ret_fw_upgrade_cmd Ret;
//
//    printf(("cmd_fw_upgrade\r\n"));
//
//    if(pxi_code_update(NULL, (uint8_t*)&p_cmd->version, p_cmd->sz, p_cmd->checksum) == 1)
//    {
//        Ret.status = ERR_COMMAND_SUCCESS;
//
//        _evt_command_complete(CMD_FW_UPGRADE, (uint8_t *)&Ret, sizeof (Ret));
//
//        printf(("ERR_COMMAND_SUCCESS\r\n"));
//    }
//    else
//    {
//        Ret.status = ERR_COMMAND_UPDATE_FAIL;
//
//        _evt_command_complete(CMD_FW_UPGRADE, (uint8_t *)&Ret, sizeof (Ret));
//
//        printf(("ERR_COMMAND_UPDATE_FAIL\r\n"));
//    }
//
//    ota_start = 0;
//}
//
//void cmd_fw_fast_write_set(struct cmd_fw_fast_write_set* p_cmd)
//{
//    struct ret_fw_fast_write_set_cmd ret;
//
//    printf(("cmd_fw_fast_write_set(%04x)\r\n", p_cmd->sz));
//
//    _fw_fast_write_size = p_cmd->sz;
//
//    ret.status = ERR_COMMAND_SUCCESS;
//
//    _evt_command_complete(CMD_FW_FAST_WRITE_SET, (uint8_t*)&ret, sizeof (ret));
//}

//void cmd_fw_conn_para_set(struct cmd_fw_conn_para_set* p_cmd)
//{
//    struct gap_update_params p_update_params;
//    struct ret_fw_conn_para_set ret;
//
//    p_update_params.updateitv_max = p_cmd->updateitv_max;
//    p_update_params.updateitv_min = p_cmd->updateitv_min;
//    p_update_params.updatelatency = p_cmd->updatelatency;
//    p_update_params.updatesvto = p_cmd->updatesvto;
//
//    pxi_gap_s_connection_update(&p_update_params);
//
//    ret.status = ERR_COMMAND_SUCCESS;
//
//    _evt_command_complete(CMD_FW_UPDATE_CONN_PARAM, (uint8_t*)&ret, sizeof (ret));
//
//}
//
//void cmd_fw_info_get(void)
//{
//    struct ret_fw_info_get fw_into_ret;
//    struct ota_fw_info fw_info;
//
//    pxi_code_info_read(fw_info.fw_version, fw_info.fw_desc,
//                       &fw_info.fw_size, &fw_info.fw_checksum);
//
//    printf("FwVer %s", fw_info.fw_version);
//    printf("Desc  %s", fw_info.fw_desc);
//    printf("FW size:%lu checksum:%x \n", fw_info.fw_size, fw_info.fw_checksum);
//
//    memcpy(fw_into_ret.version, fw_info.fw_version, 5);
//    fw_into_ret.checksum = fw_info.fw_checksum;
//    fw_into_ret.status = ERR_COMMAND_SUCCESS;
//
//    _evt_command_complete(CMD_FW_GET_INFO, (uint8_t*)&fw_into_ret, \
//                          sizeof (fw_into_ret));
//}
//
//void cmd_get_att_mtu(void)
//{
//    struct ret_get_att_mtu_cmd att_mtu_Ret;
//
//    att_mtu_Ret.att_mtu = 23;
//    att_mtu_Ret.status = ERR_COMMAND_SUCCESS;
//    _evt_command_complete(CMD_FW_GET_ATT_MTU, (uint8_t*)&att_mtu_Ret, sizeof (att_mtu_Ret));
//
//
//}

void ota_cmd_exe(uint8_t* p_cmd, uint8_t sz)
{
    struct hci_cmd* pcmd = (struct hci_cmd*)p_cmd;

    if (_fw_fast_write_size != 0)
    {
//        cmd_fw_write(p_cmd, sz);
        return;
    }

    if (!ota_init)
    {
        if (pcmd->opcode != CMD_FW_OTA_INIT)
            return;
    }

    switch (pcmd->opcode)
    {
    case CMD_FW_OTA_INIT:
        printf("OTA CMD_FW_OTA_INIT \r\n");
        printf("fw_set_address:%d",pcmd->cmdparm.fw_set_address.addr);
//        cmd_fw_ota_init(&pcmd->cmdparm.fw_set_address);
        break;

    case CMD_FW_SET_ADDRESS:
        printf("OTA CMD_FW_SET_ADDRESS \r\n");
//        cmd_fw_set_address(&pcmd->cmdparm.fw_set_address);
                 break;

    case CMD_FW_ERASE:
        printf("OTA CMD_FW_ERASE \r\n");
        _ota_erase_request = 1;
        break;

    case CMD_FW_UPGRADE:
        printf("OTA CMD_FW_UPGRADE \r\n");
//        cmd_fw_upgrade(&pcmd->cmdparm.fw_upgrade);
        break;

    case CMD_FW_FAST_WRITE_SET:
        printf("OTA CMD_FW_FAST_WRITE_SET \r\n");
//        cmd_fw_fast_write_set(&pcmd->cmdparm.fw_fast_write_set);
        break;

    case CMD_FW_UPDATE_CONN_PARAM:
        printf("OTA CMD_FW_UPDATE_CONN_PARAM \r\n");
//        cmd_fw_conn_para_set(&pcmd->cmdparm.fw_conn_para_set);
        break;

    case CMD_FW_MCU_RESET:
        printf("OTA CMD_FW_MCU_RESET \r\n");
//        pxi_system_reset();
        break;

    case CMD_FW_GET_INFO:
        printf("OTA CMD_FW_GET_INFO \r\n");
//        cmd_fw_info_get();
        break;
    case CMD_FW_GET_ATT_MTU:
        printf("CMD_FW_GET_ATT_MTU \r\n");
//        cmd_get_att_mtu();
        break;
    }
}


void ota_cmd(uint8_t* p_cmd, uint8_t sz)
{
    ota_cmd_exe(p_cmd, sz);
}

void ota_rsp(uint8_t* p_rsp, uint8_t* p_sz)
{
    memcpy(p_rsp, _evt_buf,  pevt->evtlen + 2);

    *p_sz = pevt->evtlen + 2;
}

uint8_t ota_init_check(void)
{
    return ota_init;
}
void ota_init_clear(void)
{
    ota_init = 0;
}

uint8_t ota_check(void)
{
    return ota_start;
}

void ota_code_jump(void)
{
    //pxi_ota_code_jump();
}

void ota_fw_upgrade_selection(enum ota_fw_upgrade_option option)
{
    /*
    switch (option)
    {
        case OTA_FW_UPGRADE_WITHOUT_SWITCH:
            _ota_upgrade_cb = pxi_ota_code_update_without_code_siwtch;
            break;

        default:
            _ota_upgrade_cb = pxi_ota_code_update;
    }
    */
}

void ota_scheduled_task(void)
{
//    if (_ota_erase_request)
//    {
//        printf(">>>cmd_fw_erase\r\n");
//
//
//
//        printf("code_bank_sector size %d\r\n",pxi_ota_get_code_bank_sector_size());
//
//        if(_ota_erase_sector < pxi_ota_get_code_bank_sector_size())
//        {
//            pxi_ota_code_erase_sector(_ota_erase_sector);
//            printf("size %d\r\n",_ota_erase_sector);
//            _ota_erase_sector++;
//        }
//        else
//        {
//            _ota_erase_request = 0;
//            cmd_fw_erase();
//        }
//
//
//        printf("<<<cmd_fw_erase\r\n");
//    }
}

void ota_reset(void)
{
    _fw_fast_write_size = 0;
    ota_init = 0;
    ota_start = 0;
    _ota_erase_request = 0;
    _ota_erase_sector = 0;
}



