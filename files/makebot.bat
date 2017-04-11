@echo off
echo Bot making utility. Usage makebot sourcefile executablename
cl -w -nologo -DWIN32 -TP /Fe%2 %1 /link wsock32.lib aibots.obj OsDeps.obj

