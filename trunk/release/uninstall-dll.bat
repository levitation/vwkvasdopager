@echo off
if %PROCESSOR_ARCHITECTURE% == X86 (
  regsvr32 /u vwKvasdoPagerBandwin32.dll
) else (
  regsvr32 /u vwKvasdoPagerBandx64.dll
)
