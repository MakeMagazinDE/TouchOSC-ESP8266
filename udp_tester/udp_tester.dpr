program hx35_udp;

uses
  Vcl.Forms,
  udp_tester_main in 'udp_tester_main.pas' {Form1};

{$R *.res}

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  Application.CreateForm(TForm1, Form1);
  Application.Run;
end.
