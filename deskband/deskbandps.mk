
deskbandps.dll: dlldata.obj deskband_p.obj deskband_i.obj
	link /dll /out:deskbandps.dll /def:deskbandps.def /entry:DllMain dlldata.obj deskband_p.obj deskband_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del deskbandps.dll
	@del deskbandps.lib
	@del deskbandps.exp
	@del dlldata.obj
	@del deskband_p.obj
	@del deskband_i.obj
