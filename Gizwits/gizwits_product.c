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

static uint32_t timerMsCount;
uint8_t aRxBuffer;

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
 /*
    currentDataPoint.valueAHU301_fengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_zhiBanStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_dianJiaRe1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_dianJiaRe2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_dianJiaRe3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_fengJiStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_shouZiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_dongXiaJi = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_zhongXiaoBaoJing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_dianJiaReGaoWen = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_fengJiQueFeng = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_paiFengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_diWenPanGuan = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_mieJunYunXing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_fengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_zhiBanStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_dianJiaRe1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_dianJiaRe2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_dianJiaRe3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_fengJiStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_shouZiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_dongXiaJi = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_zhongXiaoBaoJing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_dianJiaReGaoWen = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_fengJiQueFeng = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_paiFengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_diWenPanGuan = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_mieJunYunXing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_fengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_zhiBanStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_dianJiaRe1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_dianJiaRe2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_dianJiaRe3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_fengJiStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_shouZiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_dongXiaJi = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_zhongXiaoBaoJing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_dianJiaReGaoWen = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_fengJiQueFeng = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_paiFengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_diWenPanGuan = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_mieJunYunXing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_fengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_zhiBanStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_dianJiaRe1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_dianJiaRe2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_dianJiaRe3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_fengJiStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_shouZiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_dongXiaJi = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_zhongXiaoBaoJing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_dianJiaReGaoWen = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_fengJiQueFeng = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_paiFengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_diWenPanGuan = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_mieJunYunXing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_fengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_zhiBanStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_dianJiaRe1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_dianJiaRe2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_dianJiaRe3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_fengJiStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_shouZiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_dongXiaJi = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_zhongXiaoBaoJing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_dianJiaReGaoWen = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_fengJiQueFeng = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_paiFengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_diWenPanGuan = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_mieJunYunXing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_fengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_zhiBanStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_dianJiaRe1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_dianJiaRe2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_dianJiaRe3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_fengJiStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_shouZiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_dongXiaJi = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_zhongXiaoBaoJing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_dianJiaReGaoWen = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_fengJiQueFeng = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_paiFengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_diWenPanGuan = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_mieJunYunXing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_fengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_zhiBanStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_dianJiaRe1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_dianJiaRe2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_dianJiaRe3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_fengJiStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_shouZiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_dongXiaJi = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_zhongXiaoBaoJing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_dianJiaReGaoWen = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_fengJiQueFeng = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_paiFengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_diWenPanGuan = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_mieJunYunXing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_fengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_zhiBanStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_dianJiaRe1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_dianJiaRe2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_dianJiaRe3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_fengJiStatus = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_shouZiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_dongXiaJi = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_zhongXiaoBaoJing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_dianJiaReGaoWen = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_fengJiQueFeng = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_paiFengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_diWenPanGuan = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_mieJunYunXing = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_fengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_zhiBanStatus = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_dianJiaRe1 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_dianJiaRe2 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_dianJiaRe3 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_fengJiStatus = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_shouZiDong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_dongXiaJi = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_zhongXiaoBaoJing = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_dianJiaReGaoWen = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_fengJiQueFeng = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_paiFengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_dianYuRe1 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_dianYuRe2 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_dianYuRe3 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_dianYuReGaoWen = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_diWenPanGuan = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_mieJunYunXing = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_fengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_zhiBanStatus = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_fengJiStatus = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_shouZiDong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_dongXiaJi = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_zhongXiaoBaoJing = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_fengJiQueFeng = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_paiFengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_dianYuRe1 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_dianYuRe2 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_dianYuRe3 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_dianYuReGaoWen = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_diWenPanGuan = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_mieJunYunXing = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_fengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_zhiBanStatus = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_fengJiStatus = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_shouZiDong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_dongXiaJi = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_zhongXiaoBaoJing = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_fengJiQueFeng = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_paiFengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_dianYuRe1 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_dianYuRe2 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_dianYuRe3 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_dianYuReGaoWen = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_diWenPanGuan = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_mieJunYunXing = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_fengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_zhiBanStatus = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_fengJiStatus = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_shouZiDong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_dongXiaJi = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_zhongXiaoBaoJing = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_fengJiQueFeng = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_paiFengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_dianYuRe1 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_dianYuRe2 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_dianYuRe3 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_dianYuReGaoWen = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_diWenPanGuan = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_mieJunYunXing = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_fengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_zhiBanStatus = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_fengJiStatus = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_shouZiDong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_dongXiaJi = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_zhongXiaoBaoJing = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_fengJiQueFeng = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_paiFengJiYiQiDong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_dianYuRe1 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_dianYuRe2 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_dianYuRe3 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_dianYuReGaoWen = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_diWenPanGuan = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_mieJunYunXing = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_mianBanTongXunZhuangTai1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_mianBanTongXunZhuangTai2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_mianBanTongXunZhuangTai3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_mianBanTongXunZhuangTai1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_mianBanTongXunZhuangTai2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_mianBanTongXunZhuangTai3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_mianBanTongXunZhuangTai1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_mianBanTongXunZhuangTai2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_mianBanTongXunZhuangTai3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_mianBanTongXunZhuangTai1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_mianBanTongXunZhuangTai2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_mianBanTongXunZhuangTai3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_mianBanTongXunZhuangTai1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_mianBanTongXunZhuangTai2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_mianBanTongXunZhuangTai3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_mianBanTongXunZhuangTai1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_mianBanTongXunZhuangTai2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_mianBanTongXunZhuangTai3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_mianBanTongXunZhuangTai1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_mianBanTongXunZhuangTai2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_mianBanTongXunZhuangTai3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_mianBanTongXunZhuangTai1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_mianBanTongXunZhuangTai2 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_mianBanTongXunZhuangTai3 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_mianBanTongXunZhuangTai1 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_mianBanTongXunZhuangTai2 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_mianBanTongXunZhuangTai3 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_mianBanTongXunZhuangTai1 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_mianBanTongXunZhuangTai2 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_mianBanTongXunZhuangTai3 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_mianBanTongXunZhuangTai1 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_mianBanTongXunZhuangTai2 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_mianBanTongXunZhuangTai3 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_mianBanTongXunZhuangTai1 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_mianBanTongXunZhuangTai2 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_mianBanTongXunZhuangTai3 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_mianBanTongXunZhuangTai1 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_mianBanTongXunZhuangTai2 = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_mianBanTongXunZhuangTai3 = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_tempReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_humiReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_tempSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_humiSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_lengShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_reShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_xinFengWenDU = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_jiaShiQIKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU301_beiYong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_tempReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_humiReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_tempSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_humiSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_lengShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_reShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_xinFengWenDU = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_jiaShiQIKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU302_beiYong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_tempReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_humiReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_tempSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_humiSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_lengShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_reShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_xinFengWenDU = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_jiaShiQIKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU303_beiYong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_tempReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_humiReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_tempSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_humiSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_lengShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_reShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_xinFengWenDU = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_jiaShiQIKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU304_beiYong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_tempReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_humiReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_tempSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_humiSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_lengShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_reShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_xinFengWenDU = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_jiaShiQIKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU305_beiYong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_tempReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_humiReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_tempSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_humiSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_lengShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_reShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_xinFengWenDU = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_jiaShiQIKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU306_beiYong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_tempReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_humiReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_tempSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_humiSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_lengShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_reShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_xinFengWenDU = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_jiaShiQIKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU307_beiYong = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_tempReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_humiReal = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_tempSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_humiSet = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_lengShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_reShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_xinFengWenDU = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_jiaShiQIKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valueAHU308_beiYong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_tempReal = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_humiReal = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_tempSet = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_humiSet = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_lengShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_reShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_xinFengWenDU = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_jiaShiQIKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU301_beiYong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_tempReal = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_humiReal = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_tempSet = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_humiSet = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_lengShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_reShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_xinFengWenDU = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_jiaShiQIKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU302_beiYong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_tempReal = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_humiReal = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_tempSet = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_humiSet = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_lengShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_reShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_xinFengWenDU = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_jiaShiQIKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU303_beiYong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_tempReal = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_humiReal = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_tempSet = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_humiSet = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_lengShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_reShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_xinFengWenDU = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_jiaShiQIKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU304_beiYong = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_tempReal = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_humiReal = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_tempSet = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_humiSet = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_lengShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_reShuiFaKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_xinFengWenDU = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_jiaShiQIKaiDu = ;//Add Sensor Data Collection
    currentDataPoint.valuePAU305_beiYong = ;//Add Sensor Data Collection

    */
    
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
	HAL_UART_Transmit(&huart1, pBuffer, size, 0xff);
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
