program hx35_udp;

uses
  Vcl.Forms,
  hx35_udp_main in 'hx35_udp_main.pas' {Form1};

{$R *.res}

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  Application.CreateForm(TForm1, Form1);
  Application.Run;
end.
