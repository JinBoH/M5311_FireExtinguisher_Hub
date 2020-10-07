::created by xgl,2019/1/24
@set temp_path=%path%
@set path=%path%;%~dp0\utils
@set param1=%1
@if NOT "%param1%"=="LV" if not "%param1%"== "CM" if not "%param1%" == "CLEAN" (goto error1)
@if %param1% == LV (goto make_lv) 
@if %param1% == CM (goto make_cm) 
@if %param1% == CLEAN (goto make_clean) 

:error1
@echo usage:build.bat param
@echo 	LV:compile LV binary
@echo 	CM:compile CM binary
@echo 	CLEAN:clean the project
@goto exit1
:make_lv
@if exist %~dp0\firmware\LV (goto cr1)
@md %~dp0\firmware\LV
@xcopy /y %~dp0\utils\flash_download.cfg %~dp0\firmware\LV >nul
@xcopy /y %~dp0\utils\mt2625_bootloader.bin %~dp0\firmware\LV  >nul
:cr1
@echo Making LV Version
@echo HWVER :=LV >%~dp0\utils\config.mk
@echo LINK_FILE:=link_option_lv.tmp >>%~dp0\utils\config.mk
@make
@goto exit1
:make_cm
@if  exist %~dp0\firmware\CM (goto cr2)
@md %~dp0\firmware\CM
@xcopy /y %~dp0\utils\flash_download.cfg %~dp0\firmware\CM >nul
@xcopy /y %~dp0\utils\mt2625_bootloader.bin %~dp0\firmware\CM  >nul
:cr2
@echo Making CM Version
@echo HWVER :=CM > %~dp0\utils\config.mk
@echo LINK_FILE:=link_option_cm.tmp >> %~dp0\utils\config.mk
@make
@goto exit1
:make_clean
@make  clean
@set path=%temp_path%
:exit1







