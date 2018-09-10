/**
************************************************************
* @file         gizwits_protocol.h
* @brief        Corresponding gizwits_product.c header file (including product hardware and software version definition)
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

#ifndef _GIZWITS_PROTOCOL_H
#define _GIZWITS_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

                                                                                                                  
#define SEND_MAX_TIME       200                     ///< 200ms resend
#define SEND_MAX_NUM        2                       ///< resend times
                                                    
#define protocol_VERSION    "00000004"              ///< protocol version
#define P0_VERSION          "00000002"              ///< P0 protocol version

/**@name Product Key  
* @{
*/
#define PRODUCT_KEY "418482844f12480aa099ed32c6c37c57"
/**@} */
/**@name Product Secret  
* @{
*/
#define PRODUCT_SECRET "0ecdb14fc62d4440b10147ed2214dc4e"
               
/**@name Device status data reporting interval
* @{
*/
#define REPORT_TIME_MAX 6000 //6S
/**@} */    

#define CELLNUMMAX 7    


/**@name Whether the device is in the control class, 0 means no, 1 means yes
* @{
*/
#define DEV_IS_GATEWAY   0                    
/**@} */

/**@name Binding time
* @{
*/
#define NINABLETIME  0
/**@} */



#define MAX_PACKAGE_LEN    (sizeof(devStatus_t)+sizeof(dataPointFlags_t)+20)                 ///< Data buffer maximum length                                   ///< Data buffer maximum length

#define RB_MAX_LEN          (MAX_PACKAGE_LEN*2)     ///< Maximum length of ring buffer

/**@name Data point related definition
* @{
*/
#define AHU301_fengJiYiQiDong_LEN                           1
#define AHU301_zhiBanStatus_LEN                           1
#define AHU301_dianJiaRe1_LEN                           1
#define AHU301_dianJiaRe2_LEN                           1
#define AHU301_dianJiaRe3_LEN                           1
#define AHU301_fengJiStatus_LEN                           1
#define AHU301_shouZiDong_LEN                           1
#define AHU301_dongXiaJi_LEN                           1
#define AHU301_zhongXiaoBaoJing_LEN                           1
#define AHU301_dianJiaReGaoWen_LEN                           1
#define AHU301_fengJiQueFeng_LEN                           1
#define AHU301_paiFengJiYiQiDong_LEN                           1
#define AHU301_diWenPanGuan_LEN                           1
#define AHU301_mieJunYunXing_LEN                           1
#define AHU302_fengJiYiQiDong_LEN                           1
#define AHU302_zhiBanStatus_LEN                           1
#define AHU302_dianJiaRe1_LEN                           1
#define AHU302_dianJiaRe2_LEN                           1
#define AHU302_dianJiaRe3_LEN                           1
#define AHU302_fengJiStatus_LEN                           1
#define AHU302_shouZiDong_LEN                           1
#define AHU302_dongXiaJi_LEN                           1
#define AHU302_zhongXiaoBaoJing_LEN                           1
#define AHU302_dianJiaReGaoWen_LEN                           1
#define AHU302_fengJiQueFeng_LEN                           1
#define AHU302_paiFengJiYiQiDong_LEN                           1
#define AHU302_diWenPanGuan_LEN                           1
#define AHU302_mieJunYunXing_LEN                           1
#define AHU303_fengJiYiQiDong_LEN                           1
#define AHU303_zhiBanStatus_LEN                           1
#define AHU303_dianJiaRe1_LEN                           1
#define AHU303_dianJiaRe2_LEN                           1
#define AHU303_dianJiaRe3_LEN                           1
#define AHU303_fengJiStatus_LEN                           1
#define AHU303_shouZiDong_LEN                           1
#define AHU303_dongXiaJi_LEN                           1
#define AHU303_zhongXiaoBaoJing_LEN                           1
#define AHU303_dianJiaReGaoWen_LEN                           1
#define AHU303_fengJiQueFeng_LEN                           1
#define AHU303_paiFengJiYiQiDong_LEN                           1
#define AHU303_diWenPanGuan_LEN                           1
#define AHU303_mieJunYunXing_LEN                           1
#define AHU304_fengJiYiQiDong_LEN                           1
#define AHU304_zhiBanStatus_LEN                           1
#define AHU304_dianJiaRe1_LEN                           1
#define AHU304_dianJiaRe2_LEN                           1
#define AHU304_dianJiaRe3_LEN                           1
#define AHU304_fengJiStatus_LEN                           1
#define AHU304_shouZiDong_LEN                           1
#define AHU304_dongXiaJi_LEN                           1
#define AHU304_zhongXiaoBaoJing_LEN                           1
#define AHU304_dianJiaReGaoWen_LEN                           1
#define AHU304_fengJiQueFeng_LEN                           1
#define AHU304_paiFengJiYiQiDong_LEN                           1
#define AHU304_diWenPanGuan_LEN                           1
#define AHU304_mieJunYunXing_LEN                           1
#define AHU305_fengJiYiQiDong_LEN                           1
#define AHU305_zhiBanStatus_LEN                           1
#define AHU305_dianJiaRe1_LEN                           1
#define AHU305_dianJiaRe2_LEN                           1
#define AHU305_dianJiaRe3_LEN                           1
#define AHU305_fengJiStatus_LEN                           1
#define AHU305_shouZiDong_LEN                           1
#define AHU305_dongXiaJi_LEN                           1
#define AHU305_zhongXiaoBaoJing_LEN                           1
#define AHU305_dianJiaReGaoWen_LEN                           1
#define AHU305_fengJiQueFeng_LEN                           1
#define AHU305_paiFengJiYiQiDong_LEN                           1
#define AHU305_diWenPanGuan_LEN                           1
#define AHU305_mieJunYunXing_LEN                           1
#define AHU306_fengJiYiQiDong_LEN                           1
#define AHU306_zhiBanStatus_LEN                           1
#define AHU306_dianJiaRe1_LEN                           1
#define AHU306_dianJiaRe2_LEN                           1
#define AHU306_dianJiaRe3_LEN                           1
#define AHU306_fengJiStatus_LEN                           1
#define AHU306_shouZiDong_LEN                           1
#define AHU306_dongXiaJi_LEN                           1
#define AHU306_zhongXiaoBaoJing_LEN                           1
#define AHU306_dianJiaReGaoWen_LEN                           1
#define AHU306_fengJiQueFeng_LEN                           1
#define AHU306_paiFengJiYiQiDong_LEN                           1
#define AHU306_diWenPanGuan_LEN                           1
#define AHU306_mieJunYunXing_LEN                           1
#define AHU307_fengJiYiQiDong_LEN                           1
#define AHU307_zhiBanStatus_LEN                           1
#define AHU307_dianJiaRe1_LEN                           1
#define AHU307_dianJiaRe2_LEN                           1
#define AHU307_dianJiaRe3_LEN                           1
#define AHU307_fengJiStatus_LEN                           1
#define AHU307_shouZiDong_LEN                           1
#define AHU307_dongXiaJi_LEN                           1
#define AHU307_zhongXiaoBaoJing_LEN                           1
#define AHU307_dianJiaReGaoWen_LEN                           1
#define AHU307_fengJiQueFeng_LEN                           1
#define AHU307_paiFengJiYiQiDong_LEN                           1
#define AHU307_diWenPanGuan_LEN                           1
#define AHU307_mieJunYunXing_LEN                           1
#define AHU308_fengJiYiQiDong_LEN                           1
#define AHU308_zhiBanStatus_LEN                           1
#define AHU308_dianJiaRe1_LEN                           1
#define AHU308_dianJiaRe2_LEN                           1
#define AHU308_dianJiaRe3_LEN                           1
#define AHU308_fengJiStatus_LEN                           1
#define AHU308_shouZiDong_LEN                           1
#define AHU308_dongXiaJi_LEN                           1
#define AHU308_zhongXiaoBaoJing_LEN                           1
#define AHU308_dianJiaReGaoWen_LEN                           1
#define AHU308_fengJiQueFeng_LEN                           1
#define AHU308_paiFengJiYiQiDong_LEN                           1
#define AHU308_diWenPanGuan_LEN                           1
#define AHU308_mieJunYunXing_LEN                           1
#define PAU301_fengJiYiQiDong_LEN                           1
#define PAU301_zhiBanStatus_LEN                           1
#define PAU301_dianJiaRe1_LEN                           1
#define PAU301_dianJiaRe2_LEN                           1
#define PAU301_dianJiaRe3_LEN                           1
#define PAU301_fengJiStatus_LEN                           1
#define PAU301_shouZiDong_LEN                           1
#define PAU301_dongXiaJi_LEN                           1
#define PAU301_zhongXiaoBaoJing_LEN                           1
#define PAU301_dianJiaReGaoWen_LEN                           1
#define PAU301_fengJiQueFeng_LEN                           1
#define PAU301_paiFengJiYiQiDong_LEN                           1
#define PAU301_dianYuRe1_LEN                           1
#define PAU301_dianYuRe2_LEN                           1
#define PAU301_dianYuRe3_LEN                           1
#define PAU301_dianYuReGaoWen_LEN                           1
#define PAU301_diWenPanGuan_LEN                           1
#define PAU301_mieJunYunXing_LEN                           1
#define PAU302_fengJiYiQiDong_LEN                           1
#define PAU302_zhiBanStatus_LEN                           1
#define PAU302_fengJiStatus_LEN                           1
#define PAU302_shouZiDong_LEN                           1
#define PAU302_dongXiaJi_LEN                           1
#define PAU302_zhongXiaoBaoJing_LEN                           1
#define PAU302_fengJiQueFeng_LEN                           1
#define PAU302_paiFengJiYiQiDong_LEN                           1
#define PAU302_dianYuRe1_LEN                           1
#define PAU302_dianYuRe2_LEN                           1
#define PAU302_dianYuRe3_LEN                           1
#define PAU302_dianYuReGaoWen_LEN                           1
#define PAU302_diWenPanGuan_LEN                           1
#define PAU302_mieJunYunXing_LEN                           1
#define PAU303_fengJiYiQiDong_LEN                           1
#define PAU303_zhiBanStatus_LEN                           1
#define PAU303_fengJiStatus_LEN                           1
#define PAU303_shouZiDong_LEN                           1
#define PAU303_dongXiaJi_LEN                           1
#define PAU303_zhongXiaoBaoJing_LEN                           1
#define PAU303_fengJiQueFeng_LEN                           1
#define PAU303_paiFengJiYiQiDong_LEN                           1
#define PAU303_dianYuRe1_LEN                           1
#define PAU303_dianYuRe2_LEN                           1
#define PAU303_dianYuRe3_LEN                           1
#define PAU303_dianYuReGaoWen_LEN                           1
#define PAU303_diWenPanGuan_LEN                           1
#define PAU303_mieJunYunXing_LEN                           1
#define PAU304_fengJiYiQiDong_LEN                           1
#define PAU304_zhiBanStatus_LEN                           1
#define PAU304_fengJiStatus_LEN                           1
#define PAU304_shouZiDong_LEN                           1
#define PAU304_dongXiaJi_LEN                           1
#define PAU304_zhongXiaoBaoJing_LEN                           1
#define PAU304_fengJiQueFeng_LEN                           1
#define PAU304_paiFengJiYiQiDong_LEN                           1
#define PAU304_dianYuRe1_LEN                           1
#define PAU304_dianYuRe2_LEN                           1
#define PAU304_dianYuRe3_LEN                           1
#define PAU304_dianYuReGaoWen_LEN                           1
#define PAU304_diWenPanGuan_LEN                           1
#define PAU304_mieJunYunXing_LEN                           1
#define PAU305_fengJiYiQiDong_LEN                           1
#define PAU305_zhiBanStatus_LEN                           1
#define PAU305_fengJiStatus_LEN                           1
#define PAU305_shouZiDong_LEN                           1
#define PAU305_dongXiaJi_LEN                           1
#define PAU305_zhongXiaoBaoJing_LEN                           1
#define PAU305_fengJiQueFeng_LEN                           1
#define PAU305_paiFengJiYiQiDong_LEN                           1
#define PAU305_dianYuRe1_LEN                           1
#define PAU305_dianYuRe2_LEN                           1
#define PAU305_dianYuRe3_LEN                           1
#define PAU305_dianYuReGaoWen_LEN                           1
#define PAU305_diWenPanGuan_LEN                           1
#define PAU305_mieJunYunXing_LEN                           1
#define online_flag_LEN                           1

#define AHU301_mianBanTongXunZhuangTai1_RATIO                         1
#define AHU301_mianBanTongXunZhuangTai1_ADDITION                      0
#define AHU301_mianBanTongXunZhuangTai1_MIN                           0
#define AHU301_mianBanTongXunZhuangTai1_MAX                           255
#define AHU301_mianBanTongXunZhuangTai1_LEN                           1
#define AHU301_mianBanTongXunZhuangTai2_RATIO                         1
#define AHU301_mianBanTongXunZhuangTai2_ADDITION                      0
#define AHU301_mianBanTongXunZhuangTai2_MIN                           0
#define AHU301_mianBanTongXunZhuangTai2_MAX                           255
#define AHU301_mianBanTongXunZhuangTai2_LEN                           1
#define AHU301_mianBanTongXunZhuangTai3_RATIO                         1
#define AHU301_mianBanTongXunZhuangTai3_ADDITION                      0
#define AHU301_mianBanTongXunZhuangTai3_MIN                           0
#define AHU301_mianBanTongXunZhuangTai3_MAX                           255
#define AHU301_mianBanTongXunZhuangTai3_LEN                           1
#define AHU302_mianBanTongXunZhuangTai1_RATIO                         1
#define AHU302_mianBanTongXunZhuangTai1_ADDITION                      0
#define AHU302_mianBanTongXunZhuangTai1_MIN                           0
#define AHU302_mianBanTongXunZhuangTai1_MAX                           255
#define AHU302_mianBanTongXunZhuangTai1_LEN                           1
#define AHU302_mianBanTongXunZhuangTai2_RATIO                         1
#define AHU302_mianBanTongXunZhuangTai2_ADDITION                      0
#define AHU302_mianBanTongXunZhuangTai2_MIN                           0
#define AHU302_mianBanTongXunZhuangTai2_MAX                           255
#define AHU302_mianBanTongXunZhuangTai2_LEN                           1
#define AHU302_mianBanTongXunZhuangTai3_RATIO                         1
#define AHU302_mianBanTongXunZhuangTai3_ADDITION                      0
#define AHU302_mianBanTongXunZhuangTai3_MIN                           0
#define AHU302_mianBanTongXunZhuangTai3_MAX                           255
#define AHU302_mianBanTongXunZhuangTai3_LEN                           1
#define AHU303_mianBanTongXunZhuangTai1_RATIO                         1
#define AHU303_mianBanTongXunZhuangTai1_ADDITION                      0
#define AHU303_mianBanTongXunZhuangTai1_MIN                           0
#define AHU303_mianBanTongXunZhuangTai1_MAX                           255
#define AHU303_mianBanTongXunZhuangTai1_LEN                           1
#define AHU303_mianBanTongXunZhuangTai2_RATIO                         1
#define AHU303_mianBanTongXunZhuangTai2_ADDITION                      0
#define AHU303_mianBanTongXunZhuangTai2_MIN                           0
#define AHU303_mianBanTongXunZhuangTai2_MAX                           255
#define AHU303_mianBanTongXunZhuangTai2_LEN                           1
#define AHU303_mianBanTongXunZhuangTai3_RATIO                         1
#define AHU303_mianBanTongXunZhuangTai3_ADDITION                      0
#define AHU303_mianBanTongXunZhuangTai3_MIN                           0
#define AHU303_mianBanTongXunZhuangTai3_MAX                           255
#define AHU303_mianBanTongXunZhuangTai3_LEN                           1
#define AHU304_mianBanTongXunZhuangTai1_RATIO                         1
#define AHU304_mianBanTongXunZhuangTai1_ADDITION                      0
#define AHU304_mianBanTongXunZhuangTai1_MIN                           0
#define AHU304_mianBanTongXunZhuangTai1_MAX                           255
#define AHU304_mianBanTongXunZhuangTai1_LEN                           1
#define AHU304_mianBanTongXunZhuangTai2_RATIO                         1
#define AHU304_mianBanTongXunZhuangTai2_ADDITION                      0
#define AHU304_mianBanTongXunZhuangTai2_MIN                           0
#define AHU304_mianBanTongXunZhuangTai2_MAX                           255
#define AHU304_mianBanTongXunZhuangTai2_LEN                           1
#define AHU304_mianBanTongXunZhuangTai3_RATIO                         1
#define AHU304_mianBanTongXunZhuangTai3_ADDITION                      0
#define AHU304_mianBanTongXunZhuangTai3_MIN                           0
#define AHU304_mianBanTongXunZhuangTai3_MAX                           255
#define AHU304_mianBanTongXunZhuangTai3_LEN                           1
#define AHU305_mianBanTongXunZhuangTai1_RATIO                         1
#define AHU305_mianBanTongXunZhuangTai1_ADDITION                      0
#define AHU305_mianBanTongXunZhuangTai1_MIN                           0
#define AHU305_mianBanTongXunZhuangTai1_MAX                           255
#define AHU305_mianBanTongXunZhuangTai1_LEN                           1
#define AHU305_mianBanTongXunZhuangTai2_RATIO                         1
#define AHU305_mianBanTongXunZhuangTai2_ADDITION                      0
#define AHU305_mianBanTongXunZhuangTai2_MIN                           0
#define AHU305_mianBanTongXunZhuangTai2_MAX                           255
#define AHU305_mianBanTongXunZhuangTai2_LEN                           1
#define AHU305_mianBanTongXunZhuangTai3_RATIO                         1
#define AHU305_mianBanTongXunZhuangTai3_ADDITION                      0
#define AHU305_mianBanTongXunZhuangTai3_MIN                           0
#define AHU305_mianBanTongXunZhuangTai3_MAX                           255
#define AHU305_mianBanTongXunZhuangTai3_LEN                           1
#define AHU306_mianBanTongXunZhuangTai1_RATIO                         1
#define AHU306_mianBanTongXunZhuangTai1_ADDITION                      0
#define AHU306_mianBanTongXunZhuangTai1_MIN                           0
#define AHU306_mianBanTongXunZhuangTai1_MAX                           255
#define AHU306_mianBanTongXunZhuangTai1_LEN                           1
#define AHU306_mianBanTongXunZhuangTai2_RATIO                         1
#define AHU306_mianBanTongXunZhuangTai2_ADDITION                      0
#define AHU306_mianBanTongXunZhuangTai2_MIN                           0
#define AHU306_mianBanTongXunZhuangTai2_MAX                           255
#define AHU306_mianBanTongXunZhuangTai2_LEN                           1
#define AHU306_mianBanTongXunZhuangTai3_RATIO                         1
#define AHU306_mianBanTongXunZhuangTai3_ADDITION                      0
#define AHU306_mianBanTongXunZhuangTai3_MIN                           0
#define AHU306_mianBanTongXunZhuangTai3_MAX                           255
#define AHU306_mianBanTongXunZhuangTai3_LEN                           1
#define AHU307_mianBanTongXunZhuangTai1_RATIO                         1
#define AHU307_mianBanTongXunZhuangTai1_ADDITION                      0
#define AHU307_mianBanTongXunZhuangTai1_MIN                           0
#define AHU307_mianBanTongXunZhuangTai1_MAX                           255
#define AHU307_mianBanTongXunZhuangTai1_LEN                           1
#define AHU307_mianBanTongXunZhuangTai2_RATIO                         1
#define AHU307_mianBanTongXunZhuangTai2_ADDITION                      0
#define AHU307_mianBanTongXunZhuangTai2_MIN                           0
#define AHU307_mianBanTongXunZhuangTai2_MAX                           255
#define AHU307_mianBanTongXunZhuangTai2_LEN                           1
#define AHU307_mianBanTongXunZhuangTai3_RATIO                         1
#define AHU307_mianBanTongXunZhuangTai3_ADDITION                      0
#define AHU307_mianBanTongXunZhuangTai3_MIN                           0
#define AHU307_mianBanTongXunZhuangTai3_MAX                           255
#define AHU307_mianBanTongXunZhuangTai3_LEN                           1
#define AHU308_mianBanTongXunZhuangTai1_RATIO                         1
#define AHU308_mianBanTongXunZhuangTai1_ADDITION                      0
#define AHU308_mianBanTongXunZhuangTai1_MIN                           0
#define AHU308_mianBanTongXunZhuangTai1_MAX                           255
#define AHU308_mianBanTongXunZhuangTai1_LEN                           1
#define AHU308_mianBanTongXunZhuangTai2_RATIO                         1
#define AHU308_mianBanTongXunZhuangTai2_ADDITION                      0
#define AHU308_mianBanTongXunZhuangTai2_MIN                           0
#define AHU308_mianBanTongXunZhuangTai2_MAX                           255
#define AHU308_mianBanTongXunZhuangTai2_LEN                           1
#define AHU308_mianBanTongXunZhuangTai3_RATIO                         1
#define AHU308_mianBanTongXunZhuangTai3_ADDITION                      0
#define AHU308_mianBanTongXunZhuangTai3_MIN                           0
#define AHU308_mianBanTongXunZhuangTai3_MAX                           255
#define AHU308_mianBanTongXunZhuangTai3_LEN                           1
#define PAU301_mianBanTongXunZhuangTai1_RATIO                         1
#define PAU301_mianBanTongXunZhuangTai1_ADDITION                      0
#define PAU301_mianBanTongXunZhuangTai1_MIN                           0
#define PAU301_mianBanTongXunZhuangTai1_MAX                           255
#define PAU301_mianBanTongXunZhuangTai1_LEN                           1
#define PAU301_mianBanTongXunZhuangTai2_RATIO                         1
#define PAU301_mianBanTongXunZhuangTai2_ADDITION                      0
#define PAU301_mianBanTongXunZhuangTai2_MIN                           0
#define PAU301_mianBanTongXunZhuangTai2_MAX                           255
#define PAU301_mianBanTongXunZhuangTai2_LEN                           1
#define PAU301_mianBanTongXunZhuangTai3_RATIO                         1
#define PAU301_mianBanTongXunZhuangTai3_ADDITION                      0
#define PAU301_mianBanTongXunZhuangTai3_MIN                           0
#define PAU301_mianBanTongXunZhuangTai3_MAX                           255
#define PAU301_mianBanTongXunZhuangTai3_LEN                           1
#define PAU302_mianBanTongXunZhuangTai1_RATIO                         1
#define PAU302_mianBanTongXunZhuangTai1_ADDITION                      0
#define PAU302_mianBanTongXunZhuangTai1_MIN                           0
#define PAU302_mianBanTongXunZhuangTai1_MAX                           255
#define PAU302_mianBanTongXunZhuangTai1_LEN                           1
#define PAU302_mianBanTongXunZhuangTai2_RATIO                         1
#define PAU302_mianBanTongXunZhuangTai2_ADDITION                      0
#define PAU302_mianBanTongXunZhuangTai2_MIN                           0
#define PAU302_mianBanTongXunZhuangTai2_MAX                           255
#define PAU302_mianBanTongXunZhuangTai2_LEN                           1
#define PAU302_mianBanTongXunZhuangTai3_RATIO                         1
#define PAU302_mianBanTongXunZhuangTai3_ADDITION                      0
#define PAU302_mianBanTongXunZhuangTai3_MIN                           0
#define PAU302_mianBanTongXunZhuangTai3_MAX                           255
#define PAU302_mianBanTongXunZhuangTai3_LEN                           1
#define PAU303_mianBanTongXunZhuangTai1_RATIO                         1
#define PAU303_mianBanTongXunZhuangTai1_ADDITION                      0
#define PAU303_mianBanTongXunZhuangTai1_MIN                           0
#define PAU303_mianBanTongXunZhuangTai1_MAX                           255
#define PAU303_mianBanTongXunZhuangTai1_LEN                           1
#define PAU303_mianBanTongXunZhuangTai2_RATIO                         1
#define PAU303_mianBanTongXunZhuangTai2_ADDITION                      0
#define PAU303_mianBanTongXunZhuangTai2_MIN                           0
#define PAU303_mianBanTongXunZhuangTai2_MAX                           255
#define PAU303_mianBanTongXunZhuangTai2_LEN                           1
#define PAU303_mianBanTongXunZhuangTai3_RATIO                         1
#define PAU303_mianBanTongXunZhuangTai3_ADDITION                      0
#define PAU303_mianBanTongXunZhuangTai3_MIN                           0
#define PAU303_mianBanTongXunZhuangTai3_MAX                           255
#define PAU303_mianBanTongXunZhuangTai3_LEN                           1
#define PAU304_mianBanTongXunZhuangTai1_RATIO                         1
#define PAU304_mianBanTongXunZhuangTai1_ADDITION                      0
#define PAU304_mianBanTongXunZhuangTai1_MIN                           0
#define PAU304_mianBanTongXunZhuangTai1_MAX                           255
#define PAU304_mianBanTongXunZhuangTai1_LEN                           1
#define PAU304_mianBanTongXunZhuangTai2_RATIO                         1
#define PAU304_mianBanTongXunZhuangTai2_ADDITION                      0
#define PAU304_mianBanTongXunZhuangTai2_MIN                           0
#define PAU304_mianBanTongXunZhuangTai2_MAX                           255
#define PAU304_mianBanTongXunZhuangTai2_LEN                           1
#define PAU304_mianBanTongXunZhuangTai3_RATIO                         1
#define PAU304_mianBanTongXunZhuangTai3_ADDITION                      0
#define PAU304_mianBanTongXunZhuangTai3_MIN                           0
#define PAU304_mianBanTongXunZhuangTai3_MAX                           255
#define PAU304_mianBanTongXunZhuangTai3_LEN                           1
#define PAU305_mianBanTongXunZhuangTai1_RATIO                         1
#define PAU305_mianBanTongXunZhuangTai1_ADDITION                      0
#define PAU305_mianBanTongXunZhuangTai1_MIN                           0
#define PAU305_mianBanTongXunZhuangTai1_MAX                           255
#define PAU305_mianBanTongXunZhuangTai1_LEN                           1
#define PAU305_mianBanTongXunZhuangTai2_RATIO                         1
#define PAU305_mianBanTongXunZhuangTai2_ADDITION                      0
#define PAU305_mianBanTongXunZhuangTai2_MIN                           0
#define PAU305_mianBanTongXunZhuangTai2_MAX                           255
#define PAU305_mianBanTongXunZhuangTai2_LEN                           1
#define PAU305_mianBanTongXunZhuangTai3_RATIO                         1
#define PAU305_mianBanTongXunZhuangTai3_ADDITION                      0
#define PAU305_mianBanTongXunZhuangTai3_MIN                           0
#define PAU305_mianBanTongXunZhuangTai3_MAX                           255
#define PAU305_mianBanTongXunZhuangTai3_LEN                           1
#define AHU301_tempReal_RATIO                         1
#define AHU301_tempReal_ADDITION                      0
#define AHU301_tempReal_MIN                           0
#define AHU301_tempReal_MAX                           1000
#define AHU301_tempReal_LEN                           2
#define AHU301_humiReal_RATIO                         1
#define AHU301_humiReal_ADDITION                      0
#define AHU301_humiReal_MIN                           0
#define AHU301_humiReal_MAX                           1000
#define AHU301_humiReal_LEN                           2
#define AHU301_tempSet_RATIO                         1
#define AHU301_tempSet_ADDITION                      0
#define AHU301_tempSet_MIN                           0
#define AHU301_tempSet_MAX                           1000
#define AHU301_tempSet_LEN                           2
#define AHU301_humiSet_RATIO                         1
#define AHU301_humiSet_ADDITION                      0
#define AHU301_humiSet_MIN                           0
#define AHU301_humiSet_MAX                           1000
#define AHU301_humiSet_LEN                           2
#define AHU301_lengShuiFaKaiDu_RATIO                         1
#define AHU301_lengShuiFaKaiDu_ADDITION                      0
#define AHU301_lengShuiFaKaiDu_MIN                           0
#define AHU301_lengShuiFaKaiDu_MAX                           1000
#define AHU301_lengShuiFaKaiDu_LEN                           2
#define AHU301_reShuiFaKaiDu_RATIO                         1
#define AHU301_reShuiFaKaiDu_ADDITION                      0
#define AHU301_reShuiFaKaiDu_MIN                           0
#define AHU301_reShuiFaKaiDu_MAX                           1000
#define AHU301_reShuiFaKaiDu_LEN                           2
#define AHU301_xinFengWenDU_RATIO                         1
#define AHU301_xinFengWenDU_ADDITION                      0
#define AHU301_xinFengWenDU_MIN                           0
#define AHU301_xinFengWenDU_MAX                           1000
#define AHU301_xinFengWenDU_LEN                           2
#define AHU301_jiaShiQIKaiDu_RATIO                         1
#define AHU301_jiaShiQIKaiDu_ADDITION                      0
#define AHU301_jiaShiQIKaiDu_MIN                           0
#define AHU301_jiaShiQIKaiDu_MAX                           1000
#define AHU301_jiaShiQIKaiDu_LEN                           2
#define AHU301_beiYong_RATIO                         1
#define AHU301_beiYong_ADDITION                      0
#define AHU301_beiYong_MIN                           0
#define AHU301_beiYong_MAX                           65535
#define AHU301_beiYong_LEN                           2
#define AHU302_tempReal_RATIO                         1
#define AHU302_tempReal_ADDITION                      0
#define AHU302_tempReal_MIN                           0
#define AHU302_tempReal_MAX                           1000
#define AHU302_tempReal_LEN                           2
#define AHU302_humiReal_RATIO                         1
#define AHU302_humiReal_ADDITION                      0
#define AHU302_humiReal_MIN                           0
#define AHU302_humiReal_MAX                           1000
#define AHU302_humiReal_LEN                           2
#define AHU302_tempSet_RATIO                         1
#define AHU302_tempSet_ADDITION                      0
#define AHU302_tempSet_MIN                           0
#define AHU302_tempSet_MAX                           1000
#define AHU302_tempSet_LEN                           2
#define AHU302_humiSet_RATIO                         1
#define AHU302_humiSet_ADDITION                      0
#define AHU302_humiSet_MIN                           0
#define AHU302_humiSet_MAX                           1000
#define AHU302_humiSet_LEN                           2
#define AHU302_lengShuiFaKaiDu_RATIO                         1
#define AHU302_lengShuiFaKaiDu_ADDITION                      0
#define AHU302_lengShuiFaKaiDu_MIN                           0
#define AHU302_lengShuiFaKaiDu_MAX                           1000
#define AHU302_lengShuiFaKaiDu_LEN                           2
#define AHU302_reShuiFaKaiDu_RATIO                         1
#define AHU302_reShuiFaKaiDu_ADDITION                      0
#define AHU302_reShuiFaKaiDu_MIN                           0
#define AHU302_reShuiFaKaiDu_MAX                           1000
#define AHU302_reShuiFaKaiDu_LEN                           2
#define AHU302_xinFengWenDU_RATIO                         1
#define AHU302_xinFengWenDU_ADDITION                      0
#define AHU302_xinFengWenDU_MIN                           0
#define AHU302_xinFengWenDU_MAX                           1000
#define AHU302_xinFengWenDU_LEN                           2
#define AHU302_jiaShiQIKaiDu_RATIO                         1
#define AHU302_jiaShiQIKaiDu_ADDITION                      0
#define AHU302_jiaShiQIKaiDu_MIN                           0
#define AHU302_jiaShiQIKaiDu_MAX                           1000
#define AHU302_jiaShiQIKaiDu_LEN                           2
#define AHU302_beiYong_RATIO                         1
#define AHU302_beiYong_ADDITION                      0
#define AHU302_beiYong_MIN                           0
#define AHU302_beiYong_MAX                           65535
#define AHU302_beiYong_LEN                           2
#define AHU303_tempReal_RATIO                         1
#define AHU303_tempReal_ADDITION                      0
#define AHU303_tempReal_MIN                           0
#define AHU303_tempReal_MAX                           1000
#define AHU303_tempReal_LEN                           2
#define AHU303_humiReal_RATIO                         1
#define AHU303_humiReal_ADDITION                      0
#define AHU303_humiReal_MIN                           0
#define AHU303_humiReal_MAX                           1000
#define AHU303_humiReal_LEN                           2
#define AHU303_tempSet_RATIO                         1
#define AHU303_tempSet_ADDITION                      0
#define AHU303_tempSet_MIN                           0
#define AHU303_tempSet_MAX                           1000
#define AHU303_tempSet_LEN                           2
#define AHU303_humiSet_RATIO                         1
#define AHU303_humiSet_ADDITION                      0
#define AHU303_humiSet_MIN                           0
#define AHU303_humiSet_MAX                           1000
#define AHU303_humiSet_LEN                           2
#define AHU303_lengShuiFaKaiDu_RATIO                         1
#define AHU303_lengShuiFaKaiDu_ADDITION                      0
#define AHU303_lengShuiFaKaiDu_MIN                           0
#define AHU303_lengShuiFaKaiDu_MAX                           1000
#define AHU303_lengShuiFaKaiDu_LEN                           2
#define AHU303_reShuiFaKaiDu_RATIO                         1
#define AHU303_reShuiFaKaiDu_ADDITION                      0
#define AHU303_reShuiFaKaiDu_MIN                           0
#define AHU303_reShuiFaKaiDu_MAX                           1000
#define AHU303_reShuiFaKaiDu_LEN                           2
#define AHU303_xinFengWenDU_RATIO                         1
#define AHU303_xinFengWenDU_ADDITION                      0
#define AHU303_xinFengWenDU_MIN                           0
#define AHU303_xinFengWenDU_MAX                           1000
#define AHU303_xinFengWenDU_LEN                           2
#define AHU303_jiaShiQIKaiDu_RATIO                         1
#define AHU303_jiaShiQIKaiDu_ADDITION                      0
#define AHU303_jiaShiQIKaiDu_MIN                           0
#define AHU303_jiaShiQIKaiDu_MAX                           1000
#define AHU303_jiaShiQIKaiDu_LEN                           2
#define AHU303_beiYong_RATIO                         1
#define AHU303_beiYong_ADDITION                      0
#define AHU303_beiYong_MIN                           0
#define AHU303_beiYong_MAX                           65535
#define AHU303_beiYong_LEN                           2
#define AHU304_tempReal_RATIO                         1
#define AHU304_tempReal_ADDITION                      0
#define AHU304_tempReal_MIN                           0
#define AHU304_tempReal_MAX                           1000
#define AHU304_tempReal_LEN                           2
#define AHU304_humiReal_RATIO                         1
#define AHU304_humiReal_ADDITION                      0
#define AHU304_humiReal_MIN                           0
#define AHU304_humiReal_MAX                           1000
#define AHU304_humiReal_LEN                           2
#define AHU304_tempSet_RATIO                         1
#define AHU304_tempSet_ADDITION                      0
#define AHU304_tempSet_MIN                           0
#define AHU304_tempSet_MAX                           1000
#define AHU304_tempSet_LEN                           2
#define AHU304_humiSet_RATIO                         1
#define AHU304_humiSet_ADDITION                      0
#define AHU304_humiSet_MIN                           0
#define AHU304_humiSet_MAX                           1000
#define AHU304_humiSet_LEN                           2
#define AHU304_lengShuiFaKaiDu_RATIO                         1
#define AHU304_lengShuiFaKaiDu_ADDITION                      0
#define AHU304_lengShuiFaKaiDu_MIN                           0
#define AHU304_lengShuiFaKaiDu_MAX                           1000
#define AHU304_lengShuiFaKaiDu_LEN                           2
#define AHU304_reShuiFaKaiDu_RATIO                         1
#define AHU304_reShuiFaKaiDu_ADDITION                      0
#define AHU304_reShuiFaKaiDu_MIN                           0
#define AHU304_reShuiFaKaiDu_MAX                           1000
#define AHU304_reShuiFaKaiDu_LEN                           2
#define AHU304_xinFengWenDU_RATIO                         1
#define AHU304_xinFengWenDU_ADDITION                      0
#define AHU304_xinFengWenDU_MIN                           0
#define AHU304_xinFengWenDU_MAX                           1000
#define AHU304_xinFengWenDU_LEN                           2
#define AHU304_jiaShiQIKaiDu_RATIO                         1
#define AHU304_jiaShiQIKaiDu_ADDITION                      0
#define AHU304_jiaShiQIKaiDu_MIN                           0
#define AHU304_jiaShiQIKaiDu_MAX                           1000
#define AHU304_jiaShiQIKaiDu_LEN                           2
#define AHU304_beiYong_RATIO                         1
#define AHU304_beiYong_ADDITION                      0
#define AHU304_beiYong_MIN                           0
#define AHU304_beiYong_MAX                           65535
#define AHU304_beiYong_LEN                           2
#define AHU305_tempReal_RATIO                         1
#define AHU305_tempReal_ADDITION                      0
#define AHU305_tempReal_MIN                           0
#define AHU305_tempReal_MAX                           1000
#define AHU305_tempReal_LEN                           2
#define AHU305_humiReal_RATIO                         1
#define AHU305_humiReal_ADDITION                      0
#define AHU305_humiReal_MIN                           0
#define AHU305_humiReal_MAX                           1000
#define AHU305_humiReal_LEN                           2
#define AHU305_tempSet_RATIO                         1
#define AHU305_tempSet_ADDITION                      0
#define AHU305_tempSet_MIN                           0
#define AHU305_tempSet_MAX                           1000
#define AHU305_tempSet_LEN                           2
#define AHU305_humiSet_RATIO                         1
#define AHU305_humiSet_ADDITION                      0
#define AHU305_humiSet_MIN                           0
#define AHU305_humiSet_MAX                           1000
#define AHU305_humiSet_LEN                           2
#define AHU305_lengShuiFaKaiDu_RATIO                         1
#define AHU305_lengShuiFaKaiDu_ADDITION                      0
#define AHU305_lengShuiFaKaiDu_MIN                           0
#define AHU305_lengShuiFaKaiDu_MAX                           1000
#define AHU305_lengShuiFaKaiDu_LEN                           2
#define AHU305_reShuiFaKaiDu_RATIO                         1
#define AHU305_reShuiFaKaiDu_ADDITION                      0
#define AHU305_reShuiFaKaiDu_MIN                           0
#define AHU305_reShuiFaKaiDu_MAX                           1000
#define AHU305_reShuiFaKaiDu_LEN                           2
#define AHU305_xinFengWenDU_RATIO                         1
#define AHU305_xinFengWenDU_ADDITION                      0
#define AHU305_xinFengWenDU_MIN                           0
#define AHU305_xinFengWenDU_MAX                           1000
#define AHU305_xinFengWenDU_LEN                           2
#define AHU305_jiaShiQIKaiDu_RATIO                         1
#define AHU305_jiaShiQIKaiDu_ADDITION                      0
#define AHU305_jiaShiQIKaiDu_MIN                           0
#define AHU305_jiaShiQIKaiDu_MAX                           1000
#define AHU305_jiaShiQIKaiDu_LEN                           2
#define AHU305_beiYong_RATIO                         1
#define AHU305_beiYong_ADDITION                      0
#define AHU305_beiYong_MIN                           0
#define AHU305_beiYong_MAX                           65535
#define AHU305_beiYong_LEN                           2
#define AHU306_tempReal_RATIO                         1
#define AHU306_tempReal_ADDITION                      0
#define AHU306_tempReal_MIN                           0
#define AHU306_tempReal_MAX                           1000
#define AHU306_tempReal_LEN                           2
#define AHU306_humiReal_RATIO                         1
#define AHU306_humiReal_ADDITION                      0
#define AHU306_humiReal_MIN                           0
#define AHU306_humiReal_MAX                           1000
#define AHU306_humiReal_LEN                           2
#define AHU306_tempSet_RATIO                         1
#define AHU306_tempSet_ADDITION                      0
#define AHU306_tempSet_MIN                           0
#define AHU306_tempSet_MAX                           1000
#define AHU306_tempSet_LEN                           2
#define AHU306_humiSet_RATIO                         1
#define AHU306_humiSet_ADDITION                      0
#define AHU306_humiSet_MIN                           0
#define AHU306_humiSet_MAX                           1000
#define AHU306_humiSet_LEN                           2
#define AHU306_lengShuiFaKaiDu_RATIO                         1
#define AHU306_lengShuiFaKaiDu_ADDITION                      0
#define AHU306_lengShuiFaKaiDu_MIN                           0
#define AHU306_lengShuiFaKaiDu_MAX                           1000
#define AHU306_lengShuiFaKaiDu_LEN                           2
#define AHU306_reShuiFaKaiDu_RATIO                         1
#define AHU306_reShuiFaKaiDu_ADDITION                      0
#define AHU306_reShuiFaKaiDu_MIN                           0
#define AHU306_reShuiFaKaiDu_MAX                           1000
#define AHU306_reShuiFaKaiDu_LEN                           2
#define AHU306_xinFengWenDU_RATIO                         1
#define AHU306_xinFengWenDU_ADDITION                      0
#define AHU306_xinFengWenDU_MIN                           0
#define AHU306_xinFengWenDU_MAX                           1000
#define AHU306_xinFengWenDU_LEN                           2
#define AHU306_jiaShiQIKaiDu_RATIO                         1
#define AHU306_jiaShiQIKaiDu_ADDITION                      0
#define AHU306_jiaShiQIKaiDu_MIN                           0
#define AHU306_jiaShiQIKaiDu_MAX                           1000
#define AHU306_jiaShiQIKaiDu_LEN                           2
#define AHU306_beiYong_RATIO                         1
#define AHU306_beiYong_ADDITION                      0
#define AHU306_beiYong_MIN                           0
#define AHU306_beiYong_MAX                           65535
#define AHU306_beiYong_LEN                           2
#define AHU307_tempReal_RATIO                         1
#define AHU307_tempReal_ADDITION                      0
#define AHU307_tempReal_MIN                           0
#define AHU307_tempReal_MAX                           1000
#define AHU307_tempReal_LEN                           2
#define AHU307_humiReal_RATIO                         1
#define AHU307_humiReal_ADDITION                      0
#define AHU307_humiReal_MIN                           0
#define AHU307_humiReal_MAX                           1000
#define AHU307_humiReal_LEN                           2
#define AHU307_tempSet_RATIO                         1
#define AHU307_tempSet_ADDITION                      0
#define AHU307_tempSet_MIN                           0
#define AHU307_tempSet_MAX                           1000
#define AHU307_tempSet_LEN                           2
#define AHU307_humiSet_RATIO                         1
#define AHU307_humiSet_ADDITION                      0
#define AHU307_humiSet_MIN                           0
#define AHU307_humiSet_MAX                           1000
#define AHU307_humiSet_LEN                           2
#define AHU307_lengShuiFaKaiDu_RATIO                         1
#define AHU307_lengShuiFaKaiDu_ADDITION                      0
#define AHU307_lengShuiFaKaiDu_MIN                           0
#define AHU307_lengShuiFaKaiDu_MAX                           1000
#define AHU307_lengShuiFaKaiDu_LEN                           2
#define AHU307_reShuiFaKaiDu_RATIO                         1
#define AHU307_reShuiFaKaiDu_ADDITION                      0
#define AHU307_reShuiFaKaiDu_MIN                           0
#define AHU307_reShuiFaKaiDu_MAX                           1000
#define AHU307_reShuiFaKaiDu_LEN                           2
#define AHU307_xinFengWenDU_RATIO                         1
#define AHU307_xinFengWenDU_ADDITION                      0
#define AHU307_xinFengWenDU_MIN                           0
#define AHU307_xinFengWenDU_MAX                           1000
#define AHU307_xinFengWenDU_LEN                           2
#define AHU307_jiaShiQIKaiDu_RATIO                         1
#define AHU307_jiaShiQIKaiDu_ADDITION                      0
#define AHU307_jiaShiQIKaiDu_MIN                           0
#define AHU307_jiaShiQIKaiDu_MAX                           1000
#define AHU307_jiaShiQIKaiDu_LEN                           2
#define AHU307_beiYong_RATIO                         1
#define AHU307_beiYong_ADDITION                      0
#define AHU307_beiYong_MIN                           0
#define AHU307_beiYong_MAX                           65535
#define AHU307_beiYong_LEN                           2
#define AHU308_tempReal_RATIO                         1
#define AHU308_tempReal_ADDITION                      0
#define AHU308_tempReal_MIN                           0
#define AHU308_tempReal_MAX                           1000
#define AHU308_tempReal_LEN                           2
#define AHU308_humiReal_RATIO                         1
#define AHU308_humiReal_ADDITION                      0
#define AHU308_humiReal_MIN                           0
#define AHU308_humiReal_MAX                           1000
#define AHU308_humiReal_LEN                           2
#define AHU308_tempSet_RATIO                         1
#define AHU308_tempSet_ADDITION                      0
#define AHU308_tempSet_MIN                           0
#define AHU308_tempSet_MAX                           1000
#define AHU308_tempSet_LEN                           2
#define AHU308_humiSet_RATIO                         1
#define AHU308_humiSet_ADDITION                      0
#define AHU308_humiSet_MIN                           0
#define AHU308_humiSet_MAX                           1000
#define AHU308_humiSet_LEN                           2
#define AHU308_lengShuiFaKaiDu_RATIO                         1
#define AHU308_lengShuiFaKaiDu_ADDITION                      0
#define AHU308_lengShuiFaKaiDu_MIN                           0
#define AHU308_lengShuiFaKaiDu_MAX                           1000
#define AHU308_lengShuiFaKaiDu_LEN                           2
#define AHU308_reShuiFaKaiDu_RATIO                         1
#define AHU308_reShuiFaKaiDu_ADDITION                      0
#define AHU308_reShuiFaKaiDu_MIN                           0
#define AHU308_reShuiFaKaiDu_MAX                           1000
#define AHU308_reShuiFaKaiDu_LEN                           2
#define AHU308_xinFengWenDU_RATIO                         1
#define AHU308_xinFengWenDU_ADDITION                      0
#define AHU308_xinFengWenDU_MIN                           0
#define AHU308_xinFengWenDU_MAX                           1000
#define AHU308_xinFengWenDU_LEN                           2
#define AHU308_jiaShiQIKaiDu_RATIO                         1
#define AHU308_jiaShiQIKaiDu_ADDITION                      0
#define AHU308_jiaShiQIKaiDu_MIN                           0
#define AHU308_jiaShiQIKaiDu_MAX                           1000
#define AHU308_jiaShiQIKaiDu_LEN                           2
#define AHU308_beiYong_RATIO                         1
#define AHU308_beiYong_ADDITION                      0
#define AHU308_beiYong_MIN                           0
#define AHU308_beiYong_MAX                           65535
#define AHU308_beiYong_LEN                           2
#define PAU301_tempReal_RATIO                         1
#define PAU301_tempReal_ADDITION                      0
#define PAU301_tempReal_MIN                           0
#define PAU301_tempReal_MAX                           1000
#define PAU301_tempReal_LEN                           2
#define PAU301_humiReal_RATIO                         1
#define PAU301_humiReal_ADDITION                      0
#define PAU301_humiReal_MIN                           0
#define PAU301_humiReal_MAX                           1000
#define PAU301_humiReal_LEN                           2
#define PAU301_tempSet_RATIO                         1
#define PAU301_tempSet_ADDITION                      0
#define PAU301_tempSet_MIN                           0
#define PAU301_tempSet_MAX                           1000
#define PAU301_tempSet_LEN                           2
#define PAU301_humiSet_RATIO                         1
#define PAU301_humiSet_ADDITION                      0
#define PAU301_humiSet_MIN                           0
#define PAU301_humiSet_MAX                           1000
#define PAU301_humiSet_LEN                           2
#define PAU301_lengShuiFaKaiDu_RATIO                         1
#define PAU301_lengShuiFaKaiDu_ADDITION                      0
#define PAU301_lengShuiFaKaiDu_MIN                           0
#define PAU301_lengShuiFaKaiDu_MAX                           1000
#define PAU301_lengShuiFaKaiDu_LEN                           2
#define PAU301_reShuiFaKaiDu_RATIO                         1
#define PAU301_reShuiFaKaiDu_ADDITION                      0
#define PAU301_reShuiFaKaiDu_MIN                           0
#define PAU301_reShuiFaKaiDu_MAX                           1000
#define PAU301_reShuiFaKaiDu_LEN                           2
#define PAU301_xinFengWenDU_RATIO                         1
#define PAU301_xinFengWenDU_ADDITION                      0
#define PAU301_xinFengWenDU_MIN                           0
#define PAU301_xinFengWenDU_MAX                           1000
#define PAU301_xinFengWenDU_LEN                           2
#define PAU301_jiaShiQIKaiDu_RATIO                         1
#define PAU301_jiaShiQIKaiDu_ADDITION                      0
#define PAU301_jiaShiQIKaiDu_MIN                           0
#define PAU301_jiaShiQIKaiDu_MAX                           1000
#define PAU301_jiaShiQIKaiDu_LEN                           2
#define PAU301_beiYong_RATIO                         1
#define PAU301_beiYong_ADDITION                      0
#define PAU301_beiYong_MIN                           0
#define PAU301_beiYong_MAX                           65535
#define PAU301_beiYong_LEN                           2
#define PAU302_tempReal_RATIO                         1
#define PAU302_tempReal_ADDITION                      0
#define PAU302_tempReal_MIN                           0
#define PAU302_tempReal_MAX                           1000
#define PAU302_tempReal_LEN                           2
#define PAU302_humiReal_RATIO                         1
#define PAU302_humiReal_ADDITION                      0
#define PAU302_humiReal_MIN                           0
#define PAU302_humiReal_MAX                           1000
#define PAU302_humiReal_LEN                           2
#define PAU302_tempSet_RATIO                         1
#define PAU302_tempSet_ADDITION                      0
#define PAU302_tempSet_MIN                           0
#define PAU302_tempSet_MAX                           1000
#define PAU302_tempSet_LEN                           2
#define PAU302_humiSet_RATIO                         1
#define PAU302_humiSet_ADDITION                      0
#define PAU302_humiSet_MIN                           0
#define PAU302_humiSet_MAX                           1000
#define PAU302_humiSet_LEN                           2
#define PAU302_lengShuiFaKaiDu_RATIO                         1
#define PAU302_lengShuiFaKaiDu_ADDITION                      0
#define PAU302_lengShuiFaKaiDu_MIN                           0
#define PAU302_lengShuiFaKaiDu_MAX                           1000
#define PAU302_lengShuiFaKaiDu_LEN                           2
#define PAU302_reShuiFaKaiDu_RATIO                         1
#define PAU302_reShuiFaKaiDu_ADDITION                      0
#define PAU302_reShuiFaKaiDu_MIN                           0
#define PAU302_reShuiFaKaiDu_MAX                           1000
#define PAU302_reShuiFaKaiDu_LEN                           2
#define PAU302_xinFengWenDU_RATIO                         1
#define PAU302_xinFengWenDU_ADDITION                      0
#define PAU302_xinFengWenDU_MIN                           0
#define PAU302_xinFengWenDU_MAX                           1000
#define PAU302_xinFengWenDU_LEN                           2
#define PAU302_jiaShiQIKaiDu_RATIO                         1
#define PAU302_jiaShiQIKaiDu_ADDITION                      0
#define PAU302_jiaShiQIKaiDu_MIN                           0
#define PAU302_jiaShiQIKaiDu_MAX                           1000
#define PAU302_jiaShiQIKaiDu_LEN                           2
#define PAU302_beiYong_RATIO                         1
#define PAU302_beiYong_ADDITION                      0
#define PAU302_beiYong_MIN                           0
#define PAU302_beiYong_MAX                           65535
#define PAU302_beiYong_LEN                           2
#define PAU303_tempReal_RATIO                         1
#define PAU303_tempReal_ADDITION                      0
#define PAU303_tempReal_MIN                           0
#define PAU303_tempReal_MAX                           1000
#define PAU303_tempReal_LEN                           2
#define PAU303_humiReal_RATIO                         1
#define PAU303_humiReal_ADDITION                      0
#define PAU303_humiReal_MIN                           0
#define PAU303_humiReal_MAX                           1000
#define PAU303_humiReal_LEN                           2
#define PAU303_tempSet_RATIO                         1
#define PAU303_tempSet_ADDITION                      0
#define PAU303_tempSet_MIN                           0
#define PAU303_tempSet_MAX                           1000
#define PAU303_tempSet_LEN                           2
#define PAU303_humiSet_RATIO                         1
#define PAU303_humiSet_ADDITION                      0
#define PAU303_humiSet_MIN                           0
#define PAU303_humiSet_MAX                           1000
#define PAU303_humiSet_LEN                           2
#define PAU303_lengShuiFaKaiDu_RATIO                         1
#define PAU303_lengShuiFaKaiDu_ADDITION                      0
#define PAU303_lengShuiFaKaiDu_MIN                           0
#define PAU303_lengShuiFaKaiDu_MAX                           1000
#define PAU303_lengShuiFaKaiDu_LEN                           2
#define PAU303_reShuiFaKaiDu_RATIO                         1
#define PAU303_reShuiFaKaiDu_ADDITION                      0
#define PAU303_reShuiFaKaiDu_MIN                           0
#define PAU303_reShuiFaKaiDu_MAX                           1000
#define PAU303_reShuiFaKaiDu_LEN                           2
#define PAU303_xinFengWenDU_RATIO                         1
#define PAU303_xinFengWenDU_ADDITION                      0
#define PAU303_xinFengWenDU_MIN                           0
#define PAU303_xinFengWenDU_MAX                           1000
#define PAU303_xinFengWenDU_LEN                           2
#define PAU303_jiaShiQIKaiDu_RATIO                         1
#define PAU303_jiaShiQIKaiDu_ADDITION                      0
#define PAU303_jiaShiQIKaiDu_MIN                           0
#define PAU303_jiaShiQIKaiDu_MAX                           1000
#define PAU303_jiaShiQIKaiDu_LEN                           2
#define PAU303_beiYong_RATIO                         1
#define PAU303_beiYong_ADDITION                      0
#define PAU303_beiYong_MIN                           0
#define PAU303_beiYong_MAX                           65535
#define PAU303_beiYong_LEN                           2
#define PAU304_tempReal_RATIO                         1
#define PAU304_tempReal_ADDITION                      0
#define PAU304_tempReal_MIN                           0
#define PAU304_tempReal_MAX                           1000
#define PAU304_tempReal_LEN                           2
#define PAU304_humiReal_RATIO                         1
#define PAU304_humiReal_ADDITION                      0
#define PAU304_humiReal_MIN                           0
#define PAU304_humiReal_MAX                           1000
#define PAU304_humiReal_LEN                           2
#define PAU304_tempSet_RATIO                         1
#define PAU304_tempSet_ADDITION                      0
#define PAU304_tempSet_MIN                           0
#define PAU304_tempSet_MAX                           1000
#define PAU304_tempSet_LEN                           2
#define PAU304_humiSet_RATIO                         1
#define PAU304_humiSet_ADDITION                      0
#define PAU304_humiSet_MIN                           0
#define PAU304_humiSet_MAX                           1000
#define PAU304_humiSet_LEN                           2
#define PAU304_lengShuiFaKaiDu_RATIO                         1
#define PAU304_lengShuiFaKaiDu_ADDITION                      0
#define PAU304_lengShuiFaKaiDu_MIN                           0
#define PAU304_lengShuiFaKaiDu_MAX                           1000
#define PAU304_lengShuiFaKaiDu_LEN                           2
#define PAU304_reShuiFaKaiDu_RATIO                         1
#define PAU304_reShuiFaKaiDu_ADDITION                      0
#define PAU304_reShuiFaKaiDu_MIN                           0
#define PAU304_reShuiFaKaiDu_MAX                           1000
#define PAU304_reShuiFaKaiDu_LEN                           2
#define PAU304_xinFengWenDU_RATIO                         1
#define PAU304_xinFengWenDU_ADDITION                      0
#define PAU304_xinFengWenDU_MIN                           0
#define PAU304_xinFengWenDU_MAX                           1000
#define PAU304_xinFengWenDU_LEN                           2
#define PAU304_jiaShiQIKaiDu_RATIO                         1
#define PAU304_jiaShiQIKaiDu_ADDITION                      0
#define PAU304_jiaShiQIKaiDu_MIN                           0
#define PAU304_jiaShiQIKaiDu_MAX                           1000
#define PAU304_jiaShiQIKaiDu_LEN                           2
#define PAU304_beiYong_RATIO                         1
#define PAU304_beiYong_ADDITION                      0
#define PAU304_beiYong_MIN                           0
#define PAU304_beiYong_MAX                           65535
#define PAU304_beiYong_LEN                           2
#define PAU305_tempReal_RATIO                         1
#define PAU305_tempReal_ADDITION                      0
#define PAU305_tempReal_MIN                           0
#define PAU305_tempReal_MAX                           1000
#define PAU305_tempReal_LEN                           2
#define PAU305_humiReal_RATIO                         1
#define PAU305_humiReal_ADDITION                      0
#define PAU305_humiReal_MIN                           0
#define PAU305_humiReal_MAX                           1000
#define PAU305_humiReal_LEN                           2
#define PAU305_tempSet_RATIO                         1
#define PAU305_tempSet_ADDITION                      0
#define PAU305_tempSet_MIN                           0
#define PAU305_tempSet_MAX                           1000
#define PAU305_tempSet_LEN                           2
#define PAU305_humiSet_RATIO                         1
#define PAU305_humiSet_ADDITION                      0
#define PAU305_humiSet_MIN                           0
#define PAU305_humiSet_MAX                           1000
#define PAU305_humiSet_LEN                           2
#define PAU305_lengShuiFaKaiDu_RATIO                         1
#define PAU305_lengShuiFaKaiDu_ADDITION                      0
#define PAU305_lengShuiFaKaiDu_MIN                           0
#define PAU305_lengShuiFaKaiDu_MAX                           1000
#define PAU305_lengShuiFaKaiDu_LEN                           2
#define PAU305_reShuiFaKaiDu_RATIO                         1
#define PAU305_reShuiFaKaiDu_ADDITION                      0
#define PAU305_reShuiFaKaiDu_MIN                           0
#define PAU305_reShuiFaKaiDu_MAX                           1000
#define PAU305_reShuiFaKaiDu_LEN                           2
#define PAU305_xinFengWenDU_RATIO                         1
#define PAU305_xinFengWenDU_ADDITION                      0
#define PAU305_xinFengWenDU_MIN                           0
#define PAU305_xinFengWenDU_MAX                           1000
#define PAU305_xinFengWenDU_LEN                           2
#define PAU305_jiaShiQIKaiDu_RATIO                         1
#define PAU305_jiaShiQIKaiDu_ADDITION                      0
#define PAU305_jiaShiQIKaiDu_MIN                           0
#define PAU305_jiaShiQIKaiDu_MAX                           1000
#define PAU305_jiaShiQIKaiDu_LEN                           2
#define PAU305_beiYong_RATIO                         1
#define PAU305_beiYong_ADDITION                      0
#define PAU305_beiYong_MIN                           0
#define PAU305_beiYong_MAX                           65535
#define PAU305_beiYong_LEN                           2

/**@} */


/** Writable data points Boolean and enumerated variables occupy byte size */
#define COUNT_BIT 24


/** Writable data points Boolean and enumerated variables occupy byte size */
#define COUNT_W_BIT 1

/** Read-only data points Boolean and enumerated variables occupy byte size */
#define COUNT_R_BIT 24

#define DATAPOINT_FLAG_LEN sizeof(dataPointFlags_t)		 ///< All data points FLAG occupies the maximum number of bytes


/** Event enumeration */
typedef enum
{
  WIFI_SOFTAP = 0x00,                               ///< WiFi SOFTAP configuration event
  WIFI_AIRLINK,                                     ///< WiFi module AIRLINK configuration event
  WIFI_STATION,                                     ///< WiFi module STATION configuration event
  WIFI_OPEN_BINDING,                                ///< The WiFi module opens the binding event
  WIFI_CLOSE_BINDING,                               ///< The WiFi module closes the binding event
  WIFI_CON_ROUTER,                                  ///< The WiFi module is connected to a routing event
  WIFI_DISCON_ROUTER,                               ///< The WiFi module has been disconnected from the routing event
  WIFI_CON_M2M,                                     ///< The WiFi module has a server M2M event
  WIFI_DISCON_M2M,                                  ///< The WiFi module has been disconnected from the server M2M event
  WIFI_OPEN_TESTMODE,                               ///< The WiFi module turns on the test mode event
  WIFI_CLOSE_TESTMODE,                              ///< The WiFi module turns off the test mode event
  WIFI_CON_APP,                                     ///< The WiFi module connects to the APP event
  WIFI_DISCON_APP,                                  ///< The WiFi module disconnects the APP event
  WIFI_RSSI,                                        ///< WiFi module RSSI event
  WIFI_NTP,                                         ///< Network time event
  MODULE_INFO,                                      ///< Module information event
  TRANSPARENT_DATA,                                 ///< Transparency events
  EVENT_online_flag,
  EVENT_TYPE_MAX                                    ///< Enumerate the number of members to calculate (user accidentally deleted)
} EVENT_TYPE_T;


/** P0 command command code*/
typedef enum
{
    ACTION_CONTROL_DEVICE       = 0x11,             ///< Protocol 4.10 WiFi Module Control Device WiFi Module Send
    ACTION_READ_DEV_STATUS      = 0x12,             ///< Protocol 4.8 WiFi Module Reads the current status of the device WiFi module sent
    ACTION_READ_DEV_STATUS_ACK  = 0x13,             ///< Protocol 4.8 WiFi Module Read Device Current Status Device MCU Reply
    ACTION_REPORT_DEV_STATUS    = 0X14,             ///< Protocol 4.9 device MCU to the WiFi module to actively report the current status of the device to send the MCU
    ACTION_W2D_TRANSPARENT_DATA = 0x05,             ///< WiFi to device MCU transparent
    ACTION_D2W_TRANSPARENT_DATA = 0x06,             ///< Device MCU to WiFi
} actionType_t;

/** Protocol network time structure */
typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint32_t ntp;
}protocolTime_t;

     

/** WiFi Module configuration parameters*/
typedef enum
{
  WIFI_RESET_MODE = 0x00,                           ///< WIFI module reset
  WIFI_SOFTAP_MODE,                                 ///< WIFI module softAP modeF
  WIFI_AIRLINK_MODE,                                ///< WIFI module AirLink mode
  WIFI_PRODUCTION_TEST,                             ///< MCU request WiFi module into production test mode
  WIFI_NINABLE_MODE,                                ///< MCU request module to enter binding mode
  WIFI_REBOOT_MODE,                                 ///< MCU request module reboot  
}WIFI_MODE_TYPE_T;                                

/** The protocol event type*/
typedef enum
{
  STATELESS_TYPE = 0x00,                            ///< Stateless type
  ACTION_CONTROL_TYPE,                              ///< Protocol 4.10 :WiFi module control device event
  WIFI_STATUS_TYPE,                                 ///< Protocol 4.5 :WiFi module inform the device MCU of the change event of the WiFi module status
  ACTION_W2D_TRANSPARENT_TYPE,                      ///< Protocol WiFi to device MCU transparent event
  GET_NTP_TYPE,                                     ///< Protocol 4.13 :The MCU requests access to the network time event
  GET_MODULEINFO_TYPE,                              ///< Protocol 4.9 :The MCU get module information event
  PROTOCOL_EVENT_TYPE_MAX                           ///< Count enumerated member (User donot delete)
} PROTOCOL_EVENT_TYPE_T;
     
/** Protocol command code */                                   
typedef enum                               
{                                                   
    CMD_GET_DEVICE_INTO             = 0x01,         ///< Protocol：3.1 
    ACK_GET_DEVICE_INFO             = 0x02,         ///< Protocol：3.1 
            
    CMD_ISSUED_P0                   = 0x03,         ///< Protocol：3.2 3.3 
    ACK_ISSUED_P0                   = 0x04,         ///< Protocol：3.2 3.3 
            
    CMD_REPORT_P0                   = 0x05,         ///< Protocol：3.4 
    ACK_REPORT_P0                   = 0x06,         ///< Protocol：3.4 
            
    CMD_HEARTBEAT                   = 0x07,         ///< Protocol：3.5 
    ACK_HEARTBEAT                   = 0x08,         ///< Protocol：3.5 
            
    CMD_WIFI_CONFIG                 = 0x09,         ///< Protocol：3.6 
    ACK_WIFI_CONFIG                 = 0x0A,         ///< Protocol：3.6 
            
    CMD_SET_DEFAULT                 = 0x0B,         ///< Protocol：3.7 
    ACK_SET_DEFAULT                 = 0x0C,         ///< Protocol：3.7 
            
    CMD_WIFISTATUS                  = 0x0D,         ///< Protocol：3.8 
    ACK_WIFISTATUS                  = 0x0E,         ///< Protocol：3.8 
        
    CMD_MCU_REBOOT                  = 0x0F,         ///< Protocol：4.1 
    ACK_MCU_REBOOT                  = 0x10,         ///< Protocol：4.1 
            
    CMD_ERROR_PACKAGE               = 0x11,         ///< Protocol：3.9 
    ACK_ERROR_PACKAGE               = 0x12,         ///< Protocol：3.9 

    CMD_PRODUCTION_TEST             = 0x13,         ///< Protocol：
    ACK_PRODUCTION_TEST             = 0x14,         ///< Protocol：

    CMD_NINABLE_MODE                = 0x15,         ///< Protocol：3.10
    ACK_NINABLE_MODE                = 0x16,         ///< Protocol：3.10

    CMD_GET_NTP                     = 0x17,         ///< Protocol：4.3 
    ACK_GET_NTP                     = 0x18,         ///< Protocol：4.3 


    CMD_ASK_BIGDATA                 = 0x19,         ///< Protocol：4.4
    ACK_ASK_BIGDATA                 = 0x1A,         ///< Protocol：4.4

    CMD_BIGDATA_READY               = 0x1B,         ///< Protocol：4.5
    ACK_BIGDATA_READY               = 0x1C,         ///< Protocol：4.5

    CMD_BIGDATA_SEND                = 0x1D,         ///< Protocol：4.6
    ACK_BIGDATA_SEND                = 0x1E,         ///< Protocol：4.6

    CMD_S_STOP_BIGDATA_SEND         = 0x1F,         ///< Protocol：4.7
    ACK_S_STOP_BIGDATA_SEND         = 0x20,         ///< Protocol：4.7

    CMD_D_STOP_BIGDATA_SEND         = 0x27,         ///< Protocol：4.8
    ACK_D_STOP_BIGDATA_SEND         = 0x28,         ///< Protocol：4.8

    CMD_ASK_MODULE_INFO             = 0x21,         ///< Protocol：4.9
    ACK_ASK_MODULE_INFO             = 0x22,         ///< Protocol：4.9

    CMD_ASK_AFFAIR_HANDLE           = 0x23,         ///< Protocol：4.10
    ACK_ASK_AFFAIR_HANDLE           = 0x24,         ///< Protocol：4.10

    CMD_AFFAIR_RESULT               = 0x25,         ///< Protocol：4.10
    ACK_AFFAIR_RESULT               = 0x26,         ///< Protocol：4.10

    CMD_REBOOT_MODULE               = 0x29,         ///< Protocol：3.11
    ACK_REBOOT_MODULE               = 0x2A,         ///< Protocol：3.11

    CMD_CONNECT_M2M                 = 0x2D,         ///< Protocol：for Virtualization
    ACK_CONNECT_M2M                 = 0x2E,         ///< Protocol：for Virtualization

    CMD_CONNECT_M2M_BACK            = 0x2F,         ///< Protocol：for Virtualization
    ACK_CONNECT_M2M_BACK            = 0x30,         ///< Protocol：for Virtualization

    CMD_UPLOAD_DATA                 = 0x31,         ///< Protocol：for Virtualization
    ACK_UPLOAD_DATA                 = 0x32,         ///< Protocol：for Virtualization

    CMD_UPLOAD_DATA_BACK            = 0x33,         ///< Protocol：for Virtualization
    ACK_UPLOAD_DATA_BACK            = 0x34,         ///< Protocol：for Virtualization

    CMD_DISCONNECT_M2M              = 0x35,         ///< Protocol：for Virtualization
    ACK_DISCONNECT_M2M              = 0x36,         ///< Protocol：for Virtualization

    CMD_DISCONNECT_M2M_BACK         = 0x37,         ///< Protocol：for Virtualization
    ACK_DISCONNECT_M2M_BACK         = 0x38,         ///< Protocol：for Virtualization

    CMD_RESET_SIMULATOR             = 0x39,         ///< Protocol：for Virtualization
    ACK_RESET_SIMULATOR             = 0x3A,         ///< Protocol：for Virtualization

    CMD_RESET_SIMULATOR_BACK        = 0x3B,         ///< Protocol：for Virtualization
    ACK_RESET_SIMULATOR_BACK        = 0x3C,         ///< Protocol：for Virtualization
} PROTOCOL_CMDTYPE;                                                                                  
                        
/** Illegal message type*/  
typedef enum
{
    ERROR_ACK_SUM = 0x01,                           ///< check error
    ERROR_CMD     = 0x02,                           ///< Command code error
    ERROR_OTHER   = 0x03,                           ///< other
} errorPacketsType_t;

typedef enum
{
    EXE_SUCESS                      = 0x00,
    EXE_FAILE                       = 0x01,
} execute_result;  

#pragma pack(1)

/** User Area Device State Structure */
typedef struct {
  bool valueAHU301_fengJiYiQiDong;
  bool valueAHU301_zhiBanStatus;
  bool valueAHU301_dianJiaRe1;
  bool valueAHU301_dianJiaRe2;
  bool valueAHU301_dianJiaRe3;
  bool valueAHU301_fengJiStatus;
  bool valueAHU301_shouZiDong;
  bool valueAHU301_dongXiaJi;
  bool valueAHU301_zhongXiaoBaoJing;
  bool valueAHU301_dianJiaReGaoWen;
  bool valueAHU301_fengJiQueFeng;
  bool valueAHU301_paiFengJiYiQiDong;
  bool valueAHU301_diWenPanGuan;
  bool valueAHU301_mieJunYunXing;
  bool valueAHU302_fengJiYiQiDong;
  bool valueAHU302_zhiBanStatus;
  bool valueAHU302_dianJiaRe1;
  bool valueAHU302_dianJiaRe2;
  bool valueAHU302_dianJiaRe3;
  bool valueAHU302_fengJiStatus;
  bool valueAHU302_shouZiDong;
  bool valueAHU302_dongXiaJi;
  bool valueAHU302_zhongXiaoBaoJing;
  bool valueAHU302_dianJiaReGaoWen;
  bool valueAHU302_fengJiQueFeng;
  bool valueAHU302_paiFengJiYiQiDong;
  bool valueAHU302_diWenPanGuan;
  bool valueAHU302_mieJunYunXing;
  bool valueAHU303_fengJiYiQiDong;
  bool valueAHU303_zhiBanStatus;
  bool valueAHU303_dianJiaRe1;
  bool valueAHU303_dianJiaRe2;
  bool valueAHU303_dianJiaRe3;
  bool valueAHU303_fengJiStatus;
  bool valueAHU303_shouZiDong;
  bool valueAHU303_dongXiaJi;
  bool valueAHU303_zhongXiaoBaoJing;
  bool valueAHU303_dianJiaReGaoWen;
  bool valueAHU303_fengJiQueFeng;
  bool valueAHU303_paiFengJiYiQiDong;
  bool valueAHU303_diWenPanGuan;
  bool valueAHU303_mieJunYunXing;
  bool valueAHU304_fengJiYiQiDong;
  bool valueAHU304_zhiBanStatus;
  bool valueAHU304_dianJiaRe1;
  bool valueAHU304_dianJiaRe2;
  bool valueAHU304_dianJiaRe3;
  bool valueAHU304_fengJiStatus;
  bool valueAHU304_shouZiDong;
  bool valueAHU304_dongXiaJi;
  bool valueAHU304_zhongXiaoBaoJing;
  bool valueAHU304_dianJiaReGaoWen;
  bool valueAHU304_fengJiQueFeng;
  bool valueAHU304_paiFengJiYiQiDong;
  bool valueAHU304_diWenPanGuan;
  bool valueAHU304_mieJunYunXing;
  bool valueAHU305_fengJiYiQiDong;
  bool valueAHU305_zhiBanStatus;
  bool valueAHU305_dianJiaRe1;
  bool valueAHU305_dianJiaRe2;
  bool valueAHU305_dianJiaRe3;
  bool valueAHU305_fengJiStatus;
  bool valueAHU305_shouZiDong;
  bool valueAHU305_dongXiaJi;
  bool valueAHU305_zhongXiaoBaoJing;
  bool valueAHU305_dianJiaReGaoWen;
  bool valueAHU305_fengJiQueFeng;
  bool valueAHU305_paiFengJiYiQiDong;
  bool valueAHU305_diWenPanGuan;
  bool valueAHU305_mieJunYunXing;
  bool valueAHU306_fengJiYiQiDong;
  bool valueAHU306_zhiBanStatus;
  bool valueAHU306_dianJiaRe1;
  bool valueAHU306_dianJiaRe2;
  bool valueAHU306_dianJiaRe3;
  bool valueAHU306_fengJiStatus;
  bool valueAHU306_shouZiDong;
  bool valueAHU306_dongXiaJi;
  bool valueAHU306_zhongXiaoBaoJing;
  bool valueAHU306_dianJiaReGaoWen;
  bool valueAHU306_fengJiQueFeng;
  bool valueAHU306_paiFengJiYiQiDong;
  bool valueAHU306_diWenPanGuan;
  bool valueAHU306_mieJunYunXing;
  bool valueAHU307_fengJiYiQiDong;
  bool valueAHU307_zhiBanStatus;
  bool valueAHU307_dianJiaRe1;
  bool valueAHU307_dianJiaRe2;
  bool valueAHU307_dianJiaRe3;
  bool valueAHU307_fengJiStatus;
  bool valueAHU307_shouZiDong;
  bool valueAHU307_dongXiaJi;
  bool valueAHU307_zhongXiaoBaoJing;
  bool valueAHU307_dianJiaReGaoWen;
  bool valueAHU307_fengJiQueFeng;
  bool valueAHU307_paiFengJiYiQiDong;
  bool valueAHU307_diWenPanGuan;
  bool valueAHU307_mieJunYunXing;
  bool valueAHU308_fengJiYiQiDong;
  bool valueAHU308_zhiBanStatus;
  bool valueAHU308_dianJiaRe1;
  bool valueAHU308_dianJiaRe2;
  bool valueAHU308_dianJiaRe3;
  bool valueAHU308_fengJiStatus;
  bool valueAHU308_shouZiDong;
  bool valueAHU308_dongXiaJi;
  bool valueAHU308_zhongXiaoBaoJing;
  bool valueAHU308_dianJiaReGaoWen;
  bool valueAHU308_fengJiQueFeng;
  bool valueAHU308_paiFengJiYiQiDong;
  bool valueAHU308_diWenPanGuan;
  bool valueAHU308_mieJunYunXing;
  bool valuePAU301_fengJiYiQiDong;
  bool valuePAU301_zhiBanStatus;
  bool valuePAU301_dianJiaRe1;
  bool valuePAU301_dianJiaRe2;
  bool valuePAU301_dianJiaRe3;
  bool valuePAU301_fengJiStatus;
  bool valuePAU301_shouZiDong;
  bool valuePAU301_dongXiaJi;
  bool valuePAU301_zhongXiaoBaoJing;
  bool valuePAU301_dianJiaReGaoWen;
  bool valuePAU301_fengJiQueFeng;
  bool valuePAU301_paiFengJiYiQiDong;
  bool valuePAU301_dianYuRe1;
  bool valuePAU301_dianYuRe2;
  bool valuePAU301_dianYuRe3;
  bool valuePAU301_dianYuReGaoWen;
  bool valuePAU301_diWenPanGuan;
  bool valuePAU301_mieJunYunXing;
  bool valuePAU302_fengJiYiQiDong;
  bool valuePAU302_zhiBanStatus;
  bool valuePAU302_fengJiStatus;
  bool valuePAU302_shouZiDong;
  bool valuePAU302_dongXiaJi;
  bool valuePAU302_zhongXiaoBaoJing;
  bool valuePAU302_fengJiQueFeng;
  bool valuePAU302_paiFengJiYiQiDong;
  bool valuePAU302_dianYuRe1;
  bool valuePAU302_dianYuRe2;
  bool valuePAU302_dianYuRe3;
  bool valuePAU302_dianYuReGaoWen;
  bool valuePAU302_diWenPanGuan;
  bool valuePAU302_mieJunYunXing;
  bool valuePAU303_fengJiYiQiDong;
  bool valuePAU303_zhiBanStatus;
  bool valuePAU303_fengJiStatus;
  bool valuePAU303_shouZiDong;
  bool valuePAU303_dongXiaJi;
  bool valuePAU303_zhongXiaoBaoJing;
  bool valuePAU303_fengJiQueFeng;
  bool valuePAU303_paiFengJiYiQiDong;
  bool valuePAU303_dianYuRe1;
  bool valuePAU303_dianYuRe2;
  bool valuePAU303_dianYuRe3;
  bool valuePAU303_dianYuReGaoWen;
  bool valuePAU303_diWenPanGuan;
  bool valuePAU303_mieJunYunXing;
  bool valuePAU304_fengJiYiQiDong;
  bool valuePAU304_zhiBanStatus;
  bool valuePAU304_fengJiStatus;
  bool valuePAU304_shouZiDong;
  bool valuePAU304_dongXiaJi;
  bool valuePAU304_zhongXiaoBaoJing;
  bool valuePAU304_fengJiQueFeng;
  bool valuePAU304_paiFengJiYiQiDong;
  bool valuePAU304_dianYuRe1;
  bool valuePAU304_dianYuRe2;
  bool valuePAU304_dianYuRe3;
  bool valuePAU304_dianYuReGaoWen;
  bool valuePAU304_diWenPanGuan;
  bool valuePAU304_mieJunYunXing;
  bool valuePAU305_fengJiYiQiDong;
  bool valuePAU305_zhiBanStatus;
  bool valuePAU305_fengJiStatus;
  bool valuePAU305_shouZiDong;
  bool valuePAU305_dongXiaJi;
  bool valuePAU305_zhongXiaoBaoJing;
  bool valuePAU305_fengJiQueFeng;
  bool valuePAU305_paiFengJiYiQiDong;
  bool valuePAU305_dianYuRe1;
  bool valuePAU305_dianYuRe2;
  bool valuePAU305_dianYuRe3;
  bool valuePAU305_dianYuReGaoWen;
  bool valuePAU305_diWenPanGuan;
  bool valuePAU305_mieJunYunXing;
  bool valueonline_flag;
  uint32_t valueAHU301_mianBanTongXunZhuangTai1;
  uint32_t valueAHU301_mianBanTongXunZhuangTai2;
  uint32_t valueAHU301_mianBanTongXunZhuangTai3;
  uint32_t valueAHU302_mianBanTongXunZhuangTai1;
  uint32_t valueAHU302_mianBanTongXunZhuangTai2;
  uint32_t valueAHU302_mianBanTongXunZhuangTai3;
  uint32_t valueAHU303_mianBanTongXunZhuangTai1;
  uint32_t valueAHU303_mianBanTongXunZhuangTai2;
  uint32_t valueAHU303_mianBanTongXunZhuangTai3;
  uint32_t valueAHU304_mianBanTongXunZhuangTai1;
  uint32_t valueAHU304_mianBanTongXunZhuangTai2;
  uint32_t valueAHU304_mianBanTongXunZhuangTai3;
  uint32_t valueAHU305_mianBanTongXunZhuangTai1;
  uint32_t valueAHU305_mianBanTongXunZhuangTai2;
  uint32_t valueAHU305_mianBanTongXunZhuangTai3;
  uint32_t valueAHU306_mianBanTongXunZhuangTai1;
  uint32_t valueAHU306_mianBanTongXunZhuangTai2;
  uint32_t valueAHU306_mianBanTongXunZhuangTai3;
  uint32_t valueAHU307_mianBanTongXunZhuangTai1;
  uint32_t valueAHU307_mianBanTongXunZhuangTai2;
  uint32_t valueAHU307_mianBanTongXunZhuangTai3;
  uint32_t valueAHU308_mianBanTongXunZhuangTai1;
  uint32_t valueAHU308_mianBanTongXunZhuangTai2;
  uint32_t valueAHU308_mianBanTongXunZhuangTai3;
  uint32_t valuePAU301_mianBanTongXunZhuangTai1;
  uint32_t valuePAU301_mianBanTongXunZhuangTai2;
  uint32_t valuePAU301_mianBanTongXunZhuangTai3;
  uint32_t valuePAU302_mianBanTongXunZhuangTai1;
  uint32_t valuePAU302_mianBanTongXunZhuangTai2;
  uint32_t valuePAU302_mianBanTongXunZhuangTai3;
  uint32_t valuePAU303_mianBanTongXunZhuangTai1;
  uint32_t valuePAU303_mianBanTongXunZhuangTai2;
  uint32_t valuePAU303_mianBanTongXunZhuangTai3;
  uint32_t valuePAU304_mianBanTongXunZhuangTai1;
  uint32_t valuePAU304_mianBanTongXunZhuangTai2;
  uint32_t valuePAU304_mianBanTongXunZhuangTai3;
  uint32_t valuePAU305_mianBanTongXunZhuangTai1;
  uint32_t valuePAU305_mianBanTongXunZhuangTai2;
  uint32_t valuePAU305_mianBanTongXunZhuangTai3;
  uint32_t valueAHU301_tempReal;
  uint32_t valueAHU301_humiReal;
  uint32_t valueAHU301_tempSet;
  uint32_t valueAHU301_humiSet;
  uint32_t valueAHU301_lengShuiFaKaiDu;
  uint32_t valueAHU301_reShuiFaKaiDu;
  uint32_t valueAHU301_xinFengWenDU;
  uint32_t valueAHU301_jiaShiQIKaiDu;
  uint32_t valueAHU301_beiYong;
  uint32_t valueAHU302_tempReal;
  uint32_t valueAHU302_humiReal;
  uint32_t valueAHU302_tempSet;
  uint32_t valueAHU302_humiSet;
  uint32_t valueAHU302_lengShuiFaKaiDu;
  uint32_t valueAHU302_reShuiFaKaiDu;
  uint32_t valueAHU302_xinFengWenDU;
  uint32_t valueAHU302_jiaShiQIKaiDu;
  uint32_t valueAHU302_beiYong;
  uint32_t valueAHU303_tempReal;
  uint32_t valueAHU303_humiReal;
  uint32_t valueAHU303_tempSet;
  uint32_t valueAHU303_humiSet;
  uint32_t valueAHU303_lengShuiFaKaiDu;
  uint32_t valueAHU303_reShuiFaKaiDu;
  uint32_t valueAHU303_xinFengWenDU;
  uint32_t valueAHU303_jiaShiQIKaiDu;
  uint32_t valueAHU303_beiYong;
  uint32_t valueAHU304_tempReal;
  uint32_t valueAHU304_humiReal;
  uint32_t valueAHU304_tempSet;
  uint32_t valueAHU304_humiSet;
  uint32_t valueAHU304_lengShuiFaKaiDu;
  uint32_t valueAHU304_reShuiFaKaiDu;
  uint32_t valueAHU304_xinFengWenDU;
  uint32_t valueAHU304_jiaShiQIKaiDu;
  uint32_t valueAHU304_beiYong;
  uint32_t valueAHU305_tempReal;
  uint32_t valueAHU305_humiReal;
  uint32_t valueAHU305_tempSet;
  uint32_t valueAHU305_humiSet;
  uint32_t valueAHU305_lengShuiFaKaiDu;
  uint32_t valueAHU305_reShuiFaKaiDu;
  uint32_t valueAHU305_xinFengWenDU;
  uint32_t valueAHU305_jiaShiQIKaiDu;
  uint32_t valueAHU305_beiYong;
  uint32_t valueAHU306_tempReal;
  uint32_t valueAHU306_humiReal;
  uint32_t valueAHU306_tempSet;
  uint32_t valueAHU306_humiSet;
  uint32_t valueAHU306_lengShuiFaKaiDu;
  uint32_t valueAHU306_reShuiFaKaiDu;
  uint32_t valueAHU306_xinFengWenDU;
  uint32_t valueAHU306_jiaShiQIKaiDu;
  uint32_t valueAHU306_beiYong;
  uint32_t valueAHU307_tempReal;
  uint32_t valueAHU307_humiReal;
  uint32_t valueAHU307_tempSet;
  uint32_t valueAHU307_humiSet;
  uint32_t valueAHU307_lengShuiFaKaiDu;
  uint32_t valueAHU307_reShuiFaKaiDu;
  uint32_t valueAHU307_xinFengWenDU;
  uint32_t valueAHU307_jiaShiQIKaiDu;
  uint32_t valueAHU307_beiYong;
  uint32_t valueAHU308_tempReal;
  uint32_t valueAHU308_humiReal;
  uint32_t valueAHU308_tempSet;
  uint32_t valueAHU308_humiSet;
  uint32_t valueAHU308_lengShuiFaKaiDu;
  uint32_t valueAHU308_reShuiFaKaiDu;
  uint32_t valueAHU308_xinFengWenDU;
  uint32_t valueAHU308_jiaShiQIKaiDu;
  uint32_t valueAHU308_beiYong;
  uint32_t valuePAU301_tempReal;
  uint32_t valuePAU301_humiReal;
  uint32_t valuePAU301_tempSet;
  uint32_t valuePAU301_humiSet;
  uint32_t valuePAU301_lengShuiFaKaiDu;
  uint32_t valuePAU301_reShuiFaKaiDu;
  uint32_t valuePAU301_xinFengWenDU;
  uint32_t valuePAU301_jiaShiQIKaiDu;
  uint32_t valuePAU301_beiYong;
  uint32_t valuePAU302_tempReal;
  uint32_t valuePAU302_humiReal;
  uint32_t valuePAU302_tempSet;
  uint32_t valuePAU302_humiSet;
  uint32_t valuePAU302_lengShuiFaKaiDu;
  uint32_t valuePAU302_reShuiFaKaiDu;
  uint32_t valuePAU302_xinFengWenDU;
  uint32_t valuePAU302_jiaShiQIKaiDu;
  uint32_t valuePAU302_beiYong;
  uint32_t valuePAU303_tempReal;
  uint32_t valuePAU303_humiReal;
  uint32_t valuePAU303_tempSet;
  uint32_t valuePAU303_humiSet;
  uint32_t valuePAU303_lengShuiFaKaiDu;
  uint32_t valuePAU303_reShuiFaKaiDu;
  uint32_t valuePAU303_xinFengWenDU;
  uint32_t valuePAU303_jiaShiQIKaiDu;
  uint32_t valuePAU303_beiYong;
  uint32_t valuePAU304_tempReal;
  uint32_t valuePAU304_humiReal;
  uint32_t valuePAU304_tempSet;
  uint32_t valuePAU304_humiSet;
  uint32_t valuePAU304_lengShuiFaKaiDu;
  uint32_t valuePAU304_reShuiFaKaiDu;
  uint32_t valuePAU304_xinFengWenDU;
  uint32_t valuePAU304_jiaShiQIKaiDu;
  uint32_t valuePAU304_beiYong;
  uint32_t valuePAU305_tempReal;
  uint32_t valuePAU305_humiReal;
  uint32_t valuePAU305_tempSet;
  uint32_t valuePAU305_humiSet;
  uint32_t valuePAU305_lengShuiFaKaiDu;
  uint32_t valuePAU305_reShuiFaKaiDu;
  uint32_t valuePAU305_xinFengWenDU;
  uint32_t valuePAU305_jiaShiQIKaiDu;
  uint32_t valuePAU305_beiYong;
} dataPoint_t;

/** User Area Device State Structure */
typedef struct {
  uint8_t flagAHU301_fengJiYiQiDong:1;
  uint8_t flagAHU301_zhiBanStatus:1;
  uint8_t flagAHU301_dianJiaRe1:1;
  uint8_t flagAHU301_dianJiaRe2:1;
  uint8_t flagAHU301_dianJiaRe3:1;
  uint8_t flagAHU301_fengJiStatus:1;
  uint8_t flagAHU301_shouZiDong:1;
  uint8_t flagAHU301_dongXiaJi:1;
  uint8_t flagAHU301_zhongXiaoBaoJing:1;
  uint8_t flagAHU301_dianJiaReGaoWen:1;
  uint8_t flagAHU301_fengJiQueFeng:1;
  uint8_t flagAHU301_paiFengJiYiQiDong:1;
  uint8_t flagAHU301_diWenPanGuan:1;
  uint8_t flagAHU301_mieJunYunXing:1;
  uint8_t flagAHU302_fengJiYiQiDong:1;
  uint8_t flagAHU302_zhiBanStatus:1;
  uint8_t flagAHU302_dianJiaRe1:1;
  uint8_t flagAHU302_dianJiaRe2:1;
  uint8_t flagAHU302_dianJiaRe3:1;
  uint8_t flagAHU302_fengJiStatus:1;
  uint8_t flagAHU302_shouZiDong:1;
  uint8_t flagAHU302_dongXiaJi:1;
  uint8_t flagAHU302_zhongXiaoBaoJing:1;
  uint8_t flagAHU302_dianJiaReGaoWen:1;
  uint8_t flagAHU302_fengJiQueFeng:1;
  uint8_t flagAHU302_paiFengJiYiQiDong:1;
  uint8_t flagAHU302_diWenPanGuan:1;
  uint8_t flagAHU302_mieJunYunXing:1;
  uint8_t flagAHU303_fengJiYiQiDong:1;
  uint8_t flagAHU303_zhiBanStatus:1;
  uint8_t flagAHU303_dianJiaRe1:1;
  uint8_t flagAHU303_dianJiaRe2:1;
  uint8_t flagAHU303_dianJiaRe3:1;
  uint8_t flagAHU303_fengJiStatus:1;
  uint8_t flagAHU303_shouZiDong:1;
  uint8_t flagAHU303_dongXiaJi:1;
  uint8_t flagAHU303_zhongXiaoBaoJing:1;
  uint8_t flagAHU303_dianJiaReGaoWen:1;
  uint8_t flagAHU303_fengJiQueFeng:1;
  uint8_t flagAHU303_paiFengJiYiQiDong:1;
  uint8_t flagAHU303_diWenPanGuan:1;
  uint8_t flagAHU303_mieJunYunXing:1;
  uint8_t flagAHU304_fengJiYiQiDong:1;
  uint8_t flagAHU304_zhiBanStatus:1;
  uint8_t flagAHU304_dianJiaRe1:1;
  uint8_t flagAHU304_dianJiaRe2:1;
  uint8_t flagAHU304_dianJiaRe3:1;
  uint8_t flagAHU304_fengJiStatus:1;
  uint8_t flagAHU304_shouZiDong:1;
  uint8_t flagAHU304_dongXiaJi:1;
  uint8_t flagAHU304_zhongXiaoBaoJing:1;
  uint8_t flagAHU304_dianJiaReGaoWen:1;
  uint8_t flagAHU304_fengJiQueFeng:1;
  uint8_t flagAHU304_paiFengJiYiQiDong:1;
  uint8_t flagAHU304_diWenPanGuan:1;
  uint8_t flagAHU304_mieJunYunXing:1;
  uint8_t flagAHU305_fengJiYiQiDong:1;
  uint8_t flagAHU305_zhiBanStatus:1;
  uint8_t flagAHU305_dianJiaRe1:1;
  uint8_t flagAHU305_dianJiaRe2:1;
  uint8_t flagAHU305_dianJiaRe3:1;
  uint8_t flagAHU305_fengJiStatus:1;
  uint8_t flagAHU305_shouZiDong:1;
  uint8_t flagAHU305_dongXiaJi:1;
  uint8_t flagAHU305_zhongXiaoBaoJing:1;
  uint8_t flagAHU305_dianJiaReGaoWen:1;
  uint8_t flagAHU305_fengJiQueFeng:1;
  uint8_t flagAHU305_paiFengJiYiQiDong:1;
  uint8_t flagAHU305_diWenPanGuan:1;
  uint8_t flagAHU305_mieJunYunXing:1;
  uint8_t flagAHU306_fengJiYiQiDong:1;
  uint8_t flagAHU306_zhiBanStatus:1;
  uint8_t flagAHU306_dianJiaRe1:1;
  uint8_t flagAHU306_dianJiaRe2:1;
  uint8_t flagAHU306_dianJiaRe3:1;
  uint8_t flagAHU306_fengJiStatus:1;
  uint8_t flagAHU306_shouZiDong:1;
  uint8_t flagAHU306_dongXiaJi:1;
  uint8_t flagAHU306_zhongXiaoBaoJing:1;
  uint8_t flagAHU306_dianJiaReGaoWen:1;
  uint8_t flagAHU306_fengJiQueFeng:1;
  uint8_t flagAHU306_paiFengJiYiQiDong:1;
  uint8_t flagAHU306_diWenPanGuan:1;
  uint8_t flagAHU306_mieJunYunXing:1;
  uint8_t flagAHU307_fengJiYiQiDong:1;
  uint8_t flagAHU307_zhiBanStatus:1;
  uint8_t flagAHU307_dianJiaRe1:1;
  uint8_t flagAHU307_dianJiaRe2:1;
  uint8_t flagAHU307_dianJiaRe3:1;
  uint8_t flagAHU307_fengJiStatus:1;
  uint8_t flagAHU307_shouZiDong:1;
  uint8_t flagAHU307_dongXiaJi:1;
  uint8_t flagAHU307_zhongXiaoBaoJing:1;
  uint8_t flagAHU307_dianJiaReGaoWen:1;
  uint8_t flagAHU307_fengJiQueFeng:1;
  uint8_t flagAHU307_paiFengJiYiQiDong:1;
  uint8_t flagAHU307_diWenPanGuan:1;
  uint8_t flagAHU307_mieJunYunXing:1;
  uint8_t flagAHU308_fengJiYiQiDong:1;
  uint8_t flagAHU308_zhiBanStatus:1;
  uint8_t flagAHU308_dianJiaRe1:1;
  uint8_t flagAHU308_dianJiaRe2:1;
  uint8_t flagAHU308_dianJiaRe3:1;
  uint8_t flagAHU308_fengJiStatus:1;
  uint8_t flagAHU308_shouZiDong:1;
  uint8_t flagAHU308_dongXiaJi:1;
  uint8_t flagAHU308_zhongXiaoBaoJing:1;
  uint8_t flagAHU308_dianJiaReGaoWen:1;
  uint8_t flagAHU308_fengJiQueFeng:1;
  uint8_t flagAHU308_paiFengJiYiQiDong:1;
  uint8_t flagAHU308_diWenPanGuan:1;
  uint8_t flagAHU308_mieJunYunXing:1;
  uint8_t flagPAU301_fengJiYiQiDong:1;
  uint8_t flagPAU301_zhiBanStatus:1;
  uint8_t flagPAU301_dianJiaRe1:1;
  uint8_t flagPAU301_dianJiaRe2:1;
  uint8_t flagPAU301_dianJiaRe3:1;
  uint8_t flagPAU301_fengJiStatus:1;
  uint8_t flagPAU301_shouZiDong:1;
  uint8_t flagPAU301_dongXiaJi:1;
  uint8_t flagPAU301_zhongXiaoBaoJing:1;
  uint8_t flagPAU301_dianJiaReGaoWen:1;
  uint8_t flagPAU301_fengJiQueFeng:1;
  uint8_t flagPAU301_paiFengJiYiQiDong:1;
  uint8_t flagPAU301_dianYuRe1:1;
  uint8_t flagPAU301_dianYuRe2:1;
  uint8_t flagPAU301_dianYuRe3:1;
  uint8_t flagPAU301_dianYuReGaoWen:1;
  uint8_t flagPAU301_diWenPanGuan:1;
  uint8_t flagPAU301_mieJunYunXing:1;
  uint8_t flagPAU302_fengJiYiQiDong:1;
  uint8_t flagPAU302_zhiBanStatus:1;
  uint8_t flagPAU302_fengJiStatus:1;
  uint8_t flagPAU302_shouZiDong:1;
  uint8_t flagPAU302_dongXiaJi:1;
  uint8_t flagPAU302_zhongXiaoBaoJing:1;
  uint8_t flagPAU302_fengJiQueFeng:1;
  uint8_t flagPAU302_paiFengJiYiQiDong:1;
  uint8_t flagPAU302_dianYuRe1:1;
  uint8_t flagPAU302_dianYuRe2:1;
  uint8_t flagPAU302_dianYuRe3:1;
  uint8_t flagPAU302_dianYuReGaoWen:1;
  uint8_t flagPAU302_diWenPanGuan:1;
  uint8_t flagPAU302_mieJunYunXing:1;
  uint8_t flagPAU303_fengJiYiQiDong:1;
  uint8_t flagPAU303_zhiBanStatus:1;
  uint8_t flagPAU303_fengJiStatus:1;
  uint8_t flagPAU303_shouZiDong:1;
  uint8_t flagPAU303_dongXiaJi:1;
  uint8_t flagPAU303_zhongXiaoBaoJing:1;
  uint8_t flagPAU303_fengJiQueFeng:1;
  uint8_t flagPAU303_paiFengJiYiQiDong:1;
  uint8_t flagPAU303_dianYuRe1:1;
  uint8_t flagPAU303_dianYuRe2:1;
  uint8_t flagPAU303_dianYuRe3:1;
  uint8_t flagPAU303_dianYuReGaoWen:1;
  uint8_t flagPAU303_diWenPanGuan:1;
  uint8_t flagPAU303_mieJunYunXing:1;
  uint8_t flagPAU304_fengJiYiQiDong:1;
  uint8_t flagPAU304_zhiBanStatus:1;
  uint8_t flagPAU304_fengJiStatus:1;
  uint8_t flagPAU304_shouZiDong:1;
  uint8_t flagPAU304_dongXiaJi:1;
  uint8_t flagPAU304_zhongXiaoBaoJing:1;
  uint8_t flagPAU304_fengJiQueFeng:1;
  uint8_t flagPAU304_paiFengJiYiQiDong:1;
  uint8_t flagPAU304_dianYuRe1:1;
  uint8_t flagPAU304_dianYuRe2:1;
  uint8_t flagPAU304_dianYuRe3:1;
  uint8_t flagPAU304_dianYuReGaoWen:1;
  uint8_t flagPAU304_diWenPanGuan:1;
  uint8_t flagPAU304_mieJunYunXing:1;
  uint8_t flagPAU305_fengJiYiQiDong:1;
  uint8_t flagPAU305_zhiBanStatus:1;
  uint8_t flagPAU305_fengJiStatus:1;
  uint8_t flagPAU305_shouZiDong:1;
  uint8_t flagPAU305_dongXiaJi:1;
  uint8_t flagPAU305_zhongXiaoBaoJing:1;
  uint8_t flagPAU305_fengJiQueFeng:1;
  uint8_t flagPAU305_paiFengJiYiQiDong:1;
  uint8_t flagPAU305_dianYuRe1:1;
  uint8_t flagPAU305_dianYuRe2:1;
  uint8_t flagPAU305_dianYuRe3:1;
  uint8_t flagPAU305_dianYuReGaoWen:1;
  uint8_t flagPAU305_diWenPanGuan:1;
  uint8_t flagPAU305_mieJunYunXing:1;
  uint8_t flagonline_flag:1;
  uint8_t flagAHU301_mianBanTongXunZhuangTai1:1;
  uint8_t flagAHU301_mianBanTongXunZhuangTai2:1;
  uint8_t flagAHU301_mianBanTongXunZhuangTai3:1;
  uint8_t flagAHU302_mianBanTongXunZhuangTai1:1;
  uint8_t flagAHU302_mianBanTongXunZhuangTai2:1;
  uint8_t flagAHU302_mianBanTongXunZhuangTai3:1;
  uint8_t flagAHU303_mianBanTongXunZhuangTai1:1;
  uint8_t flagAHU303_mianBanTongXunZhuangTai2:1;
  uint8_t flagAHU303_mianBanTongXunZhuangTai3:1;
  uint8_t flagAHU304_mianBanTongXunZhuangTai1:1;
  uint8_t flagAHU304_mianBanTongXunZhuangTai2:1;
  uint8_t flagAHU304_mianBanTongXunZhuangTai3:1;
  uint8_t flagAHU305_mianBanTongXunZhuangTai1:1;
  uint8_t flagAHU305_mianBanTongXunZhuangTai2:1;
  uint8_t flagAHU305_mianBanTongXunZhuangTai3:1;
  uint8_t flagAHU306_mianBanTongXunZhuangTai1:1;
  uint8_t flagAHU306_mianBanTongXunZhuangTai2:1;
  uint8_t flagAHU306_mianBanTongXunZhuangTai3:1;
  uint8_t flagAHU307_mianBanTongXunZhuangTai1:1;
  uint8_t flagAHU307_mianBanTongXunZhuangTai2:1;
  uint8_t flagAHU307_mianBanTongXunZhuangTai3:1;
  uint8_t flagAHU308_mianBanTongXunZhuangTai1:1;
  uint8_t flagAHU308_mianBanTongXunZhuangTai2:1;
  uint8_t flagAHU308_mianBanTongXunZhuangTai3:1;
  uint8_t flagPAU301_mianBanTongXunZhuangTai1:1;
  uint8_t flagPAU301_mianBanTongXunZhuangTai2:1;
  uint8_t flagPAU301_mianBanTongXunZhuangTai3:1;
  uint8_t flagPAU302_mianBanTongXunZhuangTai1:1;
  uint8_t flagPAU302_mianBanTongXunZhuangTai2:1;
  uint8_t flagPAU302_mianBanTongXunZhuangTai3:1;
  uint8_t flagPAU303_mianBanTongXunZhuangTai1:1;
  uint8_t flagPAU303_mianBanTongXunZhuangTai2:1;
  uint8_t flagPAU303_mianBanTongXunZhuangTai3:1;
  uint8_t flagPAU304_mianBanTongXunZhuangTai1:1;
  uint8_t flagPAU304_mianBanTongXunZhuangTai2:1;
  uint8_t flagPAU304_mianBanTongXunZhuangTai3:1;
  uint8_t flagPAU305_mianBanTongXunZhuangTai1:1;
  uint8_t flagPAU305_mianBanTongXunZhuangTai2:1;
  uint8_t flagPAU305_mianBanTongXunZhuangTai3:1;
  uint8_t flagAHU301_tempReal:1;
  uint8_t flagAHU301_humiReal:1;
  uint8_t flagAHU301_tempSet:1;
  uint8_t flagAHU301_humiSet:1;
  uint8_t flagAHU301_lengShuiFaKaiDu:1;
  uint8_t flagAHU301_reShuiFaKaiDu:1;
  uint8_t flagAHU301_xinFengWenDU:1;
  uint8_t flagAHU301_jiaShiQIKaiDu:1;
  uint8_t flagAHU301_beiYong:1;
  uint8_t flagAHU302_tempReal:1;
  uint8_t flagAHU302_humiReal:1;
  uint8_t flagAHU302_tempSet:1;
  uint8_t flagAHU302_humiSet:1;
  uint8_t flagAHU302_lengShuiFaKaiDu:1;
  uint8_t flagAHU302_reShuiFaKaiDu:1;
  uint8_t flagAHU302_xinFengWenDU:1;
  uint8_t flagAHU302_jiaShiQIKaiDu:1;
  uint8_t flagAHU302_beiYong:1;
  uint8_t flagAHU303_tempReal:1;
  uint8_t flagAHU303_humiReal:1;
  uint8_t flagAHU303_tempSet:1;
  uint8_t flagAHU303_humiSet:1;
  uint8_t flagAHU303_lengShuiFaKaiDu:1;
  uint8_t flagAHU303_reShuiFaKaiDu:1;
  uint8_t flagAHU303_xinFengWenDU:1;
  uint8_t flagAHU303_jiaShiQIKaiDu:1;
  uint8_t flagAHU303_beiYong:1;
  uint8_t flagAHU304_tempReal:1;
  uint8_t flagAHU304_humiReal:1;
  uint8_t flagAHU304_tempSet:1;
  uint8_t flagAHU304_humiSet:1;
  uint8_t flagAHU304_lengShuiFaKaiDu:1;
  uint8_t flagAHU304_reShuiFaKaiDu:1;
  uint8_t flagAHU304_xinFengWenDU:1;
  uint8_t flagAHU304_jiaShiQIKaiDu:1;
  uint8_t flagAHU304_beiYong:1;
  uint8_t flagAHU305_tempReal:1;
  uint8_t flagAHU305_humiReal:1;
  uint8_t flagAHU305_tempSet:1;
  uint8_t flagAHU305_humiSet:1;
  uint8_t flagAHU305_lengShuiFaKaiDu:1;
  uint8_t flagAHU305_reShuiFaKaiDu:1;
  uint8_t flagAHU305_xinFengWenDU:1;
  uint8_t flagAHU305_jiaShiQIKaiDu:1;
  uint8_t flagAHU305_beiYong:1;
  uint8_t flagAHU306_tempReal:1;
  uint8_t flagAHU306_humiReal:1;
  uint8_t flagAHU306_tempSet:1;
  uint8_t flagAHU306_humiSet:1;
  uint8_t flagAHU306_lengShuiFaKaiDu:1;
  uint8_t flagAHU306_reShuiFaKaiDu:1;
  uint8_t flagAHU306_xinFengWenDU:1;
  uint8_t flagAHU306_jiaShiQIKaiDu:1;
  uint8_t flagAHU306_beiYong:1;
  uint8_t flagAHU307_tempReal:1;
  uint8_t flagAHU307_humiReal:1;
  uint8_t flagAHU307_tempSet:1;
  uint8_t flagAHU307_humiSet:1;
  uint8_t flagAHU307_lengShuiFaKaiDu:1;
  uint8_t flagAHU307_reShuiFaKaiDu:1;
  uint8_t flagAHU307_xinFengWenDU:1;
  uint8_t flagAHU307_jiaShiQIKaiDu:1;
  uint8_t flagAHU307_beiYong:1;
  uint8_t flagAHU308_tempReal:1;
  uint8_t flagAHU308_humiReal:1;
  uint8_t flagAHU308_tempSet:1;
  uint8_t flagAHU308_humiSet:1;
  uint8_t flagAHU308_lengShuiFaKaiDu:1;
  uint8_t flagAHU308_reShuiFaKaiDu:1;
  uint8_t flagAHU308_xinFengWenDU:1;
  uint8_t flagAHU308_jiaShiQIKaiDu:1;
  uint8_t flagAHU308_beiYong:1;
  uint8_t flagPAU301_tempReal:1;
  uint8_t flagPAU301_humiReal:1;
  uint8_t flagPAU301_tempSet:1;
  uint8_t flagPAU301_humiSet:1;
  uint8_t flagPAU301_lengShuiFaKaiDu:1;
  uint8_t flagPAU301_reShuiFaKaiDu:1;
  uint8_t flagPAU301_xinFengWenDU:1;
  uint8_t flagPAU301_jiaShiQIKaiDu:1;
  uint8_t flagPAU301_beiYong:1;
  uint8_t flagPAU302_tempReal:1;
  uint8_t flagPAU302_humiReal:1;
  uint8_t flagPAU302_tempSet:1;
  uint8_t flagPAU302_humiSet:1;
  uint8_t flagPAU302_lengShuiFaKaiDu:1;
  uint8_t flagPAU302_reShuiFaKaiDu:1;
  uint8_t flagPAU302_xinFengWenDU:1;
  uint8_t flagPAU302_jiaShiQIKaiDu:1;
  uint8_t flagPAU302_beiYong:1;
  uint8_t flagPAU303_tempReal:1;
  uint8_t flagPAU303_humiReal:1;
  uint8_t flagPAU303_tempSet:1;
  uint8_t flagPAU303_humiSet:1;
  uint8_t flagPAU303_lengShuiFaKaiDu:1;
  uint8_t flagPAU303_reShuiFaKaiDu:1;
  uint8_t flagPAU303_xinFengWenDU:1;
  uint8_t flagPAU303_jiaShiQIKaiDu:1;
  uint8_t flagPAU303_beiYong:1;
  uint8_t flagPAU304_tempReal:1;
  uint8_t flagPAU304_humiReal:1;
  uint8_t flagPAU304_tempSet:1;
  uint8_t flagPAU304_humiSet:1;
  uint8_t flagPAU304_lengShuiFaKaiDu:1;
  uint8_t flagPAU304_reShuiFaKaiDu:1;
  uint8_t flagPAU304_xinFengWenDU:1;
  uint8_t flagPAU304_jiaShiQIKaiDu:1;
  uint8_t flagPAU304_beiYong:1;
  uint8_t flagPAU305_tempReal:1;
  uint8_t flagPAU305_humiReal:1;
  uint8_t flagPAU305_tempSet:1;
  uint8_t flagPAU305_humiSet:1;
  uint8_t flagPAU305_lengShuiFaKaiDu:1;
  uint8_t flagPAU305_reShuiFaKaiDu:1;
  uint8_t flagPAU305_xinFengWenDU:1;
  uint8_t flagPAU305_jiaShiQIKaiDu:1;
  uint8_t flagPAU305_beiYong:1;
} dataPointFlags_t;

/** Corresponding to the protocol "4.9 device MCU to WiFi module active reporting of the current state" in the device state "dev_status (11B)" */ 
typedef struct {
  uint8_t bitFeildBuf[COUNT_BIT];
  uint8_t valueAHU301_mianBanTongXunZhuangTai1;
  uint8_t valueAHU301_mianBanTongXunZhuangTai2;
  uint8_t valueAHU301_mianBanTongXunZhuangTai3;
  uint8_t valueAHU302_mianBanTongXunZhuangTai1;
  uint8_t valueAHU302_mianBanTongXunZhuangTai2;
  uint8_t valueAHU302_mianBanTongXunZhuangTai3;
  uint8_t valueAHU303_mianBanTongXunZhuangTai1;
  uint8_t valueAHU303_mianBanTongXunZhuangTai2;
  uint8_t valueAHU303_mianBanTongXunZhuangTai3;
  uint8_t valueAHU304_mianBanTongXunZhuangTai1;
  uint8_t valueAHU304_mianBanTongXunZhuangTai2;
  uint8_t valueAHU304_mianBanTongXunZhuangTai3;
  uint8_t valueAHU305_mianBanTongXunZhuangTai1;
  uint8_t valueAHU305_mianBanTongXunZhuangTai2;
  uint8_t valueAHU305_mianBanTongXunZhuangTai3;
  uint8_t valueAHU306_mianBanTongXunZhuangTai1;
  uint8_t valueAHU306_mianBanTongXunZhuangTai2;
  uint8_t valueAHU306_mianBanTongXunZhuangTai3;
  uint8_t valueAHU307_mianBanTongXunZhuangTai1;
  uint8_t valueAHU307_mianBanTongXunZhuangTai2;
  uint8_t valueAHU307_mianBanTongXunZhuangTai3;
  uint8_t valueAHU308_mianBanTongXunZhuangTai1;
  uint8_t valueAHU308_mianBanTongXunZhuangTai2;
  uint8_t valueAHU308_mianBanTongXunZhuangTai3;
  uint8_t valuePAU301_mianBanTongXunZhuangTai1;
  uint8_t valuePAU301_mianBanTongXunZhuangTai2;
  uint8_t valuePAU301_mianBanTongXunZhuangTai3;
  uint8_t valuePAU302_mianBanTongXunZhuangTai1;
  uint8_t valuePAU302_mianBanTongXunZhuangTai2;
  uint8_t valuePAU302_mianBanTongXunZhuangTai3;
  uint8_t valuePAU303_mianBanTongXunZhuangTai1;
  uint8_t valuePAU303_mianBanTongXunZhuangTai2;
  uint8_t valuePAU303_mianBanTongXunZhuangTai3;
  uint8_t valuePAU304_mianBanTongXunZhuangTai1;
  uint8_t valuePAU304_mianBanTongXunZhuangTai2;
  uint8_t valuePAU304_mianBanTongXunZhuangTai3;
  uint8_t valuePAU305_mianBanTongXunZhuangTai1;
  uint8_t valuePAU305_mianBanTongXunZhuangTai2;
  uint8_t valuePAU305_mianBanTongXunZhuangTai3;
  uint16_t valueAHU301_tempReal;
  uint16_t valueAHU301_humiReal;
  uint16_t valueAHU301_tempSet;
  uint16_t valueAHU301_humiSet;
  uint16_t valueAHU301_lengShuiFaKaiDu;
  uint16_t valueAHU301_reShuiFaKaiDu;
  uint16_t valueAHU301_xinFengWenDU;
  uint16_t valueAHU301_jiaShiQIKaiDu;
  uint16_t valueAHU301_beiYong;
  uint16_t valueAHU302_tempReal;
  uint16_t valueAHU302_humiReal;
  uint16_t valueAHU302_tempSet;
  uint16_t valueAHU302_humiSet;
  uint16_t valueAHU302_lengShuiFaKaiDu;
  uint16_t valueAHU302_reShuiFaKaiDu;
  uint16_t valueAHU302_xinFengWenDU;
  uint16_t valueAHU302_jiaShiQIKaiDu;
  uint16_t valueAHU302_beiYong;
  uint16_t valueAHU303_tempReal;
  uint16_t valueAHU303_humiReal;
  uint16_t valueAHU303_tempSet;
  uint16_t valueAHU303_humiSet;
  uint16_t valueAHU303_lengShuiFaKaiDu;
  uint16_t valueAHU303_reShuiFaKaiDu;
  uint16_t valueAHU303_xinFengWenDU;
  uint16_t valueAHU303_jiaShiQIKaiDu;
  uint16_t valueAHU303_beiYong;
  uint16_t valueAHU304_tempReal;
  uint16_t valueAHU304_humiReal;
  uint16_t valueAHU304_tempSet;
  uint16_t valueAHU304_humiSet;
  uint16_t valueAHU304_lengShuiFaKaiDu;
  uint16_t valueAHU304_reShuiFaKaiDu;
  uint16_t valueAHU304_xinFengWenDU;
  uint16_t valueAHU304_jiaShiQIKaiDu;
  uint16_t valueAHU304_beiYong;
  uint16_t valueAHU305_tempReal;
  uint16_t valueAHU305_humiReal;
  uint16_t valueAHU305_tempSet;
  uint16_t valueAHU305_humiSet;
  uint16_t valueAHU305_lengShuiFaKaiDu;
  uint16_t valueAHU305_reShuiFaKaiDu;
  uint16_t valueAHU305_xinFengWenDU;
  uint16_t valueAHU305_jiaShiQIKaiDu;
  uint16_t valueAHU305_beiYong;
  uint16_t valueAHU306_tempReal;
  uint16_t valueAHU306_humiReal;
  uint16_t valueAHU306_tempSet;
  uint16_t valueAHU306_humiSet;
  uint16_t valueAHU306_lengShuiFaKaiDu;
  uint16_t valueAHU306_reShuiFaKaiDu;
  uint16_t valueAHU306_xinFengWenDU;
  uint16_t valueAHU306_jiaShiQIKaiDu;
  uint16_t valueAHU306_beiYong;
  uint16_t valueAHU307_tempReal;
  uint16_t valueAHU307_humiReal;
  uint16_t valueAHU307_tempSet;
  uint16_t valueAHU307_humiSet;
  uint16_t valueAHU307_lengShuiFaKaiDu;
  uint16_t valueAHU307_reShuiFaKaiDu;
  uint16_t valueAHU307_xinFengWenDU;
  uint16_t valueAHU307_jiaShiQIKaiDu;
  uint16_t valueAHU307_beiYong;
  uint16_t valueAHU308_tempReal;
  uint16_t valueAHU308_humiReal;
  uint16_t valueAHU308_tempSet;
  uint16_t valueAHU308_humiSet;
  uint16_t valueAHU308_lengShuiFaKaiDu;
  uint16_t valueAHU308_reShuiFaKaiDu;
  uint16_t valueAHU308_xinFengWenDU;
  uint16_t valueAHU308_jiaShiQIKaiDu;
  uint16_t valueAHU308_beiYong;
  uint16_t valuePAU301_tempReal;
  uint16_t valuePAU301_humiReal;
  uint16_t valuePAU301_tempSet;
  uint16_t valuePAU301_humiSet;
  uint16_t valuePAU301_lengShuiFaKaiDu;
  uint16_t valuePAU301_reShuiFaKaiDu;
  uint16_t valuePAU301_xinFengWenDU;
  uint16_t valuePAU301_jiaShiQIKaiDu;
  uint16_t valuePAU301_beiYong;
  uint16_t valuePAU302_tempReal;
  uint16_t valuePAU302_humiReal;
  uint16_t valuePAU302_tempSet;
  uint16_t valuePAU302_humiSet;
  uint16_t valuePAU302_lengShuiFaKaiDu;
  uint16_t valuePAU302_reShuiFaKaiDu;
  uint16_t valuePAU302_xinFengWenDU;
  uint16_t valuePAU302_jiaShiQIKaiDu;
  uint16_t valuePAU302_beiYong;
  uint16_t valuePAU303_tempReal;
  uint16_t valuePAU303_humiReal;
  uint16_t valuePAU303_tempSet;
  uint16_t valuePAU303_humiSet;
  uint16_t valuePAU303_lengShuiFaKaiDu;
  uint16_t valuePAU303_reShuiFaKaiDu;
  uint16_t valuePAU303_xinFengWenDU;
  uint16_t valuePAU303_jiaShiQIKaiDu;
  uint16_t valuePAU303_beiYong;
  uint16_t valuePAU304_tempReal;
  uint16_t valuePAU304_humiReal;
  uint16_t valuePAU304_tempSet;
  uint16_t valuePAU304_humiSet;
  uint16_t valuePAU304_lengShuiFaKaiDu;
  uint16_t valuePAU304_reShuiFaKaiDu;
  uint16_t valuePAU304_xinFengWenDU;
  uint16_t valuePAU304_jiaShiQIKaiDu;
  uint16_t valuePAU304_beiYong;
  uint16_t valuePAU305_tempReal;
  uint16_t valuePAU305_humiReal;
  uint16_t valuePAU305_tempSet;
  uint16_t valuePAU305_humiSet;
  uint16_t valuePAU305_lengShuiFaKaiDu;
  uint16_t valuePAU305_reShuiFaKaiDu;
  uint16_t valuePAU305_xinFengWenDU;
  uint16_t valuePAU305_jiaShiQIKaiDu;
  uint16_t valuePAU305_beiYong;
} devStatus_t;


                
/** Event queue structure */                               
typedef struct {                           
    uint8_t num;                                    ///< Number of queue member
    uint8_t event[EVENT_TYPE_MAX];                  ///< Queue member event content
}eventInfo_t;


                            
/** wifiSignal strength structure */                       
typedef struct {                           
    uint8_t rssi;                                   ///< WIFI signal strength
}moduleStatusInfo_t;                                

/** Protocol standard header structure */
typedef struct
{
    uint8_t                 head[2];                ///< The head is 0xFFFF
    uint16_t                len;                    ///< From cmd to the end of the entire packet occupied by the number of bytes
    uint8_t                 cmd;                    ///< command
    uint8_t                 sn;                     ///< 
    uint8_t                 flags[2];               ///< flag,default is 0
} protocolHead_t;

/** 4.1 WiFi module requests the device information protocol structure */
typedef struct
{
    protocolHead_t          head;                   ///< Protocol standard header structure
    uint8_t                 protocolVer[8];         ///< Protocol version
    uint8_t                 p0Ver[8];               ///< p0 Protocol version
    uint8_t                 hardVer[8];             ///< Hardware version
    uint8_t                 softVer[8];             ///< Software version
    uint8_t                 productKey[32];         ///< Product key
    uint16_t                ninableTime;            ///< Binding time(second)
    uint8_t                 devAttr[8];             ///< Device attribute
    uint8_t                 productSecret[32];      ///< Product secret
    uint8_t                 sum;                    ///< checksum
} protocolDeviceInfo_t;

/** Protocol common data frame(4.2、4.4、4.6、4.9、4.10) protocol structure */
typedef struct
{
    protocolHead_t          head;                   ///< Protocol standard header structure
    uint8_t                 sum;                    ///< checksum
} protocolCommon_t;

/** 4.3 The device MCU informs the WiFi module of the configuration mode  protocol structure */
typedef struct
{
    protocolHead_t          head;                   ///< Protocol standard header structure
    uint8_t                 cfgMode;                ///< Configuration parameters
    uint8_t                 sum;                    ///< checksum
} protocolCfgMode_t;

/** 4.13 The MCU requests the network time  protocol structure */
typedef struct
{
    protocolHead_t          head;                   ///< Protocol standard header structure
    uint8_t                 time[7];                ///< Hardware version
    uint8_t                 ntp_time[4];            ///< Software version
    uint8_t                 sum;                    ///< checksum
} protocolUTT_t;

/** WiFi module working status*/
typedef union
{
    uint16_t                value;
    struct
    {
        uint16_t            softap:1;     
        uint16_t            station:1;    
        uint16_t            onboarding:1; 
        uint16_t            binding:1;    
        uint16_t            con_route:1;  
        uint16_t            con_m2m:1;    
        uint16_t            reserve1:2;   
        uint16_t            rssi:3;       
        uint16_t            app:1;        
        uint16_t            test:1;       
        uint16_t            reserve2:3;   
    }types; 
    
} wifiStatus_t;

/** WiFi status type :protocol structure */
typedef struct
{
    protocolHead_t          head;                   ///< Protocol standard header structure
    wifiStatus_t            ststus;                 ///< WIFI status
    uint8_t                 sum;                    ///< checksum
} protocolWifiStatus_t;

/** Protocol common data frame(4.9) :protocol structure */
typedef struct
{
    protocolHead_t          head;                   ///< Protocol standard header structure
    uint8_t                 type;                   ///< Information Type
    uint8_t                 sum;                    ///< checksum
} protocolGetModuleInfo_t;

typedef struct
{
    uint8_t                 moduleType;             ///< Information Type
    uint8_t                 serialVer[8];           ///< Serial port protocol version
    uint8_t                 hardVer[8];             ///< Hardware version
    uint8_t                 softVer[8];             ///< Software version
    uint8_t                 mac[16];                ///< mac 
    uint8_t                 ip[16];                 ///< ip
    uint8_t                 devAttr[8];             ///< Device attribute
} moduleInfo_t;

/** Protocol common data frame(4.9) :protocol structure */
typedef struct
{
    protocolHead_t          head;                   ///< Protocol standard header structure
    moduleInfo_t            wifiModuleInfo;         ///< WIFI module information
    uint8_t                 sum;                    ///< checksum
} protocolModuleInfo_t;


/** GPRS information of base station */
typedef struct
{
    uint16_t                    LAC_ID;             ///<LAC area ID
    uint16_t                    CellID;             ///<Base station ID
    uint8_t                     RSSI;               ///<Signal strength of base station
} gprsCellInfo_t;


/** 3.19 The basic information of the GPRS communication module  */
typedef struct
{
    uint8_t                 Type;//2G/3g/4g
    uint8_t                 Pro_ver[8];//Universal serial port protocol version
    uint8_t                 Hard_ver[8];//Hardware version
    uint8_t                 Soft_ver[8];//Software version
    uint8_t                 Device_attribute[8];//Device attribute
    uint8_t                 IMEI[16];//string
    uint8_t                 IMSI[16];//string
    uint8_t                 MCC[8];//Mobile country code
    uint8_t                 MNC[8];//Mobile network code
    uint8_t                 CellNum;//Number of base station
    uint8_t                 CellInfoLen;//Information length of base station 
    gprsCellInfo_t          GPRS_CellINFO[CELLNUMMAX];
}gprsInfo_t;

/** 4.7 Illegal message notification :protocol structure*/
typedef struct
{
    protocolHead_t          head;                   ///< Protocol standard header structure
    uint8_t                 error;                  ///< error value
    uint8_t                 sum;                    ///< checksum
} protocolErrorType_t;


/** P0 message header */
typedef struct
{
    protocolHead_t          head;                   ///< Protocol standard header structure
    uint8_t                 action;                 ///< p0 command 
} protocolP0Head_t;


/** protocol “4.9 The device MCU reports the current status to the WiFi module” device status "dev_status(11B)"  */ 
typedef struct {
                     
    devStatus_t devStatus;                          ///< Stores the device status data
}gizwitsReport_t;

/** resend strategy structure */
typedef struct {
    uint8_t                 num;                    ///< resend times
    uint8_t                 flag;                   ///< 1,Indicates that there is a need to wait for the ACK;0,Indicates that there is no need to wait for the ACK
    uint8_t                 buf[MAX_PACKAGE_LEN];   ///< resend data buffer
    uint16_t                dataLen;                ///< resend data length
    uint32_t                sendTime;               ///< resend time
} protocolWaitAck_t;
                                                                                


typedef struct {                           
    dataPointFlags_t  devDatapointFlag;             ///< Datapoint Flag , All datapoints 
    devStatus_t devStatus;                          ///< All datapoints data
}gizwitsElongateP0Form_t;  

typedef struct {                           
    uint8_t action;                                 ///< P0 action 
    uint8_t gizdata[sizeof(gizwitsElongateP0Form_t)];  ///< Max buffer ,  Can hold all datapoints value 
}gizwitsP0Max_t;        

/** Protocol main and very important struct */
typedef struct
{
    uint8_t issuedFlag;                             ///< P0 action type
    uint8_t protocolBuf[MAX_PACKAGE_LEN];           ///< Protocol data handle buffer
    uint8_t transparentBuff[MAX_PACKAGE_LEN];       ///< Transparent data storage area
    uint32_t transparentLen;                        ///< Transmission data length
    
    uint32_t sn;                                    ///< Message SN
    uint32_t timerMsCount;                          ///< Timer Count 
    protocolWaitAck_t waitAck;                      ///< Protocol wait ACK data structure
    
    eventInfo_t issuedProcessEvent;                 ///< Control events
    eventInfo_t wifiStatusEvent;                    ///< WIFI Status events
    eventInfo_t NTPEvent;                           ///< NTP events
    eventInfo_t moduleInfoEvent;                    ///< Module Info events

    dataPointFlags_t  waitReportDatapointFlag;      ///< Store the data points to be reported flag
    uint8_t reportData[sizeof(gizwitsElongateP0Form_t)];    ///< Reporting actual data , Max , Can hold all datapoints value
    uint32_t reportDataLen;                         ///< Reporting actual data length

    dataPoint_t gizCurrentDataPoint;                ///< Current device datapoints status
    dataPoint_t gizLastDataPoint;                   ///< Last device datapoints status
    moduleStatusInfo_t wifiStatusData;              ///< WIFI signal intensity
    protocolTime_t TimeNTP;                         ///< Network time information
#if MODULE_TYPE
    gprsInfo_t   gprsInfoNews;
#else  
    moduleInfo_t  wifiModuleNews;                   ///< WIFI module Info
#endif
    
}gizwitsProtocol_t;

#pragma pack()

/**@name Gizwits user API interface
* @{
*/

extern uint32_t gizGetTimerCount(void);

void gizwitsInit(void);
int32_t gizwitsSetMode(uint8_t mode);
void gizwitsGetNTP(void);
int32_t gizwitsHandle(dataPoint_t *currentData);
int32_t gizwitsPassthroughData(uint8_t * gizdata, uint32_t len);
void gizwitsGetModuleInfo(void);
int32_t gizPutData(uint8_t *buf, uint32_t len);

/**@} */
#ifdef __cplusplus
}
#endif 

#endif
