/**
************************************************************
* @file         gizwits_protocol.c
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
#include "ringBuffer.h"
#include "gizwits_product.h"
#include "dataPointTools.h"

/** Protocol global variables **/
gizwitsProtocol_t gizwitsProtocol;


/**@name The serial port receives the ring buffer implementation
* @{
*/
rb_t pRb;                                               ///< Ring buffer structure variable
static uint8_t rbBuf[RB_MAX_LEN];                       ///< Ring buffer data cache buffer


/**@} */

/**
* @brief Write data to the ring buffer
* @param [in] buf        : buf adress
* @param [in] len        : byte length
* @return   correct : Returns the length of the written data
            failure : -1
*/
int32_t gizPutData(uint8_t *buf, uint32_t len)
{
    int32_t count = 0;

    if(NULL == buf)
    {
        GIZWITS_LOG("ERR: gizPutData buf is empty \n");
        return -1;
    }

    count = rbWrite(&pRb, buf, len);
    if(count != len)
    {
        GIZWITS_LOG("ERR: Failed to rbWrite \n");
        return -1;
    }

    return count;
}



/**
* @brief Protocol header initialization
*
* @param [out] head         : Protocol header pointer
*
* @return 0， success; other， failure    
*/
static int8_t gizProtocolHeadInit(protocolHead_t *head)
{
    if(NULL == head)
    {
        GIZWITS_LOG("ERR: gizProtocolHeadInit head is empty \n");
        return -1;
    }

    memset((uint8_t *)head, 0, sizeof(protocolHead_t));
    head->head[0] = 0xFF;
    head->head[1] = 0xFF;

    return 0;
}

/**
* @brief Protocol ACK check processing function
*
* @param [in] data            : data adress
* @param [in] len             : data length
*
* @return 0， suceess; other， failure
*/
static int8_t gizProtocolWaitAck(uint8_t *gizdata, uint32_t len)
{
    if(NULL == gizdata)
    {
        GIZWITS_LOG("ERR: data is empty \n");
        return -1;
    }

    memset((uint8_t *)&gizwitsProtocol.waitAck, 0, sizeof(protocolWaitAck_t));
    memcpy((uint8_t *)gizwitsProtocol.waitAck.buf, gizdata, len);
    gizwitsProtocol.waitAck.dataLen = (uint16_t)len;
    
    gizwitsProtocol.waitAck.flag = 1;
    gizwitsProtocol.waitAck.sendTime = gizGetTimerCount();

    return 0;
}
/**
* @brief Calculates the byte size occupied by the bit
*
* @param [in] aFlag: P0 flag data
*
* @return: byte size
*/
uint32_t ICACHE_FLASH_ATTR calculateDynamicBitPartLen(dataPointFlags_t *aFlag)
{
    uint32_t bitFieldBitLen = 0,bytetmpLen= 0;
    /* Processing only writable bool Enum type data */
    if(0x01 == aFlag->flagonline_flag)
    {
        bitFieldBitLen += online_flag_LEN;
    }

    if(0 == bitFieldBitLen)
    {
        bytetmpLen = 0;
    }
    else
    {
        if(0 == bitFieldBitLen%8)
        {
            bytetmpLen = bitFieldBitLen/8;
        }
        else
        {
            bytetmpLen = bitFieldBitLen/8 + 1;
        }
    }
    return bytetmpLen;
}

/**
* @brief generates "controlled events" according to protocol

* @param [in] issuedData: Controlled data
* @param [out] info: event queue
* @param [out] dataPoints: data point data
* @return 0, the implementation of success, non-0, failed
*/
static int8_t ICACHE_FLASH_ATTR gizDataPoint2Event(uint8_t *issuedData, eventInfo_t *info, dataPoint_t *dataPoints)
{
    uint32_t bitFieldByteLen= 0;//Bit segment length
    uint32_t bitFieldOffset = 0;//Bit position offset
    uint32_t byteFieldOffset = 0;//Byte segment offset

    gizwitsElongateP0Form_t elongateP0FormTmp;
    gizMemset((uint8_t *)&elongateP0FormTmp,0,sizeof(gizwitsElongateP0Form_t));
    gizMemcpy((uint8_t *)&elongateP0FormTmp.devDatapointFlag,issuedData,DATAPOINT_FLAG_LEN);

    if((NULL == issuedData) || (NULL == info) ||(NULL == dataPoints))
    {
        GIZWITS_LOG("gizDataPoint2Event Error , Illegal Param\n");
        return -1;
    }
	
    /** Greater than 1 byte to do bit conversion **/
    if(DATAPOINT_FLAG_LEN > 1)
    {
        if(-1 == gizByteOrderExchange((uint8_t *)&elongateP0FormTmp.devDatapointFlag,DATAPOINT_FLAG_LEN))
        {
            GIZWITS_LOG("gizByteOrderExchange Error\n");
            return -1;
        }
    }
    /* Calculates the byte length occupied by the segment */
    bitFieldByteLen = calculateDynamicBitPartLen(&elongateP0FormTmp.devDatapointFlag);
    byteFieldOffset += bitFieldByteLen + DATAPOINT_FLAG_LEN;//Value segment byte offset

    if(0x01 == elongateP0FormTmp.devDatapointFlag.flagonline_flag)
    {
        info->event[info->num] = EVENT_online_flag;
        info->num++;
        dataPoints->valueonline_flag = gizVarlenDecompressionValue(bitFieldOffset,online_flag_LEN,(uint8_t *)&issuedData[DATAPOINT_FLAG_LEN],bitFieldByteLen);
        bitFieldOffset += online_flag_LEN;
    }


    return 0;
}


/**
* @brief contrasts the current data with the last data
*
* @param [in] cur: current data point data
* @param [in] last: last data point data
*
* @return: 0, no change in data; 1, data changes
*/
static int8_t ICACHE_FLASH_ATTR gizCheckReport(dataPoint_t *cur, dataPoint_t *last)
{
    int8_t ret = 0;
    static uint32_t lastReportTime = 0;
    uint32_t currentTime = 0;

    if((NULL == cur) || (NULL == last))
    {
        GIZWITS_LOG("gizCheckReport Error , Illegal Param\n");
        return -1;
    }
    currentTime = gizGetTimerCount();

    if(last->valueAHU301_fengJiYiQiDong != cur->valueAHU301_fengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU301_fengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU301_fengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU301_zhiBanStatus != cur->valueAHU301_zhiBanStatus)
    {
        GIZWITS_LOG("valueAHU301_zhiBanStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU301_zhiBanStatus = 1;
        ret = 1;
    }
    if(last->valueAHU301_dianJiaRe1 != cur->valueAHU301_dianJiaRe1)
    {
        GIZWITS_LOG("valueAHU301_dianJiaRe1 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU301_dianJiaRe1 = 1;
        ret = 1;
    }
    if(last->valueAHU301_dianJiaRe2 != cur->valueAHU301_dianJiaRe2)
    {
        GIZWITS_LOG("valueAHU301_dianJiaRe2 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU301_dianJiaRe2 = 1;
        ret = 1;
    }
    if(last->valueAHU301_dianJiaRe3 != cur->valueAHU301_dianJiaRe3)
    {
        GIZWITS_LOG("valueAHU301_dianJiaRe3 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU301_dianJiaRe3 = 1;
        ret = 1;
    }
    if(last->valueAHU301_fengJiStatus != cur->valueAHU301_fengJiStatus)
    {
        GIZWITS_LOG("valueAHU301_fengJiStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU301_fengJiStatus = 1;
        ret = 1;
    }
    if(last->valueAHU301_shouZiDong != cur->valueAHU301_shouZiDong)
    {
        GIZWITS_LOG("valueAHU301_shouZiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU301_shouZiDong = 1;
        ret = 1;
    }
    if(last->valueAHU301_dongXiaJi != cur->valueAHU301_dongXiaJi)
    {
        GIZWITS_LOG("valueAHU301_dongXiaJi Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU301_dongXiaJi = 1;
        ret = 1;
    }
    if(last->valueAHU301_zhongXiaoBaoJing != cur->valueAHU301_zhongXiaoBaoJing)
    {
        GIZWITS_LOG("valueAHU301_zhongXiaoBaoJing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU301_zhongXiaoBaoJing = 1;
        ret = 1;
    }
    if(last->valueAHU301_dianJiaReGaoWen != cur->valueAHU301_dianJiaReGaoWen)
    {
        GIZWITS_LOG("valueAHU301_dianJiaReGaoWen Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU301_dianJiaReGaoWen = 1;
        ret = 1;
    }
    if(last->valueAHU301_fengJiQueFeng != cur->valueAHU301_fengJiQueFeng)
    {
        GIZWITS_LOG("valueAHU301_fengJiQueFeng Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU301_fengJiQueFeng = 1;
        ret = 1;
    }
    if(last->valueAHU301_paiFengJiYiQiDong != cur->valueAHU301_paiFengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU301_paiFengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU301_paiFengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU301_diWenPanGuan != cur->valueAHU301_diWenPanGuan)
    {
        GIZWITS_LOG("valueAHU301_diWenPanGuan Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU301_diWenPanGuan = 1;
        ret = 1;
    }
    if(last->valueAHU301_mieJunYunXing != cur->valueAHU301_mieJunYunXing)
    {
        GIZWITS_LOG("valueAHU301_mieJunYunXing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU301_mieJunYunXing = 1;
        ret = 1;
    }
    if(last->valueAHU302_fengJiYiQiDong != cur->valueAHU302_fengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU302_fengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU302_fengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU302_zhiBanStatus != cur->valueAHU302_zhiBanStatus)
    {
        GIZWITS_LOG("valueAHU302_zhiBanStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU302_zhiBanStatus = 1;
        ret = 1;
    }
    if(last->valueAHU302_dianJiaRe1 != cur->valueAHU302_dianJiaRe1)
    {
        GIZWITS_LOG("valueAHU302_dianJiaRe1 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU302_dianJiaRe1 = 1;
        ret = 1;
    }
    if(last->valueAHU302_dianJiaRe2 != cur->valueAHU302_dianJiaRe2)
    {
        GIZWITS_LOG("valueAHU302_dianJiaRe2 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU302_dianJiaRe2 = 1;
        ret = 1;
    }
    if(last->valueAHU302_dianJiaRe3 != cur->valueAHU302_dianJiaRe3)
    {
        GIZWITS_LOG("valueAHU302_dianJiaRe3 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU302_dianJiaRe3 = 1;
        ret = 1;
    }
    if(last->valueAHU302_fengJiStatus != cur->valueAHU302_fengJiStatus)
    {
        GIZWITS_LOG("valueAHU302_fengJiStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU302_fengJiStatus = 1;
        ret = 1;
    }
    if(last->valueAHU302_shouZiDong != cur->valueAHU302_shouZiDong)
    {
        GIZWITS_LOG("valueAHU302_shouZiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU302_shouZiDong = 1;
        ret = 1;
    }
    if(last->valueAHU302_dongXiaJi != cur->valueAHU302_dongXiaJi)
    {
        GIZWITS_LOG("valueAHU302_dongXiaJi Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU302_dongXiaJi = 1;
        ret = 1;
    }
    if(last->valueAHU302_zhongXiaoBaoJing != cur->valueAHU302_zhongXiaoBaoJing)
    {
        GIZWITS_LOG("valueAHU302_zhongXiaoBaoJing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU302_zhongXiaoBaoJing = 1;
        ret = 1;
    }
    if(last->valueAHU302_dianJiaReGaoWen != cur->valueAHU302_dianJiaReGaoWen)
    {
        GIZWITS_LOG("valueAHU302_dianJiaReGaoWen Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU302_dianJiaReGaoWen = 1;
        ret = 1;
    }
    if(last->valueAHU302_fengJiQueFeng != cur->valueAHU302_fengJiQueFeng)
    {
        GIZWITS_LOG("valueAHU302_fengJiQueFeng Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU302_fengJiQueFeng = 1;
        ret = 1;
    }
    if(last->valueAHU302_paiFengJiYiQiDong != cur->valueAHU302_paiFengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU302_paiFengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU302_paiFengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU302_diWenPanGuan != cur->valueAHU302_diWenPanGuan)
    {
        GIZWITS_LOG("valueAHU302_diWenPanGuan Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU302_diWenPanGuan = 1;
        ret = 1;
    }
    if(last->valueAHU302_mieJunYunXing != cur->valueAHU302_mieJunYunXing)
    {
        GIZWITS_LOG("valueAHU302_mieJunYunXing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU302_mieJunYunXing = 1;
        ret = 1;
    }
    if(last->valueAHU303_fengJiYiQiDong != cur->valueAHU303_fengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU303_fengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU303_fengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU303_zhiBanStatus != cur->valueAHU303_zhiBanStatus)
    {
        GIZWITS_LOG("valueAHU303_zhiBanStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU303_zhiBanStatus = 1;
        ret = 1;
    }
    if(last->valueAHU303_dianJiaRe1 != cur->valueAHU303_dianJiaRe1)
    {
        GIZWITS_LOG("valueAHU303_dianJiaRe1 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU303_dianJiaRe1 = 1;
        ret = 1;
    }
    if(last->valueAHU303_dianJiaRe2 != cur->valueAHU303_dianJiaRe2)
    {
        GIZWITS_LOG("valueAHU303_dianJiaRe2 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU303_dianJiaRe2 = 1;
        ret = 1;
    }
    if(last->valueAHU303_dianJiaRe3 != cur->valueAHU303_dianJiaRe3)
    {
        GIZWITS_LOG("valueAHU303_dianJiaRe3 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU303_dianJiaRe3 = 1;
        ret = 1;
    }
    if(last->valueAHU303_fengJiStatus != cur->valueAHU303_fengJiStatus)
    {
        GIZWITS_LOG("valueAHU303_fengJiStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU303_fengJiStatus = 1;
        ret = 1;
    }
    if(last->valueAHU303_shouZiDong != cur->valueAHU303_shouZiDong)
    {
        GIZWITS_LOG("valueAHU303_shouZiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU303_shouZiDong = 1;
        ret = 1;
    }
    if(last->valueAHU303_dongXiaJi != cur->valueAHU303_dongXiaJi)
    {
        GIZWITS_LOG("valueAHU303_dongXiaJi Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU303_dongXiaJi = 1;
        ret = 1;
    }
    if(last->valueAHU303_zhongXiaoBaoJing != cur->valueAHU303_zhongXiaoBaoJing)
    {
        GIZWITS_LOG("valueAHU303_zhongXiaoBaoJing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU303_zhongXiaoBaoJing = 1;
        ret = 1;
    }
    if(last->valueAHU303_dianJiaReGaoWen != cur->valueAHU303_dianJiaReGaoWen)
    {
        GIZWITS_LOG("valueAHU303_dianJiaReGaoWen Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU303_dianJiaReGaoWen = 1;
        ret = 1;
    }
    if(last->valueAHU303_fengJiQueFeng != cur->valueAHU303_fengJiQueFeng)
    {
        GIZWITS_LOG("valueAHU303_fengJiQueFeng Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU303_fengJiQueFeng = 1;
        ret = 1;
    }
    if(last->valueAHU303_paiFengJiYiQiDong != cur->valueAHU303_paiFengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU303_paiFengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU303_paiFengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU303_diWenPanGuan != cur->valueAHU303_diWenPanGuan)
    {
        GIZWITS_LOG("valueAHU303_diWenPanGuan Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU303_diWenPanGuan = 1;
        ret = 1;
    }
    if(last->valueAHU303_mieJunYunXing != cur->valueAHU303_mieJunYunXing)
    {
        GIZWITS_LOG("valueAHU303_mieJunYunXing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU303_mieJunYunXing = 1;
        ret = 1;
    }
    if(last->valueAHU304_fengJiYiQiDong != cur->valueAHU304_fengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU304_fengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU304_fengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU304_zhiBanStatus != cur->valueAHU304_zhiBanStatus)
    {
        GIZWITS_LOG("valueAHU304_zhiBanStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU304_zhiBanStatus = 1;
        ret = 1;
    }
    if(last->valueAHU304_dianJiaRe1 != cur->valueAHU304_dianJiaRe1)
    {
        GIZWITS_LOG("valueAHU304_dianJiaRe1 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU304_dianJiaRe1 = 1;
        ret = 1;
    }
    if(last->valueAHU304_dianJiaRe2 != cur->valueAHU304_dianJiaRe2)
    {
        GIZWITS_LOG("valueAHU304_dianJiaRe2 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU304_dianJiaRe2 = 1;
        ret = 1;
    }
    if(last->valueAHU304_dianJiaRe3 != cur->valueAHU304_dianJiaRe3)
    {
        GIZWITS_LOG("valueAHU304_dianJiaRe3 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU304_dianJiaRe3 = 1;
        ret = 1;
    }
    if(last->valueAHU304_fengJiStatus != cur->valueAHU304_fengJiStatus)
    {
        GIZWITS_LOG("valueAHU304_fengJiStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU304_fengJiStatus = 1;
        ret = 1;
    }
    if(last->valueAHU304_shouZiDong != cur->valueAHU304_shouZiDong)
    {
        GIZWITS_LOG("valueAHU304_shouZiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU304_shouZiDong = 1;
        ret = 1;
    }
    if(last->valueAHU304_dongXiaJi != cur->valueAHU304_dongXiaJi)
    {
        GIZWITS_LOG("valueAHU304_dongXiaJi Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU304_dongXiaJi = 1;
        ret = 1;
    }
    if(last->valueAHU304_zhongXiaoBaoJing != cur->valueAHU304_zhongXiaoBaoJing)
    {
        GIZWITS_LOG("valueAHU304_zhongXiaoBaoJing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU304_zhongXiaoBaoJing = 1;
        ret = 1;
    }
    if(last->valueAHU304_dianJiaReGaoWen != cur->valueAHU304_dianJiaReGaoWen)
    {
        GIZWITS_LOG("valueAHU304_dianJiaReGaoWen Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU304_dianJiaReGaoWen = 1;
        ret = 1;
    }
    if(last->valueAHU304_fengJiQueFeng != cur->valueAHU304_fengJiQueFeng)
    {
        GIZWITS_LOG("valueAHU304_fengJiQueFeng Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU304_fengJiQueFeng = 1;
        ret = 1;
    }
    if(last->valueAHU304_paiFengJiYiQiDong != cur->valueAHU304_paiFengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU304_paiFengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU304_paiFengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU304_diWenPanGuan != cur->valueAHU304_diWenPanGuan)
    {
        GIZWITS_LOG("valueAHU304_diWenPanGuan Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU304_diWenPanGuan = 1;
        ret = 1;
    }
    if(last->valueAHU304_mieJunYunXing != cur->valueAHU304_mieJunYunXing)
    {
        GIZWITS_LOG("valueAHU304_mieJunYunXing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU304_mieJunYunXing = 1;
        ret = 1;
    }
    if(last->valueAHU305_fengJiYiQiDong != cur->valueAHU305_fengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU305_fengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU305_fengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU305_zhiBanStatus != cur->valueAHU305_zhiBanStatus)
    {
        GIZWITS_LOG("valueAHU305_zhiBanStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU305_zhiBanStatus = 1;
        ret = 1;
    }
    if(last->valueAHU305_dianJiaRe1 != cur->valueAHU305_dianJiaRe1)
    {
        GIZWITS_LOG("valueAHU305_dianJiaRe1 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU305_dianJiaRe1 = 1;
        ret = 1;
    }
    if(last->valueAHU305_dianJiaRe2 != cur->valueAHU305_dianJiaRe2)
    {
        GIZWITS_LOG("valueAHU305_dianJiaRe2 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU305_dianJiaRe2 = 1;
        ret = 1;
    }
    if(last->valueAHU305_dianJiaRe3 != cur->valueAHU305_dianJiaRe3)
    {
        GIZWITS_LOG("valueAHU305_dianJiaRe3 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU305_dianJiaRe3 = 1;
        ret = 1;
    }
    if(last->valueAHU305_fengJiStatus != cur->valueAHU305_fengJiStatus)
    {
        GIZWITS_LOG("valueAHU305_fengJiStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU305_fengJiStatus = 1;
        ret = 1;
    }
    if(last->valueAHU305_shouZiDong != cur->valueAHU305_shouZiDong)
    {
        GIZWITS_LOG("valueAHU305_shouZiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU305_shouZiDong = 1;
        ret = 1;
    }
    if(last->valueAHU305_dongXiaJi != cur->valueAHU305_dongXiaJi)
    {
        GIZWITS_LOG("valueAHU305_dongXiaJi Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU305_dongXiaJi = 1;
        ret = 1;
    }
    if(last->valueAHU305_zhongXiaoBaoJing != cur->valueAHU305_zhongXiaoBaoJing)
    {
        GIZWITS_LOG("valueAHU305_zhongXiaoBaoJing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU305_zhongXiaoBaoJing = 1;
        ret = 1;
    }
    if(last->valueAHU305_dianJiaReGaoWen != cur->valueAHU305_dianJiaReGaoWen)
    {
        GIZWITS_LOG("valueAHU305_dianJiaReGaoWen Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU305_dianJiaReGaoWen = 1;
        ret = 1;
    }
    if(last->valueAHU305_fengJiQueFeng != cur->valueAHU305_fengJiQueFeng)
    {
        GIZWITS_LOG("valueAHU305_fengJiQueFeng Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU305_fengJiQueFeng = 1;
        ret = 1;
    }
    if(last->valueAHU305_paiFengJiYiQiDong != cur->valueAHU305_paiFengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU305_paiFengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU305_paiFengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU305_diWenPanGuan != cur->valueAHU305_diWenPanGuan)
    {
        GIZWITS_LOG("valueAHU305_diWenPanGuan Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU305_diWenPanGuan = 1;
        ret = 1;
    }
    if(last->valueAHU305_mieJunYunXing != cur->valueAHU305_mieJunYunXing)
    {
        GIZWITS_LOG("valueAHU305_mieJunYunXing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU305_mieJunYunXing = 1;
        ret = 1;
    }
    if(last->valueAHU306_fengJiYiQiDong != cur->valueAHU306_fengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU306_fengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU306_fengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU306_zhiBanStatus != cur->valueAHU306_zhiBanStatus)
    {
        GIZWITS_LOG("valueAHU306_zhiBanStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU306_zhiBanStatus = 1;
        ret = 1;
    }
    if(last->valueAHU306_dianJiaRe1 != cur->valueAHU306_dianJiaRe1)
    {
        GIZWITS_LOG("valueAHU306_dianJiaRe1 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU306_dianJiaRe1 = 1;
        ret = 1;
    }
    if(last->valueAHU306_dianJiaRe2 != cur->valueAHU306_dianJiaRe2)
    {
        GIZWITS_LOG("valueAHU306_dianJiaRe2 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU306_dianJiaRe2 = 1;
        ret = 1;
    }
    if(last->valueAHU306_dianJiaRe3 != cur->valueAHU306_dianJiaRe3)
    {
        GIZWITS_LOG("valueAHU306_dianJiaRe3 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU306_dianJiaRe3 = 1;
        ret = 1;
    }
    if(last->valueAHU306_fengJiStatus != cur->valueAHU306_fengJiStatus)
    {
        GIZWITS_LOG("valueAHU306_fengJiStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU306_fengJiStatus = 1;
        ret = 1;
    }
    if(last->valueAHU306_shouZiDong != cur->valueAHU306_shouZiDong)
    {
        GIZWITS_LOG("valueAHU306_shouZiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU306_shouZiDong = 1;
        ret = 1;
    }
    if(last->valueAHU306_dongXiaJi != cur->valueAHU306_dongXiaJi)
    {
        GIZWITS_LOG("valueAHU306_dongXiaJi Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU306_dongXiaJi = 1;
        ret = 1;
    }
    if(last->valueAHU306_zhongXiaoBaoJing != cur->valueAHU306_zhongXiaoBaoJing)
    {
        GIZWITS_LOG("valueAHU306_zhongXiaoBaoJing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU306_zhongXiaoBaoJing = 1;
        ret = 1;
    }
    if(last->valueAHU306_dianJiaReGaoWen != cur->valueAHU306_dianJiaReGaoWen)
    {
        GIZWITS_LOG("valueAHU306_dianJiaReGaoWen Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU306_dianJiaReGaoWen = 1;
        ret = 1;
    }
    if(last->valueAHU306_fengJiQueFeng != cur->valueAHU306_fengJiQueFeng)
    {
        GIZWITS_LOG("valueAHU306_fengJiQueFeng Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU306_fengJiQueFeng = 1;
        ret = 1;
    }
    if(last->valueAHU306_paiFengJiYiQiDong != cur->valueAHU306_paiFengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU306_paiFengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU306_paiFengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU306_diWenPanGuan != cur->valueAHU306_diWenPanGuan)
    {
        GIZWITS_LOG("valueAHU306_diWenPanGuan Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU306_diWenPanGuan = 1;
        ret = 1;
    }
    if(last->valueAHU306_mieJunYunXing != cur->valueAHU306_mieJunYunXing)
    {
        GIZWITS_LOG("valueAHU306_mieJunYunXing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU306_mieJunYunXing = 1;
        ret = 1;
    }
    if(last->valueAHU307_fengJiYiQiDong != cur->valueAHU307_fengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU307_fengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU307_fengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU307_zhiBanStatus != cur->valueAHU307_zhiBanStatus)
    {
        GIZWITS_LOG("valueAHU307_zhiBanStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU307_zhiBanStatus = 1;
        ret = 1;
    }
    if(last->valueAHU307_dianJiaRe1 != cur->valueAHU307_dianJiaRe1)
    {
        GIZWITS_LOG("valueAHU307_dianJiaRe1 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU307_dianJiaRe1 = 1;
        ret = 1;
    }
    if(last->valueAHU307_dianJiaRe2 != cur->valueAHU307_dianJiaRe2)
    {
        GIZWITS_LOG("valueAHU307_dianJiaRe2 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU307_dianJiaRe2 = 1;
        ret = 1;
    }
    if(last->valueAHU307_dianJiaRe3 != cur->valueAHU307_dianJiaRe3)
    {
        GIZWITS_LOG("valueAHU307_dianJiaRe3 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU307_dianJiaRe3 = 1;
        ret = 1;
    }
    if(last->valueAHU307_fengJiStatus != cur->valueAHU307_fengJiStatus)
    {
        GIZWITS_LOG("valueAHU307_fengJiStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU307_fengJiStatus = 1;
        ret = 1;
    }
    if(last->valueAHU307_shouZiDong != cur->valueAHU307_shouZiDong)
    {
        GIZWITS_LOG("valueAHU307_shouZiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU307_shouZiDong = 1;
        ret = 1;
    }
    if(last->valueAHU307_dongXiaJi != cur->valueAHU307_dongXiaJi)
    {
        GIZWITS_LOG("valueAHU307_dongXiaJi Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU307_dongXiaJi = 1;
        ret = 1;
    }
    if(last->valueAHU307_zhongXiaoBaoJing != cur->valueAHU307_zhongXiaoBaoJing)
    {
        GIZWITS_LOG("valueAHU307_zhongXiaoBaoJing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU307_zhongXiaoBaoJing = 1;
        ret = 1;
    }
    if(last->valueAHU307_dianJiaReGaoWen != cur->valueAHU307_dianJiaReGaoWen)
    {
        GIZWITS_LOG("valueAHU307_dianJiaReGaoWen Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU307_dianJiaReGaoWen = 1;
        ret = 1;
    }
    if(last->valueAHU307_fengJiQueFeng != cur->valueAHU307_fengJiQueFeng)
    {
        GIZWITS_LOG("valueAHU307_fengJiQueFeng Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU307_fengJiQueFeng = 1;
        ret = 1;
    }
    if(last->valueAHU307_paiFengJiYiQiDong != cur->valueAHU307_paiFengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU307_paiFengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU307_paiFengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU307_diWenPanGuan != cur->valueAHU307_diWenPanGuan)
    {
        GIZWITS_LOG("valueAHU307_diWenPanGuan Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU307_diWenPanGuan = 1;
        ret = 1;
    }
    if(last->valueAHU307_mieJunYunXing != cur->valueAHU307_mieJunYunXing)
    {
        GIZWITS_LOG("valueAHU307_mieJunYunXing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU307_mieJunYunXing = 1;
        ret = 1;
    }
    if(last->valueAHU308_fengJiYiQiDong != cur->valueAHU308_fengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU308_fengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU308_fengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU308_zhiBanStatus != cur->valueAHU308_zhiBanStatus)
    {
        GIZWITS_LOG("valueAHU308_zhiBanStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU308_zhiBanStatus = 1;
        ret = 1;
    }
    if(last->valueAHU308_dianJiaRe1 != cur->valueAHU308_dianJiaRe1)
    {
        GIZWITS_LOG("valueAHU308_dianJiaRe1 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU308_dianJiaRe1 = 1;
        ret = 1;
    }
    if(last->valueAHU308_dianJiaRe2 != cur->valueAHU308_dianJiaRe2)
    {
        GIZWITS_LOG("valueAHU308_dianJiaRe2 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU308_dianJiaRe2 = 1;
        ret = 1;
    }
    if(last->valueAHU308_dianJiaRe3 != cur->valueAHU308_dianJiaRe3)
    {
        GIZWITS_LOG("valueAHU308_dianJiaRe3 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU308_dianJiaRe3 = 1;
        ret = 1;
    }
    if(last->valueAHU308_fengJiStatus != cur->valueAHU308_fengJiStatus)
    {
        GIZWITS_LOG("valueAHU308_fengJiStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU308_fengJiStatus = 1;
        ret = 1;
    }
    if(last->valueAHU308_shouZiDong != cur->valueAHU308_shouZiDong)
    {
        GIZWITS_LOG("valueAHU308_shouZiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU308_shouZiDong = 1;
        ret = 1;
    }
    if(last->valueAHU308_dongXiaJi != cur->valueAHU308_dongXiaJi)
    {
        GIZWITS_LOG("valueAHU308_dongXiaJi Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU308_dongXiaJi = 1;
        ret = 1;
    }
    if(last->valueAHU308_zhongXiaoBaoJing != cur->valueAHU308_zhongXiaoBaoJing)
    {
        GIZWITS_LOG("valueAHU308_zhongXiaoBaoJing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU308_zhongXiaoBaoJing = 1;
        ret = 1;
    }
    if(last->valueAHU308_dianJiaReGaoWen != cur->valueAHU308_dianJiaReGaoWen)
    {
        GIZWITS_LOG("valueAHU308_dianJiaReGaoWen Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU308_dianJiaReGaoWen = 1;
        ret = 1;
    }
    if(last->valueAHU308_fengJiQueFeng != cur->valueAHU308_fengJiQueFeng)
    {
        GIZWITS_LOG("valueAHU308_fengJiQueFeng Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU308_fengJiQueFeng = 1;
        ret = 1;
    }
    if(last->valueAHU308_paiFengJiYiQiDong != cur->valueAHU308_paiFengJiYiQiDong)
    {
        GIZWITS_LOG("valueAHU308_paiFengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU308_paiFengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valueAHU308_diWenPanGuan != cur->valueAHU308_diWenPanGuan)
    {
        GIZWITS_LOG("valueAHU308_diWenPanGuan Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU308_diWenPanGuan = 1;
        ret = 1;
    }
    if(last->valueAHU308_mieJunYunXing != cur->valueAHU308_mieJunYunXing)
    {
        GIZWITS_LOG("valueAHU308_mieJunYunXing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagAHU308_mieJunYunXing = 1;
        ret = 1;
    }
    if(last->valuePAU301_fengJiYiQiDong != cur->valuePAU301_fengJiYiQiDong)
    {
        GIZWITS_LOG("valuePAU301_fengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_fengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valuePAU301_zhiBanStatus != cur->valuePAU301_zhiBanStatus)
    {
        GIZWITS_LOG("valuePAU301_zhiBanStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_zhiBanStatus = 1;
        ret = 1;
    }
    if(last->valuePAU301_dianJiaRe1 != cur->valuePAU301_dianJiaRe1)
    {
        GIZWITS_LOG("valuePAU301_dianJiaRe1 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianJiaRe1 = 1;
        ret = 1;
    }
    if(last->valuePAU301_dianJiaRe2 != cur->valuePAU301_dianJiaRe2)
    {
        GIZWITS_LOG("valuePAU301_dianJiaRe2 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianJiaRe2 = 1;
        ret = 1;
    }
    if(last->valuePAU301_dianJiaRe3 != cur->valuePAU301_dianJiaRe3)
    {
        GIZWITS_LOG("valuePAU301_dianJiaRe3 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianJiaRe3 = 1;
        ret = 1;
    }
    if(last->valuePAU301_fengJiStatus != cur->valuePAU301_fengJiStatus)
    {
        GIZWITS_LOG("valuePAU301_fengJiStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_fengJiStatus = 1;
        ret = 1;
    }
    if(last->valuePAU301_shouZiDong != cur->valuePAU301_shouZiDong)
    {
        GIZWITS_LOG("valuePAU301_shouZiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_shouZiDong = 1;
        ret = 1;
    }
    if(last->valuePAU301_dongXiaJi != cur->valuePAU301_dongXiaJi)
    {
        GIZWITS_LOG("valuePAU301_dongXiaJi Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dongXiaJi = 1;
        ret = 1;
    }
    if(last->valuePAU301_zhongXiaoBaoJing != cur->valuePAU301_zhongXiaoBaoJing)
    {
        GIZWITS_LOG("valuePAU301_zhongXiaoBaoJing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_zhongXiaoBaoJing = 1;
        ret = 1;
    }
    if(last->valuePAU301_dianJiaReGaoWen != cur->valuePAU301_dianJiaReGaoWen)
    {
        GIZWITS_LOG("valuePAU301_dianJiaReGaoWen Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianJiaReGaoWen = 1;
        ret = 1;
    }
    if(last->valuePAU301_fengJiQueFeng != cur->valuePAU301_fengJiQueFeng)
    {
        GIZWITS_LOG("valuePAU301_fengJiQueFeng Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_fengJiQueFeng = 1;
        ret = 1;
    }
    if(last->valuePAU301_paiFengJiYiQiDong != cur->valuePAU301_paiFengJiYiQiDong)
    {
        GIZWITS_LOG("valuePAU301_paiFengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_paiFengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valuePAU301_dianYuRe1 != cur->valuePAU301_dianYuRe1)
    {
        GIZWITS_LOG("valuePAU301_dianYuRe1 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianYuRe1 = 1;
        ret = 1;
    }
    if(last->valuePAU301_dianYuRe2 != cur->valuePAU301_dianYuRe2)
    {
        GIZWITS_LOG("valuePAU301_dianYuRe2 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianYuRe2 = 1;
        ret = 1;
    }
    if(last->valuePAU301_dianYuRe3 != cur->valuePAU301_dianYuRe3)
    {
        GIZWITS_LOG("valuePAU301_dianYuRe3 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianYuRe3 = 1;
        ret = 1;
    }
    if(last->valuePAU301_dianYuReGaoWen != cur->valuePAU301_dianYuReGaoWen)
    {
        GIZWITS_LOG("valuePAU301_dianYuReGaoWen Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianYuReGaoWen = 1;
        ret = 1;
    }
    if(last->valuePAU301_diWenPanGuan != cur->valuePAU301_diWenPanGuan)
    {
        GIZWITS_LOG("valuePAU301_diWenPanGuan Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_diWenPanGuan = 1;
        ret = 1;
    }
    if(last->valuePAU301_mieJunYunXing != cur->valuePAU301_mieJunYunXing)
    {
        GIZWITS_LOG("valuePAU301_mieJunYunXing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU301_mieJunYunXing = 1;
        ret = 1;
    }
    if(last->valuePAU302_fengJiYiQiDong != cur->valuePAU302_fengJiYiQiDong)
    {
        GIZWITS_LOG("valuePAU302_fengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU302_fengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valuePAU302_zhiBanStatus != cur->valuePAU302_zhiBanStatus)
    {
        GIZWITS_LOG("valuePAU302_zhiBanStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU302_zhiBanStatus = 1;
        ret = 1;
    }
    if(last->valuePAU302_fengJiStatus != cur->valuePAU302_fengJiStatus)
    {
        GIZWITS_LOG("valuePAU302_fengJiStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU302_fengJiStatus = 1;
        ret = 1;
    }
    if(last->valuePAU302_shouZiDong != cur->valuePAU302_shouZiDong)
    {
        GIZWITS_LOG("valuePAU302_shouZiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU302_shouZiDong = 1;
        ret = 1;
    }
    if(last->valuePAU302_dongXiaJi != cur->valuePAU302_dongXiaJi)
    {
        GIZWITS_LOG("valuePAU302_dongXiaJi Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU302_dongXiaJi = 1;
        ret = 1;
    }
    if(last->valuePAU302_zhongXiaoBaoJing != cur->valuePAU302_zhongXiaoBaoJing)
    {
        GIZWITS_LOG("valuePAU302_zhongXiaoBaoJing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU302_zhongXiaoBaoJing = 1;
        ret = 1;
    }
    if(last->valuePAU302_fengJiQueFeng != cur->valuePAU302_fengJiQueFeng)
    {
        GIZWITS_LOG("valuePAU302_fengJiQueFeng Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU302_fengJiQueFeng = 1;
        ret = 1;
    }
    if(last->valuePAU302_paiFengJiYiQiDong != cur->valuePAU302_paiFengJiYiQiDong)
    {
        GIZWITS_LOG("valuePAU302_paiFengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU302_paiFengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valuePAU302_dianYuRe1 != cur->valuePAU302_dianYuRe1)
    {
        GIZWITS_LOG("valuePAU302_dianYuRe1 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU302_dianYuRe1 = 1;
        ret = 1;
    }
    if(last->valuePAU302_dianYuRe2 != cur->valuePAU302_dianYuRe2)
    {
        GIZWITS_LOG("valuePAU302_dianYuRe2 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU302_dianYuRe2 = 1;
        ret = 1;
    }
    if(last->valuePAU302_dianYuRe3 != cur->valuePAU302_dianYuRe3)
    {
        GIZWITS_LOG("valuePAU302_dianYuRe3 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU302_dianYuRe3 = 1;
        ret = 1;
    }
    if(last->valuePAU302_dianYuReGaoWen != cur->valuePAU302_dianYuReGaoWen)
    {
        GIZWITS_LOG("valuePAU302_dianYuReGaoWen Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU302_dianYuReGaoWen = 1;
        ret = 1;
    }
    if(last->valuePAU302_diWenPanGuan != cur->valuePAU302_diWenPanGuan)
    {
        GIZWITS_LOG("valuePAU302_diWenPanGuan Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU302_diWenPanGuan = 1;
        ret = 1;
    }
    if(last->valuePAU302_mieJunYunXing != cur->valuePAU302_mieJunYunXing)
    {
        GIZWITS_LOG("valuePAU302_mieJunYunXing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU302_mieJunYunXing = 1;
        ret = 1;
    }
    if(last->valuePAU303_fengJiYiQiDong != cur->valuePAU303_fengJiYiQiDong)
    {
        GIZWITS_LOG("valuePAU303_fengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU303_fengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valuePAU303_zhiBanStatus != cur->valuePAU303_zhiBanStatus)
    {
        GIZWITS_LOG("valuePAU303_zhiBanStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU303_zhiBanStatus = 1;
        ret = 1;
    }
    if(last->valuePAU303_fengJiStatus != cur->valuePAU303_fengJiStatus)
    {
        GIZWITS_LOG("valuePAU303_fengJiStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU303_fengJiStatus = 1;
        ret = 1;
    }
    if(last->valuePAU303_shouZiDong != cur->valuePAU303_shouZiDong)
    {
        GIZWITS_LOG("valuePAU303_shouZiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU303_shouZiDong = 1;
        ret = 1;
    }
    if(last->valuePAU303_dongXiaJi != cur->valuePAU303_dongXiaJi)
    {
        GIZWITS_LOG("valuePAU303_dongXiaJi Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU303_dongXiaJi = 1;
        ret = 1;
    }
    if(last->valuePAU303_zhongXiaoBaoJing != cur->valuePAU303_zhongXiaoBaoJing)
    {
        GIZWITS_LOG("valuePAU303_zhongXiaoBaoJing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU303_zhongXiaoBaoJing = 1;
        ret = 1;
    }
    if(last->valuePAU303_fengJiQueFeng != cur->valuePAU303_fengJiQueFeng)
    {
        GIZWITS_LOG("valuePAU303_fengJiQueFeng Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU303_fengJiQueFeng = 1;
        ret = 1;
    }
    if(last->valuePAU303_paiFengJiYiQiDong != cur->valuePAU303_paiFengJiYiQiDong)
    {
        GIZWITS_LOG("valuePAU303_paiFengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU303_paiFengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valuePAU303_dianYuRe1 != cur->valuePAU303_dianYuRe1)
    {
        GIZWITS_LOG("valuePAU303_dianYuRe1 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU303_dianYuRe1 = 1;
        ret = 1;
    }
    if(last->valuePAU303_dianYuRe2 != cur->valuePAU303_dianYuRe2)
    {
        GIZWITS_LOG("valuePAU303_dianYuRe2 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU303_dianYuRe2 = 1;
        ret = 1;
    }
    if(last->valuePAU303_dianYuRe3 != cur->valuePAU303_dianYuRe3)
    {
        GIZWITS_LOG("valuePAU303_dianYuRe3 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU303_dianYuRe3 = 1;
        ret = 1;
    }
    if(last->valuePAU303_dianYuReGaoWen != cur->valuePAU303_dianYuReGaoWen)
    {
        GIZWITS_LOG("valuePAU303_dianYuReGaoWen Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU303_dianYuReGaoWen = 1;
        ret = 1;
    }
    if(last->valuePAU303_diWenPanGuan != cur->valuePAU303_diWenPanGuan)
    {
        GIZWITS_LOG("valuePAU303_diWenPanGuan Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU303_diWenPanGuan = 1;
        ret = 1;
    }
    if(last->valuePAU303_mieJunYunXing != cur->valuePAU303_mieJunYunXing)
    {
        GIZWITS_LOG("valuePAU303_mieJunYunXing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU303_mieJunYunXing = 1;
        ret = 1;
    }
    if(last->valuePAU304_fengJiYiQiDong != cur->valuePAU304_fengJiYiQiDong)
    {
        GIZWITS_LOG("valuePAU304_fengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU304_fengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valuePAU304_zhiBanStatus != cur->valuePAU304_zhiBanStatus)
    {
        GIZWITS_LOG("valuePAU304_zhiBanStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU304_zhiBanStatus = 1;
        ret = 1;
    }
    if(last->valuePAU304_fengJiStatus != cur->valuePAU304_fengJiStatus)
    {
        GIZWITS_LOG("valuePAU304_fengJiStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU304_fengJiStatus = 1;
        ret = 1;
    }
    if(last->valuePAU304_shouZiDong != cur->valuePAU304_shouZiDong)
    {
        GIZWITS_LOG("valuePAU304_shouZiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU304_shouZiDong = 1;
        ret = 1;
    }
    if(last->valuePAU304_dongXiaJi != cur->valuePAU304_dongXiaJi)
    {
        GIZWITS_LOG("valuePAU304_dongXiaJi Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU304_dongXiaJi = 1;
        ret = 1;
    }
    if(last->valuePAU304_zhongXiaoBaoJing != cur->valuePAU304_zhongXiaoBaoJing)
    {
        GIZWITS_LOG("valuePAU304_zhongXiaoBaoJing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU304_zhongXiaoBaoJing = 1;
        ret = 1;
    }
    if(last->valuePAU304_fengJiQueFeng != cur->valuePAU304_fengJiQueFeng)
    {
        GIZWITS_LOG("valuePAU304_fengJiQueFeng Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU304_fengJiQueFeng = 1;
        ret = 1;
    }
    if(last->valuePAU304_paiFengJiYiQiDong != cur->valuePAU304_paiFengJiYiQiDong)
    {
        GIZWITS_LOG("valuePAU304_paiFengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU304_paiFengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valuePAU304_dianYuRe1 != cur->valuePAU304_dianYuRe1)
    {
        GIZWITS_LOG("valuePAU304_dianYuRe1 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU304_dianYuRe1 = 1;
        ret = 1;
    }
    if(last->valuePAU304_dianYuRe2 != cur->valuePAU304_dianYuRe2)
    {
        GIZWITS_LOG("valuePAU304_dianYuRe2 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU304_dianYuRe2 = 1;
        ret = 1;
    }
    if(last->valuePAU304_dianYuRe3 != cur->valuePAU304_dianYuRe3)
    {
        GIZWITS_LOG("valuePAU304_dianYuRe3 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU304_dianYuRe3 = 1;
        ret = 1;
    }
    if(last->valuePAU304_dianYuReGaoWen != cur->valuePAU304_dianYuReGaoWen)
    {
        GIZWITS_LOG("valuePAU304_dianYuReGaoWen Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU304_dianYuReGaoWen = 1;
        ret = 1;
    }
    if(last->valuePAU304_diWenPanGuan != cur->valuePAU304_diWenPanGuan)
    {
        GIZWITS_LOG("valuePAU304_diWenPanGuan Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU304_diWenPanGuan = 1;
        ret = 1;
    }
    if(last->valuePAU304_mieJunYunXing != cur->valuePAU304_mieJunYunXing)
    {
        GIZWITS_LOG("valuePAU304_mieJunYunXing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU304_mieJunYunXing = 1;
        ret = 1;
    }
    if(last->valuePAU305_fengJiYiQiDong != cur->valuePAU305_fengJiYiQiDong)
    {
        GIZWITS_LOG("valuePAU305_fengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU305_fengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valuePAU305_zhiBanStatus != cur->valuePAU305_zhiBanStatus)
    {
        GIZWITS_LOG("valuePAU305_zhiBanStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU305_zhiBanStatus = 1;
        ret = 1;
    }
    if(last->valuePAU305_fengJiStatus != cur->valuePAU305_fengJiStatus)
    {
        GIZWITS_LOG("valuePAU305_fengJiStatus Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU305_fengJiStatus = 1;
        ret = 1;
    }
    if(last->valuePAU305_shouZiDong != cur->valuePAU305_shouZiDong)
    {
        GIZWITS_LOG("valuePAU305_shouZiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU305_shouZiDong = 1;
        ret = 1;
    }
    if(last->valuePAU305_dongXiaJi != cur->valuePAU305_dongXiaJi)
    {
        GIZWITS_LOG("valuePAU305_dongXiaJi Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU305_dongXiaJi = 1;
        ret = 1;
    }
    if(last->valuePAU305_zhongXiaoBaoJing != cur->valuePAU305_zhongXiaoBaoJing)
    {
        GIZWITS_LOG("valuePAU305_zhongXiaoBaoJing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU305_zhongXiaoBaoJing = 1;
        ret = 1;
    }
    if(last->valuePAU305_fengJiQueFeng != cur->valuePAU305_fengJiQueFeng)
    {
        GIZWITS_LOG("valuePAU305_fengJiQueFeng Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU305_fengJiQueFeng = 1;
        ret = 1;
    }
    if(last->valuePAU305_paiFengJiYiQiDong != cur->valuePAU305_paiFengJiYiQiDong)
    {
        GIZWITS_LOG("valuePAU305_paiFengJiYiQiDong Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU305_paiFengJiYiQiDong = 1;
        ret = 1;
    }
    if(last->valuePAU305_dianYuRe1 != cur->valuePAU305_dianYuRe1)
    {
        GIZWITS_LOG("valuePAU305_dianYuRe1 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU305_dianYuRe1 = 1;
        ret = 1;
    }
    if(last->valuePAU305_dianYuRe2 != cur->valuePAU305_dianYuRe2)
    {
        GIZWITS_LOG("valuePAU305_dianYuRe2 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU305_dianYuRe2 = 1;
        ret = 1;
    }
    if(last->valuePAU305_dianYuRe3 != cur->valuePAU305_dianYuRe3)
    {
        GIZWITS_LOG("valuePAU305_dianYuRe3 Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU305_dianYuRe3 = 1;
        ret = 1;
    }
    if(last->valuePAU305_dianYuReGaoWen != cur->valuePAU305_dianYuReGaoWen)
    {
        GIZWITS_LOG("valuePAU305_dianYuReGaoWen Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU305_dianYuReGaoWen = 1;
        ret = 1;
    }
    if(last->valuePAU305_diWenPanGuan != cur->valuePAU305_diWenPanGuan)
    {
        GIZWITS_LOG("valuePAU305_diWenPanGuan Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU305_diWenPanGuan = 1;
        ret = 1;
    }
    if(last->valuePAU305_mieJunYunXing != cur->valuePAU305_mieJunYunXing)
    {
        GIZWITS_LOG("valuePAU305_mieJunYunXing Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagPAU305_mieJunYunXing = 1;
        ret = 1;
    }
    if(last->valueonline_flag != cur->valueonline_flag)
    {
        GIZWITS_LOG("valueonline_flag Changed\n");
        gizwitsProtocol.waitReportDatapointFlag.flagonline_flag = 1;
        ret = 1;
    }

    if(last->valueAHU301_mianBanTongXunZhuangTai1 != cur->valueAHU301_mianBanTongXunZhuangTai1)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU301_mianBanTongXunZhuangTai1 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU301_mianBanTongXunZhuangTai1 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU301_mianBanTongXunZhuangTai2 != cur->valueAHU301_mianBanTongXunZhuangTai2)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU301_mianBanTongXunZhuangTai2 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU301_mianBanTongXunZhuangTai2 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU301_mianBanTongXunZhuangTai3 != cur->valueAHU301_mianBanTongXunZhuangTai3)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU301_mianBanTongXunZhuangTai3 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU301_mianBanTongXunZhuangTai3 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU302_mianBanTongXunZhuangTai1 != cur->valueAHU302_mianBanTongXunZhuangTai1)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU302_mianBanTongXunZhuangTai1 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU302_mianBanTongXunZhuangTai1 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU302_mianBanTongXunZhuangTai2 != cur->valueAHU302_mianBanTongXunZhuangTai2)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU302_mianBanTongXunZhuangTai2 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU302_mianBanTongXunZhuangTai2 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU302_mianBanTongXunZhuangTai3 != cur->valueAHU302_mianBanTongXunZhuangTai3)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU302_mianBanTongXunZhuangTai3 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU302_mianBanTongXunZhuangTai3 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU303_mianBanTongXunZhuangTai1 != cur->valueAHU303_mianBanTongXunZhuangTai1)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU303_mianBanTongXunZhuangTai1 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU303_mianBanTongXunZhuangTai1 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU303_mianBanTongXunZhuangTai2 != cur->valueAHU303_mianBanTongXunZhuangTai2)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU303_mianBanTongXunZhuangTai2 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU303_mianBanTongXunZhuangTai2 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU303_mianBanTongXunZhuangTai3 != cur->valueAHU303_mianBanTongXunZhuangTai3)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU303_mianBanTongXunZhuangTai3 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU303_mianBanTongXunZhuangTai3 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU304_mianBanTongXunZhuangTai1 != cur->valueAHU304_mianBanTongXunZhuangTai1)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU304_mianBanTongXunZhuangTai1 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU304_mianBanTongXunZhuangTai1 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU304_mianBanTongXunZhuangTai2 != cur->valueAHU304_mianBanTongXunZhuangTai2)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU304_mianBanTongXunZhuangTai2 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU304_mianBanTongXunZhuangTai2 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU304_mianBanTongXunZhuangTai3 != cur->valueAHU304_mianBanTongXunZhuangTai3)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU304_mianBanTongXunZhuangTai3 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU304_mianBanTongXunZhuangTai3 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU305_mianBanTongXunZhuangTai1 != cur->valueAHU305_mianBanTongXunZhuangTai1)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU305_mianBanTongXunZhuangTai1 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU305_mianBanTongXunZhuangTai1 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU305_mianBanTongXunZhuangTai2 != cur->valueAHU305_mianBanTongXunZhuangTai2)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU305_mianBanTongXunZhuangTai2 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU305_mianBanTongXunZhuangTai2 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU305_mianBanTongXunZhuangTai3 != cur->valueAHU305_mianBanTongXunZhuangTai3)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU305_mianBanTongXunZhuangTai3 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU305_mianBanTongXunZhuangTai3 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU306_mianBanTongXunZhuangTai1 != cur->valueAHU306_mianBanTongXunZhuangTai1)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU306_mianBanTongXunZhuangTai1 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU306_mianBanTongXunZhuangTai1 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU306_mianBanTongXunZhuangTai2 != cur->valueAHU306_mianBanTongXunZhuangTai2)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU306_mianBanTongXunZhuangTai2 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU306_mianBanTongXunZhuangTai2 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU306_mianBanTongXunZhuangTai3 != cur->valueAHU306_mianBanTongXunZhuangTai3)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU306_mianBanTongXunZhuangTai3 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU306_mianBanTongXunZhuangTai3 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU307_mianBanTongXunZhuangTai1 != cur->valueAHU307_mianBanTongXunZhuangTai1)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU307_mianBanTongXunZhuangTai1 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU307_mianBanTongXunZhuangTai1 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU307_mianBanTongXunZhuangTai2 != cur->valueAHU307_mianBanTongXunZhuangTai2)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU307_mianBanTongXunZhuangTai2 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU307_mianBanTongXunZhuangTai2 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU307_mianBanTongXunZhuangTai3 != cur->valueAHU307_mianBanTongXunZhuangTai3)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU307_mianBanTongXunZhuangTai3 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU307_mianBanTongXunZhuangTai3 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU308_mianBanTongXunZhuangTai1 != cur->valueAHU308_mianBanTongXunZhuangTai1)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU308_mianBanTongXunZhuangTai1 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU308_mianBanTongXunZhuangTai1 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU308_mianBanTongXunZhuangTai2 != cur->valueAHU308_mianBanTongXunZhuangTai2)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU308_mianBanTongXunZhuangTai2 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU308_mianBanTongXunZhuangTai2 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU308_mianBanTongXunZhuangTai3 != cur->valueAHU308_mianBanTongXunZhuangTai3)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU308_mianBanTongXunZhuangTai3 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU308_mianBanTongXunZhuangTai3 = 1;
            ret = 1;
        }
    }
    if(last->valuePAU301_mianBanTongXunZhuangTai1 != cur->valuePAU301_mianBanTongXunZhuangTai1)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU301_mianBanTongXunZhuangTai1 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU301_mianBanTongXunZhuangTai1 = 1;
            ret = 1;
        }
    }
    if(last->valuePAU301_mianBanTongXunZhuangTai2 != cur->valuePAU301_mianBanTongXunZhuangTai2)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU301_mianBanTongXunZhuangTai2 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU301_mianBanTongXunZhuangTai2 = 1;
            ret = 1;
        }
    }
    if(last->valuePAU301_mianBanTongXunZhuangTai3 != cur->valuePAU301_mianBanTongXunZhuangTai3)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU301_mianBanTongXunZhuangTai3 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU301_mianBanTongXunZhuangTai3 = 1;
            ret = 1;
        }
    }
    if(last->valuePAU302_mianBanTongXunZhuangTai1 != cur->valuePAU302_mianBanTongXunZhuangTai1)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU302_mianBanTongXunZhuangTai1 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU302_mianBanTongXunZhuangTai1 = 1;
            ret = 1;
        }
    }
    if(last->valuePAU302_mianBanTongXunZhuangTai2 != cur->valuePAU302_mianBanTongXunZhuangTai2)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU302_mianBanTongXunZhuangTai2 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU302_mianBanTongXunZhuangTai2 = 1;
            ret = 1;
        }
    }
    if(last->valuePAU302_mianBanTongXunZhuangTai3 != cur->valuePAU302_mianBanTongXunZhuangTai3)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU302_mianBanTongXunZhuangTai3 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU302_mianBanTongXunZhuangTai3 = 1;
            ret = 1;
        }
    }
    if(last->valuePAU303_mianBanTongXunZhuangTai1 != cur->valuePAU303_mianBanTongXunZhuangTai1)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU303_mianBanTongXunZhuangTai1 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU303_mianBanTongXunZhuangTai1 = 1;
            ret = 1;
        }
    }
    if(last->valuePAU303_mianBanTongXunZhuangTai2 != cur->valuePAU303_mianBanTongXunZhuangTai2)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU303_mianBanTongXunZhuangTai2 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU303_mianBanTongXunZhuangTai2 = 1;
            ret = 1;
        }
    }
    if(last->valuePAU303_mianBanTongXunZhuangTai3 != cur->valuePAU303_mianBanTongXunZhuangTai3)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU303_mianBanTongXunZhuangTai3 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU303_mianBanTongXunZhuangTai3 = 1;
            ret = 1;
        }
    }
    if(last->valuePAU304_mianBanTongXunZhuangTai1 != cur->valuePAU304_mianBanTongXunZhuangTai1)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU304_mianBanTongXunZhuangTai1 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU304_mianBanTongXunZhuangTai1 = 1;
            ret = 1;
        }
    }
    if(last->valuePAU304_mianBanTongXunZhuangTai2 != cur->valuePAU304_mianBanTongXunZhuangTai2)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU304_mianBanTongXunZhuangTai2 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU304_mianBanTongXunZhuangTai2 = 1;
            ret = 1;
        }
    }
    if(last->valuePAU304_mianBanTongXunZhuangTai3 != cur->valuePAU304_mianBanTongXunZhuangTai3)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU304_mianBanTongXunZhuangTai3 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU304_mianBanTongXunZhuangTai3 = 1;
            ret = 1;
        }
    }
    if(last->valuePAU305_mianBanTongXunZhuangTai1 != cur->valuePAU305_mianBanTongXunZhuangTai1)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU305_mianBanTongXunZhuangTai1 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU305_mianBanTongXunZhuangTai1 = 1;
            ret = 1;
        }
    }
    if(last->valuePAU305_mianBanTongXunZhuangTai2 != cur->valuePAU305_mianBanTongXunZhuangTai2)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU305_mianBanTongXunZhuangTai2 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU305_mianBanTongXunZhuangTai2 = 1;
            ret = 1;
        }
    }
    if(last->valuePAU305_mianBanTongXunZhuangTai3 != cur->valuePAU305_mianBanTongXunZhuangTai3)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU305_mianBanTongXunZhuangTai3 Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU305_mianBanTongXunZhuangTai3 = 1;
            ret = 1;
        }
    }
    if(last->valueAHU301_tempReal != cur->valueAHU301_tempReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU301_tempReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU301_tempReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU301_humiReal != cur->valueAHU301_humiReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU301_humiReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU301_humiReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU301_tempSet != cur->valueAHU301_tempSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU301_tempSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU301_tempSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU301_humiSet != cur->valueAHU301_humiSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU301_humiSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU301_humiSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU301_lengShuiFaKaiDu != cur->valueAHU301_lengShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU301_lengShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU301_lengShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU301_reShuiFaKaiDu != cur->valueAHU301_reShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU301_reShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU301_reShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU301_xinFengWenDU != cur->valueAHU301_xinFengWenDU)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU301_xinFengWenDU Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU301_xinFengWenDU = 1;
            ret = 1;
        }
    }
    if(last->valueAHU301_jiaShiQIKaiDu != cur->valueAHU301_jiaShiQIKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU301_jiaShiQIKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU301_jiaShiQIKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU301_beiYong != cur->valueAHU301_beiYong)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU301_beiYong Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU301_beiYong = 1;
            ret = 1;
        }
    }
    if(last->valueAHU302_tempReal != cur->valueAHU302_tempReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU302_tempReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU302_tempReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU302_humiReal != cur->valueAHU302_humiReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU302_humiReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU302_humiReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU302_tempSet != cur->valueAHU302_tempSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU302_tempSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU302_tempSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU302_humiSet != cur->valueAHU302_humiSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU302_humiSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU302_humiSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU302_lengShuiFaKaiDu != cur->valueAHU302_lengShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU302_lengShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU302_lengShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU302_reShuiFaKaiDu != cur->valueAHU302_reShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU302_reShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU302_reShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU302_xinFengWenDU != cur->valueAHU302_xinFengWenDU)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU302_xinFengWenDU Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU302_xinFengWenDU = 1;
            ret = 1;
        }
    }
    if(last->valueAHU302_jiaShiQIKaiDu != cur->valueAHU302_jiaShiQIKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU302_jiaShiQIKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU302_jiaShiQIKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU302_beiYong != cur->valueAHU302_beiYong)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU302_beiYong Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU302_beiYong = 1;
            ret = 1;
        }
    }
    if(last->valueAHU303_tempReal != cur->valueAHU303_tempReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU303_tempReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU303_tempReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU303_humiReal != cur->valueAHU303_humiReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU303_humiReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU303_humiReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU303_tempSet != cur->valueAHU303_tempSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU303_tempSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU303_tempSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU303_humiSet != cur->valueAHU303_humiSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU303_humiSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU303_humiSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU303_lengShuiFaKaiDu != cur->valueAHU303_lengShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU303_lengShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU303_lengShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU303_reShuiFaKaiDu != cur->valueAHU303_reShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU303_reShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU303_reShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU303_xinFengWenDU != cur->valueAHU303_xinFengWenDU)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU303_xinFengWenDU Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU303_xinFengWenDU = 1;
            ret = 1;
        }
    }
    if(last->valueAHU303_jiaShiQIKaiDu != cur->valueAHU303_jiaShiQIKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU303_jiaShiQIKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU303_jiaShiQIKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU303_beiYong != cur->valueAHU303_beiYong)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU303_beiYong Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU303_beiYong = 1;
            ret = 1;
        }
    }
    if(last->valueAHU304_tempReal != cur->valueAHU304_tempReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU304_tempReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU304_tempReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU304_humiReal != cur->valueAHU304_humiReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU304_humiReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU304_humiReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU304_tempSet != cur->valueAHU304_tempSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU304_tempSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU304_tempSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU304_humiSet != cur->valueAHU304_humiSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU304_humiSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU304_humiSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU304_lengShuiFaKaiDu != cur->valueAHU304_lengShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU304_lengShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU304_lengShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU304_reShuiFaKaiDu != cur->valueAHU304_reShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU304_reShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU304_reShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU304_xinFengWenDU != cur->valueAHU304_xinFengWenDU)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU304_xinFengWenDU Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU304_xinFengWenDU = 1;
            ret = 1;
        }
    }
    if(last->valueAHU304_jiaShiQIKaiDu != cur->valueAHU304_jiaShiQIKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU304_jiaShiQIKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU304_jiaShiQIKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU304_beiYong != cur->valueAHU304_beiYong)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU304_beiYong Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU304_beiYong = 1;
            ret = 1;
        }
    }
    if(last->valueAHU305_tempReal != cur->valueAHU305_tempReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU305_tempReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU305_tempReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU305_humiReal != cur->valueAHU305_humiReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU305_humiReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU305_humiReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU305_tempSet != cur->valueAHU305_tempSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU305_tempSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU305_tempSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU305_humiSet != cur->valueAHU305_humiSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU305_humiSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU305_humiSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU305_lengShuiFaKaiDu != cur->valueAHU305_lengShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU305_lengShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU305_lengShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU305_reShuiFaKaiDu != cur->valueAHU305_reShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU305_reShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU305_reShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU305_xinFengWenDU != cur->valueAHU305_xinFengWenDU)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU305_xinFengWenDU Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU305_xinFengWenDU = 1;
            ret = 1;
        }
    }
    if(last->valueAHU305_jiaShiQIKaiDu != cur->valueAHU305_jiaShiQIKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU305_jiaShiQIKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU305_jiaShiQIKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU305_beiYong != cur->valueAHU305_beiYong)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU305_beiYong Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU305_beiYong = 1;
            ret = 1;
        }
    }
    if(last->valueAHU306_tempReal != cur->valueAHU306_tempReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU306_tempReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU306_tempReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU306_humiReal != cur->valueAHU306_humiReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU306_humiReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU306_humiReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU306_tempSet != cur->valueAHU306_tempSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU306_tempSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU306_tempSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU306_humiSet != cur->valueAHU306_humiSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU306_humiSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU306_humiSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU306_lengShuiFaKaiDu != cur->valueAHU306_lengShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU306_lengShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU306_lengShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU306_reShuiFaKaiDu != cur->valueAHU306_reShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU306_reShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU306_reShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU306_xinFengWenDU != cur->valueAHU306_xinFengWenDU)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU306_xinFengWenDU Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU306_xinFengWenDU = 1;
            ret = 1;
        }
    }
    if(last->valueAHU306_jiaShiQIKaiDu != cur->valueAHU306_jiaShiQIKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU306_jiaShiQIKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU306_jiaShiQIKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU306_beiYong != cur->valueAHU306_beiYong)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU306_beiYong Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU306_beiYong = 1;
            ret = 1;
        }
    }
    if(last->valueAHU307_tempReal != cur->valueAHU307_tempReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU307_tempReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU307_tempReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU307_humiReal != cur->valueAHU307_humiReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU307_humiReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU307_humiReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU307_tempSet != cur->valueAHU307_tempSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU307_tempSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU307_tempSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU307_humiSet != cur->valueAHU307_humiSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU307_humiSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU307_humiSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU307_lengShuiFaKaiDu != cur->valueAHU307_lengShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU307_lengShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU307_lengShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU307_reShuiFaKaiDu != cur->valueAHU307_reShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU307_reShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU307_reShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU307_xinFengWenDU != cur->valueAHU307_xinFengWenDU)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU307_xinFengWenDU Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU307_xinFengWenDU = 1;
            ret = 1;
        }
    }
    if(last->valueAHU307_jiaShiQIKaiDu != cur->valueAHU307_jiaShiQIKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU307_jiaShiQIKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU307_jiaShiQIKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU307_beiYong != cur->valueAHU307_beiYong)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU307_beiYong Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU307_beiYong = 1;
            ret = 1;
        }
    }
    if(last->valueAHU308_tempReal != cur->valueAHU308_tempReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU308_tempReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU308_tempReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU308_humiReal != cur->valueAHU308_humiReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU308_humiReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU308_humiReal = 1;
            ret = 1;
        }
    }
    if(last->valueAHU308_tempSet != cur->valueAHU308_tempSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU308_tempSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU308_tempSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU308_humiSet != cur->valueAHU308_humiSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU308_humiSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU308_humiSet = 1;
            ret = 1;
        }
    }
    if(last->valueAHU308_lengShuiFaKaiDu != cur->valueAHU308_lengShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU308_lengShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU308_lengShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU308_reShuiFaKaiDu != cur->valueAHU308_reShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU308_reShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU308_reShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU308_xinFengWenDU != cur->valueAHU308_xinFengWenDU)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU308_xinFengWenDU Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU308_xinFengWenDU = 1;
            ret = 1;
        }
    }
    if(last->valueAHU308_jiaShiQIKaiDu != cur->valueAHU308_jiaShiQIKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU308_jiaShiQIKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU308_jiaShiQIKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valueAHU308_beiYong != cur->valueAHU308_beiYong)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valueAHU308_beiYong Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagAHU308_beiYong = 1;
            ret = 1;
        }
    }
    if(last->valuePAU301_tempReal != cur->valuePAU301_tempReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU301_tempReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU301_tempReal = 1;
            ret = 1;
        }
    }
    if(last->valuePAU301_humiReal != cur->valuePAU301_humiReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU301_humiReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU301_humiReal = 1;
            ret = 1;
        }
    }
    if(last->valuePAU301_tempSet != cur->valuePAU301_tempSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU301_tempSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU301_tempSet = 1;
            ret = 1;
        }
    }
    if(last->valuePAU301_humiSet != cur->valuePAU301_humiSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU301_humiSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU301_humiSet = 1;
            ret = 1;
        }
    }
    if(last->valuePAU301_lengShuiFaKaiDu != cur->valuePAU301_lengShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU301_lengShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU301_lengShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valuePAU301_reShuiFaKaiDu != cur->valuePAU301_reShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU301_reShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU301_reShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valuePAU301_xinFengWenDU != cur->valuePAU301_xinFengWenDU)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU301_xinFengWenDU Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU301_xinFengWenDU = 1;
            ret = 1;
        }
    }
    if(last->valuePAU301_jiaShiQIKaiDu != cur->valuePAU301_jiaShiQIKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU301_jiaShiQIKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU301_jiaShiQIKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valuePAU301_beiYong != cur->valuePAU301_beiYong)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU301_beiYong Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU301_beiYong = 1;
            ret = 1;
        }
    }
    if(last->valuePAU302_tempReal != cur->valuePAU302_tempReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU302_tempReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU302_tempReal = 1;
            ret = 1;
        }
    }
    if(last->valuePAU302_humiReal != cur->valuePAU302_humiReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU302_humiReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU302_humiReal = 1;
            ret = 1;
        }
    }
    if(last->valuePAU302_tempSet != cur->valuePAU302_tempSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU302_tempSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU302_tempSet = 1;
            ret = 1;
        }
    }
    if(last->valuePAU302_humiSet != cur->valuePAU302_humiSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU302_humiSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU302_humiSet = 1;
            ret = 1;
        }
    }
    if(last->valuePAU302_lengShuiFaKaiDu != cur->valuePAU302_lengShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU302_lengShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU302_lengShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valuePAU302_reShuiFaKaiDu != cur->valuePAU302_reShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU302_reShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU302_reShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valuePAU302_xinFengWenDU != cur->valuePAU302_xinFengWenDU)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU302_xinFengWenDU Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU302_xinFengWenDU = 1;
            ret = 1;
        }
    }
    if(last->valuePAU302_jiaShiQIKaiDu != cur->valuePAU302_jiaShiQIKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU302_jiaShiQIKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU302_jiaShiQIKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valuePAU302_beiYong != cur->valuePAU302_beiYong)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU302_beiYong Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU302_beiYong = 1;
            ret = 1;
        }
    }
    if(last->valuePAU303_tempReal != cur->valuePAU303_tempReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU303_tempReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU303_tempReal = 1;
            ret = 1;
        }
    }
    if(last->valuePAU303_humiReal != cur->valuePAU303_humiReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU303_humiReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU303_humiReal = 1;
            ret = 1;
        }
    }
    if(last->valuePAU303_tempSet != cur->valuePAU303_tempSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU303_tempSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU303_tempSet = 1;
            ret = 1;
        }
    }
    if(last->valuePAU303_humiSet != cur->valuePAU303_humiSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU303_humiSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU303_humiSet = 1;
            ret = 1;
        }
    }
    if(last->valuePAU303_lengShuiFaKaiDu != cur->valuePAU303_lengShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU303_lengShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU303_lengShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valuePAU303_reShuiFaKaiDu != cur->valuePAU303_reShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU303_reShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU303_reShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valuePAU303_xinFengWenDU != cur->valuePAU303_xinFengWenDU)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU303_xinFengWenDU Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU303_xinFengWenDU = 1;
            ret = 1;
        }
    }
    if(last->valuePAU303_jiaShiQIKaiDu != cur->valuePAU303_jiaShiQIKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU303_jiaShiQIKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU303_jiaShiQIKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valuePAU303_beiYong != cur->valuePAU303_beiYong)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU303_beiYong Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU303_beiYong = 1;
            ret = 1;
        }
    }
    if(last->valuePAU304_tempReal != cur->valuePAU304_tempReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU304_tempReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU304_tempReal = 1;
            ret = 1;
        }
    }
    if(last->valuePAU304_humiReal != cur->valuePAU304_humiReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU304_humiReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU304_humiReal = 1;
            ret = 1;
        }
    }
    if(last->valuePAU304_tempSet != cur->valuePAU304_tempSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU304_tempSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU304_tempSet = 1;
            ret = 1;
        }
    }
    if(last->valuePAU304_humiSet != cur->valuePAU304_humiSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU304_humiSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU304_humiSet = 1;
            ret = 1;
        }
    }
    if(last->valuePAU304_lengShuiFaKaiDu != cur->valuePAU304_lengShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU304_lengShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU304_lengShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valuePAU304_reShuiFaKaiDu != cur->valuePAU304_reShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU304_reShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU304_reShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valuePAU304_xinFengWenDU != cur->valuePAU304_xinFengWenDU)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU304_xinFengWenDU Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU304_xinFengWenDU = 1;
            ret = 1;
        }
    }
    if(last->valuePAU304_jiaShiQIKaiDu != cur->valuePAU304_jiaShiQIKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU304_jiaShiQIKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU304_jiaShiQIKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valuePAU304_beiYong != cur->valuePAU304_beiYong)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU304_beiYong Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU304_beiYong = 1;
            ret = 1;
        }
    }
    if(last->valuePAU305_tempReal != cur->valuePAU305_tempReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU305_tempReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU305_tempReal = 1;
            ret = 1;
        }
    }
    if(last->valuePAU305_humiReal != cur->valuePAU305_humiReal)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU305_humiReal Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU305_humiReal = 1;
            ret = 1;
        }
    }
    if(last->valuePAU305_tempSet != cur->valuePAU305_tempSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU305_tempSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU305_tempSet = 1;
            ret = 1;
        }
    }
    if(last->valuePAU305_humiSet != cur->valuePAU305_humiSet)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU305_humiSet Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU305_humiSet = 1;
            ret = 1;
        }
    }
    if(last->valuePAU305_lengShuiFaKaiDu != cur->valuePAU305_lengShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU305_lengShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU305_lengShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valuePAU305_reShuiFaKaiDu != cur->valuePAU305_reShuiFaKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU305_reShuiFaKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU305_reShuiFaKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valuePAU305_xinFengWenDU != cur->valuePAU305_xinFengWenDU)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU305_xinFengWenDU Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU305_xinFengWenDU = 1;
            ret = 1;
        }
    }
    if(last->valuePAU305_jiaShiQIKaiDu != cur->valuePAU305_jiaShiQIKaiDu)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU305_jiaShiQIKaiDu Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU305_jiaShiQIKaiDu = 1;
            ret = 1;
        }
    }
    if(last->valuePAU305_beiYong != cur->valuePAU305_beiYong)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            GIZWITS_LOG("valuePAU305_beiYong Changed\n");
            gizwitsProtocol.waitReportDatapointFlag.flagPAU305_beiYong = 1;
            ret = 1;
        }
    }

    if(1 == ret)
    {
        lastReportTime = gizGetTimerCount();
    }

    return ret;
}

/**
* @brief User data point data is converted to wit the cloud to report data point data
*
* @param [in] dataPoints: user data point data address
* @param [out] devStatusPtr: wit the cloud data point data address
*
* @return 0, the correct return; -1, the error returned
*/
static int8_t ICACHE_FLASH_ATTR gizDataPoints2ReportData(dataPoint_t *dataPoints , uint8_t *outData,uint32_t *outDataLen)
{
    uint32_t bitFieldByteLen= 0;//Bit byte size
    uint32_t bitFieldOffset = 0;//Bit offset
    uint32_t byteFieldOffset = 0;//Byte offset
	devStatus_t devStatusTmp;//Temporary device data point variable
    uint8_t allDatapointByteBuf[sizeof(gizwitsElongateP0Form_t)];//Open up the largest data point space
    gizMemset(allDatapointByteBuf,0,sizeof(gizwitsElongateP0Form_t));

    gizMemcpy(allDatapointByteBuf,(uint8_t *)&gizwitsProtocol.waitReportDatapointFlag,DATAPOINT_FLAG_LEN);
    if(DATAPOINT_FLAG_LEN > 1)
    {
        gizByteOrderExchange(allDatapointByteBuf,DATAPOINT_FLAG_LEN);
    }
    byteFieldOffset += DATAPOINT_FLAG_LEN;//First offset the flag size of the location
        
   
    if((NULL == dataPoints) || (NULL == outData))
    {
        GIZWITS_LOG("gizDataPoints2ReportData Error , Illegal Param\n");
        return -1;
    }

    /*** Fill the bit ***/
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_fengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU301_fengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU301_fengJiYiQiDong);
        bitFieldOffset += AHU301_fengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_zhiBanStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU301_zhiBanStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU301_zhiBanStatus);
        bitFieldOffset += AHU301_zhiBanStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_dianJiaRe1)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU301_dianJiaRe1_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU301_dianJiaRe1);
        bitFieldOffset += AHU301_dianJiaRe1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_dianJiaRe2)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU301_dianJiaRe2_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU301_dianJiaRe2);
        bitFieldOffset += AHU301_dianJiaRe2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_dianJiaRe3)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU301_dianJiaRe3_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU301_dianJiaRe3);
        bitFieldOffset += AHU301_dianJiaRe3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_fengJiStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU301_fengJiStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU301_fengJiStatus);
        bitFieldOffset += AHU301_fengJiStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_shouZiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU301_shouZiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU301_shouZiDong);
        bitFieldOffset += AHU301_shouZiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_dongXiaJi)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU301_dongXiaJi_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU301_dongXiaJi);
        bitFieldOffset += AHU301_dongXiaJi_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_zhongXiaoBaoJing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU301_zhongXiaoBaoJing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU301_zhongXiaoBaoJing);
        bitFieldOffset += AHU301_zhongXiaoBaoJing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_dianJiaReGaoWen)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU301_dianJiaReGaoWen_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU301_dianJiaReGaoWen);
        bitFieldOffset += AHU301_dianJiaReGaoWen_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_fengJiQueFeng)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU301_fengJiQueFeng_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU301_fengJiQueFeng);
        bitFieldOffset += AHU301_fengJiQueFeng_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_paiFengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU301_paiFengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU301_paiFengJiYiQiDong);
        bitFieldOffset += AHU301_paiFengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_diWenPanGuan)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU301_diWenPanGuan_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU301_diWenPanGuan);
        bitFieldOffset += AHU301_diWenPanGuan_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_mieJunYunXing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU301_mieJunYunXing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU301_mieJunYunXing);
        bitFieldOffset += AHU301_mieJunYunXing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_fengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU302_fengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU302_fengJiYiQiDong);
        bitFieldOffset += AHU302_fengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_zhiBanStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU302_zhiBanStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU302_zhiBanStatus);
        bitFieldOffset += AHU302_zhiBanStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_dianJiaRe1)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU302_dianJiaRe1_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU302_dianJiaRe1);
        bitFieldOffset += AHU302_dianJiaRe1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_dianJiaRe2)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU302_dianJiaRe2_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU302_dianJiaRe2);
        bitFieldOffset += AHU302_dianJiaRe2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_dianJiaRe3)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU302_dianJiaRe3_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU302_dianJiaRe3);
        bitFieldOffset += AHU302_dianJiaRe3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_fengJiStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU302_fengJiStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU302_fengJiStatus);
        bitFieldOffset += AHU302_fengJiStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_shouZiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU302_shouZiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU302_shouZiDong);
        bitFieldOffset += AHU302_shouZiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_dongXiaJi)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU302_dongXiaJi_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU302_dongXiaJi);
        bitFieldOffset += AHU302_dongXiaJi_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_zhongXiaoBaoJing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU302_zhongXiaoBaoJing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU302_zhongXiaoBaoJing);
        bitFieldOffset += AHU302_zhongXiaoBaoJing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_dianJiaReGaoWen)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU302_dianJiaReGaoWen_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU302_dianJiaReGaoWen);
        bitFieldOffset += AHU302_dianJiaReGaoWen_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_fengJiQueFeng)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU302_fengJiQueFeng_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU302_fengJiQueFeng);
        bitFieldOffset += AHU302_fengJiQueFeng_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_paiFengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU302_paiFengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU302_paiFengJiYiQiDong);
        bitFieldOffset += AHU302_paiFengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_diWenPanGuan)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU302_diWenPanGuan_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU302_diWenPanGuan);
        bitFieldOffset += AHU302_diWenPanGuan_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_mieJunYunXing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU302_mieJunYunXing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU302_mieJunYunXing);
        bitFieldOffset += AHU302_mieJunYunXing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_fengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU303_fengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU303_fengJiYiQiDong);
        bitFieldOffset += AHU303_fengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_zhiBanStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU303_zhiBanStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU303_zhiBanStatus);
        bitFieldOffset += AHU303_zhiBanStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_dianJiaRe1)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU303_dianJiaRe1_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU303_dianJiaRe1);
        bitFieldOffset += AHU303_dianJiaRe1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_dianJiaRe2)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU303_dianJiaRe2_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU303_dianJiaRe2);
        bitFieldOffset += AHU303_dianJiaRe2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_dianJiaRe3)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU303_dianJiaRe3_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU303_dianJiaRe3);
        bitFieldOffset += AHU303_dianJiaRe3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_fengJiStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU303_fengJiStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU303_fengJiStatus);
        bitFieldOffset += AHU303_fengJiStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_shouZiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU303_shouZiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU303_shouZiDong);
        bitFieldOffset += AHU303_shouZiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_dongXiaJi)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU303_dongXiaJi_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU303_dongXiaJi);
        bitFieldOffset += AHU303_dongXiaJi_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_zhongXiaoBaoJing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU303_zhongXiaoBaoJing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU303_zhongXiaoBaoJing);
        bitFieldOffset += AHU303_zhongXiaoBaoJing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_dianJiaReGaoWen)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU303_dianJiaReGaoWen_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU303_dianJiaReGaoWen);
        bitFieldOffset += AHU303_dianJiaReGaoWen_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_fengJiQueFeng)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU303_fengJiQueFeng_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU303_fengJiQueFeng);
        bitFieldOffset += AHU303_fengJiQueFeng_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_paiFengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU303_paiFengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU303_paiFengJiYiQiDong);
        bitFieldOffset += AHU303_paiFengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_diWenPanGuan)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU303_diWenPanGuan_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU303_diWenPanGuan);
        bitFieldOffset += AHU303_diWenPanGuan_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_mieJunYunXing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU303_mieJunYunXing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU303_mieJunYunXing);
        bitFieldOffset += AHU303_mieJunYunXing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_fengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU304_fengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU304_fengJiYiQiDong);
        bitFieldOffset += AHU304_fengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_zhiBanStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU304_zhiBanStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU304_zhiBanStatus);
        bitFieldOffset += AHU304_zhiBanStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_dianJiaRe1)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU304_dianJiaRe1_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU304_dianJiaRe1);
        bitFieldOffset += AHU304_dianJiaRe1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_dianJiaRe2)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU304_dianJiaRe2_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU304_dianJiaRe2);
        bitFieldOffset += AHU304_dianJiaRe2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_dianJiaRe3)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU304_dianJiaRe3_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU304_dianJiaRe3);
        bitFieldOffset += AHU304_dianJiaRe3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_fengJiStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU304_fengJiStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU304_fengJiStatus);
        bitFieldOffset += AHU304_fengJiStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_shouZiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU304_shouZiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU304_shouZiDong);
        bitFieldOffset += AHU304_shouZiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_dongXiaJi)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU304_dongXiaJi_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU304_dongXiaJi);
        bitFieldOffset += AHU304_dongXiaJi_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_zhongXiaoBaoJing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU304_zhongXiaoBaoJing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU304_zhongXiaoBaoJing);
        bitFieldOffset += AHU304_zhongXiaoBaoJing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_dianJiaReGaoWen)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU304_dianJiaReGaoWen_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU304_dianJiaReGaoWen);
        bitFieldOffset += AHU304_dianJiaReGaoWen_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_fengJiQueFeng)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU304_fengJiQueFeng_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU304_fengJiQueFeng);
        bitFieldOffset += AHU304_fengJiQueFeng_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_paiFengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU304_paiFengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU304_paiFengJiYiQiDong);
        bitFieldOffset += AHU304_paiFengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_diWenPanGuan)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU304_diWenPanGuan_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU304_diWenPanGuan);
        bitFieldOffset += AHU304_diWenPanGuan_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_mieJunYunXing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU304_mieJunYunXing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU304_mieJunYunXing);
        bitFieldOffset += AHU304_mieJunYunXing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_fengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU305_fengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU305_fengJiYiQiDong);
        bitFieldOffset += AHU305_fengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_zhiBanStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU305_zhiBanStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU305_zhiBanStatus);
        bitFieldOffset += AHU305_zhiBanStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_dianJiaRe1)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU305_dianJiaRe1_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU305_dianJiaRe1);
        bitFieldOffset += AHU305_dianJiaRe1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_dianJiaRe2)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU305_dianJiaRe2_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU305_dianJiaRe2);
        bitFieldOffset += AHU305_dianJiaRe2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_dianJiaRe3)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU305_dianJiaRe3_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU305_dianJiaRe3);
        bitFieldOffset += AHU305_dianJiaRe3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_fengJiStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU305_fengJiStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU305_fengJiStatus);
        bitFieldOffset += AHU305_fengJiStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_shouZiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU305_shouZiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU305_shouZiDong);
        bitFieldOffset += AHU305_shouZiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_dongXiaJi)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU305_dongXiaJi_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU305_dongXiaJi);
        bitFieldOffset += AHU305_dongXiaJi_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_zhongXiaoBaoJing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU305_zhongXiaoBaoJing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU305_zhongXiaoBaoJing);
        bitFieldOffset += AHU305_zhongXiaoBaoJing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_dianJiaReGaoWen)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU305_dianJiaReGaoWen_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU305_dianJiaReGaoWen);
        bitFieldOffset += AHU305_dianJiaReGaoWen_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_fengJiQueFeng)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU305_fengJiQueFeng_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU305_fengJiQueFeng);
        bitFieldOffset += AHU305_fengJiQueFeng_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_paiFengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU305_paiFengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU305_paiFengJiYiQiDong);
        bitFieldOffset += AHU305_paiFengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_diWenPanGuan)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU305_diWenPanGuan_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU305_diWenPanGuan);
        bitFieldOffset += AHU305_diWenPanGuan_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_mieJunYunXing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU305_mieJunYunXing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU305_mieJunYunXing);
        bitFieldOffset += AHU305_mieJunYunXing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_fengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU306_fengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU306_fengJiYiQiDong);
        bitFieldOffset += AHU306_fengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_zhiBanStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU306_zhiBanStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU306_zhiBanStatus);
        bitFieldOffset += AHU306_zhiBanStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_dianJiaRe1)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU306_dianJiaRe1_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU306_dianJiaRe1);
        bitFieldOffset += AHU306_dianJiaRe1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_dianJiaRe2)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU306_dianJiaRe2_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU306_dianJiaRe2);
        bitFieldOffset += AHU306_dianJiaRe2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_dianJiaRe3)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU306_dianJiaRe3_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU306_dianJiaRe3);
        bitFieldOffset += AHU306_dianJiaRe3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_fengJiStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU306_fengJiStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU306_fengJiStatus);
        bitFieldOffset += AHU306_fengJiStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_shouZiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU306_shouZiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU306_shouZiDong);
        bitFieldOffset += AHU306_shouZiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_dongXiaJi)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU306_dongXiaJi_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU306_dongXiaJi);
        bitFieldOffset += AHU306_dongXiaJi_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_zhongXiaoBaoJing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU306_zhongXiaoBaoJing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU306_zhongXiaoBaoJing);
        bitFieldOffset += AHU306_zhongXiaoBaoJing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_dianJiaReGaoWen)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU306_dianJiaReGaoWen_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU306_dianJiaReGaoWen);
        bitFieldOffset += AHU306_dianJiaReGaoWen_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_fengJiQueFeng)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU306_fengJiQueFeng_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU306_fengJiQueFeng);
        bitFieldOffset += AHU306_fengJiQueFeng_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_paiFengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU306_paiFengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU306_paiFengJiYiQiDong);
        bitFieldOffset += AHU306_paiFengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_diWenPanGuan)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU306_diWenPanGuan_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU306_diWenPanGuan);
        bitFieldOffset += AHU306_diWenPanGuan_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_mieJunYunXing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU306_mieJunYunXing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU306_mieJunYunXing);
        bitFieldOffset += AHU306_mieJunYunXing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_fengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU307_fengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU307_fengJiYiQiDong);
        bitFieldOffset += AHU307_fengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_zhiBanStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU307_zhiBanStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU307_zhiBanStatus);
        bitFieldOffset += AHU307_zhiBanStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_dianJiaRe1)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU307_dianJiaRe1_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU307_dianJiaRe1);
        bitFieldOffset += AHU307_dianJiaRe1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_dianJiaRe2)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU307_dianJiaRe2_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU307_dianJiaRe2);
        bitFieldOffset += AHU307_dianJiaRe2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_dianJiaRe3)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU307_dianJiaRe3_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU307_dianJiaRe3);
        bitFieldOffset += AHU307_dianJiaRe3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_fengJiStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU307_fengJiStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU307_fengJiStatus);
        bitFieldOffset += AHU307_fengJiStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_shouZiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU307_shouZiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU307_shouZiDong);
        bitFieldOffset += AHU307_shouZiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_dongXiaJi)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU307_dongXiaJi_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU307_dongXiaJi);
        bitFieldOffset += AHU307_dongXiaJi_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_zhongXiaoBaoJing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU307_zhongXiaoBaoJing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU307_zhongXiaoBaoJing);
        bitFieldOffset += AHU307_zhongXiaoBaoJing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_dianJiaReGaoWen)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU307_dianJiaReGaoWen_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU307_dianJiaReGaoWen);
        bitFieldOffset += AHU307_dianJiaReGaoWen_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_fengJiQueFeng)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU307_fengJiQueFeng_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU307_fengJiQueFeng);
        bitFieldOffset += AHU307_fengJiQueFeng_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_paiFengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU307_paiFengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU307_paiFengJiYiQiDong);
        bitFieldOffset += AHU307_paiFengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_diWenPanGuan)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU307_diWenPanGuan_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU307_diWenPanGuan);
        bitFieldOffset += AHU307_diWenPanGuan_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_mieJunYunXing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU307_mieJunYunXing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU307_mieJunYunXing);
        bitFieldOffset += AHU307_mieJunYunXing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_fengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU308_fengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU308_fengJiYiQiDong);
        bitFieldOffset += AHU308_fengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_zhiBanStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU308_zhiBanStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU308_zhiBanStatus);
        bitFieldOffset += AHU308_zhiBanStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_dianJiaRe1)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU308_dianJiaRe1_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU308_dianJiaRe1);
        bitFieldOffset += AHU308_dianJiaRe1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_dianJiaRe2)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU308_dianJiaRe2_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU308_dianJiaRe2);
        bitFieldOffset += AHU308_dianJiaRe2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_dianJiaRe3)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU308_dianJiaRe3_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU308_dianJiaRe3);
        bitFieldOffset += AHU308_dianJiaRe3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_fengJiStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU308_fengJiStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU308_fengJiStatus);
        bitFieldOffset += AHU308_fengJiStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_shouZiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU308_shouZiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU308_shouZiDong);
        bitFieldOffset += AHU308_shouZiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_dongXiaJi)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU308_dongXiaJi_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU308_dongXiaJi);
        bitFieldOffset += AHU308_dongXiaJi_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_zhongXiaoBaoJing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU308_zhongXiaoBaoJing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU308_zhongXiaoBaoJing);
        bitFieldOffset += AHU308_zhongXiaoBaoJing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_dianJiaReGaoWen)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU308_dianJiaReGaoWen_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU308_dianJiaReGaoWen);
        bitFieldOffset += AHU308_dianJiaReGaoWen_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_fengJiQueFeng)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU308_fengJiQueFeng_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU308_fengJiQueFeng);
        bitFieldOffset += AHU308_fengJiQueFeng_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_paiFengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU308_paiFengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU308_paiFengJiYiQiDong);
        bitFieldOffset += AHU308_paiFengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_diWenPanGuan)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU308_diWenPanGuan_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU308_diWenPanGuan);
        bitFieldOffset += AHU308_diWenPanGuan_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_mieJunYunXing)
    {
        gizVarlenCompressValue(bitFieldOffset,AHU308_mieJunYunXing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueAHU308_mieJunYunXing);
        bitFieldOffset += AHU308_mieJunYunXing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_fengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_fengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_fengJiYiQiDong);
        bitFieldOffset += PAU301_fengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_zhiBanStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_zhiBanStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_zhiBanStatus);
        bitFieldOffset += PAU301_zhiBanStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianJiaRe1)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_dianJiaRe1_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_dianJiaRe1);
        bitFieldOffset += PAU301_dianJiaRe1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianJiaRe2)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_dianJiaRe2_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_dianJiaRe2);
        bitFieldOffset += PAU301_dianJiaRe2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianJiaRe3)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_dianJiaRe3_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_dianJiaRe3);
        bitFieldOffset += PAU301_dianJiaRe3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_fengJiStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_fengJiStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_fengJiStatus);
        bitFieldOffset += PAU301_fengJiStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_shouZiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_shouZiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_shouZiDong);
        bitFieldOffset += PAU301_shouZiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dongXiaJi)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_dongXiaJi_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_dongXiaJi);
        bitFieldOffset += PAU301_dongXiaJi_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_zhongXiaoBaoJing)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_zhongXiaoBaoJing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_zhongXiaoBaoJing);
        bitFieldOffset += PAU301_zhongXiaoBaoJing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianJiaReGaoWen)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_dianJiaReGaoWen_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_dianJiaReGaoWen);
        bitFieldOffset += PAU301_dianJiaReGaoWen_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_fengJiQueFeng)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_fengJiQueFeng_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_fengJiQueFeng);
        bitFieldOffset += PAU301_fengJiQueFeng_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_paiFengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_paiFengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_paiFengJiYiQiDong);
        bitFieldOffset += PAU301_paiFengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianYuRe1)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_dianYuRe1_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_dianYuRe1);
        bitFieldOffset += PAU301_dianYuRe1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianYuRe2)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_dianYuRe2_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_dianYuRe2);
        bitFieldOffset += PAU301_dianYuRe2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianYuRe3)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_dianYuRe3_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_dianYuRe3);
        bitFieldOffset += PAU301_dianYuRe3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_dianYuReGaoWen)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_dianYuReGaoWen_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_dianYuReGaoWen);
        bitFieldOffset += PAU301_dianYuReGaoWen_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_diWenPanGuan)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_diWenPanGuan_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_diWenPanGuan);
        bitFieldOffset += PAU301_diWenPanGuan_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_mieJunYunXing)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU301_mieJunYunXing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU301_mieJunYunXing);
        bitFieldOffset += PAU301_mieJunYunXing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_fengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU302_fengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU302_fengJiYiQiDong);
        bitFieldOffset += PAU302_fengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_zhiBanStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU302_zhiBanStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU302_zhiBanStatus);
        bitFieldOffset += PAU302_zhiBanStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_fengJiStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU302_fengJiStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU302_fengJiStatus);
        bitFieldOffset += PAU302_fengJiStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_shouZiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU302_shouZiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU302_shouZiDong);
        bitFieldOffset += PAU302_shouZiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_dongXiaJi)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU302_dongXiaJi_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU302_dongXiaJi);
        bitFieldOffset += PAU302_dongXiaJi_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_zhongXiaoBaoJing)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU302_zhongXiaoBaoJing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU302_zhongXiaoBaoJing);
        bitFieldOffset += PAU302_zhongXiaoBaoJing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_fengJiQueFeng)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU302_fengJiQueFeng_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU302_fengJiQueFeng);
        bitFieldOffset += PAU302_fengJiQueFeng_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_paiFengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU302_paiFengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU302_paiFengJiYiQiDong);
        bitFieldOffset += PAU302_paiFengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_dianYuRe1)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU302_dianYuRe1_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU302_dianYuRe1);
        bitFieldOffset += PAU302_dianYuRe1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_dianYuRe2)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU302_dianYuRe2_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU302_dianYuRe2);
        bitFieldOffset += PAU302_dianYuRe2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_dianYuRe3)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU302_dianYuRe3_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU302_dianYuRe3);
        bitFieldOffset += PAU302_dianYuRe3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_dianYuReGaoWen)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU302_dianYuReGaoWen_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU302_dianYuReGaoWen);
        bitFieldOffset += PAU302_dianYuReGaoWen_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_diWenPanGuan)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU302_diWenPanGuan_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU302_diWenPanGuan);
        bitFieldOffset += PAU302_diWenPanGuan_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_mieJunYunXing)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU302_mieJunYunXing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU302_mieJunYunXing);
        bitFieldOffset += PAU302_mieJunYunXing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_fengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU303_fengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU303_fengJiYiQiDong);
        bitFieldOffset += PAU303_fengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_zhiBanStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU303_zhiBanStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU303_zhiBanStatus);
        bitFieldOffset += PAU303_zhiBanStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_fengJiStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU303_fengJiStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU303_fengJiStatus);
        bitFieldOffset += PAU303_fengJiStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_shouZiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU303_shouZiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU303_shouZiDong);
        bitFieldOffset += PAU303_shouZiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_dongXiaJi)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU303_dongXiaJi_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU303_dongXiaJi);
        bitFieldOffset += PAU303_dongXiaJi_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_zhongXiaoBaoJing)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU303_zhongXiaoBaoJing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU303_zhongXiaoBaoJing);
        bitFieldOffset += PAU303_zhongXiaoBaoJing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_fengJiQueFeng)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU303_fengJiQueFeng_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU303_fengJiQueFeng);
        bitFieldOffset += PAU303_fengJiQueFeng_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_paiFengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU303_paiFengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU303_paiFengJiYiQiDong);
        bitFieldOffset += PAU303_paiFengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_dianYuRe1)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU303_dianYuRe1_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU303_dianYuRe1);
        bitFieldOffset += PAU303_dianYuRe1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_dianYuRe2)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU303_dianYuRe2_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU303_dianYuRe2);
        bitFieldOffset += PAU303_dianYuRe2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_dianYuRe3)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU303_dianYuRe3_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU303_dianYuRe3);
        bitFieldOffset += PAU303_dianYuRe3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_dianYuReGaoWen)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU303_dianYuReGaoWen_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU303_dianYuReGaoWen);
        bitFieldOffset += PAU303_dianYuReGaoWen_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_diWenPanGuan)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU303_diWenPanGuan_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU303_diWenPanGuan);
        bitFieldOffset += PAU303_diWenPanGuan_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_mieJunYunXing)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU303_mieJunYunXing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU303_mieJunYunXing);
        bitFieldOffset += PAU303_mieJunYunXing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_fengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU304_fengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU304_fengJiYiQiDong);
        bitFieldOffset += PAU304_fengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_zhiBanStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU304_zhiBanStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU304_zhiBanStatus);
        bitFieldOffset += PAU304_zhiBanStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_fengJiStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU304_fengJiStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU304_fengJiStatus);
        bitFieldOffset += PAU304_fengJiStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_shouZiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU304_shouZiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU304_shouZiDong);
        bitFieldOffset += PAU304_shouZiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_dongXiaJi)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU304_dongXiaJi_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU304_dongXiaJi);
        bitFieldOffset += PAU304_dongXiaJi_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_zhongXiaoBaoJing)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU304_zhongXiaoBaoJing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU304_zhongXiaoBaoJing);
        bitFieldOffset += PAU304_zhongXiaoBaoJing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_fengJiQueFeng)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU304_fengJiQueFeng_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU304_fengJiQueFeng);
        bitFieldOffset += PAU304_fengJiQueFeng_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_paiFengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU304_paiFengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU304_paiFengJiYiQiDong);
        bitFieldOffset += PAU304_paiFengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_dianYuRe1)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU304_dianYuRe1_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU304_dianYuRe1);
        bitFieldOffset += PAU304_dianYuRe1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_dianYuRe2)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU304_dianYuRe2_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU304_dianYuRe2);
        bitFieldOffset += PAU304_dianYuRe2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_dianYuRe3)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU304_dianYuRe3_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU304_dianYuRe3);
        bitFieldOffset += PAU304_dianYuRe3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_dianYuReGaoWen)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU304_dianYuReGaoWen_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU304_dianYuReGaoWen);
        bitFieldOffset += PAU304_dianYuReGaoWen_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_diWenPanGuan)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU304_diWenPanGuan_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU304_diWenPanGuan);
        bitFieldOffset += PAU304_diWenPanGuan_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_mieJunYunXing)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU304_mieJunYunXing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU304_mieJunYunXing);
        bitFieldOffset += PAU304_mieJunYunXing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_fengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU305_fengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU305_fengJiYiQiDong);
        bitFieldOffset += PAU305_fengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_zhiBanStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU305_zhiBanStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU305_zhiBanStatus);
        bitFieldOffset += PAU305_zhiBanStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_fengJiStatus)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU305_fengJiStatus_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU305_fengJiStatus);
        bitFieldOffset += PAU305_fengJiStatus_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_shouZiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU305_shouZiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU305_shouZiDong);
        bitFieldOffset += PAU305_shouZiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_dongXiaJi)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU305_dongXiaJi_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU305_dongXiaJi);
        bitFieldOffset += PAU305_dongXiaJi_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_zhongXiaoBaoJing)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU305_zhongXiaoBaoJing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU305_zhongXiaoBaoJing);
        bitFieldOffset += PAU305_zhongXiaoBaoJing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_fengJiQueFeng)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU305_fengJiQueFeng_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU305_fengJiQueFeng);
        bitFieldOffset += PAU305_fengJiQueFeng_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_paiFengJiYiQiDong)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU305_paiFengJiYiQiDong_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU305_paiFengJiYiQiDong);
        bitFieldOffset += PAU305_paiFengJiYiQiDong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_dianYuRe1)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU305_dianYuRe1_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU305_dianYuRe1);
        bitFieldOffset += PAU305_dianYuRe1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_dianYuRe2)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU305_dianYuRe2_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU305_dianYuRe2);
        bitFieldOffset += PAU305_dianYuRe2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_dianYuRe3)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU305_dianYuRe3_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU305_dianYuRe3);
        bitFieldOffset += PAU305_dianYuRe3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_dianYuReGaoWen)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU305_dianYuReGaoWen_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU305_dianYuReGaoWen);
        bitFieldOffset += PAU305_dianYuReGaoWen_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_diWenPanGuan)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU305_diWenPanGuan_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU305_diWenPanGuan);
        bitFieldOffset += PAU305_diWenPanGuan_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_mieJunYunXing)
    {
        gizVarlenCompressValue(bitFieldOffset,PAU305_mieJunYunXing_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valuePAU305_mieJunYunXing);
        bitFieldOffset += PAU305_mieJunYunXing_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagonline_flag)
    {
        gizVarlenCompressValue(bitFieldOffset,online_flag_LEN,(uint8_t *)&allDatapointByteBuf[byteFieldOffset],dataPoints->valueonline_flag);
        bitFieldOffset += online_flag_LEN;
    }

    /* The bit segment is assembled and the offset of the value segment is calculated */
    if(0 == bitFieldOffset)
    {
        bitFieldByteLen = 0;
    }
    else
    {
        if(0 == bitFieldOffset%8)
        {
            bitFieldByteLen = bitFieldOffset/8;
        }
        else
        {
            bitFieldByteLen = bitFieldOffset/8 + 1;
        }
    }
    /** Bitwise byte order conversion **/
    if(bitFieldByteLen > 1)
    {
        gizByteOrderExchange((uint8_t *)&allDatapointByteBuf[byteFieldOffset],bitFieldByteLen);
    }
    
    byteFieldOffset += bitFieldByteLen;//Offset the number of bytes occupied by the bit segment

    /*** Handle the value segment ***/

    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_mianBanTongXunZhuangTai1)
    {
        devStatusTmp.valueAHU301_mianBanTongXunZhuangTai1 = gizY2X(AHU301_mianBanTongXunZhuangTai1_RATIO,  AHU301_mianBanTongXunZhuangTai1_ADDITION, dataPoints->valueAHU301_mianBanTongXunZhuangTai1); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU301_mianBanTongXunZhuangTai1,AHU301_mianBanTongXunZhuangTai1_LEN);
        byteFieldOffset += AHU301_mianBanTongXunZhuangTai1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_mianBanTongXunZhuangTai2)
    {
        devStatusTmp.valueAHU301_mianBanTongXunZhuangTai2 = gizY2X(AHU301_mianBanTongXunZhuangTai2_RATIO,  AHU301_mianBanTongXunZhuangTai2_ADDITION, dataPoints->valueAHU301_mianBanTongXunZhuangTai2); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU301_mianBanTongXunZhuangTai2,AHU301_mianBanTongXunZhuangTai2_LEN);
        byteFieldOffset += AHU301_mianBanTongXunZhuangTai2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_mianBanTongXunZhuangTai3)
    {
        devStatusTmp.valueAHU301_mianBanTongXunZhuangTai3 = gizY2X(AHU301_mianBanTongXunZhuangTai3_RATIO,  AHU301_mianBanTongXunZhuangTai3_ADDITION, dataPoints->valueAHU301_mianBanTongXunZhuangTai3); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU301_mianBanTongXunZhuangTai3,AHU301_mianBanTongXunZhuangTai3_LEN);
        byteFieldOffset += AHU301_mianBanTongXunZhuangTai3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_mianBanTongXunZhuangTai1)
    {
        devStatusTmp.valueAHU302_mianBanTongXunZhuangTai1 = gizY2X(AHU302_mianBanTongXunZhuangTai1_RATIO,  AHU302_mianBanTongXunZhuangTai1_ADDITION, dataPoints->valueAHU302_mianBanTongXunZhuangTai1); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU302_mianBanTongXunZhuangTai1,AHU302_mianBanTongXunZhuangTai1_LEN);
        byteFieldOffset += AHU302_mianBanTongXunZhuangTai1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_mianBanTongXunZhuangTai2)
    {
        devStatusTmp.valueAHU302_mianBanTongXunZhuangTai2 = gizY2X(AHU302_mianBanTongXunZhuangTai2_RATIO,  AHU302_mianBanTongXunZhuangTai2_ADDITION, dataPoints->valueAHU302_mianBanTongXunZhuangTai2); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU302_mianBanTongXunZhuangTai2,AHU302_mianBanTongXunZhuangTai2_LEN);
        byteFieldOffset += AHU302_mianBanTongXunZhuangTai2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_mianBanTongXunZhuangTai3)
    {
        devStatusTmp.valueAHU302_mianBanTongXunZhuangTai3 = gizY2X(AHU302_mianBanTongXunZhuangTai3_RATIO,  AHU302_mianBanTongXunZhuangTai3_ADDITION, dataPoints->valueAHU302_mianBanTongXunZhuangTai3); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU302_mianBanTongXunZhuangTai3,AHU302_mianBanTongXunZhuangTai3_LEN);
        byteFieldOffset += AHU302_mianBanTongXunZhuangTai3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_mianBanTongXunZhuangTai1)
    {
        devStatusTmp.valueAHU303_mianBanTongXunZhuangTai1 = gizY2X(AHU303_mianBanTongXunZhuangTai1_RATIO,  AHU303_mianBanTongXunZhuangTai1_ADDITION, dataPoints->valueAHU303_mianBanTongXunZhuangTai1); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU303_mianBanTongXunZhuangTai1,AHU303_mianBanTongXunZhuangTai1_LEN);
        byteFieldOffset += AHU303_mianBanTongXunZhuangTai1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_mianBanTongXunZhuangTai2)
    {
        devStatusTmp.valueAHU303_mianBanTongXunZhuangTai2 = gizY2X(AHU303_mianBanTongXunZhuangTai2_RATIO,  AHU303_mianBanTongXunZhuangTai2_ADDITION, dataPoints->valueAHU303_mianBanTongXunZhuangTai2); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU303_mianBanTongXunZhuangTai2,AHU303_mianBanTongXunZhuangTai2_LEN);
        byteFieldOffset += AHU303_mianBanTongXunZhuangTai2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_mianBanTongXunZhuangTai3)
    {
        devStatusTmp.valueAHU303_mianBanTongXunZhuangTai3 = gizY2X(AHU303_mianBanTongXunZhuangTai3_RATIO,  AHU303_mianBanTongXunZhuangTai3_ADDITION, dataPoints->valueAHU303_mianBanTongXunZhuangTai3); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU303_mianBanTongXunZhuangTai3,AHU303_mianBanTongXunZhuangTai3_LEN);
        byteFieldOffset += AHU303_mianBanTongXunZhuangTai3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_mianBanTongXunZhuangTai1)
    {
        devStatusTmp.valueAHU304_mianBanTongXunZhuangTai1 = gizY2X(AHU304_mianBanTongXunZhuangTai1_RATIO,  AHU304_mianBanTongXunZhuangTai1_ADDITION, dataPoints->valueAHU304_mianBanTongXunZhuangTai1); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU304_mianBanTongXunZhuangTai1,AHU304_mianBanTongXunZhuangTai1_LEN);
        byteFieldOffset += AHU304_mianBanTongXunZhuangTai1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_mianBanTongXunZhuangTai2)
    {
        devStatusTmp.valueAHU304_mianBanTongXunZhuangTai2 = gizY2X(AHU304_mianBanTongXunZhuangTai2_RATIO,  AHU304_mianBanTongXunZhuangTai2_ADDITION, dataPoints->valueAHU304_mianBanTongXunZhuangTai2); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU304_mianBanTongXunZhuangTai2,AHU304_mianBanTongXunZhuangTai2_LEN);
        byteFieldOffset += AHU304_mianBanTongXunZhuangTai2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_mianBanTongXunZhuangTai3)
    {
        devStatusTmp.valueAHU304_mianBanTongXunZhuangTai3 = gizY2X(AHU304_mianBanTongXunZhuangTai3_RATIO,  AHU304_mianBanTongXunZhuangTai3_ADDITION, dataPoints->valueAHU304_mianBanTongXunZhuangTai3); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU304_mianBanTongXunZhuangTai3,AHU304_mianBanTongXunZhuangTai3_LEN);
        byteFieldOffset += AHU304_mianBanTongXunZhuangTai3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_mianBanTongXunZhuangTai1)
    {
        devStatusTmp.valueAHU305_mianBanTongXunZhuangTai1 = gizY2X(AHU305_mianBanTongXunZhuangTai1_RATIO,  AHU305_mianBanTongXunZhuangTai1_ADDITION, dataPoints->valueAHU305_mianBanTongXunZhuangTai1); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU305_mianBanTongXunZhuangTai1,AHU305_mianBanTongXunZhuangTai1_LEN);
        byteFieldOffset += AHU305_mianBanTongXunZhuangTai1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_mianBanTongXunZhuangTai2)
    {
        devStatusTmp.valueAHU305_mianBanTongXunZhuangTai2 = gizY2X(AHU305_mianBanTongXunZhuangTai2_RATIO,  AHU305_mianBanTongXunZhuangTai2_ADDITION, dataPoints->valueAHU305_mianBanTongXunZhuangTai2); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU305_mianBanTongXunZhuangTai2,AHU305_mianBanTongXunZhuangTai2_LEN);
        byteFieldOffset += AHU305_mianBanTongXunZhuangTai2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_mianBanTongXunZhuangTai3)
    {
        devStatusTmp.valueAHU305_mianBanTongXunZhuangTai3 = gizY2X(AHU305_mianBanTongXunZhuangTai3_RATIO,  AHU305_mianBanTongXunZhuangTai3_ADDITION, dataPoints->valueAHU305_mianBanTongXunZhuangTai3); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU305_mianBanTongXunZhuangTai3,AHU305_mianBanTongXunZhuangTai3_LEN);
        byteFieldOffset += AHU305_mianBanTongXunZhuangTai3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_mianBanTongXunZhuangTai1)
    {
        devStatusTmp.valueAHU306_mianBanTongXunZhuangTai1 = gizY2X(AHU306_mianBanTongXunZhuangTai1_RATIO,  AHU306_mianBanTongXunZhuangTai1_ADDITION, dataPoints->valueAHU306_mianBanTongXunZhuangTai1); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU306_mianBanTongXunZhuangTai1,AHU306_mianBanTongXunZhuangTai1_LEN);
        byteFieldOffset += AHU306_mianBanTongXunZhuangTai1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_mianBanTongXunZhuangTai2)
    {
        devStatusTmp.valueAHU306_mianBanTongXunZhuangTai2 = gizY2X(AHU306_mianBanTongXunZhuangTai2_RATIO,  AHU306_mianBanTongXunZhuangTai2_ADDITION, dataPoints->valueAHU306_mianBanTongXunZhuangTai2); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU306_mianBanTongXunZhuangTai2,AHU306_mianBanTongXunZhuangTai2_LEN);
        byteFieldOffset += AHU306_mianBanTongXunZhuangTai2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_mianBanTongXunZhuangTai3)
    {
        devStatusTmp.valueAHU306_mianBanTongXunZhuangTai3 = gizY2X(AHU306_mianBanTongXunZhuangTai3_RATIO,  AHU306_mianBanTongXunZhuangTai3_ADDITION, dataPoints->valueAHU306_mianBanTongXunZhuangTai3); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU306_mianBanTongXunZhuangTai3,AHU306_mianBanTongXunZhuangTai3_LEN);
        byteFieldOffset += AHU306_mianBanTongXunZhuangTai3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_mianBanTongXunZhuangTai1)
    {
        devStatusTmp.valueAHU307_mianBanTongXunZhuangTai1 = gizY2X(AHU307_mianBanTongXunZhuangTai1_RATIO,  AHU307_mianBanTongXunZhuangTai1_ADDITION, dataPoints->valueAHU307_mianBanTongXunZhuangTai1); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU307_mianBanTongXunZhuangTai1,AHU307_mianBanTongXunZhuangTai1_LEN);
        byteFieldOffset += AHU307_mianBanTongXunZhuangTai1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_mianBanTongXunZhuangTai2)
    {
        devStatusTmp.valueAHU307_mianBanTongXunZhuangTai2 = gizY2X(AHU307_mianBanTongXunZhuangTai2_RATIO,  AHU307_mianBanTongXunZhuangTai2_ADDITION, dataPoints->valueAHU307_mianBanTongXunZhuangTai2); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU307_mianBanTongXunZhuangTai2,AHU307_mianBanTongXunZhuangTai2_LEN);
        byteFieldOffset += AHU307_mianBanTongXunZhuangTai2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_mianBanTongXunZhuangTai3)
    {
        devStatusTmp.valueAHU307_mianBanTongXunZhuangTai3 = gizY2X(AHU307_mianBanTongXunZhuangTai3_RATIO,  AHU307_mianBanTongXunZhuangTai3_ADDITION, dataPoints->valueAHU307_mianBanTongXunZhuangTai3); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU307_mianBanTongXunZhuangTai3,AHU307_mianBanTongXunZhuangTai3_LEN);
        byteFieldOffset += AHU307_mianBanTongXunZhuangTai3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_mianBanTongXunZhuangTai1)
    {
        devStatusTmp.valueAHU308_mianBanTongXunZhuangTai1 = gizY2X(AHU308_mianBanTongXunZhuangTai1_RATIO,  AHU308_mianBanTongXunZhuangTai1_ADDITION, dataPoints->valueAHU308_mianBanTongXunZhuangTai1); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU308_mianBanTongXunZhuangTai1,AHU308_mianBanTongXunZhuangTai1_LEN);
        byteFieldOffset += AHU308_mianBanTongXunZhuangTai1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_mianBanTongXunZhuangTai2)
    {
        devStatusTmp.valueAHU308_mianBanTongXunZhuangTai2 = gizY2X(AHU308_mianBanTongXunZhuangTai2_RATIO,  AHU308_mianBanTongXunZhuangTai2_ADDITION, dataPoints->valueAHU308_mianBanTongXunZhuangTai2); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU308_mianBanTongXunZhuangTai2,AHU308_mianBanTongXunZhuangTai2_LEN);
        byteFieldOffset += AHU308_mianBanTongXunZhuangTai2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_mianBanTongXunZhuangTai3)
    {
        devStatusTmp.valueAHU308_mianBanTongXunZhuangTai3 = gizY2X(AHU308_mianBanTongXunZhuangTai3_RATIO,  AHU308_mianBanTongXunZhuangTai3_ADDITION, dataPoints->valueAHU308_mianBanTongXunZhuangTai3); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU308_mianBanTongXunZhuangTai3,AHU308_mianBanTongXunZhuangTai3_LEN);
        byteFieldOffset += AHU308_mianBanTongXunZhuangTai3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_mianBanTongXunZhuangTai1)
    {
        devStatusTmp.valuePAU301_mianBanTongXunZhuangTai1 = gizY2X(PAU301_mianBanTongXunZhuangTai1_RATIO,  PAU301_mianBanTongXunZhuangTai1_ADDITION, dataPoints->valuePAU301_mianBanTongXunZhuangTai1); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU301_mianBanTongXunZhuangTai1,PAU301_mianBanTongXunZhuangTai1_LEN);
        byteFieldOffset += PAU301_mianBanTongXunZhuangTai1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_mianBanTongXunZhuangTai2)
    {
        devStatusTmp.valuePAU301_mianBanTongXunZhuangTai2 = gizY2X(PAU301_mianBanTongXunZhuangTai2_RATIO,  PAU301_mianBanTongXunZhuangTai2_ADDITION, dataPoints->valuePAU301_mianBanTongXunZhuangTai2); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU301_mianBanTongXunZhuangTai2,PAU301_mianBanTongXunZhuangTai2_LEN);
        byteFieldOffset += PAU301_mianBanTongXunZhuangTai2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_mianBanTongXunZhuangTai3)
    {
        devStatusTmp.valuePAU301_mianBanTongXunZhuangTai3 = gizY2X(PAU301_mianBanTongXunZhuangTai3_RATIO,  PAU301_mianBanTongXunZhuangTai3_ADDITION, dataPoints->valuePAU301_mianBanTongXunZhuangTai3); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU301_mianBanTongXunZhuangTai3,PAU301_mianBanTongXunZhuangTai3_LEN);
        byteFieldOffset += PAU301_mianBanTongXunZhuangTai3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_mianBanTongXunZhuangTai1)
    {
        devStatusTmp.valuePAU302_mianBanTongXunZhuangTai1 = gizY2X(PAU302_mianBanTongXunZhuangTai1_RATIO,  PAU302_mianBanTongXunZhuangTai1_ADDITION, dataPoints->valuePAU302_mianBanTongXunZhuangTai1); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU302_mianBanTongXunZhuangTai1,PAU302_mianBanTongXunZhuangTai1_LEN);
        byteFieldOffset += PAU302_mianBanTongXunZhuangTai1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_mianBanTongXunZhuangTai2)
    {
        devStatusTmp.valuePAU302_mianBanTongXunZhuangTai2 = gizY2X(PAU302_mianBanTongXunZhuangTai2_RATIO,  PAU302_mianBanTongXunZhuangTai2_ADDITION, dataPoints->valuePAU302_mianBanTongXunZhuangTai2); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU302_mianBanTongXunZhuangTai2,PAU302_mianBanTongXunZhuangTai2_LEN);
        byteFieldOffset += PAU302_mianBanTongXunZhuangTai2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_mianBanTongXunZhuangTai3)
    {
        devStatusTmp.valuePAU302_mianBanTongXunZhuangTai3 = gizY2X(PAU302_mianBanTongXunZhuangTai3_RATIO,  PAU302_mianBanTongXunZhuangTai3_ADDITION, dataPoints->valuePAU302_mianBanTongXunZhuangTai3); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU302_mianBanTongXunZhuangTai3,PAU302_mianBanTongXunZhuangTai3_LEN);
        byteFieldOffset += PAU302_mianBanTongXunZhuangTai3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_mianBanTongXunZhuangTai1)
    {
        devStatusTmp.valuePAU303_mianBanTongXunZhuangTai1 = gizY2X(PAU303_mianBanTongXunZhuangTai1_RATIO,  PAU303_mianBanTongXunZhuangTai1_ADDITION, dataPoints->valuePAU303_mianBanTongXunZhuangTai1); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU303_mianBanTongXunZhuangTai1,PAU303_mianBanTongXunZhuangTai1_LEN);
        byteFieldOffset += PAU303_mianBanTongXunZhuangTai1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_mianBanTongXunZhuangTai2)
    {
        devStatusTmp.valuePAU303_mianBanTongXunZhuangTai2 = gizY2X(PAU303_mianBanTongXunZhuangTai2_RATIO,  PAU303_mianBanTongXunZhuangTai2_ADDITION, dataPoints->valuePAU303_mianBanTongXunZhuangTai2); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU303_mianBanTongXunZhuangTai2,PAU303_mianBanTongXunZhuangTai2_LEN);
        byteFieldOffset += PAU303_mianBanTongXunZhuangTai2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_mianBanTongXunZhuangTai3)
    {
        devStatusTmp.valuePAU303_mianBanTongXunZhuangTai3 = gizY2X(PAU303_mianBanTongXunZhuangTai3_RATIO,  PAU303_mianBanTongXunZhuangTai3_ADDITION, dataPoints->valuePAU303_mianBanTongXunZhuangTai3); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU303_mianBanTongXunZhuangTai3,PAU303_mianBanTongXunZhuangTai3_LEN);
        byteFieldOffset += PAU303_mianBanTongXunZhuangTai3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_mianBanTongXunZhuangTai1)
    {
        devStatusTmp.valuePAU304_mianBanTongXunZhuangTai1 = gizY2X(PAU304_mianBanTongXunZhuangTai1_RATIO,  PAU304_mianBanTongXunZhuangTai1_ADDITION, dataPoints->valuePAU304_mianBanTongXunZhuangTai1); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU304_mianBanTongXunZhuangTai1,PAU304_mianBanTongXunZhuangTai1_LEN);
        byteFieldOffset += PAU304_mianBanTongXunZhuangTai1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_mianBanTongXunZhuangTai2)
    {
        devStatusTmp.valuePAU304_mianBanTongXunZhuangTai2 = gizY2X(PAU304_mianBanTongXunZhuangTai2_RATIO,  PAU304_mianBanTongXunZhuangTai2_ADDITION, dataPoints->valuePAU304_mianBanTongXunZhuangTai2); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU304_mianBanTongXunZhuangTai2,PAU304_mianBanTongXunZhuangTai2_LEN);
        byteFieldOffset += PAU304_mianBanTongXunZhuangTai2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_mianBanTongXunZhuangTai3)
    {
        devStatusTmp.valuePAU304_mianBanTongXunZhuangTai3 = gizY2X(PAU304_mianBanTongXunZhuangTai3_RATIO,  PAU304_mianBanTongXunZhuangTai3_ADDITION, dataPoints->valuePAU304_mianBanTongXunZhuangTai3); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU304_mianBanTongXunZhuangTai3,PAU304_mianBanTongXunZhuangTai3_LEN);
        byteFieldOffset += PAU304_mianBanTongXunZhuangTai3_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_mianBanTongXunZhuangTai1)
    {
        devStatusTmp.valuePAU305_mianBanTongXunZhuangTai1 = gizY2X(PAU305_mianBanTongXunZhuangTai1_RATIO,  PAU305_mianBanTongXunZhuangTai1_ADDITION, dataPoints->valuePAU305_mianBanTongXunZhuangTai1); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU305_mianBanTongXunZhuangTai1,PAU305_mianBanTongXunZhuangTai1_LEN);
        byteFieldOffset += PAU305_mianBanTongXunZhuangTai1_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_mianBanTongXunZhuangTai2)
    {
        devStatusTmp.valuePAU305_mianBanTongXunZhuangTai2 = gizY2X(PAU305_mianBanTongXunZhuangTai2_RATIO,  PAU305_mianBanTongXunZhuangTai2_ADDITION, dataPoints->valuePAU305_mianBanTongXunZhuangTai2); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU305_mianBanTongXunZhuangTai2,PAU305_mianBanTongXunZhuangTai2_LEN);
        byteFieldOffset += PAU305_mianBanTongXunZhuangTai2_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_mianBanTongXunZhuangTai3)
    {
        devStatusTmp.valuePAU305_mianBanTongXunZhuangTai3 = gizY2X(PAU305_mianBanTongXunZhuangTai3_RATIO,  PAU305_mianBanTongXunZhuangTai3_ADDITION, dataPoints->valuePAU305_mianBanTongXunZhuangTai3); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU305_mianBanTongXunZhuangTai3,PAU305_mianBanTongXunZhuangTai3_LEN);
        byteFieldOffset += PAU305_mianBanTongXunZhuangTai3_LEN;
    }

    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_tempReal)
    {
        devStatusTmp.valueAHU301_tempReal = exchangeBytes(gizY2X(AHU301_tempReal_RATIO,  AHU301_tempReal_ADDITION, dataPoints->valueAHU301_tempReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU301_tempReal,AHU301_tempReal_LEN);
        byteFieldOffset += AHU301_tempReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_humiReal)
    {
        devStatusTmp.valueAHU301_humiReal = exchangeBytes(gizY2X(AHU301_humiReal_RATIO,  AHU301_humiReal_ADDITION, dataPoints->valueAHU301_humiReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU301_humiReal,AHU301_humiReal_LEN);
        byteFieldOffset += AHU301_humiReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_tempSet)
    {
        devStatusTmp.valueAHU301_tempSet = exchangeBytes(gizY2X(AHU301_tempSet_RATIO,  AHU301_tempSet_ADDITION, dataPoints->valueAHU301_tempSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU301_tempSet,AHU301_tempSet_LEN);
        byteFieldOffset += AHU301_tempSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_humiSet)
    {
        devStatusTmp.valueAHU301_humiSet = exchangeBytes(gizY2X(AHU301_humiSet_RATIO,  AHU301_humiSet_ADDITION, dataPoints->valueAHU301_humiSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU301_humiSet,AHU301_humiSet_LEN);
        byteFieldOffset += AHU301_humiSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_lengShuiFaKaiDu)
    {
        devStatusTmp.valueAHU301_lengShuiFaKaiDu = exchangeBytes(gizY2X(AHU301_lengShuiFaKaiDu_RATIO,  AHU301_lengShuiFaKaiDu_ADDITION, dataPoints->valueAHU301_lengShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU301_lengShuiFaKaiDu,AHU301_lengShuiFaKaiDu_LEN);
        byteFieldOffset += AHU301_lengShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_reShuiFaKaiDu)
    {
        devStatusTmp.valueAHU301_reShuiFaKaiDu = exchangeBytes(gizY2X(AHU301_reShuiFaKaiDu_RATIO,  AHU301_reShuiFaKaiDu_ADDITION, dataPoints->valueAHU301_reShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU301_reShuiFaKaiDu,AHU301_reShuiFaKaiDu_LEN);
        byteFieldOffset += AHU301_reShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_xinFengWenDU)
    {
        devStatusTmp.valueAHU301_xinFengWenDU = exchangeBytes(gizY2X(AHU301_xinFengWenDU_RATIO,  AHU301_xinFengWenDU_ADDITION, dataPoints->valueAHU301_xinFengWenDU)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU301_xinFengWenDU,AHU301_xinFengWenDU_LEN);
        byteFieldOffset += AHU301_xinFengWenDU_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_jiaShiQIKaiDu)
    {
        devStatusTmp.valueAHU301_jiaShiQIKaiDu = exchangeBytes(gizY2X(AHU301_jiaShiQIKaiDu_RATIO,  AHU301_jiaShiQIKaiDu_ADDITION, dataPoints->valueAHU301_jiaShiQIKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU301_jiaShiQIKaiDu,AHU301_jiaShiQIKaiDu_LEN);
        byteFieldOffset += AHU301_jiaShiQIKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU301_beiYong)
    {
        devStatusTmp.valueAHU301_beiYong = exchangeBytes(gizY2X(AHU301_beiYong_RATIO,  AHU301_beiYong_ADDITION, dataPoints->valueAHU301_beiYong)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU301_beiYong,AHU301_beiYong_LEN);
        byteFieldOffset += AHU301_beiYong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_tempReal)
    {
        devStatusTmp.valueAHU302_tempReal = exchangeBytes(gizY2X(AHU302_tempReal_RATIO,  AHU302_tempReal_ADDITION, dataPoints->valueAHU302_tempReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU302_tempReal,AHU302_tempReal_LEN);
        byteFieldOffset += AHU302_tempReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_humiReal)
    {
        devStatusTmp.valueAHU302_humiReal = exchangeBytes(gizY2X(AHU302_humiReal_RATIO,  AHU302_humiReal_ADDITION, dataPoints->valueAHU302_humiReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU302_humiReal,AHU302_humiReal_LEN);
        byteFieldOffset += AHU302_humiReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_tempSet)
    {
        devStatusTmp.valueAHU302_tempSet = exchangeBytes(gizY2X(AHU302_tempSet_RATIO,  AHU302_tempSet_ADDITION, dataPoints->valueAHU302_tempSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU302_tempSet,AHU302_tempSet_LEN);
        byteFieldOffset += AHU302_tempSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_humiSet)
    {
        devStatusTmp.valueAHU302_humiSet = exchangeBytes(gizY2X(AHU302_humiSet_RATIO,  AHU302_humiSet_ADDITION, dataPoints->valueAHU302_humiSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU302_humiSet,AHU302_humiSet_LEN);
        byteFieldOffset += AHU302_humiSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_lengShuiFaKaiDu)
    {
        devStatusTmp.valueAHU302_lengShuiFaKaiDu = exchangeBytes(gizY2X(AHU302_lengShuiFaKaiDu_RATIO,  AHU302_lengShuiFaKaiDu_ADDITION, dataPoints->valueAHU302_lengShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU302_lengShuiFaKaiDu,AHU302_lengShuiFaKaiDu_LEN);
        byteFieldOffset += AHU302_lengShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_reShuiFaKaiDu)
    {
        devStatusTmp.valueAHU302_reShuiFaKaiDu = exchangeBytes(gizY2X(AHU302_reShuiFaKaiDu_RATIO,  AHU302_reShuiFaKaiDu_ADDITION, dataPoints->valueAHU302_reShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU302_reShuiFaKaiDu,AHU302_reShuiFaKaiDu_LEN);
        byteFieldOffset += AHU302_reShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_xinFengWenDU)
    {
        devStatusTmp.valueAHU302_xinFengWenDU = exchangeBytes(gizY2X(AHU302_xinFengWenDU_RATIO,  AHU302_xinFengWenDU_ADDITION, dataPoints->valueAHU302_xinFengWenDU)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU302_xinFengWenDU,AHU302_xinFengWenDU_LEN);
        byteFieldOffset += AHU302_xinFengWenDU_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_jiaShiQIKaiDu)
    {
        devStatusTmp.valueAHU302_jiaShiQIKaiDu = exchangeBytes(gizY2X(AHU302_jiaShiQIKaiDu_RATIO,  AHU302_jiaShiQIKaiDu_ADDITION, dataPoints->valueAHU302_jiaShiQIKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU302_jiaShiQIKaiDu,AHU302_jiaShiQIKaiDu_LEN);
        byteFieldOffset += AHU302_jiaShiQIKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU302_beiYong)
    {
        devStatusTmp.valueAHU302_beiYong = exchangeBytes(gizY2X(AHU302_beiYong_RATIO,  AHU302_beiYong_ADDITION, dataPoints->valueAHU302_beiYong)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU302_beiYong,AHU302_beiYong_LEN);
        byteFieldOffset += AHU302_beiYong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_tempReal)
    {
        devStatusTmp.valueAHU303_tempReal = exchangeBytes(gizY2X(AHU303_tempReal_RATIO,  AHU303_tempReal_ADDITION, dataPoints->valueAHU303_tempReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU303_tempReal,AHU303_tempReal_LEN);
        byteFieldOffset += AHU303_tempReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_humiReal)
    {
        devStatusTmp.valueAHU303_humiReal = exchangeBytes(gizY2X(AHU303_humiReal_RATIO,  AHU303_humiReal_ADDITION, dataPoints->valueAHU303_humiReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU303_humiReal,AHU303_humiReal_LEN);
        byteFieldOffset += AHU303_humiReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_tempSet)
    {
        devStatusTmp.valueAHU303_tempSet = exchangeBytes(gizY2X(AHU303_tempSet_RATIO,  AHU303_tempSet_ADDITION, dataPoints->valueAHU303_tempSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU303_tempSet,AHU303_tempSet_LEN);
        byteFieldOffset += AHU303_tempSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_humiSet)
    {
        devStatusTmp.valueAHU303_humiSet = exchangeBytes(gizY2X(AHU303_humiSet_RATIO,  AHU303_humiSet_ADDITION, dataPoints->valueAHU303_humiSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU303_humiSet,AHU303_humiSet_LEN);
        byteFieldOffset += AHU303_humiSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_lengShuiFaKaiDu)
    {
        devStatusTmp.valueAHU303_lengShuiFaKaiDu = exchangeBytes(gizY2X(AHU303_lengShuiFaKaiDu_RATIO,  AHU303_lengShuiFaKaiDu_ADDITION, dataPoints->valueAHU303_lengShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU303_lengShuiFaKaiDu,AHU303_lengShuiFaKaiDu_LEN);
        byteFieldOffset += AHU303_lengShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_reShuiFaKaiDu)
    {
        devStatusTmp.valueAHU303_reShuiFaKaiDu = exchangeBytes(gizY2X(AHU303_reShuiFaKaiDu_RATIO,  AHU303_reShuiFaKaiDu_ADDITION, dataPoints->valueAHU303_reShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU303_reShuiFaKaiDu,AHU303_reShuiFaKaiDu_LEN);
        byteFieldOffset += AHU303_reShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_xinFengWenDU)
    {
        devStatusTmp.valueAHU303_xinFengWenDU = exchangeBytes(gizY2X(AHU303_xinFengWenDU_RATIO,  AHU303_xinFengWenDU_ADDITION, dataPoints->valueAHU303_xinFengWenDU)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU303_xinFengWenDU,AHU303_xinFengWenDU_LEN);
        byteFieldOffset += AHU303_xinFengWenDU_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_jiaShiQIKaiDu)
    {
        devStatusTmp.valueAHU303_jiaShiQIKaiDu = exchangeBytes(gizY2X(AHU303_jiaShiQIKaiDu_RATIO,  AHU303_jiaShiQIKaiDu_ADDITION, dataPoints->valueAHU303_jiaShiQIKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU303_jiaShiQIKaiDu,AHU303_jiaShiQIKaiDu_LEN);
        byteFieldOffset += AHU303_jiaShiQIKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU303_beiYong)
    {
        devStatusTmp.valueAHU303_beiYong = exchangeBytes(gizY2X(AHU303_beiYong_RATIO,  AHU303_beiYong_ADDITION, dataPoints->valueAHU303_beiYong)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU303_beiYong,AHU303_beiYong_LEN);
        byteFieldOffset += AHU303_beiYong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_tempReal)
    {
        devStatusTmp.valueAHU304_tempReal = exchangeBytes(gizY2X(AHU304_tempReal_RATIO,  AHU304_tempReal_ADDITION, dataPoints->valueAHU304_tempReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU304_tempReal,AHU304_tempReal_LEN);
        byteFieldOffset += AHU304_tempReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_humiReal)
    {
        devStatusTmp.valueAHU304_humiReal = exchangeBytes(gizY2X(AHU304_humiReal_RATIO,  AHU304_humiReal_ADDITION, dataPoints->valueAHU304_humiReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU304_humiReal,AHU304_humiReal_LEN);
        byteFieldOffset += AHU304_humiReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_tempSet)
    {
        devStatusTmp.valueAHU304_tempSet = exchangeBytes(gizY2X(AHU304_tempSet_RATIO,  AHU304_tempSet_ADDITION, dataPoints->valueAHU304_tempSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU304_tempSet,AHU304_tempSet_LEN);
        byteFieldOffset += AHU304_tempSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_humiSet)
    {
        devStatusTmp.valueAHU304_humiSet = exchangeBytes(gizY2X(AHU304_humiSet_RATIO,  AHU304_humiSet_ADDITION, dataPoints->valueAHU304_humiSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU304_humiSet,AHU304_humiSet_LEN);
        byteFieldOffset += AHU304_humiSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_lengShuiFaKaiDu)
    {
        devStatusTmp.valueAHU304_lengShuiFaKaiDu = exchangeBytes(gizY2X(AHU304_lengShuiFaKaiDu_RATIO,  AHU304_lengShuiFaKaiDu_ADDITION, dataPoints->valueAHU304_lengShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU304_lengShuiFaKaiDu,AHU304_lengShuiFaKaiDu_LEN);
        byteFieldOffset += AHU304_lengShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_reShuiFaKaiDu)
    {
        devStatusTmp.valueAHU304_reShuiFaKaiDu = exchangeBytes(gizY2X(AHU304_reShuiFaKaiDu_RATIO,  AHU304_reShuiFaKaiDu_ADDITION, dataPoints->valueAHU304_reShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU304_reShuiFaKaiDu,AHU304_reShuiFaKaiDu_LEN);
        byteFieldOffset += AHU304_reShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_xinFengWenDU)
    {
        devStatusTmp.valueAHU304_xinFengWenDU = exchangeBytes(gizY2X(AHU304_xinFengWenDU_RATIO,  AHU304_xinFengWenDU_ADDITION, dataPoints->valueAHU304_xinFengWenDU)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU304_xinFengWenDU,AHU304_xinFengWenDU_LEN);
        byteFieldOffset += AHU304_xinFengWenDU_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_jiaShiQIKaiDu)
    {
        devStatusTmp.valueAHU304_jiaShiQIKaiDu = exchangeBytes(gizY2X(AHU304_jiaShiQIKaiDu_RATIO,  AHU304_jiaShiQIKaiDu_ADDITION, dataPoints->valueAHU304_jiaShiQIKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU304_jiaShiQIKaiDu,AHU304_jiaShiQIKaiDu_LEN);
        byteFieldOffset += AHU304_jiaShiQIKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU304_beiYong)
    {
        devStatusTmp.valueAHU304_beiYong = exchangeBytes(gizY2X(AHU304_beiYong_RATIO,  AHU304_beiYong_ADDITION, dataPoints->valueAHU304_beiYong)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU304_beiYong,AHU304_beiYong_LEN);
        byteFieldOffset += AHU304_beiYong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_tempReal)
    {
        devStatusTmp.valueAHU305_tempReal = exchangeBytes(gizY2X(AHU305_tempReal_RATIO,  AHU305_tempReal_ADDITION, dataPoints->valueAHU305_tempReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU305_tempReal,AHU305_tempReal_LEN);
        byteFieldOffset += AHU305_tempReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_humiReal)
    {
        devStatusTmp.valueAHU305_humiReal = exchangeBytes(gizY2X(AHU305_humiReal_RATIO,  AHU305_humiReal_ADDITION, dataPoints->valueAHU305_humiReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU305_humiReal,AHU305_humiReal_LEN);
        byteFieldOffset += AHU305_humiReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_tempSet)
    {
        devStatusTmp.valueAHU305_tempSet = exchangeBytes(gizY2X(AHU305_tempSet_RATIO,  AHU305_tempSet_ADDITION, dataPoints->valueAHU305_tempSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU305_tempSet,AHU305_tempSet_LEN);
        byteFieldOffset += AHU305_tempSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_humiSet)
    {
        devStatusTmp.valueAHU305_humiSet = exchangeBytes(gizY2X(AHU305_humiSet_RATIO,  AHU305_humiSet_ADDITION, dataPoints->valueAHU305_humiSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU305_humiSet,AHU305_humiSet_LEN);
        byteFieldOffset += AHU305_humiSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_lengShuiFaKaiDu)
    {
        devStatusTmp.valueAHU305_lengShuiFaKaiDu = exchangeBytes(gizY2X(AHU305_lengShuiFaKaiDu_RATIO,  AHU305_lengShuiFaKaiDu_ADDITION, dataPoints->valueAHU305_lengShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU305_lengShuiFaKaiDu,AHU305_lengShuiFaKaiDu_LEN);
        byteFieldOffset += AHU305_lengShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_reShuiFaKaiDu)
    {
        devStatusTmp.valueAHU305_reShuiFaKaiDu = exchangeBytes(gizY2X(AHU305_reShuiFaKaiDu_RATIO,  AHU305_reShuiFaKaiDu_ADDITION, dataPoints->valueAHU305_reShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU305_reShuiFaKaiDu,AHU305_reShuiFaKaiDu_LEN);
        byteFieldOffset += AHU305_reShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_xinFengWenDU)
    {
        devStatusTmp.valueAHU305_xinFengWenDU = exchangeBytes(gizY2X(AHU305_xinFengWenDU_RATIO,  AHU305_xinFengWenDU_ADDITION, dataPoints->valueAHU305_xinFengWenDU)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU305_xinFengWenDU,AHU305_xinFengWenDU_LEN);
        byteFieldOffset += AHU305_xinFengWenDU_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_jiaShiQIKaiDu)
    {
        devStatusTmp.valueAHU305_jiaShiQIKaiDu = exchangeBytes(gizY2X(AHU305_jiaShiQIKaiDu_RATIO,  AHU305_jiaShiQIKaiDu_ADDITION, dataPoints->valueAHU305_jiaShiQIKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU305_jiaShiQIKaiDu,AHU305_jiaShiQIKaiDu_LEN);
        byteFieldOffset += AHU305_jiaShiQIKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU305_beiYong)
    {
        devStatusTmp.valueAHU305_beiYong = exchangeBytes(gizY2X(AHU305_beiYong_RATIO,  AHU305_beiYong_ADDITION, dataPoints->valueAHU305_beiYong)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU305_beiYong,AHU305_beiYong_LEN);
        byteFieldOffset += AHU305_beiYong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_tempReal)
    {
        devStatusTmp.valueAHU306_tempReal = exchangeBytes(gizY2X(AHU306_tempReal_RATIO,  AHU306_tempReal_ADDITION, dataPoints->valueAHU306_tempReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU306_tempReal,AHU306_tempReal_LEN);
        byteFieldOffset += AHU306_tempReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_humiReal)
    {
        devStatusTmp.valueAHU306_humiReal = exchangeBytes(gizY2X(AHU306_humiReal_RATIO,  AHU306_humiReal_ADDITION, dataPoints->valueAHU306_humiReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU306_humiReal,AHU306_humiReal_LEN);
        byteFieldOffset += AHU306_humiReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_tempSet)
    {
        devStatusTmp.valueAHU306_tempSet = exchangeBytes(gizY2X(AHU306_tempSet_RATIO,  AHU306_tempSet_ADDITION, dataPoints->valueAHU306_tempSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU306_tempSet,AHU306_tempSet_LEN);
        byteFieldOffset += AHU306_tempSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_humiSet)
    {
        devStatusTmp.valueAHU306_humiSet = exchangeBytes(gizY2X(AHU306_humiSet_RATIO,  AHU306_humiSet_ADDITION, dataPoints->valueAHU306_humiSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU306_humiSet,AHU306_humiSet_LEN);
        byteFieldOffset += AHU306_humiSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_lengShuiFaKaiDu)
    {
        devStatusTmp.valueAHU306_lengShuiFaKaiDu = exchangeBytes(gizY2X(AHU306_lengShuiFaKaiDu_RATIO,  AHU306_lengShuiFaKaiDu_ADDITION, dataPoints->valueAHU306_lengShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU306_lengShuiFaKaiDu,AHU306_lengShuiFaKaiDu_LEN);
        byteFieldOffset += AHU306_lengShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_reShuiFaKaiDu)
    {
        devStatusTmp.valueAHU306_reShuiFaKaiDu = exchangeBytes(gizY2X(AHU306_reShuiFaKaiDu_RATIO,  AHU306_reShuiFaKaiDu_ADDITION, dataPoints->valueAHU306_reShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU306_reShuiFaKaiDu,AHU306_reShuiFaKaiDu_LEN);
        byteFieldOffset += AHU306_reShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_xinFengWenDU)
    {
        devStatusTmp.valueAHU306_xinFengWenDU = exchangeBytes(gizY2X(AHU306_xinFengWenDU_RATIO,  AHU306_xinFengWenDU_ADDITION, dataPoints->valueAHU306_xinFengWenDU)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU306_xinFengWenDU,AHU306_xinFengWenDU_LEN);
        byteFieldOffset += AHU306_xinFengWenDU_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_jiaShiQIKaiDu)
    {
        devStatusTmp.valueAHU306_jiaShiQIKaiDu = exchangeBytes(gizY2X(AHU306_jiaShiQIKaiDu_RATIO,  AHU306_jiaShiQIKaiDu_ADDITION, dataPoints->valueAHU306_jiaShiQIKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU306_jiaShiQIKaiDu,AHU306_jiaShiQIKaiDu_LEN);
        byteFieldOffset += AHU306_jiaShiQIKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU306_beiYong)
    {
        devStatusTmp.valueAHU306_beiYong = exchangeBytes(gizY2X(AHU306_beiYong_RATIO,  AHU306_beiYong_ADDITION, dataPoints->valueAHU306_beiYong)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU306_beiYong,AHU306_beiYong_LEN);
        byteFieldOffset += AHU306_beiYong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_tempReal)
    {
        devStatusTmp.valueAHU307_tempReal = exchangeBytes(gizY2X(AHU307_tempReal_RATIO,  AHU307_tempReal_ADDITION, dataPoints->valueAHU307_tempReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU307_tempReal,AHU307_tempReal_LEN);
        byteFieldOffset += AHU307_tempReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_humiReal)
    {
        devStatusTmp.valueAHU307_humiReal = exchangeBytes(gizY2X(AHU307_humiReal_RATIO,  AHU307_humiReal_ADDITION, dataPoints->valueAHU307_humiReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU307_humiReal,AHU307_humiReal_LEN);
        byteFieldOffset += AHU307_humiReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_tempSet)
    {
        devStatusTmp.valueAHU307_tempSet = exchangeBytes(gizY2X(AHU307_tempSet_RATIO,  AHU307_tempSet_ADDITION, dataPoints->valueAHU307_tempSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU307_tempSet,AHU307_tempSet_LEN);
        byteFieldOffset += AHU307_tempSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_humiSet)
    {
        devStatusTmp.valueAHU307_humiSet = exchangeBytes(gizY2X(AHU307_humiSet_RATIO,  AHU307_humiSet_ADDITION, dataPoints->valueAHU307_humiSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU307_humiSet,AHU307_humiSet_LEN);
        byteFieldOffset += AHU307_humiSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_lengShuiFaKaiDu)
    {
        devStatusTmp.valueAHU307_lengShuiFaKaiDu = exchangeBytes(gizY2X(AHU307_lengShuiFaKaiDu_RATIO,  AHU307_lengShuiFaKaiDu_ADDITION, dataPoints->valueAHU307_lengShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU307_lengShuiFaKaiDu,AHU307_lengShuiFaKaiDu_LEN);
        byteFieldOffset += AHU307_lengShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_reShuiFaKaiDu)
    {
        devStatusTmp.valueAHU307_reShuiFaKaiDu = exchangeBytes(gizY2X(AHU307_reShuiFaKaiDu_RATIO,  AHU307_reShuiFaKaiDu_ADDITION, dataPoints->valueAHU307_reShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU307_reShuiFaKaiDu,AHU307_reShuiFaKaiDu_LEN);
        byteFieldOffset += AHU307_reShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_xinFengWenDU)
    {
        devStatusTmp.valueAHU307_xinFengWenDU = exchangeBytes(gizY2X(AHU307_xinFengWenDU_RATIO,  AHU307_xinFengWenDU_ADDITION, dataPoints->valueAHU307_xinFengWenDU)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU307_xinFengWenDU,AHU307_xinFengWenDU_LEN);
        byteFieldOffset += AHU307_xinFengWenDU_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_jiaShiQIKaiDu)
    {
        devStatusTmp.valueAHU307_jiaShiQIKaiDu = exchangeBytes(gizY2X(AHU307_jiaShiQIKaiDu_RATIO,  AHU307_jiaShiQIKaiDu_ADDITION, dataPoints->valueAHU307_jiaShiQIKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU307_jiaShiQIKaiDu,AHU307_jiaShiQIKaiDu_LEN);
        byteFieldOffset += AHU307_jiaShiQIKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU307_beiYong)
    {
        devStatusTmp.valueAHU307_beiYong = exchangeBytes(gizY2X(AHU307_beiYong_RATIO,  AHU307_beiYong_ADDITION, dataPoints->valueAHU307_beiYong)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU307_beiYong,AHU307_beiYong_LEN);
        byteFieldOffset += AHU307_beiYong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_tempReal)
    {
        devStatusTmp.valueAHU308_tempReal = exchangeBytes(gizY2X(AHU308_tempReal_RATIO,  AHU308_tempReal_ADDITION, dataPoints->valueAHU308_tempReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU308_tempReal,AHU308_tempReal_LEN);
        byteFieldOffset += AHU308_tempReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_humiReal)
    {
        devStatusTmp.valueAHU308_humiReal = exchangeBytes(gizY2X(AHU308_humiReal_RATIO,  AHU308_humiReal_ADDITION, dataPoints->valueAHU308_humiReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU308_humiReal,AHU308_humiReal_LEN);
        byteFieldOffset += AHU308_humiReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_tempSet)
    {
        devStatusTmp.valueAHU308_tempSet = exchangeBytes(gizY2X(AHU308_tempSet_RATIO,  AHU308_tempSet_ADDITION, dataPoints->valueAHU308_tempSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU308_tempSet,AHU308_tempSet_LEN);
        byteFieldOffset += AHU308_tempSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_humiSet)
    {
        devStatusTmp.valueAHU308_humiSet = exchangeBytes(gizY2X(AHU308_humiSet_RATIO,  AHU308_humiSet_ADDITION, dataPoints->valueAHU308_humiSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU308_humiSet,AHU308_humiSet_LEN);
        byteFieldOffset += AHU308_humiSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_lengShuiFaKaiDu)
    {
        devStatusTmp.valueAHU308_lengShuiFaKaiDu = exchangeBytes(gizY2X(AHU308_lengShuiFaKaiDu_RATIO,  AHU308_lengShuiFaKaiDu_ADDITION, dataPoints->valueAHU308_lengShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU308_lengShuiFaKaiDu,AHU308_lengShuiFaKaiDu_LEN);
        byteFieldOffset += AHU308_lengShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_reShuiFaKaiDu)
    {
        devStatusTmp.valueAHU308_reShuiFaKaiDu = exchangeBytes(gizY2X(AHU308_reShuiFaKaiDu_RATIO,  AHU308_reShuiFaKaiDu_ADDITION, dataPoints->valueAHU308_reShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU308_reShuiFaKaiDu,AHU308_reShuiFaKaiDu_LEN);
        byteFieldOffset += AHU308_reShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_xinFengWenDU)
    {
        devStatusTmp.valueAHU308_xinFengWenDU = exchangeBytes(gizY2X(AHU308_xinFengWenDU_RATIO,  AHU308_xinFengWenDU_ADDITION, dataPoints->valueAHU308_xinFengWenDU)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU308_xinFengWenDU,AHU308_xinFengWenDU_LEN);
        byteFieldOffset += AHU308_xinFengWenDU_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_jiaShiQIKaiDu)
    {
        devStatusTmp.valueAHU308_jiaShiQIKaiDu = exchangeBytes(gizY2X(AHU308_jiaShiQIKaiDu_RATIO,  AHU308_jiaShiQIKaiDu_ADDITION, dataPoints->valueAHU308_jiaShiQIKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU308_jiaShiQIKaiDu,AHU308_jiaShiQIKaiDu_LEN);
        byteFieldOffset += AHU308_jiaShiQIKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagAHU308_beiYong)
    {
        devStatusTmp.valueAHU308_beiYong = exchangeBytes(gizY2X(AHU308_beiYong_RATIO,  AHU308_beiYong_ADDITION, dataPoints->valueAHU308_beiYong)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valueAHU308_beiYong,AHU308_beiYong_LEN);
        byteFieldOffset += AHU308_beiYong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_tempReal)
    {
        devStatusTmp.valuePAU301_tempReal = exchangeBytes(gizY2X(PAU301_tempReal_RATIO,  PAU301_tempReal_ADDITION, dataPoints->valuePAU301_tempReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU301_tempReal,PAU301_tempReal_LEN);
        byteFieldOffset += PAU301_tempReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_humiReal)
    {
        devStatusTmp.valuePAU301_humiReal = exchangeBytes(gizY2X(PAU301_humiReal_RATIO,  PAU301_humiReal_ADDITION, dataPoints->valuePAU301_humiReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU301_humiReal,PAU301_humiReal_LEN);
        byteFieldOffset += PAU301_humiReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_tempSet)
    {
        devStatusTmp.valuePAU301_tempSet = exchangeBytes(gizY2X(PAU301_tempSet_RATIO,  PAU301_tempSet_ADDITION, dataPoints->valuePAU301_tempSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU301_tempSet,PAU301_tempSet_LEN);
        byteFieldOffset += PAU301_tempSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_humiSet)
    {
        devStatusTmp.valuePAU301_humiSet = exchangeBytes(gizY2X(PAU301_humiSet_RATIO,  PAU301_humiSet_ADDITION, dataPoints->valuePAU301_humiSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU301_humiSet,PAU301_humiSet_LEN);
        byteFieldOffset += PAU301_humiSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_lengShuiFaKaiDu)
    {
        devStatusTmp.valuePAU301_lengShuiFaKaiDu = exchangeBytes(gizY2X(PAU301_lengShuiFaKaiDu_RATIO,  PAU301_lengShuiFaKaiDu_ADDITION, dataPoints->valuePAU301_lengShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU301_lengShuiFaKaiDu,PAU301_lengShuiFaKaiDu_LEN);
        byteFieldOffset += PAU301_lengShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_reShuiFaKaiDu)
    {
        devStatusTmp.valuePAU301_reShuiFaKaiDu = exchangeBytes(gizY2X(PAU301_reShuiFaKaiDu_RATIO,  PAU301_reShuiFaKaiDu_ADDITION, dataPoints->valuePAU301_reShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU301_reShuiFaKaiDu,PAU301_reShuiFaKaiDu_LEN);
        byteFieldOffset += PAU301_reShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_xinFengWenDU)
    {
        devStatusTmp.valuePAU301_xinFengWenDU = exchangeBytes(gizY2X(PAU301_xinFengWenDU_RATIO,  PAU301_xinFengWenDU_ADDITION, dataPoints->valuePAU301_xinFengWenDU)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU301_xinFengWenDU,PAU301_xinFengWenDU_LEN);
        byteFieldOffset += PAU301_xinFengWenDU_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_jiaShiQIKaiDu)
    {
        devStatusTmp.valuePAU301_jiaShiQIKaiDu = exchangeBytes(gizY2X(PAU301_jiaShiQIKaiDu_RATIO,  PAU301_jiaShiQIKaiDu_ADDITION, dataPoints->valuePAU301_jiaShiQIKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU301_jiaShiQIKaiDu,PAU301_jiaShiQIKaiDu_LEN);
        byteFieldOffset += PAU301_jiaShiQIKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU301_beiYong)
    {
        devStatusTmp.valuePAU301_beiYong = exchangeBytes(gizY2X(PAU301_beiYong_RATIO,  PAU301_beiYong_ADDITION, dataPoints->valuePAU301_beiYong)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU301_beiYong,PAU301_beiYong_LEN);
        byteFieldOffset += PAU301_beiYong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_tempReal)
    {
        devStatusTmp.valuePAU302_tempReal = exchangeBytes(gizY2X(PAU302_tempReal_RATIO,  PAU302_tempReal_ADDITION, dataPoints->valuePAU302_tempReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU302_tempReal,PAU302_tempReal_LEN);
        byteFieldOffset += PAU302_tempReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_humiReal)
    {
        devStatusTmp.valuePAU302_humiReal = exchangeBytes(gizY2X(PAU302_humiReal_RATIO,  PAU302_humiReal_ADDITION, dataPoints->valuePAU302_humiReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU302_humiReal,PAU302_humiReal_LEN);
        byteFieldOffset += PAU302_humiReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_tempSet)
    {
        devStatusTmp.valuePAU302_tempSet = exchangeBytes(gizY2X(PAU302_tempSet_RATIO,  PAU302_tempSet_ADDITION, dataPoints->valuePAU302_tempSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU302_tempSet,PAU302_tempSet_LEN);
        byteFieldOffset += PAU302_tempSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_humiSet)
    {
        devStatusTmp.valuePAU302_humiSet = exchangeBytes(gizY2X(PAU302_humiSet_RATIO,  PAU302_humiSet_ADDITION, dataPoints->valuePAU302_humiSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU302_humiSet,PAU302_humiSet_LEN);
        byteFieldOffset += PAU302_humiSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_lengShuiFaKaiDu)
    {
        devStatusTmp.valuePAU302_lengShuiFaKaiDu = exchangeBytes(gizY2X(PAU302_lengShuiFaKaiDu_RATIO,  PAU302_lengShuiFaKaiDu_ADDITION, dataPoints->valuePAU302_lengShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU302_lengShuiFaKaiDu,PAU302_lengShuiFaKaiDu_LEN);
        byteFieldOffset += PAU302_lengShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_reShuiFaKaiDu)
    {
        devStatusTmp.valuePAU302_reShuiFaKaiDu = exchangeBytes(gizY2X(PAU302_reShuiFaKaiDu_RATIO,  PAU302_reShuiFaKaiDu_ADDITION, dataPoints->valuePAU302_reShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU302_reShuiFaKaiDu,PAU302_reShuiFaKaiDu_LEN);
        byteFieldOffset += PAU302_reShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_xinFengWenDU)
    {
        devStatusTmp.valuePAU302_xinFengWenDU = exchangeBytes(gizY2X(PAU302_xinFengWenDU_RATIO,  PAU302_xinFengWenDU_ADDITION, dataPoints->valuePAU302_xinFengWenDU)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU302_xinFengWenDU,PAU302_xinFengWenDU_LEN);
        byteFieldOffset += PAU302_xinFengWenDU_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_jiaShiQIKaiDu)
    {
        devStatusTmp.valuePAU302_jiaShiQIKaiDu = exchangeBytes(gizY2X(PAU302_jiaShiQIKaiDu_RATIO,  PAU302_jiaShiQIKaiDu_ADDITION, dataPoints->valuePAU302_jiaShiQIKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU302_jiaShiQIKaiDu,PAU302_jiaShiQIKaiDu_LEN);
        byteFieldOffset += PAU302_jiaShiQIKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU302_beiYong)
    {
        devStatusTmp.valuePAU302_beiYong = exchangeBytes(gizY2X(PAU302_beiYong_RATIO,  PAU302_beiYong_ADDITION, dataPoints->valuePAU302_beiYong)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU302_beiYong,PAU302_beiYong_LEN);
        byteFieldOffset += PAU302_beiYong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_tempReal)
    {
        devStatusTmp.valuePAU303_tempReal = exchangeBytes(gizY2X(PAU303_tempReal_RATIO,  PAU303_tempReal_ADDITION, dataPoints->valuePAU303_tempReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU303_tempReal,PAU303_tempReal_LEN);
        byteFieldOffset += PAU303_tempReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_humiReal)
    {
        devStatusTmp.valuePAU303_humiReal = exchangeBytes(gizY2X(PAU303_humiReal_RATIO,  PAU303_humiReal_ADDITION, dataPoints->valuePAU303_humiReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU303_humiReal,PAU303_humiReal_LEN);
        byteFieldOffset += PAU303_humiReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_tempSet)
    {
        devStatusTmp.valuePAU303_tempSet = exchangeBytes(gizY2X(PAU303_tempSet_RATIO,  PAU303_tempSet_ADDITION, dataPoints->valuePAU303_tempSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU303_tempSet,PAU303_tempSet_LEN);
        byteFieldOffset += PAU303_tempSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_humiSet)
    {
        devStatusTmp.valuePAU303_humiSet = exchangeBytes(gizY2X(PAU303_humiSet_RATIO,  PAU303_humiSet_ADDITION, dataPoints->valuePAU303_humiSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU303_humiSet,PAU303_humiSet_LEN);
        byteFieldOffset += PAU303_humiSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_lengShuiFaKaiDu)
    {
        devStatusTmp.valuePAU303_lengShuiFaKaiDu = exchangeBytes(gizY2X(PAU303_lengShuiFaKaiDu_RATIO,  PAU303_lengShuiFaKaiDu_ADDITION, dataPoints->valuePAU303_lengShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU303_lengShuiFaKaiDu,PAU303_lengShuiFaKaiDu_LEN);
        byteFieldOffset += PAU303_lengShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_reShuiFaKaiDu)
    {
        devStatusTmp.valuePAU303_reShuiFaKaiDu = exchangeBytes(gizY2X(PAU303_reShuiFaKaiDu_RATIO,  PAU303_reShuiFaKaiDu_ADDITION, dataPoints->valuePAU303_reShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU303_reShuiFaKaiDu,PAU303_reShuiFaKaiDu_LEN);
        byteFieldOffset += PAU303_reShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_xinFengWenDU)
    {
        devStatusTmp.valuePAU303_xinFengWenDU = exchangeBytes(gizY2X(PAU303_xinFengWenDU_RATIO,  PAU303_xinFengWenDU_ADDITION, dataPoints->valuePAU303_xinFengWenDU)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU303_xinFengWenDU,PAU303_xinFengWenDU_LEN);
        byteFieldOffset += PAU303_xinFengWenDU_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_jiaShiQIKaiDu)
    {
        devStatusTmp.valuePAU303_jiaShiQIKaiDu = exchangeBytes(gizY2X(PAU303_jiaShiQIKaiDu_RATIO,  PAU303_jiaShiQIKaiDu_ADDITION, dataPoints->valuePAU303_jiaShiQIKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU303_jiaShiQIKaiDu,PAU303_jiaShiQIKaiDu_LEN);
        byteFieldOffset += PAU303_jiaShiQIKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU303_beiYong)
    {
        devStatusTmp.valuePAU303_beiYong = exchangeBytes(gizY2X(PAU303_beiYong_RATIO,  PAU303_beiYong_ADDITION, dataPoints->valuePAU303_beiYong)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU303_beiYong,PAU303_beiYong_LEN);
        byteFieldOffset += PAU303_beiYong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_tempReal)
    {
        devStatusTmp.valuePAU304_tempReal = exchangeBytes(gizY2X(PAU304_tempReal_RATIO,  PAU304_tempReal_ADDITION, dataPoints->valuePAU304_tempReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU304_tempReal,PAU304_tempReal_LEN);
        byteFieldOffset += PAU304_tempReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_humiReal)
    {
        devStatusTmp.valuePAU304_humiReal = exchangeBytes(gizY2X(PAU304_humiReal_RATIO,  PAU304_humiReal_ADDITION, dataPoints->valuePAU304_humiReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU304_humiReal,PAU304_humiReal_LEN);
        byteFieldOffset += PAU304_humiReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_tempSet)
    {
        devStatusTmp.valuePAU304_tempSet = exchangeBytes(gizY2X(PAU304_tempSet_RATIO,  PAU304_tempSet_ADDITION, dataPoints->valuePAU304_tempSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU304_tempSet,PAU304_tempSet_LEN);
        byteFieldOffset += PAU304_tempSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_humiSet)
    {
        devStatusTmp.valuePAU304_humiSet = exchangeBytes(gizY2X(PAU304_humiSet_RATIO,  PAU304_humiSet_ADDITION, dataPoints->valuePAU304_humiSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU304_humiSet,PAU304_humiSet_LEN);
        byteFieldOffset += PAU304_humiSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_lengShuiFaKaiDu)
    {
        devStatusTmp.valuePAU304_lengShuiFaKaiDu = exchangeBytes(gizY2X(PAU304_lengShuiFaKaiDu_RATIO,  PAU304_lengShuiFaKaiDu_ADDITION, dataPoints->valuePAU304_lengShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU304_lengShuiFaKaiDu,PAU304_lengShuiFaKaiDu_LEN);
        byteFieldOffset += PAU304_lengShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_reShuiFaKaiDu)
    {
        devStatusTmp.valuePAU304_reShuiFaKaiDu = exchangeBytes(gizY2X(PAU304_reShuiFaKaiDu_RATIO,  PAU304_reShuiFaKaiDu_ADDITION, dataPoints->valuePAU304_reShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU304_reShuiFaKaiDu,PAU304_reShuiFaKaiDu_LEN);
        byteFieldOffset += PAU304_reShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_xinFengWenDU)
    {
        devStatusTmp.valuePAU304_xinFengWenDU = exchangeBytes(gizY2X(PAU304_xinFengWenDU_RATIO,  PAU304_xinFengWenDU_ADDITION, dataPoints->valuePAU304_xinFengWenDU)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU304_xinFengWenDU,PAU304_xinFengWenDU_LEN);
        byteFieldOffset += PAU304_xinFengWenDU_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_jiaShiQIKaiDu)
    {
        devStatusTmp.valuePAU304_jiaShiQIKaiDu = exchangeBytes(gizY2X(PAU304_jiaShiQIKaiDu_RATIO,  PAU304_jiaShiQIKaiDu_ADDITION, dataPoints->valuePAU304_jiaShiQIKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU304_jiaShiQIKaiDu,PAU304_jiaShiQIKaiDu_LEN);
        byteFieldOffset += PAU304_jiaShiQIKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU304_beiYong)
    {
        devStatusTmp.valuePAU304_beiYong = exchangeBytes(gizY2X(PAU304_beiYong_RATIO,  PAU304_beiYong_ADDITION, dataPoints->valuePAU304_beiYong)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU304_beiYong,PAU304_beiYong_LEN);
        byteFieldOffset += PAU304_beiYong_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_tempReal)
    {
        devStatusTmp.valuePAU305_tempReal = exchangeBytes(gizY2X(PAU305_tempReal_RATIO,  PAU305_tempReal_ADDITION, dataPoints->valuePAU305_tempReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU305_tempReal,PAU305_tempReal_LEN);
        byteFieldOffset += PAU305_tempReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_humiReal)
    {
        devStatusTmp.valuePAU305_humiReal = exchangeBytes(gizY2X(PAU305_humiReal_RATIO,  PAU305_humiReal_ADDITION, dataPoints->valuePAU305_humiReal)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU305_humiReal,PAU305_humiReal_LEN);
        byteFieldOffset += PAU305_humiReal_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_tempSet)
    {
        devStatusTmp.valuePAU305_tempSet = exchangeBytes(gizY2X(PAU305_tempSet_RATIO,  PAU305_tempSet_ADDITION, dataPoints->valuePAU305_tempSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU305_tempSet,PAU305_tempSet_LEN);
        byteFieldOffset += PAU305_tempSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_humiSet)
    {
        devStatusTmp.valuePAU305_humiSet = exchangeBytes(gizY2X(PAU305_humiSet_RATIO,  PAU305_humiSet_ADDITION, dataPoints->valuePAU305_humiSet)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU305_humiSet,PAU305_humiSet_LEN);
        byteFieldOffset += PAU305_humiSet_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_lengShuiFaKaiDu)
    {
        devStatusTmp.valuePAU305_lengShuiFaKaiDu = exchangeBytes(gizY2X(PAU305_lengShuiFaKaiDu_RATIO,  PAU305_lengShuiFaKaiDu_ADDITION, dataPoints->valuePAU305_lengShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU305_lengShuiFaKaiDu,PAU305_lengShuiFaKaiDu_LEN);
        byteFieldOffset += PAU305_lengShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_reShuiFaKaiDu)
    {
        devStatusTmp.valuePAU305_reShuiFaKaiDu = exchangeBytes(gizY2X(PAU305_reShuiFaKaiDu_RATIO,  PAU305_reShuiFaKaiDu_ADDITION, dataPoints->valuePAU305_reShuiFaKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU305_reShuiFaKaiDu,PAU305_reShuiFaKaiDu_LEN);
        byteFieldOffset += PAU305_reShuiFaKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_xinFengWenDU)
    {
        devStatusTmp.valuePAU305_xinFengWenDU = exchangeBytes(gizY2X(PAU305_xinFengWenDU_RATIO,  PAU305_xinFengWenDU_ADDITION, dataPoints->valuePAU305_xinFengWenDU)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU305_xinFengWenDU,PAU305_xinFengWenDU_LEN);
        byteFieldOffset += PAU305_xinFengWenDU_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_jiaShiQIKaiDu)
    {
        devStatusTmp.valuePAU305_jiaShiQIKaiDu = exchangeBytes(gizY2X(PAU305_jiaShiQIKaiDu_RATIO,  PAU305_jiaShiQIKaiDu_ADDITION, dataPoints->valuePAU305_jiaShiQIKaiDu)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU305_jiaShiQIKaiDu,PAU305_jiaShiQIKaiDu_LEN);
        byteFieldOffset += PAU305_jiaShiQIKaiDu_LEN;
    }
    if(gizwitsProtocol.waitReportDatapointFlag.flagPAU305_beiYong)
    {
        devStatusTmp.valuePAU305_beiYong = exchangeBytes(gizY2X(PAU305_beiYong_RATIO,  PAU305_beiYong_ADDITION, dataPoints->valuePAU305_beiYong)); 
        gizMemcpy((uint8_t *)&allDatapointByteBuf[byteFieldOffset],(uint8_t *)&devStatusTmp.valuePAU305_beiYong,PAU305_beiYong_LEN);
        byteFieldOffset += PAU305_beiYong_LEN;
    }




    gizMemset((uint8_t *)&gizwitsProtocol.waitReportDatapointFlag,0,DATAPOINT_FLAG_LEN);//Clear the flag
    *outDataLen = byteFieldOffset;
    gizMemcpy(outData,allDatapointByteBuf,*outDataLen);
    return 0;
}


/**
* @brief This function is called by the Gagent module to receive the relevant protocol data from the cloud or APP
* @param [in] inData The protocol data entered
* @param [in] inLen Enter the length of the data
* @param [out] outData The output of the protocol data
* @param [out] outLen The length of the output data
* @return 0, the implementation of success, non-0, failed
*/
static int8_t gizProtocolIssuedProcess(char *did, uint8_t *inData, uint32_t inLen, uint8_t *outData, uint32_t *outLen)
{
    uint8_t issuedAction = inData[0];

    if((NULL == inData)||(NULL == outData)||(NULL == outLen))
    {
        GIZWITS_LOG("gizProtocolIssuedProcess Error , Illegal Param\n");
        return -1;
    }

    if(NULL == did)
    {
    memset((uint8_t *)&gizwitsProtocol.issuedProcessEvent, 0, sizeof(eventInfo_t));
    switch(issuedAction)
    {
        case ACTION_CONTROL_DEVICE:
            GIZWITS_LOG("ACTION_ELONGATE_CONTROL_DEVICE \n");
            gizDataPoint2Event(&inData[1], &gizwitsProtocol.issuedProcessEvent,&gizwitsProtocol.gizCurrentDataPoint);
            gizwitsProtocol.issuedFlag = ACTION_CONTROL_TYPE;
            outData = NULL;
            *outLen = 0;
            break;
        
        case ACTION_READ_DEV_STATUS:
            GIZWITS_LOG("ACTION_ELONGATE_READ_DEV_STATUS \n");          
            memcpy((uint8_t *)&gizwitsProtocol.waitReportDatapointFlag,&inData[1],DATAPOINT_FLAG_LEN);//拷贝查询FLAG               
            if(DATAPOINT_FLAG_LEN > 1)
            {
                    gizByteOrderExchange((uint8_t *)&gizwitsProtocol.waitReportDatapointFlag,DATAPOINT_FLAG_LEN);
            }
                        
            if(0 == gizDataPoints2ReportData(&gizwitsProtocol.gizLastDataPoint,gizwitsProtocol.reportData,(uint32_t *)&gizwitsProtocol.reportDataLen))
            {  
                memcpy(outData+1, (uint8_t *)&gizwitsProtocol.reportData, gizwitsProtocol.reportDataLen);
                outData[0] = ACTION_READ_DEV_STATUS_ACK;
                *outLen = DATAPOINT_FLAG_LEN + sizeof(gizwitsReport_t)+1;
            }
            else
            {
                return -1;
            }
            break;
        case ACTION_W2D_TRANSPARENT_DATA:
            memcpy(gizwitsProtocol.transparentBuff, &inData[1], inLen-1);
            gizwitsProtocol.transparentLen = inLen - 1;
            
            gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = TRANSPARENT_DATA;
            gizwitsProtocol.issuedProcessEvent.num++;
            gizwitsProtocol.issuedFlag = ACTION_W2D_TRANSPARENT_TYPE;
            outData = NULL;
            *outLen = 0;
            break;
        
        default:
                    break;
        }
    }

    return 0;
}

/**
* @brief The protocol sends data back , P0 ACK
*
* @param [in] head                  : Protocol head pointer
* @param [in] data                  : Payload data 
* @param [in] len                   : Payload data length
* @param [in] proFlag               : DID flag ,1 for Virtual sub device did ,0 for single product or gateway 
*
* @return : 0,Ack success;
*           -1，Input Param Illegal
*           -2，Serial send faild
*/
static int32_t gizProtocolIssuedDataAck(protocolHead_t *head, uint8_t *gizdata, uint32_t len, uint8_t proFlag)
{
    int32_t ret = 0;
    uint8_t tx_buf[RB_MAX_LEN];
    uint32_t offset = 0;
    uint8_t sDidLen = 0;
    uint16_t data_len = 0;
	uint8_t *pTxBuf = tx_buf;
    if(NULL == gizdata)
    {
        GIZWITS_LOG("[ERR]  data Is Null \n");
        return -1;
    }
    

    if(0x1 == proFlag)
    {
        sDidLen = *((uint8_t *)head + sizeof(protocolHead_t));
        data_len = 5 + 1 + sDidLen + len;   
    }
    else
    {
        data_len = 5 + len;
    }
    GIZWITS_LOG("len = %d , sDidLen = %d ,data_len = %d\n", len,sDidLen,data_len);
    *pTxBuf ++= 0xFF;
    *pTxBuf ++= 0xFF;
    *pTxBuf ++= (uint8_t)(data_len>>8);//len
    *pTxBuf ++= (uint8_t)(data_len);
    *pTxBuf ++= head->cmd + 1;
    *pTxBuf ++= head->sn;
    *pTxBuf ++= 0x00;
    *pTxBuf ++= proFlag;//flag
    offset = 8;
    if(0x1 == proFlag)
    {
        *pTxBuf ++= sDidLen;//didlen
        offset += 1;
        memcpy(&tx_buf[offset],(uint8_t *)head+sizeof(protocolHead_t)+1,sDidLen);
        offset += sDidLen;
        pTxBuf += sDidLen;

    }
    if(0 != len)
    {
        memcpy(&tx_buf[offset],gizdata,len);
    }
    tx_buf[data_len + 4 - 1 ] = gizProtocolSum( tx_buf , (data_len+4));

    ret = uartWrite(tx_buf, data_len+4);
    if(ret < 0)
    {
        GIZWITS_LOG("uart write error %d \n", ret);
        return -2;
    }

    return 0;
}
/**
* @brief Report data interface
*
* @param [in] action            : PO action
* @param [in] data              : Payload data
* @param [in] len               : Payload data length
*
* @return : 0,Ack success;
*           -1，Input Param Illegal
*           -2，Serial send faild
*/
static int32_t gizReportData(uint8_t action, uint8_t *gizdata, uint32_t len)
{
    int32_t ret = 0;
    uint8_t tx_buf[MAX_PACKAGE_LEN];
	uint8_t *pTxBuf = tx_buf;
    uint16_t data_len = 6 + len;
    if(NULL == gizdata)
    {
        GIZWITS_LOG("gizReportData Error , Illegal Param\n");
        return -1;
    }

    *pTxBuf ++= 0xFF;
    *pTxBuf ++= 0xFF;
    *pTxBuf ++= (uint8_t)(data_len>>8);
    *pTxBuf ++= (uint8_t)(data_len);
    *pTxBuf ++= CMD_REPORT_P0;
    *pTxBuf ++= gizwitsProtocol.sn++;
    *pTxBuf ++= 0x00;
    *pTxBuf ++= 0x00;
    *pTxBuf ++= action;

    memcpy(&tx_buf[9],gizdata,len);
    
    tx_buf[data_len + 4 - 1 ] = gizProtocolSum( tx_buf , (data_len+4));
    
    ret = uartWrite(tx_buf, data_len+4);
    if(ret < 0)
    {
        GIZWITS_LOG("ERR: uart write error %d \n", ret);
        return -2;
    }

    gizProtocolWaitAck(tx_buf, data_len+4);

    return 0;
}/**
* @brief Datapoints reporting mechanism		数据报机制
*
* 1. Changes are reported immediately		立即报告更改

* 2. Data timing report , 600000 Millisecond 数据定时报告
* 
*@param [in] currentData       : Current datapoints value
* @return : NULL
*/
static void gizDevReportPolicy(dataPoint_t *currentData)
{
    static uint32_t lastRepTime = 0;
    uint32_t timeNow = gizGetTimerCount();
    uint8_t *waitReportDataPtr = NULL;

    if((1 == gizCheckReport(currentData, (dataPoint_t *)&gizwitsProtocol.gizLastDataPoint)))
    {
        GIZWITS_LOG("changed, report data\n");
        if(0 == gizDataPoints2ReportData(currentData,gizwitsProtocol.reportData,(uint32_t *)&gizwitsProtocol.reportDataLen))
        {
            gizReportData(ACTION_REPORT_DEV_STATUS, gizwitsProtocol.reportData, gizwitsProtocol.reportDataLen);        }
        memcpy((uint8_t *)&gizwitsProtocol.gizLastDataPoint, (uint8_t *)currentData, sizeof(dataPoint_t));
    }

    if((0 == (timeNow % (600000))) && (lastRepTime != timeNow))
    {
        GIZWITS_LOG("Info: 600S report data\n");
        memset((uint8_t *)&gizwitsProtocol.waitReportDatapointFlag,0xFF,DATAPOINT_FLAG_LEN);
        if(0 == gizDataPoints2ReportData(currentData,gizwitsProtocol.reportData,(uint32_t *)&gizwitsProtocol.reportDataLen))
        {
            gizReportData(ACTION_REPORT_DEV_STATUS, gizwitsProtocol.reportData, gizwitsProtocol.reportDataLen);        }
        memcpy((uint8_t *)&gizwitsProtocol.gizLastDataPoint, (uint8_t *)currentData, sizeof(dataPoint_t));

        lastRepTime = timeNow;
    }

	if (1 == currentDataPoint.valueonline_flag)
	{
		currentDataPoint.valueonline_flag = 0;
		GIZWITS_LOG("dev online report data\n");
		memset((uint8_t *)&gizwitsProtocol.waitReportDatapointFlag, 0xFF, DATAPOINT_FLAG_LEN);
		if (0 == gizDataPoints2ReportData(currentData, gizwitsProtocol.reportData, (uint32_t *)&gizwitsProtocol.reportDataLen))
		{
			gizReportData(ACTION_REPORT_DEV_STATUS, gizwitsProtocol.reportData, gizwitsProtocol.reportDataLen);
		}
		memcpy((uint8_t *)&gizwitsProtocol.gizLastDataPoint, (uint8_t *)currentData, sizeof(dataPoint_t));

		lastRepTime = timeNow;
	}

    free(waitReportDataPtr);
}

/**
* @brief Get a packet of data from the ring buffer
*
* @param [in]  rb                  : Input data address
* @param [out] data                : Output data address
* @param [out] len                 : Output data length
*
* @return : 0,Return correct ;-1，Return failure;-2，Data check failure
*/
static int8_t gizProtocolGetOnePacket(rb_t *rb, uint8_t *gizdata, uint16_t *len)
{
    int32_t ret = 0;
    uint8_t sum = 0;
    int32_t i = 0;
    uint8_t tmpData;
    uint8_t tmpLen = 0;
    uint16_t tmpCount = 0;
    static uint8_t protocolFlag = 0;
    static uint16_t protocolCount = 0;
    static uint8_t lastData = 0;
    static uint8_t debugCount = 0;
    uint8_t *protocolBuff = gizdata;
    protocolHead_t *head = NULL;

    if((NULL == rb) || (NULL == gizdata) ||(NULL == len))
    {
        GIZWITS_LOG("gizProtocolGetOnePacket Error , Illegal Param\n");
        return -1;
    }

    tmpLen = rbCanRead(rb);
    if(0 == tmpLen)
    {
        return -1;
    }

    for(i=0; i<tmpLen; i++)
    {
        ret = rbRead(rb, &tmpData, 1);
        if(0 != ret)
        {
            if((0xFF == lastData) && (0xFF == tmpData))
            {
                if(0 == protocolFlag)
                {
                    protocolBuff[0] = 0xFF;
                    protocolBuff[1] = 0xFF;
                    protocolCount = 2;
                    protocolFlag = 1;
                }
                else
                {
                    if((protocolCount > 4) && (protocolCount != tmpCount))
                    {
                        protocolBuff[0] = 0xFF;
                        protocolBuff[1] = 0xFF;
                        protocolCount = 2;
                    }
                }
            }
            else if((0xFF == lastData) && (0x55 == tmpData))
            {
            }
            else
            {
                if(1 == protocolFlag)
                {
                    protocolBuff[protocolCount] = tmpData;
                    protocolCount++;

                    if(protocolCount > 4)
                    {
                        head = (protocolHead_t *)protocolBuff;
                        tmpCount = exchangeBytes(head->len)+4;
                        if(protocolCount == tmpCount)
                        {
                            break;
                        }
                    }
                }
            }

            lastData = tmpData;
            debugCount++;
        }
    }

    if((protocolCount > 4) && (protocolCount == tmpCount))
    {
        sum = gizProtocolSum(protocolBuff, protocolCount);

        if(protocolBuff[protocolCount-1] == sum)
        {
            memcpy(gizdata, protocolBuff, tmpCount);
            *len = tmpCount;
            protocolFlag = 0;

            protocolCount = 0;
            debugCount = 0;
            lastData = 0;

            return 0;
        }
        else
        {
            return -2;
        }
    }

    return 1;
}



/**
* @brief Protocol data resend

* The protocol data resend when check timeout and meet the resend limiting

* @param none    
*
* @return none
*/
static void gizProtocolResendData(void)
{
    int32_t ret = 0;

    if(0 == gizwitsProtocol.waitAck.flag)
    {
        return;
    }

    GIZWITS_LOG("Warning: timeout, resend data \n");
    
    ret = uartWrite(gizwitsProtocol.waitAck.buf, gizwitsProtocol.waitAck.dataLen);
    if(ret != gizwitsProtocol.waitAck.dataLen)
    {
        GIZWITS_LOG("ERR: resend data error\n");
    }

    gizwitsProtocol.waitAck.sendTime = gizGetTimerCount();
}

/**
* @brief Clear the ACK protocol message
*
* @param [in] head : Protocol header address
*
* @return 0， success; other， failure
*/
static int8_t gizProtocolWaitAckCheck(protocolHead_t *head)
{
    protocolHead_t *waitAckHead = (protocolHead_t *)gizwitsProtocol.waitAck.buf;

    if(NULL == head)
    {
        GIZWITS_LOG("ERR: data is empty \n");
        return -1;
    }

    if(waitAckHead->cmd+1 == head->cmd)
    {
        memset((uint8_t *)&gizwitsProtocol.waitAck, 0, sizeof(protocolWaitAck_t));
    }

    return 0;
}

/**
* @brief Send general protocol message data
* 
* @param [in] head              : Protocol header address
*
* @return : Return effective data length;-1，return failure
*/
static int32_t gizProtocolCommonAck(protocolHead_t *head)
{
    int32_t ret = 0;
    protocolCommon_t ack;

    if(NULL == head)
    {
        GIZWITS_LOG("ERR: gizProtocolCommonAck data is empty \n");
        return -1;
    }
    memcpy((uint8_t *)&ack, (uint8_t *)head, sizeof(protocolHead_t));
    ack.head.cmd = ack.head.cmd+1;
    ack.head.len = exchangeBytes(sizeof(protocolCommon_t)-4);
    ack.sum = gizProtocolSum((uint8_t *)&ack, sizeof(protocolCommon_t));

    ret = uartWrite((uint8_t *)&ack, sizeof(protocolCommon_t));
    if(ret < 0)
    {
        GIZWITS_LOG("ERR: uart write error %d \n", ret);
    }

    return ret;
}

/**
* @brief ACK processing function

* Time-out 200ms no ACK resend，resend two times at most

* @param none 
*
* @return none
*/
static void gizProtocolAckHandle(void)
{
    if(1 == gizwitsProtocol.waitAck.flag)
    {
        if(SEND_MAX_NUM > gizwitsProtocol.waitAck.num)
        {
            // Time-out no ACK resend
            if(SEND_MAX_TIME < (gizGetTimerCount() - gizwitsProtocol.waitAck.sendTime))
            {
                GIZWITS_LOG("Warning:gizProtocolResendData %d %d %d\n", gizGetTimerCount(), gizwitsProtocol.waitAck.sendTime, gizwitsProtocol.waitAck.num);
                gizProtocolResendData();
                gizwitsProtocol.waitAck.num++;
            }
        }
        else
        {
            memset((uint8_t *)&gizwitsProtocol.waitAck, 0, sizeof(protocolWaitAck_t));
			mcuRestart();
        }
    }
}

/**
* @brief Protocol 4.1 WiFi module requests device information
*
* @param[in] head : Protocol header address
*
* @return Return effective data length;-1，return failure
*/
static int32_t gizProtocolGetDeviceInfo(protocolHead_t * head)
{
    int32_t ret = 0;
    protocolDeviceInfo_t deviceInfo;

    if(NULL == head)
    {
        GIZWITS_LOG("gizProtocolGetDeviceInfo Error , Illegal Param\n");
        return -1;
    }

    gizProtocolHeadInit((protocolHead_t *)&deviceInfo);
    deviceInfo.head.cmd = ACK_GET_DEVICE_INFO;
    deviceInfo.head.sn = head->sn;
    memcpy((uint8_t *)deviceInfo.protocolVer, protocol_VERSION, 8);
    memcpy((uint8_t *)deviceInfo.p0Ver, P0_VERSION, 8);
    memcpy((uint8_t *)deviceInfo.softVer, SOFTWARE_VERSION, 8);
    memcpy((uint8_t *)deviceInfo.hardVer, HARDWARE_VERSION, 8);
    memcpy((uint8_t *)deviceInfo.productKey, PRODUCT_KEY, strlen(PRODUCT_KEY));
    memcpy((uint8_t *)deviceInfo.productSecret, PRODUCT_SECRET, strlen(PRODUCT_SECRET));
    memset((uint8_t *)deviceInfo.devAttr, 0, 8);
    deviceInfo.devAttr[7] |= DEV_IS_GATEWAY<<0;
    deviceInfo.devAttr[7] |= (0x01<<1);
    deviceInfo.ninableTime = exchangeBytes(NINABLETIME);
    deviceInfo.head.len = exchangeBytes(sizeof(protocolDeviceInfo_t)-4);
    deviceInfo.sum = gizProtocolSum((uint8_t *)&deviceInfo, sizeof(protocolDeviceInfo_t));

    ret = uartWrite((uint8_t *)&deviceInfo, sizeof(protocolDeviceInfo_t));
    if(ret < 0)
    {
        GIZWITS_LOG("ERR: uart write error %d \n", ret);
    }
    
    return ret;
}

/**
* @brief Protocol 4.7 Handling of illegal message notification

* @param[in] head  : Protocol header address
* @param[in] errno : Illegal message notification type
* @return 0， success; other， failure
*/
static int32_t gizProtocolErrorCmd(protocolHead_t *head,errorPacketsType_t errno)
{
    int32_t ret = 0;
    protocolErrorType_t errorType;

    if(NULL == head)
    {
        GIZWITS_LOG("gizProtocolErrorCmd Error , Illegal Param\n");
        return -1;
    }
    gizProtocolHeadInit((protocolHead_t *)&errorType);
    errorType.head.cmd = ACK_ERROR_PACKAGE;
    errorType.head.sn = head->sn;
    
    errorType.head.len = exchangeBytes(sizeof(protocolErrorType_t)-4);
    errorType.error = errno;
    errorType.sum = gizProtocolSum((uint8_t *)&errorType, sizeof(protocolErrorType_t));
    
    ret = uartWrite((uint8_t *)&errorType, sizeof(protocolErrorType_t));
    if(ret < 0)
    {
        GIZWITS_LOG("ERR: uart write error %d \n", ret);
    }

    return ret;
}

/**
* @brief Protocol 4.13 Get and process network time
*
* @param [in] head : Protocol header address
*
* @return 0， success; other， failure
*/
static int8_t gizProtocolNTP(protocolHead_t *head)
{  
    protocolUTT_t *UTTInfo = (protocolUTT_t *)head;
    
    if(NULL == head)
    {
        GIZWITS_LOG("ERR: NTP is empty \n");
        return -1;
    }
    
    memcpy((uint8_t *)&gizwitsProtocol.TimeNTP,(uint8_t *)UTTInfo->time, (7 + 4));
    gizwitsProtocol.TimeNTP.year = exchangeBytes(gizwitsProtocol.TimeNTP.year);
    gizwitsProtocol.TimeNTP.ntp =exchangeWord(gizwitsProtocol.TimeNTP.ntp);

    gizwitsProtocol.NTPEvent.event[gizwitsProtocol.NTPEvent.num] = WIFI_NTP;
    gizwitsProtocol.NTPEvent.num++;
    
    gizwitsProtocol.issuedFlag = GET_NTP_TYPE;
    
    
    return 0;
}

/**
* @brief Protocol 4.4 Device MCU restarts function

* @param none
* @return none
*/
static void gizProtocolReboot(void)
{
    uint32_t timeDelay = gizGetTimerCount();
    
    /*Wait 600ms*/
    while((gizGetTimerCount() - timeDelay) <= 600);
    mcuRestart();
}

/**
* @brief Protocol 4.5 :The WiFi module informs the device MCU of working status about the WiFi module

* @param[in] status WiFi module working status
* @return none
*/
static int8_t gizProtocolModuleStatus(protocolWifiStatus_t *status)
{
    static wifiStatus_t lastStatus;

    if(NULL == status)
    {
        GIZWITS_LOG("gizProtocolModuleStatus Error , Illegal Param\n");
        return -1;
    }

    status->ststus.value = exchangeBytes(status->ststus.value);
   
    //OnBoarding mode status
    if(lastStatus.types.onboarding != status->ststus.types.onboarding)
    {
        if(1 == status->ststus.types.onboarding)
        {
            if(1 == status->ststus.types.softap)
            {
                gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_SOFTAP;
                gizwitsProtocol.wifiStatusEvent.num++;
                GIZWITS_LOG("OnBoarding: SoftAP or Web mode\n");
            }

            if(1 == status->ststus.types.station)
            {
                gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_AIRLINK;
                gizwitsProtocol.wifiStatusEvent.num++;
                GIZWITS_LOG("OnBoarding: AirLink mode\n");
            }
        }
        else
        {
            if(1 == status->ststus.types.softap)
            {
                gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_SOFTAP;
                gizwitsProtocol.wifiStatusEvent.num++;
                GIZWITS_LOG("OnBoarding: SoftAP or Web mode\n");
            }

            if(1 == status->ststus.types.station)
            {
                gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_STATION;
                gizwitsProtocol.wifiStatusEvent.num++;
                GIZWITS_LOG("OnBoarding: Station mode\n");
            }
        }
    }

    //binding mode status
    if(lastStatus.types.binding != status->ststus.types.binding)
    {
        lastStatus.types.binding = status->ststus.types.binding;
        if(1 == status->ststus.types.binding)
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_OPEN_BINDING;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: in binding mode\n");
        }
        else
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_CLOSE_BINDING;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: out binding mode\n");
        }
    }

    //router status
    if(lastStatus.types.con_route != status->ststus.types.con_route)
    {
        lastStatus.types.con_route = status->ststus.types.con_route;
        if(1 == status->ststus.types.con_route)
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_CON_ROUTER;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: connected router\n");
        }
        else
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_DISCON_ROUTER;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: disconnected router\n");
        }
    }

    //M2M server status
    if(lastStatus.types.con_m2m != status->ststus.types.con_m2m)
    {
        lastStatus.types.con_m2m = status->ststus.types.con_m2m;
        if(1 == status->ststus.types.con_m2m)
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_CON_M2M;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: connected m2m\n");
        }
        else
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_DISCON_M2M;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: disconnected m2m\n");
        }
    }

    //APP status
    if(lastStatus.types.app != status->ststus.types.app)
    {
        lastStatus.types.app = status->ststus.types.app;
        if(1 == status->ststus.types.app)
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_CON_APP;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: app connect\n");
        }
        else
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_DISCON_APP;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: no app connect\n");
        }
    }

    //test mode status
    if(lastStatus.types.test != status->ststus.types.test)
    {
        lastStatus.types.test = status->ststus.types.test;
        if(1 == status->ststus.types.test)
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_OPEN_TESTMODE;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: in test mode\n");
        }
        else
        {
            gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_CLOSE_TESTMODE;
            gizwitsProtocol.wifiStatusEvent.num++;
            GIZWITS_LOG("WiFi status: out test mode\n");
        }
    }

    gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_RSSI;
    gizwitsProtocol.wifiStatusEvent.num++;
    gizwitsProtocol.wifiStatusData.rssi = status->ststus.types.rssi;
    GIZWITS_LOG("RSSI is %d \n", gizwitsProtocol.wifiStatusData.rssi);

    gizwitsProtocol.issuedFlag = WIFI_STATUS_TYPE;

    return 0;
}


/**@name Gizwits User API interface
* @{
*/

/**
* @brief gizwits Protocol initialization interface

* Protocol-related timer, serial port initialization

* Datapoint initialization

* @param none
* @return none
*/
void gizwitsInit(void)
{    
    pRb.rbCapacity = RB_MAX_LEN;
    pRb.rbBuff = rbBuf;
    if(0 == rbCreate(&pRb))
	{
		GIZWITS_LOG("rbCreate Success \n");
	}
	else
	{
		GIZWITS_LOG("rbCreate Faild \n");
	}
    
    memset((uint8_t *)&gizwitsProtocol, 0, sizeof(gizwitsProtocol_t));
}

/**
* @brief WiFi configure interface

* Set the WiFi module into the corresponding configuration mode or reset the module

* @param[in] mode ：0x0， reset the module ;0x01， SoftAp mode ;0x02， AirLink mode ;0x03， Production test mode; 0x04:allow users to bind devices

* @return Error command code
*/
int32_t gizwitsSetMode(uint8_t mode)
{
    int32_t ret = 0;
    protocolCfgMode_t cfgMode;
    protocolCommon_t setDefault;

    switch(mode)
    {
        case WIFI_RESET_MODE:
            gizProtocolHeadInit((protocolHead_t *)&setDefault);
            setDefault.head.cmd = CMD_SET_DEFAULT;
            setDefault.head.sn = gizwitsProtocol.sn++;
            setDefault.head.len = exchangeBytes(sizeof(protocolCommon_t)-4);
            setDefault.sum = gizProtocolSum((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            ret = uartWrite((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            if(ret < 0)
            {
                GIZWITS_LOG("ERR: uart write error %d \n", ret);
            }

            gizProtocolWaitAck((uint8_t *)&setDefault, sizeof(protocolCommon_t));   
            break;
        case WIFI_SOFTAP_MODE:
            gizProtocolHeadInit((protocolHead_t *)&cfgMode);
            cfgMode.head.cmd = CMD_WIFI_CONFIG;
            cfgMode.head.sn = gizwitsProtocol.sn++;
            cfgMode.cfgMode = mode;
            cfgMode.head.len = exchangeBytes(sizeof(protocolCfgMode_t)-4);
            cfgMode.sum = gizProtocolSum((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t));
            ret = uartWrite((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t));
            if(ret < 0)
            {
                GIZWITS_LOG("ERR: uart write error %d \n", ret);
            }
            gizProtocolWaitAck((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t)); 
            break;
        case WIFI_AIRLINK_MODE:
            gizProtocolHeadInit((protocolHead_t *)&cfgMode);
            cfgMode.head.cmd = CMD_WIFI_CONFIG;
            cfgMode.head.sn = gizwitsProtocol.sn++;
            cfgMode.cfgMode = mode;
            cfgMode.head.len = exchangeBytes(sizeof(protocolCfgMode_t)-4);
            cfgMode.sum = gizProtocolSum((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t));
            ret = uartWrite((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t));
            if(ret < 0)
            {
                GIZWITS_LOG("ERR: uart write error %d \n", ret);
            }
            gizProtocolWaitAck((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t)); 
            break;
        case WIFI_PRODUCTION_TEST:
            gizProtocolHeadInit((protocolHead_t *)&setDefault);
            setDefault.head.cmd = CMD_PRODUCTION_TEST;
            setDefault.head.sn = gizwitsProtocol.sn++;
            setDefault.head.len = exchangeBytes(sizeof(protocolCommon_t)-4);
            setDefault.sum = gizProtocolSum((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            ret = uartWrite((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            if(ret < 0)
            {
                GIZWITS_LOG("ERR: uart write error %d \n", ret);
            }

            gizProtocolWaitAck((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            break;
        case WIFI_NINABLE_MODE:
            gizProtocolHeadInit((protocolHead_t *)&setDefault);
            setDefault.head.cmd = CMD_NINABLE_MODE;
            setDefault.head.sn = gizwitsProtocol.sn++;
            setDefault.head.len = exchangeBytes(sizeof(protocolCommon_t)-4);
            setDefault.sum = gizProtocolSum((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            ret = uartWrite((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            if(ret < 0)
            {
                GIZWITS_LOG("ERR: uart write error %d \n", ret);
            }

            gizProtocolWaitAck((uint8_t *)&setDefault, sizeof(protocolCommon_t)); 
            break;
        case WIFI_REBOOT_MODE:
            gizProtocolHeadInit((protocolHead_t *)&setDefault);
            setDefault.head.cmd = CMD_REBOOT_MODULE;
            setDefault.head.sn = gizwitsProtocol.sn++;
            setDefault.head.len = exchangeBytes(sizeof(protocolCommon_t)-4);
            setDefault.sum = gizProtocolSum((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            ret = uartWrite((uint8_t *)&setDefault, sizeof(protocolCommon_t));
            if(ret < 0)
            {
                GIZWITS_LOG("ERR: uart write error %d \n", ret);
            }

            gizProtocolWaitAck((uint8_t *)&setDefault, sizeof(protocolCommon_t)); 
            break;
        default:
            GIZWITS_LOG("ERR: CfgMode error!\n");
            break;
    }

    return ret;
}

/**
* @brief Get the the network time

* Protocol 4.13:"Device MCU send" of "the MCU requests access to the network time"

* @param[in] none
* @return none
*/
void gizwitsGetNTP(void)
{
    int32_t ret = 0;
    protocolCommon_t getNTP;

    gizProtocolHeadInit((protocolHead_t *)&getNTP);
    getNTP.head.cmd = CMD_GET_NTP;
    getNTP.head.sn = gizwitsProtocol.sn++;
    getNTP.head.len = exchangeBytes(sizeof(protocolCommon_t)-4);
    getNTP.sum = gizProtocolSum((uint8_t *)&getNTP, sizeof(protocolCommon_t));
    ret = uartWrite((uint8_t *)&getNTP, sizeof(protocolCommon_t));
    if(ret < 0)
    {
        GIZWITS_LOG("ERR[NTP]: uart write error %d \n", ret);
    }

    gizProtocolWaitAck((uint8_t *)&getNTP, sizeof(protocolCommon_t));
}


/**
* @brief Get Module Info

* 

* @param[in] none
* @return none
*/
void gizwitsGetModuleInfo(void)
{
    int32_t ret = 0;
    protocolGetModuleInfo_t getModuleInfo;

    gizProtocolHeadInit((protocolHead_t *)&getModuleInfo);
    getModuleInfo.head.cmd = CMD_ASK_MODULE_INFO;
    getModuleInfo.head.sn = gizwitsProtocol.sn++;
    getModuleInfo.type = 0x0;
    getModuleInfo.head.len = exchangeBytes(sizeof(protocolGetModuleInfo_t)-4);
    getModuleInfo.sum = gizProtocolSum((uint8_t *)&getModuleInfo, sizeof(protocolGetModuleInfo_t));
    ret = uartWrite((uint8_t *)&getModuleInfo, sizeof(protocolGetModuleInfo_t));
    if(ret < 0)
    {
        GIZWITS_LOG("ERR[NTP]: uart write error %d \n", ret);
    }

    gizProtocolWaitAck((uint8_t *)&getModuleInfo, sizeof(protocolGetModuleInfo_t));
}


/**
* @brief Module Info Analyse
*
* @param [in] head : 
*
* @return 0, Success， , other,Faild
*/
static int8_t gizProtocolModuleInfoHandle(protocolHead_t *head)
{
    protocolModuleInfo_t *moduleInfo = (protocolModuleInfo_t *)head;

    if(NULL == head)
    {
        GIZWITS_LOG("NTP is empty \n");
        return -1;
    }

    memcpy((uint8_t *)&gizwitsProtocol.wifiModuleNews,(uint8_t *)&moduleInfo->wifiModuleInfo, sizeof(moduleInfo_t));

    gizwitsProtocol.moduleInfoEvent.event[gizwitsProtocol.moduleInfoEvent.num] = MODULE_INFO;
    gizwitsProtocol.moduleInfoEvent.num++;

    gizwitsProtocol.issuedFlag = GET_MODULEINFO_TYPE;


    return 0;
}

/**
* @brief Protocol handling function

* 

* @param [in] currentData :The protocol data pointer
* @return none
*/
int32_t gizwitsHandle(dataPoint_t *currentData)
{
    int8_t ret = 0;
#ifdef PROTOCOL_DEBUG
    uint16_t i = 0;
#endif
    uint8_t ackData[RB_MAX_LEN];
    uint16_t protocolLen = 0;
    uint32_t ackLen = 0;
    protocolHead_t *recvHead = NULL;
    char *didPtr = NULL;
    uint16_t offset = 0;


    if(NULL == currentData)
    {
        GIZWITS_LOG("GizwitsHandle Error , Illegal Param\n");
        return -1;
    }

    /*resend strategy*/
    gizProtocolAckHandle();
    ret = gizProtocolGetOnePacket(&pRb, gizwitsProtocol.protocolBuf, &protocolLen);

    if(0 == ret)
    {
        GIZWITS_LOG("Get One Packet!\n");
        
#ifdef PROTOCOL_DEBUG
        GIZWITS_LOG("WiFi2MCU[%4d:%4d]: ", gizGetTimerCount(), protocolLen);
        for(i=0; i<protocolLen;i++)
        {
            GIZWITS_LOG("%02x ", gizwitsProtocol.protocolBuf[i]);
        }
        GIZWITS_LOG("\n");
#endif

        recvHead = (protocolHead_t *)gizwitsProtocol.protocolBuf;
        switch (recvHead->cmd)
        {
            case CMD_GET_DEVICE_INTO:
                gizProtocolGetDeviceInfo(recvHead);
                break;
            case CMD_ISSUED_P0:
                GIZWITS_LOG("flag %x %x \n", recvHead->flags[0], recvHead->flags[1]);
                //offset = 1;
               
                if(0 == gizProtocolIssuedProcess(didPtr, gizwitsProtocol.protocolBuf+sizeof(protocolHead_t)+offset, protocolLen-(sizeof(protocolHead_t)+offset+1), ackData, &ackLen))
                {
                    gizProtocolIssuedDataAck(recvHead, ackData, ackLen,recvHead->flags[1]);
                    GIZWITS_LOG("AckData : \n");
                }
                break;
            case CMD_HEARTBEAT:
                gizProtocolCommonAck(recvHead);
                break;
            case CMD_WIFISTATUS:
                gizProtocolCommonAck(recvHead);
                gizProtocolModuleStatus((protocolWifiStatus_t *)recvHead);
                break;
            case ACK_REPORT_P0:
            case ACK_WIFI_CONFIG:
            case ACK_SET_DEFAULT:
            case ACK_NINABLE_MODE:
            case ACK_REBOOT_MODULE:
                gizProtocolWaitAckCheck(recvHead);
                break;
            case CMD_MCU_REBOOT:
                gizProtocolCommonAck(recvHead);
                GIZWITS_LOG("report:MCU reboot!\n");
                
                gizProtocolReboot();
                break;
            case CMD_ERROR_PACKAGE:
                break;
            case ACK_PRODUCTION_TEST:
                gizProtocolWaitAckCheck(recvHead);
                GIZWITS_LOG("Ack PRODUCTION_MODE success \n");
                break;           
            case ACK_GET_NTP:
                gizProtocolWaitAckCheck(recvHead);
                gizProtocolNTP(recvHead);
                GIZWITS_LOG("Ack GET_UTT success \n");
                break; 
            case ACK_ASK_MODULE_INFO:
                gizProtocolWaitAckCheck(recvHead);
                gizProtocolModuleInfoHandle(recvHead);
                GIZWITS_LOG("Ack GET_Module success \n");
            break;
 
            default:
                gizProtocolErrorCmd(recvHead,ERROR_CMD);
                GIZWITS_LOG("ERR: cmd code error!\n");
                break;
        }
    }
    else if(-2 == ret)
    {
        //Check failed, report exception
        recvHead = (protocolHead_t *)gizwitsProtocol.protocolBuf;
        gizProtocolErrorCmd(recvHead,ERROR_ACK_SUM);
        GIZWITS_LOG("ERR: check sum error!\n");
        return -2;
    }
    
    switch(gizwitsProtocol.issuedFlag)
    {
        case ACTION_CONTROL_TYPE:
            gizwitsProtocol.issuedFlag = STATELESS_TYPE;
            gizwitsEventProcess(&gizwitsProtocol.issuedProcessEvent, (uint8_t *)&gizwitsProtocol.gizCurrentDataPoint, sizeof(dataPoint_t));
            memset((uint8_t *)&gizwitsProtocol.issuedProcessEvent,0x0,sizeof(gizwitsProtocol.issuedProcessEvent));  
            break;
        case WIFI_STATUS_TYPE:
            gizwitsProtocol.issuedFlag = STATELESS_TYPE;
            gizwitsEventProcess(&gizwitsProtocol.wifiStatusEvent, (uint8_t *)&gizwitsProtocol.wifiStatusData, sizeof(moduleStatusInfo_t));
            memset((uint8_t *)&gizwitsProtocol.wifiStatusEvent,0x0,sizeof(gizwitsProtocol.wifiStatusEvent));
            break;
        case ACTION_W2D_TRANSPARENT_TYPE:
            gizwitsProtocol.issuedFlag = STATELESS_TYPE;
            gizwitsEventProcess(&gizwitsProtocol.issuedProcessEvent, (uint8_t *)gizwitsProtocol.transparentBuff, gizwitsProtocol.transparentLen);
            break;
        case GET_NTP_TYPE:
            gizwitsProtocol.issuedFlag = STATELESS_TYPE;
            gizwitsEventProcess(&gizwitsProtocol.NTPEvent, (uint8_t *)&gizwitsProtocol.TimeNTP, sizeof(protocolTime_t));
            memset((uint8_t *)&gizwitsProtocol.NTPEvent,0x0,sizeof(gizwitsProtocol.NTPEvent));
            break;
        case GET_MODULEINFO_TYPE:
            gizwitsProtocol.issuedFlag = STATELESS_TYPE;
            gizwitsEventProcess(&gizwitsProtocol.moduleInfoEvent, (uint8_t *)&gizwitsProtocol.wifiModuleNews, sizeof(moduleInfo_t));
            memset((uint8_t *)&gizwitsProtocol.moduleInfoEvent,0x0,sizeof(moduleInfo_t));
            break;
        default:
            break;      
    }

    gizDevReportPolicy(currentData);

    return 0;
}

/**
* @brief gizwits report transparent data interface

* The user can call the interface to complete the reporting of private protocol data

* @param [in] data :Private protocol data
* @param [in] len  :Private protocol data length
* @return 0，success ;other，failure
*/
int32_t gizwitsPassthroughData(uint8_t * gizdata, uint32_t len)
{
	int32_t ret = 0;
	uint8_t tx_buf[MAX_PACKAGE_LEN];
	uint8_t *pTxBuf = tx_buf;
	uint16_t data_len = 6+len;
    if(NULL == gizdata)
    {
        GIZWITS_LOG("[ERR] gizwitsPassthroughData Error \n");
        return (-1);
    }

	*pTxBuf ++= 0xFF;
	*pTxBuf ++= 0xFF;
	*pTxBuf ++= (uint8_t)(data_len>>8);//len
	*pTxBuf ++= (uint8_t)(data_len);
	*pTxBuf ++= CMD_REPORT_P0;//0x1b cmd
	*pTxBuf ++= gizwitsProtocol.sn++;//sn
	*pTxBuf ++= 0x00;//flag
	*pTxBuf ++= 0x00;//flag
	*pTxBuf ++= ACTION_D2W_TRANSPARENT_DATA;//P0_Cmd

    memcpy(&tx_buf[9],gizdata,len);
    tx_buf[data_len + 4 - 1 ] = gizProtocolSum( tx_buf , (data_len+4));
    
	ret = uartWrite(tx_buf, data_len+4);
    if(ret < 0)
    {
        GIZWITS_LOG("ERR: uart write error %d \n", ret);
    }

    gizProtocolWaitAck(tx_buf, data_len+4);

    return 0;
}

/**@} */
