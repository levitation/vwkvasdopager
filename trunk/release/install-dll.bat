@echo off
if %PROCESSOR_ARCHITECTURE% == X86 (
  regsvr32 vwKvasdoPagerBandwin32.dll
) else (
  if %PROCESSOR_ARCHITECTURE% == x86 (
    regsvr32 vwKvasdoPagerBandwin32.dll
  ) else (
    regsvr32 vwKvasdoPagerBandx64.dll
  )
)
