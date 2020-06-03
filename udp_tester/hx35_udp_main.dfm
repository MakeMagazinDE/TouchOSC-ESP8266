object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'TouchOSC & UDP Tester'
  ClientHeight = 395
  ClientWidth = 753
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 144
    Top = 174
    Width = 90
    Height = 13
    Caption = 'IP of Tablet/Phone'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clRed
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Label2: TLabel
    Left = 159
    Top = 70
    Width = 79
    Height = 13
    Caption = 'Value 1 received'
  end
  object Label3: TLabel
    Left = 407
    Top = 70
    Width = 79
    Height = 13
    Caption = 'Value 2 received'
  end
  object Label4: TLabel
    Left = 8
    Top = 43
    Width = 131
    Height = 17
    Caption = 'Received (Port 8000)'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlue
    Font.Height = -14
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Label5: TLabel
    Left = 8
    Top = 304
    Width = 28
    Height = 17
    Caption = 'Sent'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clRed
    Font.Height = -14
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Label6: TLabel
    Left = 13
    Top = 99
    Width = 714
    Height = 13
    Caption = 
      '00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 2' +
      '1 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Consolas'
    Font.Style = []
    ParentFont = False
  end
  object Label7: TLabel
    Left = 13
    Top = 327
    Width = 714
    Height = 13
    Caption = 
      '00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 2' +
      '1 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Consolas'
    Font.Style = []
    ParentFont = False
  end
  object Label8: TLabel
    Left = 8
    Top = 229
    Width = 94
    Height = 13
    Caption = 'Button Name (OSC)'
  end
  object Label9: TLabel
    Left = 208
    Top = 229
    Width = 88
    Height = 13
    Caption = 'Slider Name (OSC)'
  end
  object Label10: TLabel
    Left = 462
    Top = 8
    Width = 283
    Height = 29
    Caption = 'Make: TouchOSC Tester'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -24
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label11: TLabel
    Left = 460
    Top = 229
    Width = 87
    Height = 13
    Caption = 'Label Name (OSC)'
  end
  object Label12: TLabel
    Left = 277
    Top = 174
    Width = 60
    Height = 13
    Caption = 'Send to Port'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clRed
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Label13: TLabel
    Left = 8
    Top = 183
    Width = 74
    Height = 29
    Caption = 'Sender'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clRed
    Font.Height = -24
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Label14: TLabel
    Left = 8
    Top = 8
    Width = 91
    Height = 29
    Caption = 'Receiver'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlue
    Font.Height = -24
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Memo1: TMemo
    Left = 8
    Top = 113
    Width = 737
    Height = 48
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Consolas'
    Font.Style = []
    ParentFont = False
    TabOrder = 0
  end
  object Button1: TButton
    Left = 8
    Top = 273
    Width = 75
    Height = 25
    Caption = 'ButtonOff'
    TabOrder = 1
    OnClick = ButtonOffClick
  end
  object TrackBar1: TTrackBar
    Left = 200
    Top = 273
    Width = 162
    Height = 25
    Max = 127
    PageSize = 1
    TabOrder = 2
    TickStyle = tsNone
    OnChange = TrackBar1Change
  end
  object Button2: TButton
    Left = 83
    Top = 273
    Width = 75
    Height = 25
    Caption = 'ButtonOn'
    TabOrder = 3
    OnClick = ButtonOnClick
  end
  object EditTouchpadIP: TEdit
    Left = 144
    Top = 191
    Width = 121
    Height = 21
    TabOrder = 4
    Text = '192.168.178.60'
  end
  object TrackBar2: TTrackBar
    Left = 24
    Top = 66
    Width = 129
    Height = 26
    Enabled = False
    Max = 127
    TabOrder = 5
    TickStyle = tsNone
  end
  object TrackBar3: TTrackBar
    Left = 272
    Top = 66
    Width = 129
    Height = 26
    Enabled = False
    Max = 127
    TabOrder = 6
    TickStyle = tsNone
  end
  object Memo2: TMemo
    Left = 8
    Top = 341
    Width = 737
    Height = 36
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Consolas'
    Font.Style = []
    ParentFont = False
    TabOrder = 7
  end
  object LED_busy: TPanel
    Left = 499
    Top = 65
    Width = 62
    Height = 22
    Caption = 'ON'
    Color = 64
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentBackground = False
    ParentFont = False
    TabOrder = 8
  end
  object CheckBox1: TCheckBox
    Left = 616
    Top = 69
    Width = 97
    Height = 17
    Caption = 'Feedback ON'
    TabOrder = 9
  end
  object EditOSCname1: TEdit
    Left = 8
    Top = 246
    Width = 150
    Height = 21
    TabOrder = 10
    Text = '/push/105'
  end
  object EditOSCname2: TEdit
    Left = 208
    Top = 246
    Width = 154
    Height = 21
    TabOrder = 11
    Text = '/fader/119'
  end
  object EditSendString: TEdit
    Left = 460
    Top = 246
    Width = 141
    Height = 21
    TabOrder = 12
    Text = '/label/123'
  end
  object Button3: TButton
    Left = 460
    Top = 273
    Width = 75
    Height = 25
    Caption = 'Send String'
    TabOrder = 13
    OnClick = ButtonSendStringClick
  end
  object Edit1: TEdit
    Left = 616
    Top = 246
    Width = 89
    Height = 21
    TabOrder = 14
    Text = 'Hallo'
  end
  object EditMax: TEdit
    Left = 368
    Top = 273
    Width = 41
    Height = 21
    NumbersOnly = True
    TabOrder = 15
    Text = '127'
    OnChange = EditMaxChange
  end
  object CheckBoxInt: TCheckBox
    Left = 368
    Top = 248
    Width = 57
    Height = 17
    Caption = 'Integer'
    Checked = True
    State = cbChecked
    TabOrder = 16
  end
  object EditSendToPort: TEdit
    Left = 277
    Top = 191
    Width = 41
    Height = 21
    NumbersOnly = True
    TabOrder = 17
    Text = '9000'
    OnChange = EditMaxChange
  end
  object IdUDPServer1: TIdUDPServer
    Active = True
    Bindings = <>
    DefaultPort = 8000
    OnUDPRead = IdUDPServer1UDPRead
    Left = 588
    Top = 284
  end
end
