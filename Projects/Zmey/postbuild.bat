set Root="%1"
set OutputDir="%2"
echo "Copying third parties..."
xcopy /S /Y "%Root%\ThirdParty\binx64" "%OutputDir%"
