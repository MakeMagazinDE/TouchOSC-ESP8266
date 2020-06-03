unit udp_tester_main;

// UDP-Tester für TouchOSC, C. Meyer 3/2020
// kompiliert mit Delphi 10.1

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, IdUDPServer, IdGlobal, IdSocketHandle,
  IdBaseComponent, IdComponent, IdUDPBase, Vcl.StdCtrls, IdUDPClient,
  Vcl.ComCtrls, Vcl.ExtCtrls;

type
  TForm1 = class(TForm)
    Memo1: TMemo;
    IdUDPServer1: TIdUDPServer;
    Button1: TButton;
    TrackBar1: TTrackBar;
    Button2: TButton;
    EditTouchpadIP: TEdit;
    Label1: TLabel;
    TrackBar2: TTrackBar;
    Label2: TLabel;
    TrackBar3: TTrackBar;
    Label3: TLabel;
    Memo2: TMemo;
    Label4: TLabel;
    Label5: TLabel;
    LED_busy: TPanel;
    CheckBox1: TCheckBox;
    Label6: TLabel;
    Label7: TLabel;
    EditOSCname1: TEdit;
    Label8: TLabel;
    EditOSCname2: TEdit;
    Label9: TLabel;
    EditSendString: TEdit;
    Button3: TButton;
    Label10: TLabel;
    Label11: TLabel;
    Edit1: TEdit;
    EditMax: TEdit;
    CheckBoxInt: TCheckBox;
    EditSendToPort: TEdit;
    Label12: TLabel;
    Label13: TLabel;
    Label14: TLabel;
    procedure IdUDPServer1UDPRead(AThread: TIdUDPListenerThread;
      const AData: TIdBytes; ABinding: TIdSocketHandle);
    procedure ButtonOffClick(Sender: TObject);
    procedure TrackBar1Change(Sender: TObject);
    procedure ButtonOnClick(Sender: TObject);
    procedure ButtonSendStringClick(Sender: TObject);
    procedure EditMaxChange(Sender: TObject);
  private
    { Private-Deklarationen }
  public
    { Public-Deklarationen }
  end;

var
  Form1: TForm1;

implementation

{$R *.dfm}

procedure LEDstate(is_on: boolean);
begin
  if is_on then begin
    Form1.LED_busy.Color:=$000010FF;
    Form1.LED_busy.Tag:= 1;
  end else begin
    Form1.LED_busy.Color:=$00000020;
    Form1.LED_busy.Tag:= 0;
  end;
end;


procedure display_controlstring(control_str: AnsiString; memo_field: TMemo);
var
  result_str: String;
  i: Integer;
begin
  memo_field.lines.Clear;
  result_str:= '';
  for i:= 1 to length(control_str) do
    if (control_str[i] >= #32) then
      result_str:= result_str + control_str[i] + '  '
    else
      result_str:= result_str + '#  ';
  memo_field.lines.add(result_str);
  result_str:= '';
  for i:= 1 to length(control_str) do
    result_str:= result_str + IntToHex(byte(control_str[i]), 2) + ' ';
  memo_field.lines.add(result_str);
end;

procedure SendOSC(control_str: AnsiString; val: Single);
type
  FloatOrBytes = record
    case Byte of
        0: (AsBytes: array[0..3] of Byte);
        1: (AsFloat: Single);
  end;
  IntOrBytes = record
    case Byte of
        0: (AsBytes: array[0..3] of Byte);
        1: (AsInt: Int32);
  end;
var
  i, n: Integer;
  sf: FloatOrBytes;
  si: IntOrBytes;
begin
  n:= length(control_str) mod 4;
  for i:= n to 3 do
    control_str:= control_str + #0;
  if Form1.CheckBoxInt.Checked then
    control_str:= control_str + ',i'
  else
    control_str:= control_str + ',f';
  n:= length(control_str) mod 4;
  for i:= n to 3 do
    control_str:= control_str + #0;

  sf.AsFloat:= val;
  si.AsInt:= round(val);
  if Form1.CheckBoxInt.Checked then begin
    for i:= 3 downto 0 do
      control_str:= control_str + char(si.AsBytes[i]);
  end else begin
    for i:= 3 downto 0 do
      control_str:= control_str + char(sf.AsBytes[i]);
  end;

  display_controlstring(control_str, Form1.Memo2);

  Form1.IdUDPServer1.Send(Form1.EditTouchpadIP.Text,
    StrtoInt(Form1.EditSendToPort.Text), control_str);
end;

procedure SendOSCstring(control_str, val_str: AnsiString);
var
  i, n: Integer;
begin
  n:= length(control_str) mod 4;
  for i:= n to 3 do
    control_str:= control_str + #0;

  if length(val_str) > 0 then begin
    control_str:= control_str + ',s';
    n:= length(control_str) mod 4;
    for i:= n to 3 do
      control_str:= control_str + #0;

    control_str:= control_str + val_str;

    n:= length(control_str) mod 4;
    for i:= n to 3 do
      control_str:= control_str + #0;
  end;
  display_controlstring(control_str, Form1.Memo2);
  Form1.IdUDPServer1.Send(Form1.EditTouchpadIP.Text, 9000, control_str);
end;

procedure TForm1.IdUDPServer1UDPRead(AThread: TIdUDPListenerThread;
  const AData: TIdBytes; ABinding: TIdSocketHandle);
// UDP Message Simple Control:
// "/<pagename>/<controlname>" 00 [00] [00] [00] ",f" 00 00 <4 Bytes IEEE754>
// UDP Message XY-Control:
// "/<pagename>/<controlname>" 00 [00] [00] [00] ",ff" 00  <4 + 4 Bytes IEEE754>
// [00] = Füller auf durch 4 teilbare Anzahl

type
  FloatOrBytes = record
    case Byte of
        0: (AsBytes: array[0..3] of Byte);
        1: (AsFloat: Single);
  end;
var
   s: string;
   b: Byte;
   f_v1, f_v2: FloatOrBytes;
   i, f_idx, v1, v2: Integer;
   has_first_val, has_second_val: Boolean;

begin
  if (Length(AData) > 0) then begin
    s:= BytesToString(AData);
    // Feedback
    display_controlstring(s, Form1.Memo1);
    if Checkbox1.Checked then begin
      display_controlstring(s, Form1.Memo2);
      IdUDPServer1.Send(Form1.EditTouchpadIP.Text, 9000, s);
    end;

    for i:= 0 to Length(AData) - 1 do begin
      if AData[i] = $2C then // Komma?
        break;
    end;
    has_first_val:= AData[i + 1] = $66; // ="f"?
    has_second_val:= AData[i + 2] = $66; // ="ff"?
    f_idx:= i + 4; // zeigt jetzt auf 1. Byte vom 1. IEEE754-Float, Big Endian!

    if has_first_val then begin
      // Bei XY-Controls: Y-Wert
      for i:= 0 to 3 do
        f_v1.AsBytes[i]:= AData[f_idx + 3 - i]; // Reihenfolge tauschen
      v1:= round(f_v1.AsFloat);        // Skalieren auf 0..127
      Trackbar2.Position:= v1;
      LEDstate(v1 > 63);
    end;

    f_idx:= f_idx + 4; // zeigt jetzt auf 1. Byte vom 2. IEEE754-Float, Big Endian!

    if has_second_val then begin
      // Bei XY-Controls: X-Wert
      for i:= 0 to 3 do
        f_v2.AsBytes[i]:= AData[f_idx + 3 - i]; // Reihenfolge tauschen
      v2:= round(f_v2.AsFloat);        // Skalieren auf 0..127
      Trackbar3.Position:= v2;
    end;
  end;
end;


procedure TForm1.ButtonOffClick(Sender: TObject);
begin
  SendOSC(EditOSCname1.Text, 0);
end;

procedure TForm1.ButtonOnClick(Sender: TObject);
begin
  SendOSC(EditOSCname1.Text, 127);
end;

procedure TForm1.ButtonSendStringClick(Sender: TObject);
begin
  SendOSCstring(EditSendString.Text, Edit1.Text);
end;

procedure TForm1.EditMaxChange(Sender: TObject);
begin
  Trackbar1.Max:= StrToInt(EditMax.Text);
end;

procedure TForm1.TrackBar1Change(Sender: TObject);
begin
  SendOSC(EditOSCname2.Text, TrackBar1.Position);
end;


end.
