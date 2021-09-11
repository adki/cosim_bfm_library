@ECHO OFF

IF EXIST __init__.pyc  DEL /Q __init__.pyc
IF EXIST cosim_bfm.pyc DEL /Q cosim_bfm.pyc

@FOR /D %%I in (*) DO @(
	@IF EXIST %%I\Clean.bat (
		@PUSHD %%I
		@CALL .\Clean.bat
		@POPD
	)
)
