# -*- coding: utf-8 -*-

import wx
import serial
from wx._misc import Usleep
from serial.tools.list_ports_windows import NULL

def crc32(lst, lens):
    i = 0
    dwpoly = 0x04c11db7
    crc = 0xffffffff
    
    for i in range(0, lens):
        xbit = 0x80000000
        data = lst[i]&0xffffffff
        for bits in range(0,32):
            if ((crc&0x80000000)==0x80000000):
                crc = (crc<<1)&0xffffffff
                crc = (crc^dwpoly)&0xffffffff
            else:
                crc = (crc<<1)&0xffffffff
            if ((data&xbit)==xbit):
                crc = (crc^dwpoly)&0xffffffff
            xbit = (xbit>>1)&0xffffffff
    return crc 

class MainWindow(wx.Frame):
    def __init__(self, parent, title):
        self.dirname  =''
        self.strParam = [u"输出电压",u"脉冲宽度",u"触发延时",u"重复频率",u"重复次数",u"输出极性",u"步进电压",u"电压上限",u"电压下限"]
        self.strUnits = [u"kV",u"us",u"us",u"Hz",u"次",u"  ",u"kV",u"kV",u"kV"]
        self.strAddr  = []
        for i in range(0,10):
            self.strAddr.append(u"电源"+str(i))
        wx.Frame.__init__(self, parent, title=title, size=(200,-1))
          

        #creating the main sizer and the global/Stage settings staticbox in it
        self.szrMain          = wx.BoxSizer(wx.VERTICAL)
        self.szrSettings      = wx.BoxSizer(wx.HORIZONTAL)
        self.Global           = wx.StaticBox(self, label=u"整体参数设置")
        self.Global.szrMain   = wx.StaticBoxSizer(self.Global, wx.VERTICAL)
        self.Stage         = wx.StaticBox(self, label=u"分立参数设置")
        self.Stage.szrMain = wx.StaticBoxSizer(self.Stage, wx.VERTICAL)
        #construct the frame
        self.szrMain.Add(self.szrSettings, 1, wx.TOP|wx.BOTTOM, 10)
        self.szrSettings.Add(self.Global.szrMain, 1, wx.LEFT|wx.RIGHT, 10)
        self.szrSettings.Add(self.Stage.szrMain, 1, wx.LEFT|wx.RIGHT, 10)
        self.Global.szr = []
        self.Stage.szr  = []
        for i in range(0,10):
            self.Global.szr.append(wx.BoxSizer(wx.HORIZONTAL))
            self.Global.szrMain.Add(self.Global.szr[i], 0, wx.TOP|wx.BOTTOM, 5)
            self.Stage.szr.append(wx.BoxSizer(wx.HORIZONTAL))
            self.Stage.szrMain.Add(self.Stage.szr[i], 0, wx.TOP|wx.BOTTOM, 5)

        #create the elements for each parameters
        #Parameter Label and Text
        self.Global.txtParam = []
        for i in range(0,9):
            self.Global.txtParam.append(wx.TextCtrl(self,value=str(global_param[global_addr[0]][i]/param_ratio[i]), size=(70,22),style=wx.TE_RIGHT))
            self.Global.szr[i].Add(wx.StaticText(self,label=self.strParam[i],style=wx.TE_CENTER), 1, wx.LEFT|wx.RIGHT, 5)
            self.Global.szr[i].Add(self.Global.txtParam[i], 1, wx.LEFT|wx.RIGHT, 5)
            self.Global.szr[i].Add(wx.StaticText(self,label=self.strUnits[i],style=wx.TE_LEFT), 0, wx.RIGHT, 15)
        #Address Choice Box
        self.Global.chnAddr = wx.Choice(self, size=(-1,25),choices=self.strAddr)
        self.Global.chnAddr.SetSelection(0)
        self.Global.szr[9].Add(self.Global.chnAddr, 0, wx.LEFT|wx.RIGHT|wx.ALIGN_CENTER, 5)
        #Parameter setting Send Button
        self.Global.btnSend = wx.Button(self,label=u"参数设定",size=(110,35))
        self.Global.szr[9].Add(self.Global.btnSend, 1, wx.LEFT|wx.RIGHT|wx.ALIGN_CENTER, 5)

        #bind event
        #textctrl event
        for i in range(0,9):
            self.Global.txtParam[i].Bind(wx.EVT_LEFT_DOWN , lambda evt, idx=i: self.TextOnMouse(evt,idx))
            self.Global.txtParam[i].Bind(wx.EVT_MOUSEWHEEL, lambda evt, idx=i: self.TextOnMouse(evt,idx))
            self.Global.txtParam[i].Bind(wx.EVT_RIGHT_DOWN, lambda evt, idx=i: self.TextOnMouse(evt,idx))
            self.Global.txtParam[i].Bind(wx.EVT_KEY_DOWN  , lambda evt, idx=i: self.TextOnKey(evt,idx))
            self.Global.txtParam[i].Bind(wx.EVT_KILL_FOCUS, lambda evt, idx=i: self.TextOnLeft(evt,idx))
        #send button event
        self.Global.btnSend.Bind(wx.EVT_BUTTON, self.OnGlobalSend)
        self.Global.chnAddr.Bind(wx.EVT_CHOICE, self.OnChoiceAddr)
        














        self.SetSizer(self.szrMain)
        self.SetAutoLayout(1)
        self.szrMain.Fit(self)
        self.Show()

        self.tmrCheckModf = wx.Timer(self)
        self.Bind(wx.EVT_TIMER, self.OnTimerCheckModf, self.tmrCheckModf)
        self.tmrCheckModf.Start(5000)

    def printStatus(self, p, BuffData):
        tout=0
        while (p.inWaiting()<30):
            tout+=1
            if (tout>50000):
                break;
        if (tout>50000):
            rpbyte = 3
        else:
            recv = p.read(p.inWaiting())
            rpbyte = ord(recv[3])
#         for x in BuffData:
#             print hex(x),
#         print 
#         for x in recv:
#             print hex(ord(x)),
        print "Target:%s, Command:0x%02x, Data:0x%02x    Response:%s"%(BuffData[1]==0x00 and "All " or "No."+str(BuffData[1]-1), BuffData[2], BuffData[21], response_str[rpbyte])
        

    def checkModf(self, idx, val):
        if val != global_param[global_addr[0]][idx]:
            self.Global.btnSend.SetLabel(u"参数设定(缓存)")
        # else:
        #     self.Global.btnSend.SetLabel(u"参数设定")

    def TextOnMouse(self,evt,idx):
        #auto select all and enable the wheel to adjust the value , when right pressed , the adjust rate will multiply by 10
        self.Global.txtParam[idx].SetFocus()  
        self.Global.txtParam[idx].SelectAll()
        try:
            w = evt.GetWheelRotation()
            m = evt.RightIsDown() and 12 or 120
            val = int(float(self.Global.txtParam[idx].GetValue())*param_ratio[idx])+w/m
            if val>=0:
                self.Global.txtParam[idx].SetValue(str(val/param_ratio[idx]))
        except:
            pass

    def TextOnKey(self,evt,idx):
        #limit the allowed key press
        kc = evt.GetKeyCode()
        # print kc
        if kc==13:
            if evt.ShiftDown():
                idx = (idx+8)%9
            else:
                idx = (idx+1)%9
            self.Global.txtParam[idx].SetFocus()  
            self.Global.txtParam[idx].SelectAll()
        elif (kc>=48 and kc<=57) or kc==314 or kc==316 or kc == 8:
            evt.Skip()
        elif kc==46:
            txt = self.Global.txtParam[idx].GetValue()
            for i in txt:
                if i == '.':
                    return
            evt.Skip()
        elif kc==317:
            self.Global.txtParam[idx].SetInsertionPointEnd()
        elif kc==315:
            self.Global.txtParam[idx].SetInsertionPoint(0)

    def TextOnLeft(self,evt,idx):
        #check the display format of each parameter
        if len(self.Global.txtParam[idx].GetValue()) == 0:
            self.Global.txtParam[idx].SetValue(str(global_param[global_addr[0]][idx]/param_ratio[idx]))
        else:
            val = int(float(self.Global.txtParam[idx].GetValue())*param_ratio[idx])
            self.Global.txtParam[idx].SetValue(str(val/param_ratio[idx]))
            self.checkModf(idx, val)
        evt.Skip()

    def OnTimerCheckModf(self,evt):
        for i in range(0,9):
            val = int(float(self.Global.txtParam[i].GetValue())*param_ratio[i])
            self.checkModf(i, val)

    def OnGlobalSend(self,evt):
        #initialize the command
        BuffData = [0x3A,global_addr[0]+1,0x20]
        for i in range(0, 25):
            BuffData.append(0)
        BuffData.extend([0x0D,0x0A])

        #checkout the value of global parameters from global textctrl
        for i in range(0,9):
            val = int(float(self.Global.txtParam[i].GetValue())*param_ratio[i])
            global_param[global_addr[0]][i] = val
            # if (i)%3 == 0: print
            # print self.strParam[i].encode('utf-8')+":"+str(val/param_ratio[i])+" ",

        #sending command via usart
        usart_ctrl = serial.Serial(0,115200)
        for i in range(0,9):
            BuffData[i*2+3] = global_param[global_addr[0]][param_mapping[i]]/0x100
            BuffData[i*2+4] = global_param[global_addr[0]][param_mapping[i]]%0x100
        crc32calc = crc32(BuffData,23)
        for i in range(27,23,-1):
            BuffData[i] = int(crc32calc%0x100)
            crc32calc /= 0x100
        usart_ctrl.write(BuffData)
        self.printStatus(usart_ctrl, BuffData)
        usart_ctrl.close()

        self.Global.btnSend.SetLabel(u"参数设定")

    def OnChoiceAddr(self,evt):
        global_addr[0] = int(self.Global.chnAddr.GetString(self.Global.chnAddr.GetSelection())[2])
        print u"Selected Power Source " + str(global_addr[0])
        for i in range(0,9):
            self.Global.txtParam[i].SetValue(str(global_param[global_addr[0]][i]/param_ratio[i]))




#         #define on-off button
#         #set sizer -- sizerBtnDev
#         self.sizerBtnDev = wx.BoxSizer(wx.HORIZONTAL)
#         self.btnDev = []
#         for i in range(0, 12):
#             if i==0:
#                 self.btnDev.append(wx.Button(self, label=u"全部开机", size=(60,25), style=wx.CENTER))
#             elif i==1:
#                 self.btnDev.append(wx.Button(self, label=u"统一设置", size=(60,25), style=wx.CENTER))
#             else:
#                 self.btnDev.append(wx.Button(self, label=u"电源"+str(i-2)+u"关", size=(60,25), style=wx.CENTER))
#             self.Bind(wx.EVT_BUTTON, lambda evt, idx=i : self.OnButton_btnDev(evt, idx), self.btnDev[i])
#             self.sizerBtnDev.Add(self.btnDev[i], 0, wx.RIGHT|wx.LEFT|wx.CENTER, 5)
            
#         #define parameter input text -- txtParam
#         #set sizer -- sizerParam
#         self.lablParam = [u"输出电压",u"宽度设置",u"延时设置",u"重复频率",u"重复次数",u"输出极性",u"加电步进",u"步进下限",u"步进上限"]
#         self.sizerParam = []
#         self.txtParam = []
#         self.txtAll = []
#         for i in range(0, 3):
#             self.sizerParam.append(wx.BoxSizer(wx.HORIZONTAL))
#             self.sizerParam[i].Add(wx.StaticText(self, label=self.lablParam[i], size=(60,20), style = wx.ALIGN_CENTER), 0, wx.RIGHT|wx.LEFT|wx.CENTER, 5)
#             self.txtAll.append(wx.TextCtrl(self, value=str(param[0][i]/p_ratio[i]), size=(60,20), style=wx.TE_RIGHT))
#             self.sizerParam[i].Add(self.txtAll[i], 0, wx.RIGHT|wx.LEFT|wx.CENTER, 5)
#             txtTemp = []
#             for j in range(0, 10):
#                 txtTemp.append(wx.TextCtrl(self, value=str(param[j][i]/p_ratio[i]), size=(60,20), style=wx.TE_RIGHT))
#                 self.sizerParam[i].Add(txtTemp[j], 0, wx.RIGHT|wx.LEFT|wx.CENTER, 5)
#             self.txtParam.append(txtTemp)
            
#         #define control button
#         self.btnSend = wx.Button(self, label=u"参数设定", size=(80,30))
#         self.btnOn = wx.Button(self, label=u"电源上电", size=(80,30))
#         self.btnOff = wx.Button(self, label=u"电源断电", size=(80,30))
#         self.btnchg = wx.Button(self, label=u"电容充电", size=(80,30))
#         self.btndischg = wx.Button(self, label=u"电容掉电", size=(80,30))
#         self.btnManu = wx.Button(self, label=u"手动控制", size=(80,30))
#         self.btnAuto = wx.Button(self, label=u"自动控制", size=(80,30))
#         self.btnPause = wx.Button(self, label=u"暂停", size=(80,30))
#         self.btnConti = wx.Button(self, label=u"继续", size=(80,30))
#         self.Bind(wx.EVT_BUTTON, self.OnButton_btnSend, self.btnSend)
#         self.Bind(wx.EVT_BUTTON, lambda evt, idx=0x13 : self.OnButton_btnCtrl(evt, idx), self.btnOn)
#         self.Bind(wx.EVT_BUTTON, lambda evt, idx=0x14 : self.OnButton_btnCtrl(evt, idx), self.btnOff)
#         self.Bind(wx.EVT_BUTTON, lambda evt, idx=0xB0 : self.OnButton_btnCtrl(evt, idx), self.btnchg)
#         self.Bind(wx.EVT_BUTTON, lambda evt, idx=0xB1 : self.OnButton_btnCtrl(evt, idx), self.btndischg)
#         self.Bind(wx.EVT_BUTTON, lambda evt, idx=0x11 : self.OnButton_btnCtrl(evt, idx), self.btnManu)
#         self.Bind(wx.EVT_BUTTON, lambda evt, idx=0x12 : self.OnButton_btnCtrl(evt, idx), self.btnAuto)
#         self.Bind(wx.EVT_BUTTON, lambda evt, idx=0x15 : self.OnButton_btnCtrl(evt, idx), self.btnPause)
#         self.Bind(wx.EVT_BUTTON, lambda evt, idx=0x16 : self.OnButton_btnCtrl(evt, idx), self.btnConti)
#         #set sizer -- sizerCtrl
#         self.sizerCtrl = wx.BoxSizer(wx.HORIZONTAL)
#         self.sizerCtrl.Add(self.btnSend, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 6)
#         self.sizerCtrl.Add(self.btnOn, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 6)
#         self.sizerCtrl.Add(self.btnOff, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 6)
#         self.sizerCtrl.Add(self.btnchg, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 6)
#         self.sizerCtrl.Add(self.btndischg, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 6)
#         self.sizerCtrl.Add(self.btnManu, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 6)
#         self.sizerCtrl.Add(self.btnAuto, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 6)
#         self.sizerCtrl.Add(self.btnPause, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 6)
#         self.sizerCtrl.Add(self.btnConti, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 6)
        
        
#         #define other parameter
#         #set sizer - sizerOther
#         self.sizerOther = wx.BoxSizer(wx.HORIZONTAL)
#         self.txtOther = []
#         for i in range(0, 6):
#             self.txtOther.append(wx.TextCtrl(self, value=str(param[0][i+3]/p_ratio[i+3]), size=(60,20), style=wx.TE_RIGHT))
#             self.sizerOther.Add(wx.StaticText(self,label=self.lablParam[i+3], size=(60,20), style = wx.ALIGN_CENTER), 0, wx.RIGHT|wx.LEFT|wx.CENTER, 5)
#             self.sizerOther.Add(self.txtOther[i], 0, wx.RIGHT|wx.LEFT|wx.CENTER, 5)
        
        
#         #testing setting -- sizerTs
#         #define elements
#         self.txtDZ = wx.TextCtrl(self, value=u"500", size=(60,20), style=wx.TE_RIGHT)
#         self.btnDZ = wx.Button(self, label=u"死区设置/ns")
#         self.txtCF = wx.TextCtrl(self, value=u"20", size=(60,20), style=wx.TE_RIGHT)
#         self.btnCF = wx.Button(self, label=u"截止宽度/us")
#         self.txtPR = wx.TextCtrl(self, value=u"64", size=(60,20), style=wx.TE_RIGHT)
#         self.btnPR = wx.Button(self, label=u"PWM比率/16666")
#         self.chnCOM = wx.Choice(self, size=(60,20)) 
#         self.btnCOM = wx.Button(self, label=u"刷新串口")
#         #bind events
#         self.Bind(wx.EVT_BUTTON, lambda evt, idx=0xA0, txtC=self.txtDZ : self.OnButton_btnTS(evt, idx, txtC), self.btnDZ)
#         self.Bind(wx.EVT_BUTTON, lambda evt, idx=0xA1, txtC=self.txtPR : self.OnButton_btnTS(evt, idx, txtC), self.btnPR)
#         self.Bind(wx.EVT_BUTTON, lambda evt, idx=0xA2, txtC=self.txtCF : self.OnButton_btnTS(evt, idx, txtC), self.btnCF)
#         self.Bind(wx.EVT_BUTTON, self.OnButton_btnCOM, self.btnCOM)
#         self.Bind(wx.EVT_CHOICE, self.OnChoice_chnCOM, self.chnCOM)
#         #set sizer
# #         self.sizerTS = wx.BoxSizer(wx.HORIZONTAL)
#         sbTS = wx.StaticBox(self, label=u"测试参数设置")  
#         self.sizerTS = wx.StaticBoxSizer(sbTS, wx.HORIZONTAL)  
#         self.sizerTS.Add(self.chnCOM, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 5)
#         self.sizerTS.Add(self.btnCOM, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 5)
#         self.sizerTS.Add(self.txtCF, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 5)
#         self.sizerTS.Add(self.btnCF, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 5)
#         self.sizerTS.Add(self.txtDZ, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 5)
#         self.sizerTS.Add(self.btnDZ, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 5)
#         self.sizerTS.Add(self.txtPR, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 5)
#         self.sizerTS.Add(self.btnPR, 0, wx.RIGHT|wx.LEFT|wx.CENTER, 5)
        
#         #trigger button -- btnTrig
#         self.btnTrig = wx.Button(self, label=u"触发", size=(150,50))
#         self.Bind(wx.EVT_BUTTON, lambda evt, idx=0x17 : self.OnButton_btnCtrl(evt, idx), self.btnTrig)
        
#         #set main sizer -- sizer
#         #set sizer
#         self.sizer = wx.BoxSizer(wx.VERTICAL)
#         self.sizer.Add(self.sizerBtnDev, 0, wx.TOP|wx.BOTTOM|wx.CENTER, 10)
#         for i in range(0,3):
#             self.sizer.Add(self.sizerParam[i], 0, wx.TOP|wx.BOTTOM|wx.CENTER, 10)
#         self.sizer.Add(self.sizerOther, 0, wx.TOP|wx.BOTTOM|wx.CENTER, 10)
#         self.sizer.Add(self.sizerCtrl, 0, wx.TOP|wx.BOTTOM|wx.CENTER, 10)
#         self.sizer.Add(self.sizerTS, 0, wx.TOP|wx.BOTTOM|wx.CENTER, 10)
#         self.sizer.Add(self.btnTrig, 0, wx.TOP|wx.BOTTOM|wx.CENTER, 10)

        #enable sizer
        

    
        
#     def printStatus(self, p, BuffData):
#         tout=0
#         while (p.inWaiting()<30):
#             tout+=1
#             if (tout>50000):
#                 break;
#         if (tout>50000):
#             rpbyte = 3
#         else:
#             recv = p.read(p.inWaiting())
#             rpbyte = ord(recv[3])
# #         for x in BuffData:
# #             print hex(x),
# #         print 
# #         for x in recv:
# #             print hex(ord(x)),
#         print "Target:%s, Command:0x%02x, Data:0x%02x    Response:%s"%(BuffData[1]==0x00 and "All " or "No."+str(BuffData[1]-1), BuffData[2], BuffData[21], response_str[rpbyte])
        
#     def OnChoice_chnCOM(self, evt):
#         s_port[0][0] = int(self.chnCOM.GetString(self.chnCOM.GetSelection())[3])-1
#         print "Serial Port select to COM" + str(s_port[0][0]+1)
        
#     def OnButton_btnCOM(self, evt):
#         com_list = []
#         for i in range (0,20):
#             try:
#                 ser = serial.Serial(i)
#                 ser.close()
#                 com_list.append("COM"+str(i+1))
#             except:
#                 ser = 1
#         self.chnCOM.Set(com_list)
#         if (len(com_list)==0):
#             print "No Serial Avaliable"
#         else:
#             self.chnCOM.SetSelection(0)
#             s_port[0][0] = int(self.chnCOM.GetString(self.chnCOM.GetSelection())[3])-1
#             print "Serial Port select to COM" + str(s_port[0][0]+1)
        
#     def OnButton_btnTS(self, evt, idx, txtC):
#         BuffData = [0x3A,0x01]
#         for i in range(0, 26):
#             BuffData.append(0)
#         BuffData.extend([0x0D,0x0A])
        
#         BuffData[2] = 0x21
#         BuffData[3] = idx
#         value = int(float(txtC.GetValue()))
#         if (idx == 0xa0):
#             if (value%20 != 0):
#                 value = value - value%20 + 20; 
#         txtC.SetValue(str(value)) 
#         BuffData[4] = (value>>8)&0xff
#         BuffData[5] = value&0xff
        
#         usart_ctrl = serial.Serial(s_port[0][0],115200)
#         crc32calc = crc32(BuffData,23)
#         for j in range(27,23,-1):
#             BuffData[j] = int(crc32calc%0x100)
#             crc32calc /= 0x100
#         usart_ctrl.write(BuffData)
#         self.printStatus(usart_ctrl, BuffData)
#         usart_ctrl.close() 
                
#     def OnButton_btnDev(self, evt, idx):
#         BuffData = [0x3A,0x01]
#         for i in range(0, 26):
#             BuffData.append(0)
#         BuffData.extend([0x0D,0x0A])
        
#         if idx==0:
#             if self.btnDev[0].GetLabel() == u"全部开机":
#                 dev_status[0][0] = 0x3ff
#                 self.btnDev[idx].SetLabel(u"全部关机")
#             else:
#                 dev_status[0][0] = 0x000
#                 self.btnDev[idx].SetLabel(u"全部开机")
#             for i in range(2,12):
#                 if dev_status[0][0]&(0x01<<(i-2)):
#                     self.btnDev[i].SetLabel(u"电源"+str(i-2)+u"开")
#                 else:
#                     self.btnDev[i].SetLabel(u"电源"+str(i-2)+u"关")
#         elif idx==1:
#             #!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#             #the index of txtParam is invert!!!!!!!!!!!!!
#             for i in range(0, 10):
#                 for j in range(0, 3):
#                     tmp_val = int(float(self.txtAll[j].GetValue())*dev_param_ratio[j])
#                     dev_param[i][j] = tmp_val
#                     self.txtAll[j].SetValue(str(tmp_val/dev_param_ratio[j]))
#                     self.txtParam[j][i].SetValue(str(dev_param[i][j]/dev_param_ratio[j]))
#         else:
#             dev_status[0][0] ^= (0x01<<(idx-2))
#             btn = evt.GetEventObject()
#             if (dev_status[0][0]&(0x01<<(idx-2))):
#                 btn.SetLabel(u"电源"+str(idx-2)+u"开")
#             else:
#                 btn.SetLabel(u"电源"+str(idx-2)+u"关")
#         if (idx != 1):
#             usart_ctrl = serial.Serial(s_port[0][0],115200)
#             BuffData[2] = 0x21
#             BuffData[3] = 0xc0
#             BuffData[4] = (dev_status[0][0]>>8)&0xff
#             BuffData[5] = dev_status[0][0]&0xff
#             crc32calc = crc32(BuffData,23)
#             for j in range(27,23,-1):
#                 BuffData[j] = int(crc32calc%0x100)
#                 crc32calc /= 0x100
#             usart_ctrl.write(BuffData)
#             self.printStatus(usart_ctrl, BuffData)
#             usart_ctrl.close()
            
#     def OnButton_btnCtrl(self, evt, idx):
#         BuffData = [0x3A,0x01]
#         for i in range(0, 26):
#             BuffData.append(0)
#         BuffData.extend([0x0D,0x0A])
         
#         usart_ctrl = serial.Serial(s_port[0][0],115200)
#         BuffData[2] = 0x21
#         BuffData[3] = idx
#         BuffData[4] = idx==0x13 and 0x03 or 0x00
#         BuffData[5] = idx==0x13 and 0xff or 0x00
#         crc32calc = crc32(BuffData,23)
#         for j in range(27,23,-1):
#             BuffData[j] = int(crc32calc%0x100)
#             crc32calc /= 0x100
#         usart_ctrl.write(BuffData)
#         self.printStatus(usart_ctrl, BuffData)
#         usart_ctrl.close()
                
#     def OnButton_btnSend(self,evt):
#         BuffData = [0x3A, 0x01, 0x22]
#         for i in range(0, 25):
#             BuffData.append(0)
#         BuffData.extend([0x0D,0x0A])
        
#         for i in range(0, 10):
#             for j in range(0, 3):
#                 dev_param[i][j] = int(float(self.txtParam[j][i].GetValue())*dev_param_ratio[j])
#                 self.txtParam[j][i].SetValue(str(dev_param[i][j]/dev_param_ratio[j]))
#         for i in range(3,9):
#             dev_param[0][i] = int(float(self.txtOther[i-3].GetValue())*dev_param_ratio[i])
#             self.txtOther[i-3].SetValue(str(dev_param[0][i]/dev_param_ratio[i]))

#         usart_ctrl = serial.Serial(s_port[0][0],115200)
#         for i in range(0,10):
#             BuffData[21] = i+1
#             for j in range(0,9):
#                 BuffData[j*2+3] = dev_param[i][dev_param_mapping[j]]/0x100
#                 BuffData[j*2+4] = dev_param[i][dev_param_mapping[j]]%0x100
#             crc32calc = crc32(BuffData,23)
#             for j in range(27,23,-1):
#                 BuffData[j] = int(crc32calc%0x100)
#                 crc32calc /= 0x100
#             usart_ctrl.write(BuffData)
#             self.printStatus(usart_ctrl, BuffData)
#         usart_ctrl.close()

global_addr   = [0]
s_port        = [0]
dev_status    = [0]
response_str  = [u"OK",u"Err",u"Operation Denied",u"Timeout"]
param_mapping = [0,7,8,6,4,3,1,2,5]
param_ratio   = [10.0,10.0,10.0,100.0,1,1,10.0,10.0,10.0]
global_param  = []
stage_param   = []
for i in range(0,10):
    stage_param.append([0,0,0,0,0,0,0,0,0])
    global_param.append([0,0,0,0,0,0,0,0,0])
# createTable(crc32table)
#print "%x" %(crc32(dev_param_default,1))

app = wx.App(False)
frame = MainWindow(None, "Marx")
frame.Global.chnAddr.SetFocus()
frame.OnChoiceAddr(NULL)
# frame.OnButton_btnCOM(NULL)
# if len(frame.chnCOM.GetItems()) != 0:
#     frame.OnButton_btnSend(NULL)
# #     frame.OnButton_btnCtrl(NULL, 0x14)
#     frame.OnButton_btnCtrl(NULL, 0x11)
app.MainLoop()