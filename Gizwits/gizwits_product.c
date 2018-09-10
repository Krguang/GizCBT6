/**
************************************************************
* @file         gizwits_product.c
* @brief        Gizwits control protocol processing, and platform-related       hardware initialization 
* @author       Gizwits
* @date         2017-07-19
* @version      V03030000
* @copyright    Gizwits
* 
* @note         机智云.只为智能硬件而生
*               Gizwits Smart Cloud  for Smart Products
*               链接|增值ֵ|开放|中立|安全|自有|自由|生态
*               www.gizwits.com
*
***********************************************************/

#include <stdio.h>
#include <string.h>
#include "gizwits_product.h"
#include "common.h"
#include "usart.h"
#include "tim.h"

static uint32_t timerMsCount;
uint8_t aRxBuffer;

volatile uint16_t localArray[512];
uint16_t modbusSendCount;
volatile uint8_t modbusPullFlag01;
volatile uint8_t modbusPullFlag02;
volatile uint8_t modbusPullFlag03;
volatile uint8_t modbusPullFlag04;


/** User area the current device state structure*/
dataPoint_t currentDataPoint;

extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

/**@} */
/**@name Gizwits User Interface
* @{
*/

/**
* @brief Event handling interface

* Description:

* 1. Users can customize the changes in WiFi module status

* 2. Users can add data points in the function of event processing logic, such as calling the relevant hardware peripherals operating interface

* @param [in] info: event queue
* @param [in] data: protocol data
* @param [in] len: protocol data length
* @return NULL
* @ref gizwits_protocol.h
*/
int8_t gizwitsEventProcess(eventInfo_t *info, uint8_t *gizdata, uint32_t len)
{
  uint8_t i = 0;
  dataPoint_t *dataPointPtr = (dataPoint_t *)gizdata;
  moduleStatusInfo_t *wifiData = (moduleStatusInfo_t *)gizdata;
  protocolTime_t *ptime = (protocolTime_t *)gizdata;
  
#if MODULE_TYPE
  gprsInfo_t *gprsInfoData = (gprsInfo_t *)gizdata;
#else
  moduleInfo_t *ptModuleInfo = (moduleInfo_t *)gizdata;
#endif

  if((NULL == info) || (NULL == gizdata))
  {
    return -1;
  }

  for(i=0; i<info->num; i++)
  {
    switch(info->event[i])
    {
      case EVENT_online_flag:
        currentDataPoint.valueonline_flag = dataPointPtr->valueonline_flag;
        GIZWITS_LOG("Evt: EVENT_online_flag %d \n", currentDataPoint.valueonline_flag);
        if(0x01 == currentDataPoint.valueonline_flag)
        {
          //user handle
        }
        else
        {
          //user handle    
        }
        break;




      case WIFI_SOFTAP:
        break;
      case WIFI_AIRLINK:
        break;
      case WIFI_STATION:
        break;
      case WIFI_CON_ROUTER:
 
        break;
      case WIFI_DISCON_ROUTER:
 
        break;
      case WIFI_CON_M2M:
 
        break;
      case WIFI_DISCON_M2M:
        break;
      case WIFI_RSSI:
        GIZWITS_LOG("RSSI %d\n", wifiData->rssi);
        break;
      case TRANSPARENT_DATA:
        GIZWITS_LOG("TRANSPARENT_DATA \n");
        //user handle , Fetch data from [data] , size is [len]
        break;
      case WIFI_NTP:
        GIZWITS_LOG("WIFI_NTP : [%d-%d-%d %02d:%02d:%02d][%d] \n",ptime->year,ptime->month,ptime->day,ptime->hour,ptime->minute,ptime->second,ptime->ntp);
        break;
      case MODULE_INFO:
            GIZWITS_LOG("MODULE INFO ...\n");
      #if MODULE_TYPE
            GIZWITS_LOG("GPRS MODULE ...\n");
            //Format By gprsInfo_t
      #else
            GIZWITS_LOG("WIF MODULE ...\n");
            //Format By moduleInfo_t
            GIZWITS_LOG("moduleType : [%d] \n",ptModuleInfo->moduleType);
      #endif
    break;
      default:
        break;
    }
  }

  return 0;
}

/**
* User data acquisition

* Here users need to achieve in addition to data points other than the collection of data collection, can be self-defined acquisition frequency and design data filtering algorithm

* @param none
* @return none
*/
void userHandle(void)
{
	currentDataPoint.valueAHU301_fengJiYiQiDong = localArray[8];
	currentDataPoint.valueAHU301_zhiBanStatus = localArray[9];
	currentDataPoint.valueAHU301_dianJiaRe1 = localArray[10];
	currentDataPoint.valueAHU301_dianJiaRe2 = localArray[11];
	currentDataPoint.valueAHU301_dianJiaRe3 = localArray[12];
	currentDataPoint.valueAHU301_fengJiStatus = localArray[13];
	currentDataPoint.valueAHU301_shouZiDong = localArray[14];
	currentDataPoint.valueAHU301_dongXiaJi = localArray[15];
	currentDataPoint.valueAHU301_zhongXiaoBaoJing = localArray[16];
	currentDataPoint.valueAHU301_dianJiaReGaoWen = localArray[17];
	currentDataPoint.valueAHU301_fengJiQueFeng = localArray[18];
	currentDataPoint.valueAHU301_paiFengJiYiQiDong = localArray[19];
	currentDataPoint.valueAHU301_diWenPanGuan = localArray[27];
	currentDataPoint.valueAHU301_mieJunYunXing = localArray[28];
	currentDataPoint.valueAHU302_fengJiYiQiDong = localArray[38];
	currentDataPoint.valueAHU302_zhiBanStatus = localArray[39];
	currentDataPoint.valueAHU302_dianJiaRe1 = localArray[40];
	currentDataPoint.valueAHU302_dianJiaRe2 = localArray[41];
	currentDataPoint.valueAHU302_dianJiaRe3 = localArray[42];
	currentDataPoint.valueAHU302_fengJiStatus = localArray[43];
	currentDataPoint.valueAHU302_shouZiDong = localArray[44];
	currentDataPoint.valueAHU302_dongXiaJi = localArray[45];
	currentDataPoint.valueAHU302_zhongXiaoBaoJing = localArray[46];
	currentDataPoint.valueAHU302_dianJiaReGaoWen = localArray[47];
	currentDataPoint.valueAHU302_fengJiQueFeng = localArray[48];
	currentDataPoint.valueAHU302_paiFengJiYiQiDong = localArray[49];
	currentDataPoint.valueAHU302_diWenPanGuan = localArray[57];
	currentDataPoint.valueAHU302_mieJunYunXing = localArray[58];
	currentDataPoint.valueAHU303_fengJiYiQiDong = localArray[68];
	currentDataPoint.valueAHU303_zhiBanStatus = localArray[69];
	currentDataPoint.valueAHU303_dianJiaRe1 = localArray[70];
	currentDataPoint.valueAHU303_dianJiaRe2 = localArray[71];
	currentDataPoint.valueAHU303_dianJiaRe3 = localArray[72];
	currentDataPoint.valueAHU303_fengJiStatus = localArray[73];
	currentDataPoint.valueAHU303_shouZiDong = localArray[74];
	currentDataPoint.valueAHU303_dongXiaJi = localArray[75];
	currentDataPoint.valueAHU303_zhongXiaoBaoJing = localArray[76];
	currentDataPoint.valueAHU303_dianJiaReGaoWen = localArray[77];
	currentDataPoint.valueAHU303_fengJiQueFeng = localArray[78];
	currentDataPoint.valueAHU303_paiFengJiYiQiDong = localArray[79];
	currentDataPoint.valueAHU303_diWenPanGuan = localArray[87];
	currentDataPoint.valueAHU303_mieJunYunXing = localArray[88];
	currentDataPoint.valueAHU304_fengJiYiQiDong = localArray[98];
	currentDataPoint.valueAHU304_zhiBanStatus = localArray[99];
	currentDataPoint.valueAHU304_dianJiaRe1 = localArray[100];
	currentDataPoint.valueAHU304_dianJiaRe2 = localArray[101];
	currentDataPoint.valueAHU304_dianJiaRe3 = localArray[102];
	currentDataPoint.valueAHU304_fengJiStatus = localArray[103];
	currentDataPoint.valueAHU304_shouZiDong = localArray[104];
	currentDataPoint.valueAHU304_dongXiaJi = localArray[105];
	currentDataPoint.valueAHU304_zhongXiaoBaoJing = localArray[106];
	currentDataPoint.valueAHU304_dianJiaReGaoWen = localArray[107];
	currentDataPoint.valueAHU304_fengJiQueFeng = localArray[108];
	currentDataPoint.valueAHU304_paiFengJiYiQiDong = localArray[109];
	currentDataPoint.valueAHU304_diWenPanGuan = localArray[117];
	currentDataPoint.valueAHU304_mieJunYunXing = localArray[118];
	currentDataPoint.valueAHU305_fengJiYiQiDong = localArray[128];
	currentDataPoint.valueAHU305_zhiBanStatus = localArray[129];
	currentDataPoint.valueAHU305_dianJiaRe1 = localArray[130];
	currentDataPoint.valueAHU305_dianJiaRe2 = localArray[131];
	currentDataPoint.valueAHU305_dianJiaRe3 = localArray[132];
	currentDataPoint.valueAHU305_fengJiStatus = localArray[133];
	currentDataPoint.valueAHU305_shouZiDong = localArray[134];
	currentDataPoint.valueAHU305_dongXiaJi = localArray[135];
	currentDataPoint.valueAHU305_zhongXiaoBaoJing = localArray[136];
	currentDataPoint.valueAHU305_dianJiaReGaoWen = localArray[137];
	currentDataPoint.valueAHU305_fengJiQueFeng = localArray[138];
	currentDataPoint.valueAHU305_paiFengJiYiQiDong = localArray[139];
	currentDataPoint.valueAHU305_diWenPanGuan = localArray[147];
	currentDataPoint.valueAHU305_mieJunYunXing = localArray[148];
	currentDataPoint.valueAHU306_fengJiYiQiDong = localArray[158];
	currentDataPoint.valueAHU306_zhiBanStatus = localArray[159];
	currentDataPoint.valueAHU306_dianJiaRe1 = localArray[160];
	currentDataPoint.valueAHU306_dianJiaRe2 = localArray[161];
	currentDataPoint.valueAHU306_dianJiaRe3 = localArray[162];
	currentDataPoint.valueAHU306_fengJiStatus = localArray[163];
	currentDataPoint.valueAHU306_shouZiDong = localArray[164];
	currentDataPoint.valueAHU306_dongXiaJi = localArray[165];
	currentDataPoint.valueAHU306_zhongXiaoBaoJing = localArray[166];
	currentDataPoint.valueAHU306_dianJiaReGaoWen = localArray[167];
	currentDataPoint.valueAHU306_fengJiQueFeng = localArray[168];
	currentDataPoint.valueAHU306_paiFengJiYiQiDong = localArray[169];
	currentDataPoint.valueAHU306_diWenPanGuan = localArray[177];
	currentDataPoint.valueAHU306_mieJunYunXing = localArray[178];
	currentDataPoint.valueAHU307_fengJiYiQiDong = localArray[188];
	currentDataPoint.valueAHU307_zhiBanStatus = localArray[189];
	currentDataPoint.valueAHU307_dianJiaRe1 = localArray[190];
	currentDataPoint.valueAHU307_dianJiaRe2 = localArray[191];
	currentDataPoint.valueAHU307_dianJiaRe3 = localArray[192];
	currentDataPoint.valueAHU307_fengJiStatus = localArray[193];
	currentDataPoint.valueAHU307_shouZiDong = localArray[194];
	currentDataPoint.valueAHU307_dongXiaJi = localArray[195];
	currentDataPoint.valueAHU307_zhongXiaoBaoJing = localArray[196];
	currentDataPoint.valueAHU307_dianJiaReGaoWen = localArray[197];
	currentDataPoint.valueAHU307_fengJiQueFeng = localArray[198];
	currentDataPoint.valueAHU307_paiFengJiYiQiDong = localArray[199];
	currentDataPoint.valueAHU307_diWenPanGuan = localArray[207];
	currentDataPoint.valueAHU307_mieJunYunXing = localArray[208];
	currentDataPoint.valueAHU308_fengJiYiQiDong = localArray[218];
	currentDataPoint.valueAHU308_zhiBanStatus = localArray[219];
	currentDataPoint.valueAHU308_dianJiaRe1 = localArray[220];
	currentDataPoint.valueAHU308_dianJiaRe2 = localArray[221];
	currentDataPoint.valueAHU308_dianJiaRe3 = localArray[222];
	currentDataPoint.valueAHU308_fengJiStatus = localArray[223];
	currentDataPoint.valueAHU308_shouZiDong = localArray[224];
	currentDataPoint.valueAHU308_dongXiaJi = localArray[225];
	currentDataPoint.valueAHU308_zhongXiaoBaoJing = localArray[226];
	currentDataPoint.valueAHU308_dianJiaReGaoWen = localArray[227];
	currentDataPoint.valueAHU308_fengJiQueFeng = localArray[228];
	currentDataPoint.valueAHU308_paiFengJiYiQiDong = localArray[229];
	currentDataPoint.valueAHU308_diWenPanGuan = localArray[237];
	currentDataPoint.valueAHU308_mieJunYunXing = localArray[238];
	currentDataPoint.valuePAU301_fengJiYiQiDong = localArray[248];
	currentDataPoint.valuePAU301_zhiBanStatus = localArray[249];
	currentDataPoint.valuePAU301_dianJiaRe1 = localArray[250];
	currentDataPoint.valuePAU301_dianJiaRe2 = localArray[251];
	currentDataPoint.valuePAU301_dianJiaRe3 = localArray[252];
	currentDataPoint.valuePAU301_fengJiStatus = localArray[253];
	currentDataPoint.valuePAU301_shouZiDong = localArray[254];
	currentDataPoint.valuePAU301_dongXiaJi = localArray[255];
	currentDataPoint.valuePAU301_zhongXiaoBaoJing = localArray[256];
	currentDataPoint.valuePAU301_dianJiaReGaoWen = localArray[257];
	currentDataPoint.valuePAU301_fengJiQueFeng = localArray[258];
	currentDataPoint.valuePAU301_paiFengJiYiQiDong = localArray[259];
	currentDataPoint.valuePAU301_dianYuRe1 = localArray[263];
	currentDataPoint.valuePAU301_dianYuRe2 = localArray[264];
	currentDataPoint.valuePAU301_dianYuRe3 = localArray[265];
	currentDataPoint.valuePAU301_dianYuReGaoWen = localArray[266];
	currentDataPoint.valuePAU301_diWenPanGuan = localArray[267];
	currentDataPoint.valuePAU301_mieJunYunXing = localArray[268];
	currentDataPoint.valuePAU302_fengJiYiQiDong = localArray[278];
	currentDataPoint.valuePAU302_zhiBanStatus = localArray[279];
	currentDataPoint.valuePAU302_fengJiStatus = localArray[283];
	currentDataPoint.valuePAU302_shouZiDong = localArray[284];
	currentDataPoint.valuePAU302_dongXiaJi = localArray[285];
	currentDataPoint.valuePAU302_zhongXiaoBaoJing = localArray[286];
	currentDataPoint.valuePAU302_fengJiQueFeng = localArray[288];
	currentDataPoint.valuePAU302_paiFengJiYiQiDong = localArray[289];
	currentDataPoint.valuePAU302_dianYuRe1 = localArray[293];
	currentDataPoint.valuePAU302_dianYuRe2 = localArray[294];
	currentDataPoint.valuePAU302_dianYuRe3 = localArray[295];
	currentDataPoint.valuePAU302_dianYuReGaoWen = localArray[296];
	currentDataPoint.valuePAU302_diWenPanGuan = localArray[297];
	currentDataPoint.valuePAU302_mieJunYunXing = localArray[298];
	currentDataPoint.valuePAU303_fengJiYiQiDong = localArray[308];
	currentDataPoint.valuePAU303_zhiBanStatus = localArray[309];
	currentDataPoint.valuePAU303_fengJiStatus = localArray[313];
	currentDataPoint.valuePAU303_shouZiDong = localArray[314];
	currentDataPoint.valuePAU303_dongXiaJi = localArray[315];
	currentDataPoint.valuePAU303_zhongXiaoBaoJing = localArray[316];
	currentDataPoint.valuePAU303_fengJiQueFeng = localArray[318];
	currentDataPoint.valuePAU303_paiFengJiYiQiDong = localArray[319];
	currentDataPoint.valuePAU303_dianYuRe1 = localArray[323];
	currentDataPoint.valuePAU303_dianYuRe2 = localArray[324];
	currentDataPoint.valuePAU303_dianYuRe3 = localArray[325];
	currentDataPoint.valuePAU303_dianYuReGaoWen = localArray[326];
	currentDataPoint.valuePAU303_diWenPanGuan = localArray[327];
	currentDataPoint.valuePAU303_mieJunYunXing = localArray[328];
	currentDataPoint.valuePAU304_fengJiYiQiDong = localArray[338];
	currentDataPoint.valuePAU304_zhiBanStatus = localArray[339];
	currentDataPoint.valuePAU304_fengJiStatus = localArray[343];
	currentDataPoint.valuePAU304_shouZiDong = localArray[344];
	currentDataPoint.valuePAU304_dongXiaJi = localArray[345];
	currentDataPoint.valuePAU304_zhongXiaoBaoJing = localArray[346];
	currentDataPoint.valuePAU304_fengJiQueFeng = localArray[348];
	currentDataPoint.valuePAU304_paiFengJiYiQiDong = localArray[349];
	currentDataPoint.valuePAU304_dianYuRe1 = localArray[353];
	currentDataPoint.valuePAU304_dianYuRe2 = localArray[354];
	currentDataPoint.valuePAU304_dianYuRe3 = localArray[355];
	currentDataPoint.valuePAU304_dianYuReGaoWen = localArray[356];
	currentDataPoint.valuePAU304_diWenPanGuan = localArray[357];
	currentDataPoint.valuePAU304_mieJunYunXing = localArray[358];
	currentDataPoint.valuePAU305_fengJiYiQiDong = localArray[368];
	currentDataPoint.valuePAU305_zhiBanStatus = localArray[369];
	currentDataPoint.valuePAU305_fengJiStatus = localArray[373];
	currentDataPoint.valuePAU305_shouZiDong = localArray[374];
	currentDataPoint.valuePAU305_dongXiaJi = localArray[375];
	currentDataPoint.valuePAU305_zhongXiaoBaoJing = localArray[376];
	currentDataPoint.valuePAU305_fengJiQueFeng = localArray[378];
	currentDataPoint.valuePAU305_paiFengJiYiQiDong = localArray[379];
	currentDataPoint.valuePAU305_dianYuRe1 = localArray[383];
	currentDataPoint.valuePAU305_dianYuRe2 = localArray[384];
	currentDataPoint.valuePAU305_dianYuRe3 = localArray[385];
	currentDataPoint.valuePAU305_dianYuReGaoWen = localArray[386];
	currentDataPoint.valuePAU305_diWenPanGuan = localArray[387];
	currentDataPoint.valuePAU305_mieJunYunXing = localArray[388];
	currentDataPoint.valueAHU301_mianBanTongXunZhuangTai1 = localArray[20];
	currentDataPoint.valueAHU301_mianBanTongXunZhuangTai2 = localArray[21];
	currentDataPoint.valueAHU301_mianBanTongXunZhuangTai3 = localArray[22];
	currentDataPoint.valueAHU302_mianBanTongXunZhuangTai1 = localArray[50];
	currentDataPoint.valueAHU302_mianBanTongXunZhuangTai2 = localArray[51];
	currentDataPoint.valueAHU302_mianBanTongXunZhuangTai3 = localArray[52];
	currentDataPoint.valueAHU303_mianBanTongXunZhuangTai1 = localArray[80];
	currentDataPoint.valueAHU303_mianBanTongXunZhuangTai2 = localArray[81];
	currentDataPoint.valueAHU303_mianBanTongXunZhuangTai3 = localArray[82];
	currentDataPoint.valueAHU304_mianBanTongXunZhuangTai1 = localArray[110];
	currentDataPoint.valueAHU304_mianBanTongXunZhuangTai2 = localArray[111];
	currentDataPoint.valueAHU304_mianBanTongXunZhuangTai3 = localArray[112];
	currentDataPoint.valueAHU305_mianBanTongXunZhuangTai1 = localArray[140];
	currentDataPoint.valueAHU305_mianBanTongXunZhuangTai2 = localArray[141];
	currentDataPoint.valueAHU305_mianBanTongXunZhuangTai3 = localArray[142];
	currentDataPoint.valueAHU306_mianBanTongXunZhuangTai1 = localArray[170];
	currentDataPoint.valueAHU306_mianBanTongXunZhuangTai2 = localArray[171];
	currentDataPoint.valueAHU306_mianBanTongXunZhuangTai3 = localArray[172];
	currentDataPoint.valueAHU307_mianBanTongXunZhuangTai1 = localArray[200];
	currentDataPoint.valueAHU307_mianBanTongXunZhuangTai2 = localArray[201];
	currentDataPoint.valueAHU307_mianBanTongXunZhuangTai3 = localArray[202];
	currentDataPoint.valueAHU308_mianBanTongXunZhuangTai1 = localArray[230];
	currentDataPoint.valueAHU308_mianBanTongXunZhuangTai2 = localArray[231];
	currentDataPoint.valueAHU308_mianBanTongXunZhuangTai3 = localArray[232];
	currentDataPoint.valuePAU301_mianBanTongXunZhuangTai1 = localArray[260];
	currentDataPoint.valuePAU301_mianBanTongXunZhuangTai2 = localArray[261];
	currentDataPoint.valuePAU301_mianBanTongXunZhuangTai3 = localArray[262];
	currentDataPoint.valuePAU302_mianBanTongXunZhuangTai1 = localArray[290];
	currentDataPoint.valuePAU302_mianBanTongXunZhuangTai2 = localArray[291];
	currentDataPoint.valuePAU302_mianBanTongXunZhuangTai3 = localArray[292];
	currentDataPoint.valuePAU303_mianBanTongXunZhuangTai1 = localArray[320];
	currentDataPoint.valuePAU303_mianBanTongXunZhuangTai2 = localArray[321];
	currentDataPoint.valuePAU303_mianBanTongXunZhuangTai3 = localArray[322];
	currentDataPoint.valuePAU304_mianBanTongXunZhuangTai1 = localArray[350];
	currentDataPoint.valuePAU304_mianBanTongXunZhuangTai2 = localArray[351];
	currentDataPoint.valuePAU304_mianBanTongXunZhuangTai3 = localArray[352];
	currentDataPoint.valuePAU305_mianBanTongXunZhuangTai1 = localArray[380];
	currentDataPoint.valuePAU305_mianBanTongXunZhuangTai2 = localArray[381];
	currentDataPoint.valuePAU305_mianBanTongXunZhuangTai3 = localArray[382];
	currentDataPoint.valueAHU301_tempReal = localArray[0];
	currentDataPoint.valueAHU301_humiReal = localArray[1];
	currentDataPoint.valueAHU301_tempSet = localArray[2];
	currentDataPoint.valueAHU301_humiSet = localArray[3];
	currentDataPoint.valueAHU301_lengShuiFaKaiDu = localArray[4];
	currentDataPoint.valueAHU301_reShuiFaKaiDu = localArray[5];
	currentDataPoint.valueAHU301_xinFengWenDU = localArray[6];
	currentDataPoint.valueAHU301_jiaShiQIKaiDu = localArray[7];
	currentDataPoint.valueAHU301_beiYong = localArray[29];
	currentDataPoint.valueAHU302_tempReal = localArray[30];
	currentDataPoint.valueAHU302_humiReal = localArray[31];
	currentDataPoint.valueAHU302_tempSet = localArray[32];
	currentDataPoint.valueAHU302_humiSet = localArray[33];
	currentDataPoint.valueAHU302_lengShuiFaKaiDu = localArray[34];
	currentDataPoint.valueAHU302_reShuiFaKaiDu = localArray[35];
	currentDataPoint.valueAHU302_xinFengWenDU = localArray[36];
	currentDataPoint.valueAHU302_jiaShiQIKaiDu = localArray[37];
	currentDataPoint.valueAHU302_beiYong = localArray[59];
	currentDataPoint.valueAHU303_tempReal = localArray[60];
	currentDataPoint.valueAHU303_humiReal = localArray[61];
	currentDataPoint.valueAHU303_tempSet = localArray[62];
	currentDataPoint.valueAHU303_humiSet = localArray[63];
	currentDataPoint.valueAHU303_lengShuiFaKaiDu = localArray[64];
	currentDataPoint.valueAHU303_reShuiFaKaiDu = localArray[65];
	currentDataPoint.valueAHU303_xinFengWenDU = localArray[66];
	currentDataPoint.valueAHU303_jiaShiQIKaiDu = localArray[67];
	currentDataPoint.valueAHU303_beiYong = localArray[89];
	currentDataPoint.valueAHU304_tempReal = localArray[90];
	currentDataPoint.valueAHU304_humiReal = localArray[91];
	currentDataPoint.valueAHU304_tempSet = localArray[92];
	currentDataPoint.valueAHU304_humiSet = localArray[93];
	currentDataPoint.valueAHU304_lengShuiFaKaiDu = localArray[94];
	currentDataPoint.valueAHU304_reShuiFaKaiDu = localArray[95];
	currentDataPoint.valueAHU304_xinFengWenDU = localArray[96];
	currentDataPoint.valueAHU304_jiaShiQIKaiDu = localArray[97];
	currentDataPoint.valueAHU304_beiYong = localArray[119];
	currentDataPoint.valueAHU305_tempReal = localArray[120];
	currentDataPoint.valueAHU305_humiReal = localArray[121];
	currentDataPoint.valueAHU305_tempSet = localArray[122];
	currentDataPoint.valueAHU305_humiSet = localArray[123];
	currentDataPoint.valueAHU305_lengShuiFaKaiDu = localArray[124];
	currentDataPoint.valueAHU305_reShuiFaKaiDu = localArray[125];
	currentDataPoint.valueAHU305_xinFengWenDU = localArray[126];
	currentDataPoint.valueAHU305_jiaShiQIKaiDu = localArray[127];
	currentDataPoint.valueAHU305_beiYong = localArray[149];
	currentDataPoint.valueAHU306_tempReal = localArray[150];
	currentDataPoint.valueAHU306_humiReal = localArray[151];
	currentDataPoint.valueAHU306_tempSet = localArray[152];
	currentDataPoint.valueAHU306_humiSet = localArray[153];
	currentDataPoint.valueAHU306_lengShuiFaKaiDu = localArray[154];
	currentDataPoint.valueAHU306_reShuiFaKaiDu = localArray[155];
	currentDataPoint.valueAHU306_xinFengWenDU = localArray[156];
	currentDataPoint.valueAHU306_jiaShiQIKaiDu = localArray[157];
	currentDataPoint.valueAHU306_beiYong = localArray[179];
	currentDataPoint.valueAHU307_tempReal = localArray[180];
	currentDataPoint.valueAHU307_humiReal = localArray[181];
	currentDataPoint.valueAHU307_tempSet = localArray[182];
	currentDataPoint.valueAHU307_humiSet = localArray[183];
	currentDataPoint.valueAHU307_lengShuiFaKaiDu = localArray[184];
	currentDataPoint.valueAHU307_reShuiFaKaiDu = localArray[185];
	currentDataPoint.valueAHU307_xinFengWenDU = localArray[186];
	currentDataPoint.valueAHU307_jiaShiQIKaiDu = localArray[187];
	currentDataPoint.valueAHU307_beiYong = localArray[209];
	currentDataPoint.valueAHU308_tempReal = localArray[210];
	currentDataPoint.valueAHU308_humiReal = localArray[211];
	currentDataPoint.valueAHU308_tempSet = localArray[212];
	currentDataPoint.valueAHU308_humiSet = localArray[213];
	currentDataPoint.valueAHU308_lengShuiFaKaiDu = localArray[214];
	currentDataPoint.valueAHU308_reShuiFaKaiDu = localArray[215];
	currentDataPoint.valueAHU308_xinFengWenDU = localArray[216];
	currentDataPoint.valueAHU308_jiaShiQIKaiDu = localArray[217];
	currentDataPoint.valueAHU308_beiYong = localArray[239];
	currentDataPoint.valuePAU301_tempReal = localArray[240];
	currentDataPoint.valuePAU301_humiReal = localArray[241];
	currentDataPoint.valuePAU301_tempSet = localArray[242];
	currentDataPoint.valuePAU301_humiSet = localArray[243];
	currentDataPoint.valuePAU301_lengShuiFaKaiDu = localArray[244];
	currentDataPoint.valuePAU301_reShuiFaKaiDu = localArray[245];
	currentDataPoint.valuePAU301_xinFengWenDU = localArray[246];
	currentDataPoint.valuePAU301_jiaShiQIKaiDu = localArray[247];
	currentDataPoint.valuePAU301_beiYong = localArray[269];
	currentDataPoint.valuePAU302_tempReal = localArray[270];
	currentDataPoint.valuePAU302_humiReal = localArray[271];
	currentDataPoint.valuePAU302_tempSet = localArray[272];
	currentDataPoint.valuePAU302_humiSet = localArray[273];
	currentDataPoint.valuePAU302_lengShuiFaKaiDu = localArray[274];
	currentDataPoint.valuePAU302_reShuiFaKaiDu = localArray[275];
	currentDataPoint.valuePAU302_xinFengWenDU = localArray[276];
	currentDataPoint.valuePAU302_jiaShiQIKaiDu = localArray[277];
	currentDataPoint.valuePAU302_beiYong = localArray[299];
	currentDataPoint.valuePAU303_tempReal = localArray[300];
	currentDataPoint.valuePAU303_humiReal = localArray[301];
	currentDataPoint.valuePAU303_tempSet = localArray[302];
	currentDataPoint.valuePAU303_humiSet = localArray[303];
	currentDataPoint.valuePAU303_lengShuiFaKaiDu = localArray[304];
	currentDataPoint.valuePAU303_reShuiFaKaiDu = localArray[305];
	currentDataPoint.valuePAU303_xinFengWenDU = localArray[306];
	currentDataPoint.valuePAU303_jiaShiQIKaiDu = localArray[307];
	currentDataPoint.valuePAU303_beiYong = localArray[329];
	currentDataPoint.valuePAU304_tempReal = localArray[330];
	currentDataPoint.valuePAU304_humiReal = localArray[331];
	currentDataPoint.valuePAU304_tempSet = localArray[332];
	currentDataPoint.valuePAU304_humiSet = localArray[333];
	currentDataPoint.valuePAU304_lengShuiFaKaiDu = localArray[334];
	currentDataPoint.valuePAU304_reShuiFaKaiDu = localArray[335];
	currentDataPoint.valuePAU304_xinFengWenDU = localArray[336];
	currentDataPoint.valuePAU304_jiaShiQIKaiDu = localArray[337];
	currentDataPoint.valuePAU304_beiYong = localArray[359];
	currentDataPoint.valuePAU305_tempReal = localArray[360];
	currentDataPoint.valuePAU305_humiReal = localArray[361];
	currentDataPoint.valuePAU305_tempSet = localArray[362];
	currentDataPoint.valuePAU305_humiSet = localArray[363];
	currentDataPoint.valuePAU305_lengShuiFaKaiDu = localArray[364];
	currentDataPoint.valuePAU305_reShuiFaKaiDu = localArray[365];
	currentDataPoint.valuePAU305_xinFengWenDU = localArray[366];
	currentDataPoint.valuePAU305_jiaShiQIKaiDu = localArray[367];
	currentDataPoint.valuePAU305_beiYong = localArray[389];
	currentDataPoint.valueonline_flag = localArray[390];
}

/**
* Data point initialization function

* In the function to complete the initial user-related data
* @param none
* @return none
* @note The developer can add a data point state initialization value within this function
*/
void userInit(void)
{
    memset((uint8_t*)&currentDataPoint, 0, sizeof(dataPoint_t));
    
    /** Warning !!! DataPoint Variables Init , Must Within The Data Range **/ 
    /*
      currentDataPoint.valueAHU301_fengJiYiQiDong = ;
      currentDataPoint.valueAHU301_zhiBanStatus = ;
      currentDataPoint.valueAHU301_dianJiaRe1 = ;
      currentDataPoint.valueAHU301_dianJiaRe2 = ;
      currentDataPoint.valueAHU301_dianJiaRe3 = ;
      currentDataPoint.valueAHU301_fengJiStatus = ;
      currentDataPoint.valueAHU301_shouZiDong = ;
      currentDataPoint.valueAHU301_dongXiaJi = ;
      currentDataPoint.valueAHU301_zhongXiaoBaoJing = ;
      currentDataPoint.valueAHU301_dianJiaReGaoWen = ;
      currentDataPoint.valueAHU301_fengJiQueFeng = ;
      currentDataPoint.valueAHU301_paiFengJiYiQiDong = ;
      currentDataPoint.valueAHU301_diWenPanGuan = ;
      currentDataPoint.valueAHU301_mieJunYunXing = ;
      currentDataPoint.valueAHU302_fengJiYiQiDong = ;
      currentDataPoint.valueAHU302_zhiBanStatus = ;
      currentDataPoint.valueAHU302_dianJiaRe1 = ;
      currentDataPoint.valueAHU302_dianJiaRe2 = ;
      currentDataPoint.valueAHU302_dianJiaRe3 = ;
      currentDataPoint.valueAHU302_fengJiStatus = ;
      currentDataPoint.valueAHU302_shouZiDong = ;
      currentDataPoint.valueAHU302_dongXiaJi = ;
      currentDataPoint.valueAHU302_zhongXiaoBaoJing = ;
      currentDataPoint.valueAHU302_dianJiaReGaoWen = ;
      currentDataPoint.valueAHU302_fengJiQueFeng = ;
      currentDataPoint.valueAHU302_paiFengJiYiQiDong = ;
      currentDataPoint.valueAHU302_diWenPanGuan = ;
      currentDataPoint.valueAHU302_mieJunYunXing = ;
      currentDataPoint.valueAHU303_fengJiYiQiDong = ;
      currentDataPoint.valueAHU303_zhiBanStatus = ;
      currentDataPoint.valueAHU303_dianJiaRe1 = ;
      currentDataPoint.valueAHU303_dianJiaRe2 = ;
      currentDataPoint.valueAHU303_dianJiaRe3 = ;
      currentDataPoint.valueAHU303_fengJiStatus = ;
      currentDataPoint.valueAHU303_shouZiDong = ;
      currentDataPoint.valueAHU303_dongXiaJi = ;
      currentDataPoint.valueAHU303_zhongXiaoBaoJing = ;
      currentDataPoint.valueAHU303_dianJiaReGaoWen = ;
      currentDataPoint.valueAHU303_fengJiQueFeng = ;
      currentDataPoint.valueAHU303_paiFengJiYiQiDong = ;
      currentDataPoint.valueAHU303_diWenPanGuan = ;
      currentDataPoint.valueAHU303_mieJunYunXing = ;
      currentDataPoint.valueAHU304_fengJiYiQiDong = ;
      currentDataPoint.valueAHU304_zhiBanStatus = ;
      currentDataPoint.valueAHU304_dianJiaRe1 = ;
      currentDataPoint.valueAHU304_dianJiaRe2 = ;
      currentDataPoint.valueAHU304_dianJiaRe3 = ;
      currentDataPoint.valueAHU304_fengJiStatus = ;
      currentDataPoint.valueAHU304_shouZiDong = ;
      currentDataPoint.valueAHU304_dongXiaJi = ;
      currentDataPoint.valueAHU304_zhongXiaoBaoJing = ;
      currentDataPoint.valueAHU304_dianJiaReGaoWen = ;
      currentDataPoint.valueAHU304_fengJiQueFeng = ;
      currentDataPoint.valueAHU304_paiFengJiYiQiDong = ;
      currentDataPoint.valueAHU304_diWenPanGuan = ;
      currentDataPoint.valueAHU304_mieJunYunXing = ;
      currentDataPoint.valueAHU305_fengJiYiQiDong = ;
      currentDataPoint.valueAHU305_zhiBanStatus = ;
      currentDataPoint.valueAHU305_dianJiaRe1 = ;
      currentDataPoint.valueAHU305_dianJiaRe2 = ;
      currentDataPoint.valueAHU305_dianJiaRe3 = ;
      currentDataPoint.valueAHU305_fengJiStatus = ;
      currentDataPoint.valueAHU305_shouZiDong = ;
      currentDataPoint.valueAHU305_dongXiaJi = ;
      currentDataPoint.valueAHU305_zhongXiaoBaoJing = ;
      currentDataPoint.valueAHU305_dianJiaReGaoWen = ;
      currentDataPoint.valueAHU305_fengJiQueFeng = ;
      currentDataPoint.valueAHU305_paiFengJiYiQiDong = ;
      currentDataPoint.valueAHU305_diWenPanGuan = ;
      currentDataPoint.valueAHU305_mieJunYunXing = ;
      currentDataPoint.valueAHU306_fengJiYiQiDong = ;
      currentDataPoint.valueAHU306_zhiBanStatus = ;
      currentDataPoint.valueAHU306_dianJiaRe1 = ;
      currentDataPoint.valueAHU306_dianJiaRe2 = ;
      currentDataPoint.valueAHU306_dianJiaRe3 = ;
      currentDataPoint.valueAHU306_fengJiStatus = ;
      currentDataPoint.valueAHU306_shouZiDong = ;
      currentDataPoint.valueAHU306_dongXiaJi = ;
      currentDataPoint.valueAHU306_zhongXiaoBaoJing = ;
      currentDataPoint.valueAHU306_dianJiaReGaoWen = ;
      currentDataPoint.valueAHU306_fengJiQueFeng = ;
      currentDataPoint.valueAHU306_paiFengJiYiQiDong = ;
      currentDataPoint.valueAHU306_diWenPanGuan = ;
      currentDataPoint.valueAHU306_mieJunYunXing = ;
      currentDataPoint.valueAHU307_fengJiYiQiDong = ;
      currentDataPoint.valueAHU307_zhiBanStatus = ;
      currentDataPoint.valueAHU307_dianJiaRe1 = ;
      currentDataPoint.valueAHU307_dianJiaRe2 = ;
      currentDataPoint.valueAHU307_dianJiaRe3 = ;
      currentDataPoint.valueAHU307_fengJiStatus = ;
      currentDataPoint.valueAHU307_shouZiDong = ;
      currentDataPoint.valueAHU307_dongXiaJi = ;
      currentDataPoint.valueAHU307_zhongXiaoBaoJing = ;
      currentDataPoint.valueAHU307_dianJiaReGaoWen = ;
      currentDataPoint.valueAHU307_fengJiQueFeng = ;
      currentDataPoint.valueAHU307_paiFengJiYiQiDong = ;
      currentDataPoint.valueAHU307_diWenPanGuan = ;
      currentDataPoint.valueAHU307_mieJunYunXing = ;
      currentDataPoint.valueAHU308_fengJiYiQiDong = ;
      currentDataPoint.valueAHU308_zhiBanStatus = ;
      currentDataPoint.valueAHU308_dianJiaRe1 = ;
      currentDataPoint.valueAHU308_dianJiaRe2 = ;
      currentDataPoint.valueAHU308_dianJiaRe3 = ;
      currentDataPoint.valueAHU308_fengJiStatus = ;
      currentDataPoint.valueAHU308_shouZiDong = ;
      currentDataPoint.valueAHU308_dongXiaJi = ;
      currentDataPoint.valueAHU308_zhongXiaoBaoJing = ;
      currentDataPoint.valueAHU308_dianJiaReGaoWen = ;
      currentDataPoint.valueAHU308_fengJiQueFeng = ;
      currentDataPoint.valueAHU308_paiFengJiYiQiDong = ;
      currentDataPoint.valueAHU308_diWenPanGuan = ;
      currentDataPoint.valueAHU308_mieJunYunXing = ;
      currentDataPoint.valuePAU301_fengJiYiQiDong = ;
      currentDataPoint.valuePAU301_zhiBanStatus = ;
      currentDataPoint.valuePAU301_dianJiaRe1 = ;
      currentDataPoint.valuePAU301_dianJiaRe2 = ;
      currentDataPoint.valuePAU301_dianJiaRe3 = ;
      currentDataPoint.valuePAU301_fengJiStatus = ;
      currentDataPoint.valuePAU301_shouZiDong = ;
      currentDataPoint.valuePAU301_dongXiaJi = ;
      currentDataPoint.valuePAU301_zhongXiaoBaoJing = ;
      currentDataPoint.valuePAU301_dianJiaReGaoWen = ;
      currentDataPoint.valuePAU301_fengJiQueFeng = ;
      currentDataPoint.valuePAU301_paiFengJiYiQiDong = ;
      currentDataPoint.valuePAU301_dianYuRe1 = ;
      currentDataPoint.valuePAU301_dianYuRe2 = ;
      currentDataPoint.valuePAU301_dianYuRe3 = ;
      currentDataPoint.valuePAU301_dianYuReGaoWen = ;
      currentDataPoint.valuePAU301_diWenPanGuan = ;
      currentDataPoint.valuePAU301_mieJunYunXing = ;
      currentDataPoint.valuePAU302_fengJiYiQiDong = ;
      currentDataPoint.valuePAU302_zhiBanStatus = ;
      currentDataPoint.valuePAU302_fengJiStatus = ;
      currentDataPoint.valuePAU302_shouZiDong = ;
      currentDataPoint.valuePAU302_dongXiaJi = ;
      currentDataPoint.valuePAU302_zhongXiaoBaoJing = ;
      currentDataPoint.valuePAU302_fengJiQueFeng = ;
      currentDataPoint.valuePAU302_paiFengJiYiQiDong = ;
      currentDataPoint.valuePAU302_dianYuRe1 = ;
      currentDataPoint.valuePAU302_dianYuRe2 = ;
      currentDataPoint.valuePAU302_dianYuRe3 = ;
      currentDataPoint.valuePAU302_dianYuReGaoWen = ;
      currentDataPoint.valuePAU302_diWenPanGuan = ;
      currentDataPoint.valuePAU302_mieJunYunXing = ;
      currentDataPoint.valuePAU303_fengJiYiQiDong = ;
      currentDataPoint.valuePAU303_zhiBanStatus = ;
      currentDataPoint.valuePAU303_fengJiStatus = ;
      currentDataPoint.valuePAU303_shouZiDong = ;
      currentDataPoint.valuePAU303_dongXiaJi = ;
      currentDataPoint.valuePAU303_zhongXiaoBaoJing = ;
      currentDataPoint.valuePAU303_fengJiQueFeng = ;
      currentDataPoint.valuePAU303_paiFengJiYiQiDong = ;
      currentDataPoint.valuePAU303_dianYuRe1 = ;
      currentDataPoint.valuePAU303_dianYuRe2 = ;
      currentDataPoint.valuePAU303_dianYuRe3 = ;
      currentDataPoint.valuePAU303_dianYuReGaoWen = ;
      currentDataPoint.valuePAU303_diWenPanGuan = ;
      currentDataPoint.valuePAU303_mieJunYunXing = ;
      currentDataPoint.valuePAU304_fengJiYiQiDong = ;
      currentDataPoint.valuePAU304_zhiBanStatus = ;
      currentDataPoint.valuePAU304_fengJiStatus = ;
      currentDataPoint.valuePAU304_shouZiDong = ;
      currentDataPoint.valuePAU304_dongXiaJi = ;
      currentDataPoint.valuePAU304_zhongXiaoBaoJing = ;
      currentDataPoint.valuePAU304_fengJiQueFeng = ;
      currentDataPoint.valuePAU304_paiFengJiYiQiDong = ;
      currentDataPoint.valuePAU304_dianYuRe1 = ;
      currentDataPoint.valuePAU304_dianYuRe2 = ;
      currentDataPoint.valuePAU304_dianYuRe3 = ;
      currentDataPoint.valuePAU304_dianYuReGaoWen = ;
      currentDataPoint.valuePAU304_diWenPanGuan = ;
      currentDataPoint.valuePAU304_mieJunYunXing = ;
      currentDataPoint.valuePAU305_fengJiYiQiDong = ;
      currentDataPoint.valuePAU305_zhiBanStatus = ;
      currentDataPoint.valuePAU305_fengJiStatus = ;
      currentDataPoint.valuePAU305_shouZiDong = ;
      currentDataPoint.valuePAU305_dongXiaJi = ;
      currentDataPoint.valuePAU305_zhongXiaoBaoJing = ;
      currentDataPoint.valuePAU305_fengJiQueFeng = ;
      currentDataPoint.valuePAU305_paiFengJiYiQiDong = ;
      currentDataPoint.valuePAU305_dianYuRe1 = ;
      currentDataPoint.valuePAU305_dianYuRe2 = ;
      currentDataPoint.valuePAU305_dianYuRe3 = ;
      currentDataPoint.valuePAU305_dianYuReGaoWen = ;
      currentDataPoint.valuePAU305_diWenPanGuan = ;
      currentDataPoint.valuePAU305_mieJunYunXing = ;
      currentDataPoint.valueonline_flag = ;
      currentDataPoint.valueAHU301_mianBanTongXunZhuangTai1 = ;
      currentDataPoint.valueAHU301_mianBanTongXunZhuangTai2 = ;
      currentDataPoint.valueAHU301_mianBanTongXunZhuangTai3 = ;
      currentDataPoint.valueAHU302_mianBanTongXunZhuangTai1 = ;
      currentDataPoint.valueAHU302_mianBanTongXunZhuangTai2 = ;
      currentDataPoint.valueAHU302_mianBanTongXunZhuangTai3 = ;
      currentDataPoint.valueAHU303_mianBanTongXunZhuangTai1 = ;
      currentDataPoint.valueAHU303_mianBanTongXunZhuangTai2 = ;
      currentDataPoint.valueAHU303_mianBanTongXunZhuangTai3 = ;
      currentDataPoint.valueAHU304_mianBanTongXunZhuangTai1 = ;
      currentDataPoint.valueAHU304_mianBanTongXunZhuangTai2 = ;
      currentDataPoint.valueAHU304_mianBanTongXunZhuangTai3 = ;
      currentDataPoint.valueAHU305_mianBanTongXunZhuangTai1 = ;
      currentDataPoint.valueAHU305_mianBanTongXunZhuangTai2 = ;
      currentDataPoint.valueAHU305_mianBanTongXunZhuangTai3 = ;
      currentDataPoint.valueAHU306_mianBanTongXunZhuangTai1 = ;
      currentDataPoint.valueAHU306_mianBanTongXunZhuangTai2 = ;
      currentDataPoint.valueAHU306_mianBanTongXunZhuangTai3 = ;
      currentDataPoint.valueAHU307_mianBanTongXunZhuangTai1 = ;
      currentDataPoint.valueAHU307_mianBanTongXunZhuangTai2 = ;
      currentDataPoint.valueAHU307_mianBanTongXunZhuangTai3 = ;
      currentDataPoint.valueAHU308_mianBanTongXunZhuangTai1 = ;
      currentDataPoint.valueAHU308_mianBanTongXunZhuangTai2 = ;
      currentDataPoint.valueAHU308_mianBanTongXunZhuangTai3 = ;
      currentDataPoint.valuePAU301_mianBanTongXunZhuangTai1 = ;
      currentDataPoint.valuePAU301_mianBanTongXunZhuangTai2 = ;
      currentDataPoint.valuePAU301_mianBanTongXunZhuangTai3 = ;
      currentDataPoint.valuePAU302_mianBanTongXunZhuangTai1 = ;
      currentDataPoint.valuePAU302_mianBanTongXunZhuangTai2 = ;
      currentDataPoint.valuePAU302_mianBanTongXunZhuangTai3 = ;
      currentDataPoint.valuePAU303_mianBanTongXunZhuangTai1 = ;
      currentDataPoint.valuePAU303_mianBanTongXunZhuangTai2 = ;
      currentDataPoint.valuePAU303_mianBanTongXunZhuangTai3 = ;
      currentDataPoint.valuePAU304_mianBanTongXunZhuangTai1 = ;
      currentDataPoint.valuePAU304_mianBanTongXunZhuangTai2 = ;
      currentDataPoint.valuePAU304_mianBanTongXunZhuangTai3 = ;
      currentDataPoint.valuePAU305_mianBanTongXunZhuangTai1 = ;
      currentDataPoint.valuePAU305_mianBanTongXunZhuangTai2 = ;
      currentDataPoint.valuePAU305_mianBanTongXunZhuangTai3 = ;
      currentDataPoint.valueAHU301_tempReal = ;
      currentDataPoint.valueAHU301_humiReal = ;
      currentDataPoint.valueAHU301_tempSet = ;
      currentDataPoint.valueAHU301_humiSet = ;
      currentDataPoint.valueAHU301_lengShuiFaKaiDu = ;
      currentDataPoint.valueAHU301_reShuiFaKaiDu = ;
      currentDataPoint.valueAHU301_xinFengWenDU = ;
      currentDataPoint.valueAHU301_jiaShiQIKaiDu = ;
      currentDataPoint.valueAHU301_beiYong = ;
      currentDataPoint.valueAHU302_tempReal = ;
      currentDataPoint.valueAHU302_humiReal = ;
      currentDataPoint.valueAHU302_tempSet = ;
      currentDataPoint.valueAHU302_humiSet = ;
      currentDataPoint.valueAHU302_lengShuiFaKaiDu = ;
      currentDataPoint.valueAHU302_reShuiFaKaiDu = ;
      currentDataPoint.valueAHU302_xinFengWenDU = ;
      currentDataPoint.valueAHU302_jiaShiQIKaiDu = ;
      currentDataPoint.valueAHU302_beiYong = ;
      currentDataPoint.valueAHU303_tempReal = ;
      currentDataPoint.valueAHU303_humiReal = ;
      currentDataPoint.valueAHU303_tempSet = ;
      currentDataPoint.valueAHU303_humiSet = ;
      currentDataPoint.valueAHU303_lengShuiFaKaiDu = ;
      currentDataPoint.valueAHU303_reShuiFaKaiDu = ;
      currentDataPoint.valueAHU303_xinFengWenDU = ;
      currentDataPoint.valueAHU303_jiaShiQIKaiDu = ;
      currentDataPoint.valueAHU303_beiYong = ;
      currentDataPoint.valueAHU304_tempReal = ;
      currentDataPoint.valueAHU304_humiReal = ;
      currentDataPoint.valueAHU304_tempSet = ;
      currentDataPoint.valueAHU304_humiSet = ;
      currentDataPoint.valueAHU304_lengShuiFaKaiDu = ;
      currentDataPoint.valueAHU304_reShuiFaKaiDu = ;
      currentDataPoint.valueAHU304_xinFengWenDU = ;
      currentDataPoint.valueAHU304_jiaShiQIKaiDu = ;
      currentDataPoint.valueAHU304_beiYong = ;
      currentDataPoint.valueAHU305_tempReal = ;
      currentDataPoint.valueAHU305_humiReal = ;
      currentDataPoint.valueAHU305_tempSet = ;
      currentDataPoint.valueAHU305_humiSet = ;
      currentDataPoint.valueAHU305_lengShuiFaKaiDu = ;
      currentDataPoint.valueAHU305_reShuiFaKaiDu = ;
      currentDataPoint.valueAHU305_xinFengWenDU = ;
      currentDataPoint.valueAHU305_jiaShiQIKaiDu = ;
      currentDataPoint.valueAHU305_beiYong = ;
      currentDataPoint.valueAHU306_tempReal = ;
      currentDataPoint.valueAHU306_humiReal = ;
      currentDataPoint.valueAHU306_tempSet = ;
      currentDataPoint.valueAHU306_humiSet = ;
      currentDataPoint.valueAHU306_lengShuiFaKaiDu = ;
      currentDataPoint.valueAHU306_reShuiFaKaiDu = ;
      currentDataPoint.valueAHU306_xinFengWenDU = ;
      currentDataPoint.valueAHU306_jiaShiQIKaiDu = ;
      currentDataPoint.valueAHU306_beiYong = ;
      currentDataPoint.valueAHU307_tempReal = ;
      currentDataPoint.valueAHU307_humiReal = ;
      currentDataPoint.valueAHU307_tempSet = ;
      currentDataPoint.valueAHU307_humiSet = ;
      currentDataPoint.valueAHU307_lengShuiFaKaiDu = ;
      currentDataPoint.valueAHU307_reShuiFaKaiDu = ;
      currentDataPoint.valueAHU307_xinFengWenDU = ;
      currentDataPoint.valueAHU307_jiaShiQIKaiDu = ;
      currentDataPoint.valueAHU307_beiYong = ;
      currentDataPoint.valueAHU308_tempReal = ;
      currentDataPoint.valueAHU308_humiReal = ;
      currentDataPoint.valueAHU308_tempSet = ;
      currentDataPoint.valueAHU308_humiSet = ;
      currentDataPoint.valueAHU308_lengShuiFaKaiDu = ;
      currentDataPoint.valueAHU308_reShuiFaKaiDu = ;
      currentDataPoint.valueAHU308_xinFengWenDU = ;
      currentDataPoint.valueAHU308_jiaShiQIKaiDu = ;
      currentDataPoint.valueAHU308_beiYong = ;
      currentDataPoint.valuePAU301_tempReal = ;
      currentDataPoint.valuePAU301_humiReal = ;
      currentDataPoint.valuePAU301_tempSet = ;
      currentDataPoint.valuePAU301_humiSet = ;
      currentDataPoint.valuePAU301_lengShuiFaKaiDu = ;
      currentDataPoint.valuePAU301_reShuiFaKaiDu = ;
      currentDataPoint.valuePAU301_xinFengWenDU = ;
      currentDataPoint.valuePAU301_jiaShiQIKaiDu = ;
      currentDataPoint.valuePAU301_beiYong = ;
      currentDataPoint.valuePAU302_tempReal = ;
      currentDataPoint.valuePAU302_humiReal = ;
      currentDataPoint.valuePAU302_tempSet = ;
      currentDataPoint.valuePAU302_humiSet = ;
      currentDataPoint.valuePAU302_lengShuiFaKaiDu = ;
      currentDataPoint.valuePAU302_reShuiFaKaiDu = ;
      currentDataPoint.valuePAU302_xinFengWenDU = ;
      currentDataPoint.valuePAU302_jiaShiQIKaiDu = ;
      currentDataPoint.valuePAU302_beiYong = ;
      currentDataPoint.valuePAU303_tempReal = ;
      currentDataPoint.valuePAU303_humiReal = ;
      currentDataPoint.valuePAU303_tempSet = ;
      currentDataPoint.valuePAU303_humiSet = ;
      currentDataPoint.valuePAU303_lengShuiFaKaiDu = ;
      currentDataPoint.valuePAU303_reShuiFaKaiDu = ;
      currentDataPoint.valuePAU303_xinFengWenDU = ;
      currentDataPoint.valuePAU303_jiaShiQIKaiDu = ;
      currentDataPoint.valuePAU303_beiYong = ;
      currentDataPoint.valuePAU304_tempReal = ;
      currentDataPoint.valuePAU304_humiReal = ;
      currentDataPoint.valuePAU304_tempSet = ;
      currentDataPoint.valuePAU304_humiSet = ;
      currentDataPoint.valuePAU304_lengShuiFaKaiDu = ;
      currentDataPoint.valuePAU304_reShuiFaKaiDu = ;
      currentDataPoint.valuePAU304_xinFengWenDU = ;
      currentDataPoint.valuePAU304_jiaShiQIKaiDu = ;
      currentDataPoint.valuePAU304_beiYong = ;
      currentDataPoint.valuePAU305_tempReal = ;
      currentDataPoint.valuePAU305_humiReal = ;
      currentDataPoint.valuePAU305_tempSet = ;
      currentDataPoint.valuePAU305_humiSet = ;
      currentDataPoint.valuePAU305_lengShuiFaKaiDu = ;
      currentDataPoint.valuePAU305_reShuiFaKaiDu = ;
      currentDataPoint.valuePAU305_xinFengWenDU = ;
      currentDataPoint.valuePAU305_jiaShiQIKaiDu = ;
      currentDataPoint.valuePAU305_beiYong = ;
    */

}


/**
* @brief Millisecond timing maintenance function, milliseconds increment, overflow to zero

* @param none
* @return none
*/
void gizTimerMs(void)
{
    timerMsCount++;
}

/**
* @brief Read millisecond count

* @param none
* @return millisecond count
*/
uint32_t gizGetTimerCount(void)
{
    return timerMsCount;
}

/**
* @brief MCU reset function

* @param none
* @return none
*/
void mcuRestart(void)
{
    __set_FAULTMASK(1);
    HAL_NVIC_SystemReset();
}


int _write(int fd, char *pBuffer, int size)
{
	HAL_UART_Transmit(&huart3, pBuffer, size, 0xff);
	return size;
}

/**
  * @brief  Period elapsed callback in non blocking mode 
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim==&htim2)
	{
	//		keyHandle((keysTypedef_t *)&keys);
			gizTimerMs();

			modbusSendCount++;
			switch (modbusSendCount)
			{
			case 1000:modbusPullFlag01 = 1;
				break;
			case 2000:modbusPullFlag02 = 1;
				break;
			case 3000:modbusPullFlag03 = 1;
				break;
			case 4000:modbusPullFlag04 = 1;
				modbusSendCount = 0;
				break;
			default:
				break;
			}

	}
}

/**
* @brief Timer TIM3 init function

* @param none
* @return none
*/
void timerInit(void)
{
	HAL_TIM_Base_Start_IT(&htim2);
}

/**
  * @brief  This function handles USART IDLE interrupt.
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef*UartHandle)  
{  
    if(UartHandle->Instance == USART2)  
    {  
				gizPutData((uint8_t *)&aRxBuffer, 1);

        HAL_UART_Receive_IT(&huart2, (uint8_t *)&aRxBuffer, 1);//开启下一次接收中断  
    }  
}  

/**
* @brief USART init function

* Serial communication between WiFi modules and device MCU
* @param none
* @return none
*/
void uartInit(void)
{

	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
	HAL_UART_Receive_IT(&huart2, (uint8_t *)&aRxBuffer, 1);//开启下一次接收中断  
}

/**
* @brief Serial port write operation, send data to WiFi module
*
* @param buf      : buf address
* @param len      : buf length
*
* @return : Return effective data length;-1，return failure
*/
int32_t uartWrite(uint8_t *buf, uint32_t len)
{
		uint8_t crc[1] = {0x55};
    uint32_t i = 0;
	
    if(NULL == buf)
    {
        return -1;
    }

    for(i=0; i<len; i++)
    {
        HAL_UART_Transmit_IT(&huart2, (uint8_t *)&buf[i], 1);
				while (huart2.gState != HAL_UART_STATE_READY);//Loop until the end of transmission

        if(i >=2 && buf[i] == 0xFF)
        {
						HAL_UART_Transmit_IT(&huart2, (uint8_t *)&crc, 1);
						while (huart2.gState != HAL_UART_STATE_READY);//Loop until the end of transmission
        }
    }

#ifdef PROTOCOL_DEBUG
    GIZWITS_LOG("MCU2WiFi[%4d:%4d]: ", gizGetTimerCount(), len);
    for(i=0; i<len; i++)
    {
        GIZWITS_LOG("%02x ", buf[i]);

        if(i >=2 && buf[i] == 0xFF)
        {
            GIZWITS_LOG("%02x ", 0x55);
        }
    }
    GIZWITS_LOG("\n");
#endif
		
		return len;
}  
