//----------------------------------------------------------------------------------------------------
// ID Code      : RTD2528R_Hdmi.c No.0000
// Update Note  :
//
//----------------------------------------------------------------------------------------------------

#define __HDMI__

#include "Core\Header\Include.h"


BYTE code cCodeHDMITemp = 0x00;
#if(_HDMI_SUPPORT == _ON)
//--------------------------------------------------
// Description  : Detect DVI/HDMI input format
// Input Value  : None
// Output Value : Return _FALSE if Input Format isn't HDMI, _TRUE while Input Format is HDMI
//--------------------------------------------------
bit CHdmiFormatDetect(void)
{
#if (0)
    BYTE count=3;
    
    CScalerPageSelect(_PAGE2);
    do{
        CScalerRead(_P2_HDMI_SR_CB, 1, pData, _NON_AUTOINC);
        if((pData[0] & 0x01) == 0x01){//Input source is the HDMI format.
            return _TRUE;
        }
        CTimerDelayXms(10);
        count--;
    }while(count!=0);
    return _FALSE;
#else
    CScalerPageSelect(_PAGE2);
    CScalerRead(_P2_HDMI_SR_CB, 1, pData, _NON_AUTOINC);
    if((pData[0] & 0x01) == 0x01)//Input source is the HDMI format.
        return _TRUE;
    else
        return _FALSE;
#endif
}

#if (_HDMI_HOT_PLUG_OPTION == _ENABLE) 
/*
//eric 20070326 philips DVD
void CHDMIStableConfirm(void)
{
    if((_GET_INPUT_SOURCE() == _SOURCE_HDMI || _GET_INPUT_SOURCE() == _SOURCE_DVI)||(CHdmiFormatDetect()))
    {
        if(bHDMIFrameOk==_FALSE)
            ucHDMIFrameOkCnt ++;
        
        if(ucHDMIFrameOkCnt>=_HDMI_OK_CNT)
        {
            bHDMIFrameOk=_TRUE;
            ucHDMIChangeCnt=0;
            ucHDMIFrameOkCnt=0;
        }   
    }   
    
}
  */
void CModeHdmiHP(void)
{
/*
    #if 0//(_WATCHDOG_SUPPORT)
    WD_RESET();
    #endif
    //20070326, change for Philips DVP5965K mute
    //CLR_HDMI_CTRL();//Hot plug
    bHot_Plug = _HOT_PLUG_LOW;
    CScalerSetBit(_P2_HDCP_CTRL_C0, ~ _BIT0, 0);    
    CScalerSetBit(_P2_Z0_CALIBRATION_CTRL_AC, ~ _BIT6, 0);      
    //20070326, change for Philips DVP5965K mute
    CTimerDelayXms(500);// 500
    CScalerSetBit(_P2_HDCP_CTRL_C0, ~ _BIT0, _BIT0);    
    CScalerSetBit(_P2_Z0_CALIBRATION_CTRL_AC, ~ _BIT6, _BIT6);
    //20070326, change for Philips DVP5965K mute
    CTimerDelayXms(500);// 200      
    bHot_Plug = _HOT_PLUG_HI;
    //SET_HDMI_CTRL();  
    #if 0//(_WATCHDOG_SUPPORT)
    WD_RESET();
    #endif */
}

void CModeHDMIChageDelay(void)
{
    if((_GET_INPUT_SOURCE() == _SOURCE_HDMI || _GET_INPUT_SOURCE() == _SOURCE_DVI)&&CHdmiFormatDetect())
    {
        //eric 20070326 philips DVD                     
        ucHDMIChangeCnt++;
        bHDMIFrameOk=_FALSE;
        
        if(ucHDMIChangeCnt>=_HDMI_CHANGE_CNT)
        {
            ucHDMIChangeCnt=0;                          
            CModeHdmiHP();
        }   
    }       
}   
#endif

//--------------------------------------------------
// Description  : Detect Audio Lock status
// Input Value  : None
// Output Value : Return _FALSE if Audio Lock is ok, _TRUE while Audio mislock, FIFO underflow/overflow
//--------------------------------------------------
bit CHdmiAudioFIFODetect(void)
{
    CScalerPageSelect(_PAGE2);
    CScalerRead(_P2_HDMI_SR_CB, 1, pData, _NON_AUTOINC);
    
    if((pData[0] & 0x06) == 0)
    {
        return _FALSE;
    }
    
    return _TRUE;
}

//730702***
void CAdjustHdmiCbCr422(void)
{
    if(stModeInfo.IVHeight <= 288)
    {
        CScalerPageSelect(_PAGE2);
        CScalerGetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_ACRCR_51, 1, pData, _NON_AUTOINC);
        CScalerPageSelect(_PAGE0);
        if((pData[0] & 0x30) == 0x10)
        {       
            CTimerWaitForEvent(_EVENT_IVS);
            pData[0] = HIBYTE(stModeInfo.IHTotal - 2);
            pData[1] = 0x02;
            pData[2] = LOBYTE(stModeInfo.IHTotal - 2);
            pData[3] = HIBYTE(stModeInfo.IVTotal - 2);
            pData[4] = 0x02;
            pData[5] = LOBYTE(stModeInfo.IVTotal - 2);
            pData[6] = 0x00;
            pData[7] = 0x00;
            pData[8] = 0x00;
            pData[9] = 0x00;
            pData[10] = 0x03;
            pData[11] = 0x00;
            pData[12] = 0x00;
            pData[13] = 0x81;
            CScalerWrite(_H_BOUNDARY_H_70, 14, pData, _AUTOINC);
            pData[0]    = CAutoWaitFinish();
            //if(pData[0] != _ERROR_SUCCESS)
            //DebugPrintf("\nmeasure result = %x",pData[0]);    
            
            CScalerRead(_V_START_END_H_7E, 6, pData, _AUTOINC);
            usHStartPos = (((WORD) pData[3] & 0xf0) << 4) | (WORD) pData[4];
            usHEndPos = (((WORD) pData[3] & 0x0f) << 8) | (WORD) pData[5];
            usVStartPos = (((WORD) pData[0] & 0xf0) << 4) | (WORD) pData[1];
            usVEndPos = (((WORD) pData[0] & 0x0f) << 8) | (WORD) pData[2];          
            
            pData[0]    = ((usHStartPos >> 4) & 0x70) | (HIBYTE(usHStartPos) & 0x0f);
            pData[1]    = (LOBYTE(usHStartPos)+0);
            pData[2]    = (LOBYTE(usHStartPos)+0);
            pData[3]    = (HIBYTE(usVStartPos) & 0x0f);
            pData[4]    = (LOBYTE(usVStartPos)+0);
            pData[5]    = (LOBYTE(usVStartPos)+0);
            CScalerWrite(_H_BOUNDARY_H_70, 6, pData, _AUTOINC); 
            
            CScalerSetByte(_AUTO_ADJ_CTRL1_7D, 0x20);
            
            CScalerSetByte(_AUTO_ADJ_CTRL0_7A, 0x00|_BIT1);//red    
            CScalerSetBit(_AUTO_ADJ_CTRL1_7D, ~_BIT0 , _BIT0);//start
            
            pData[0] = CAutoWaitFinish();
            //if(pData[0] != _ERROR_SUCCESS)       
            //DebugPrintf("\nmeasureMAX_R result = %x",pData[0]);
            
            CScalerRead(_AUTO_PHASE_0_87, 1, &pData[1], _AUTOINC);
            //DebugPrintf("\n****RED**** = %x",pData[1]);
            
            CScalerSetByte(_AUTO_ADJ_CTRL0_7A, 0x00);//blue 
            CScalerSetBit(_AUTO_ADJ_CTRL1_7D, ~_BIT0 , _BIT0);//start
            
            pData[0] = CAutoWaitFinish();
            //if(pData[0] != _ERROR_SUCCESS)       
            //DebugPrintf("\nmeasureMAX_B result = %x",pData[0]);
            
            CScalerRead(_AUTO_PHASE_0_87, 1, &pData[2], _AUTOINC);
            //DebugPrintf("\n****BLUE**** = %x",pData[2]);  
            
            if((pData[1] != 0) || (pData[2] != 0))
            {
                //DebugPrintf("\n\n=========MODIFY!!!!=========  \n\n",'0');
                CScalerRead(_IPH_ACT_STA_L_15, 1, pData, _NON_AUTOINC);         
                pData[0] = pData[0] - 1 ;
                CScalerSetBit(_VGIP_CTRL_10, ~_BIT4 , 0x00);
                CScalerSetByte(_IPH_ACT_STA_L_15, pData[0]);
                CScalerSetBit(_VGIP_CTRL_10, ~_BIT4 , _BIT4);
            }
        }
    }
}
//730702###
//731301***
//--------------------------------------------------
// Description  : Enable HDMI watch dog
// Input Value  : ucPar --> Parameter for HDMI watch dog
// Output Value : None
//--------------------------------------------------
void CAdjustEnableHDMIWatchDog(BYTE ucPar)
{
    CScalerPageSelect(_PAGE2);  //Issac-0129-Fixed
    
    if((ucPar & _WD_SET_AVMUTE_ENABLE) == 0x01)
        CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_WDCR0_31, ~_BIT7, _BIT7); //Enable Set_AVMute Watch Dog
    
    if((ucPar & _WD_AUDIO_FOR_TMDS_CLOCK) == 0x02)
        CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_WDCR1_32, ~_BIT7, _BIT7);//Enable Audio Watch Dog for TMDS clock
    
    if((ucPar & _WD_PACKET_VARIATION) == 0x04)
        CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_VWDSR_41, ~_BIT0, _BIT0);//Enable packet variation Watch Dog
    
    if((ucPar & _WD_AUDIO_FIFO) == 0x08)
        CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_WDCR0_31, ~(_BIT2 | _BIT1), (_BIT2 | _BIT1)); //Enable Audio Watch Dog
}

//--------------------------------------------------
// Description  : Disable HDMI watch dog
// Input Value  : ucPar --> Parameter for HDMI watch dog
// Output Value : None
//--------------------------------------------------
void CAdjustDisableHDMIWatchDog(BYTE ucPar)
{
    CScalerPageSelect(_PAGE2);  //Issac-0129-Fixed
    
    if((ucPar & _WD_SET_AVMUTE_ENABLE) == 0x01)
        CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_WDCR0_31, ~_BIT7, 0x00); //Disable Set_AVMute Watch Dog
    
    if((ucPar & _WD_AUDIO_FOR_TMDS_CLOCK) == 0x02)
        CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_WDCR1_32, ~_BIT7, 0x00);//Disable Audio Watch Dog for TMDS clock
    
    if((ucPar & _WD_PACKET_VARIATION) == 0x04)
        CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_VWDSR_41, ~_BIT0, 0x00);//Disable packet variation Watch Dog
    
    if((ucPar & _WD_AUDIO_FIFO) == 0x08)
        CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_WDCR0_31, ~(_BIT2 | _BIT1), 0x00); //Disable Audio Watch Dog
}

#if(_HDCP_SUPPORT)
//--------------------------------------------------
// Description  : HDMI HDCP key initial
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void CHdmiKeyInit(void)
{
    CScalerWrite(_P2_DEVICE_KEY_ACCESS_PORT_C1, 320, tHDCP_KEY_TABLE_0, _NON_AUTOINC);
}

//--------------------------------------------------
// Description  : HDMI HDCP control
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void CHdmiHdcpCtrl(void)
{
    CScalerCodeW(tHDCP_CTRL); 
}

//--------------------------------------------------
// Description  : Adjust HDCP key
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void CAdjustHDCP(void)
{
    CScalerPageSelect(_PAGE2);
    CScalerSetByte(_P2_HDCP_CTRL_C0, 0x06);
    //CScalerWrite(_P2_DEVICE_KEY_ACCESS_PORT_C1, 320, tHDCP_KEY_TABLE_0, _NON_AUTOINC);
    //CScalerCodeW(tHDCP_CTRL);
    CHdmiKeyInit();
    CHdmiHdcpCtrl();
    
    CScalerSetByte(_P2_HDCP_CTRL_C0, 0x01);
}
#endif

//--------------------------------------------------
// Description  : HDMI Video Setting
// Input Value  : None
// Output Value : Return _FALSE if Set_AVMute is true, _TRUE while Video Setting is OK.
//--------------------------------------------------
bit CHdmiVideoSetting(void)
{
    CScalerPageSelect(_PAGE2);
    CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_SCR_00, ~(_BIT1 | _BIT0), _BIT1);//Set HDMI/DVI decide condition//731301
    CTimerDelayXms(50);//731301             
    
    if(CHdmiFormatDetect())//Input source is the HDMI format.
    {
        SET_HDMIINPUT();
        
        CScalerSetBit(_P2_ANALOG_COMMON_CTRL2_AB, ~(_BIT3 | _BIT2), 0x00);
        CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_ACRCR_51, ~_BIT2, _BIT2);//Enable Pixel Repetition down sampling auto mode
        
#if(_HDCP_SUPPORT == _ON)
        CScalerSetDataPortByte(_P2_HDCP_ADDR_PORT_C3, 0x40, 0x93);//Change to HDCP1.1 for HDMI
#endif
        //HDMI Video & Audio Part
        CScalerSetBit(_P2_HDMI_SR_CB, ~(_BIT5 | _BIT3 | _BIT2),_BIT5 | _BIT3 | _BIT2);
        CTimerDelayXms(100);
        CScalerRead(_P2_HDMI_SR_CB, 1, pData, _NON_AUTOINC);
        if(!(bit)(pData[0] & 0x40))//For Clear_AVMute
        {
            SET_AVRESUME();//Audio WD can't action, when Set_AVMute happen.
            CAdjustDisableHDMIWatchDog(_WD_SET_AVMUTE_ENABLE);//Disable Set_AVMute Watch Dog //731301
            CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_AVMCR_30, ~_BIT3, _BIT3);//Enable DVI/HDMI video output
            CAdjustEnableHDMIWatchDog(_WD_SET_AVMUTE_ENABLE);//Enable Set_AVMute Watch Dog //731301
        }
        else
        {
            CTimerDelayXms(100);//For AV_Mute Delay
            CLR_AVRESUME();
            return _FALSE;
        }
        SET_VIDEOMODECHANGE();//check color space everytimes
        //HDMI Video Part
        CScalerRead(_P2_HDMI_GPVS_CC, 1, pData, _NON_AUTOINC);
        if(((bit)(pData[0] & 0x01)) || GET_VIDEOMODECHANGE())//For HDMI switch between RGB/YCbCr
        {
            CLR_VIDEOMODECHANGE();
            CScalerSetBit(_P2_HDMI_GPVS_CC, ~_BIT0, _BIT0);
            CScalerGetDataPortByte(_P2_HDMI_PSAP_CD, 0x00, 1, pData, _NON_AUTOINC);
            if((pData[0] & 0x23) == 0)//Check BCH data(Package error flag)
            {
                CScalerGetDataPortByte(_P2_HDMI_PSAP_CD, 0x04, 2, pData, _NON_AUTOINC);
                if((bit)(pData[0] & 0x40) != (bit)(pData[0] & 0x20))//For HDMI switch between RGB/YUV
                {
                    if((pData[0]&0x20)==0x20)   //422
                    {
                        CScalerPageSelect(_PAGE6);
                        CScalerSetBit(_P6_YUV422_TO_YUV444_D4, ~_BIT7, _BIT7);//enable
                    }
                    
                    if((pData[1] & 0xc0) != 0xc0)//For HDMI switch between ITU601/ITU709
                    {
                        CScalerSetByte(_YUV2RGB_CTRL_9C, 0x08);
                        
                        if((bit)(pData[1] & 0x40))
                        {
                            //DebugPrintf("\n ITU601%c ",0x20);
                            CScalerCodeW(tHDMI_YPBPR_ITU601);
                        }
                        else
                        {
                            //DebugPrintf("\n ITU709%c ",0x20);
                            CScalerCodeW(tHDMI_YPBPR_ITU709);
                        }
                        CScalerSetByte(_YUV2RGB_CTRL_9C, 0x05);
                    }
                }
                else if(!(bit)(pData[0] & 0x60))
                {
                    CScalerSetByte(_YUV2RGB_CTRL_9C, 0x00);
                }
            }
        }
    }
    else
    {
        CLR_HDMIINPUT();
        CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_ACRCR_51, ~_BIT2, 0x00);//Disable Pixel Repetition down sampling auto mode
        CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_VCR_50, ~(_BIT3 | _BIT2 |_BIT1 |_BIT0), 0x00);
        CAdjustDisableHDMIWatchDog(_WD_HDMI_ALL);
        CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_AVMCR_30, ~_BIT3, _BIT3);//Enable DVI/HDMI video output
        CScalerSetDataPortByte(_P2_HDCP_ADDR_PORT_C3, 0x40, 0x91);// Change to HDCP1.0 for DVI
        CScalerSetByte(_YUV2RGB_CTRL_9C, 0x00);
    }
    return _TRUE;
}

//--------------------------------------------------
// Description  : Setting Audio Frequence Mode
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void CHdmiAudioFirstTracking(void)
{
    BYTE coeff = 0, s = 0, o = 1;
    WORD a = 1024, b = 0, m = 0;
    DWORD cts = 0, n = 0, freq = 0;
    
    CScalerPageSelect(_PAGE2);
    CScalerRead(_P2_HDMI_SR_CB, 1, pData, _NON_AUTOINC);
    
    if((CHdmiAudioFIFODetect() || GET_AVRESUME()) && (!(bit)(pData[0] & 0x40)))//For HDMI audio pll setting
    {
        CLR_AVRESUME();
        CAdjustDisableHDMIWatchDog(_WD_AUDIO_FIFO);//Disable Audio Watch Dog //731301
        CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_PSCR_15, 0x00);//Disable FIFO Trend
        CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_CMCR_10, 0x50);//Update Double Buffer
        CScalerSetBit(_P2_HDMI_APC_C8, ~_BIT0, _BIT0);//HDMI Address Auto Increase Enable
        
        CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_ACRCR_51, ~_BIT1, _BIT1);
        CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_NTX1024TR0_28, ~_BIT3, _BIT3);
        CTimerDelayXms(2);
        
        CScalerGetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_ACRSR0_52, 5, pData, _NON_AUTOINC);
        cts = ((DWORD)pData[0] << 12) | ((DWORD)pData[1] << 4) | (((DWORD)pData[2] >> 4) & 0x0f);
        n =   (((DWORD)pData[2] & 0x0f) << 16) | ((DWORD)pData[3] << 8) | (DWORD)pData[4];
        
        CScalerGetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_NTX1024TR0_28, 2, pData, _NON_AUTOINC);
        b = (((WORD)pData[0] & 0x07) << 8) | (WORD)pData[1];
        
        // Fa = (a*Fx*n)/(b*cts*128) = (1024*Fx*n)/(b*cts*128) = (8*Fx*n)/(b*cts)
        // calculate freq in 0.1kHz unit
        freq = (DWORD)8 * 2 * 1000 * _RTD_XTAL / cts *(10*n) / ((DWORD)b * 1000);       
        freq = (freq >> 1) + (freq & 0x01);
        if((freq >= 318) && (freq <= 322))
        {
            coeff = _AUDIO_MCK_32000;
            freq  = 32000;
        }
        else if((freq >= 438) && (freq <= 444))
        {
            coeff = _AUDIO_MCK_44100;
            freq  = 44100;
        }
        else if((freq >= 476) && (freq <= 484))
        {
            coeff = _AUDIO_MCK_48000;
            freq  = 48000;
        }
        else if((freq >= 877) && (freq <= 887))
        {
            coeff = _AUDIO_MCK_88200;
            freq  = 88200;
        }
        else if((freq >= 955) && (freq <= 965))
        {
            coeff = _AUDIO_MCK_96000;
            freq  = 96000;
        }
        else if((freq >= 1754) && (freq <= 1774))
        {
            coeff = _AUDIO_MCK_176400;
            freq  = 176400;
        }
        else if((freq >= 1910) && (freq <= 1930))
        {
            coeff = _AUDIO_MCK_192000;
            freq  = 192000;
        }
        else
        {
            SET_AVRESUME();
        }
        
        if(!GET_AVRESUME())
        {
            do
            {
                s = s + 4;
                ((DWORD *)pData)[0] = (DWORD)128 * freq * coeff * s;
            }
            while (((DWORD *)pData)[0] < 180000000);
            
            o = 1;
            m = ((DWORD *)pData)[0] * 2 / ((DWORD)_RTD_XTAL * 1000);
            m = m + 1;
            s = s / (o * 2);
            
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_AAPNR_2D, 0x08);  // Disable SDM
            
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_MCAPR_11, (m - 2));
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_SCAPR_12, (coeff == _AUDIO_256_TIMES) ? ((s / 2) | 0x80) : (s / 2));
#if(_AUDIO_LOCK_MODE == _HARDWARE_TRACKING)
            // Calculate D code
            ((DWORD *)pData)[1] = (DWORD)1000 * _RTD_XTAL * m / 2;  // PLL freq
            if (((DWORD *)pData)[0] > ((DWORD *)pData)[1])
            {
                a = (((DWORD *)pData)[0] - ((DWORD *)pData)[1]) * 128 / (((DWORD *)pData)[1] / 2048);
                
                a = 0xffff - a;
            }
            else
            {
                a = (((DWORD *)pData)[1] - ((DWORD *)pData)[0]) * 128 / (((DWORD *)pData)[1] / 2048);
                
                a += 100; // MUST for compatibility
            }
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_DCAPR0_13, a >> 8);
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_DCAPR1_14, a & 0xff);
#endif
            
            CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_DPCR0_38, ~(_BIT5 | _BIT4), (o << 4));
            
            // Calculate Ich for audio PLL
            pData[0] = (m < 5) ? 0 : ((m / 5) - 1);
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_DPCR1_39, pData[0] | 0x80);
            
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_DPCR3_3B, 0x03); // Enable K and enable VCOSTART
            
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_CMCR_10, 0x50);  //Enable Double Buffer for K/M/S/D/O
            
            CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_DPCR0_38, ~(_BIT7 | _BIT6), 0x00);    // Enable PLL
            CTimerDelayXms(1);
#if(_AUDIO_LOCK_MODE == _HARDWARE_TRACKING)
            do
            {
                CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_AAPNR_2D, 0x00);  // Disable SDM
                CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_AAPNR_2D, 0x02);  // Enable SDM
                CTimerDelayXms(1);
                CScalerGetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_DPCR3_3B + 1, 2, pData, _NON_AUTOINC);
            }
            while((((a >> 8) & 0xff) != pData[0]) || (((a >> 0) & 0xff) != pData[1]));
#endif
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_AOCR_62, 0x00);//Disable SPDIF/I2S Output
            CAdjustDisableHDMIWatchDog(_WD_SET_AVMUTE_ENABLE);//Disable Set_AVMute Watch Dog //731301
            CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_AVMCR_30, ~_BIT5, _BIT5);//Enable Audio Output
            CAdjustEnableHDMIWatchDog(_WD_SET_AVMUTE_ENABLE);//Enable Set_AVMute Watch Dog //731301
            
#if(_AUDIO_LOCK_MODE == _HARDWARE_TRACKING)
            //H/W FIFO Tracking
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_PSCR_15, 0x04);//Enable boundary tracking
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_ICBPSR1_25, 0x01);//Set I code
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_PCBPSR1_27, 0x01);//Set P code
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_STBPR_2A, 0x80);//Set Boundary Tracking Update Response Time
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_AAPNR_2D, 0xC2);
            
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_FBR_1B, 0xe2);//0xe5 for DVR team ?
            CScalerSetDataPortBit(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_WDCR0_31, ~_BIT5, _BIT5);//Enable FIFO Tracking//731301
            
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_CMCR_10, 0x50);//update double buffer
            
            CScalerSetByte(_P2_HDMI_SR_CB, 0x06);//Write 1 clear
            
            //Fine tune
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_PSCR_15, 0xEC);//Enable FIFO Trend
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_FTR_1A, 0x03);
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_ICTPSR1_21, 0x07);
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_CMCR_10, 0x50);//Update Double Buffer
#else
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_PSCR_15, 0xfe);//Enable N/CTS tracking
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, 0x1d, 0x05);//Set I code 
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, 0x1f, 0x9F);//Set P code
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_AAPNR_2D, 0x02);
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_CMCR_10, 0x50);//update double buffer
#endif
            
            SET_AUDIOWAITINGFLAG();
        }
        else
        {
            CLR_AUDIOPLLLOCKREADY();
        }
    }
    else
    {
        CLR_AUDIOPLLLOCKREADY();
    }
}

//--------------------------------------------------
// Description  : Enable Audio Output
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void CHdmiEnableAudioOutput(void)
{
    CLR_AUDIOWAITINGTIMEOUT();//731301
    //DebugPrintf("\n HA%c",0x20);
    if(GET_AUDIOWAITINGFLAG())
    {
        CScalerSetByte(_P2_HDMI_SR_CB, 0x06);//Write 1 clear //731301
        //CTimerReactiveTimerEvent(SEC(1), CHdmiAudioWaitingFlagReadyEven);//731301
        CLR_AUDIOWAITINGFLAG();
        SET_AUDIOPLLLOCKREADY();
    }
    else
    {
        //DebugPrintf(" b%c",0x20);
        if (CHdmiAudioFIFODetect() || GET_AVRESUME())//For HDMI audio pll setting
        {
            //DebugPrintf(" c%c",0x20);
            CHdmiAudioFirstTracking();
            //CTimerReactiveTimerEvent(SEC(1), CHdmiAudioWaitingFlagReadyEven);//731301
            CLR_AUDIOPLLLOCKREADY();
        }
        else if (GET_AUDIOPLLLOCKREADY())
        {
            //DebugPrintf(" d%c",0x20);
            CLR_AUDIOPLLLOCKREADY();
            
            
            //modify Start
            //            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_AOCR_62, 0x0f);//Enable I2S Output
            CScalerRead(_P2_HDMI_SR_CB, 1, pData, _NON_AUTOINC);
            if((pData[0] & 0x16) == 0x00) //LPCM & no overflow/underflow in Audio FIFO
                CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_AOCR_62, 0xff);//Enable I2S Output
            else
                CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_AOCR_62, 0xf0);//Disable I2S Output
            CScalerSetDataPortByte(_P2_HDMI_ADDR_PORT_C9, _P2_HDMI_AFCR_03, 0x26);//Enable Audio FIFO
            CAdjustEnableHDMIWatchDog(_WD_AUDIO_FOR_TMDS_CLOCK | _WD_AUDIO_FIFO);
            //modify End
            
        }
    }
    CTimerActiveTimerEvent(SEC(0.5), CHdmiAudioWaitingFlagReadyEven);//731301
}

//--------------------------------------------------
// Description  : Audio Waiting Time Flag Ready
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void CHdmiAudioWaitingFlagReadyEven(void)
{
    SET_AUDIOWAITINGTIMEOUT();
}


#if (0)//_HDMI_HOT_PLUG_OPTION == _ENABLE)  
//741001***
void CHdmiModeChange()
{
    bHot_Plug = _HOT_PLUG_LOW;//bHot_Plug = 0;
    CTimerDelayXms(10);
    bHot_Plug = _HOT_PLUG_HI;//bHot_Plug = 1;
    //DebugPrintf("\n bHot_Plug!!!\n",'i');
}
//741001###
#endif

#endif //End of #if(_HDMI_SUPPORT == _ON)


