@echo off
echo Installing PaddleOCR for enhanced OCR capabilities...
echo.

REM Install PaddlePaddle (CPU version)
pip install paddlepaddle

REM Install PaddleOCR
pip install paddleocr

echo.
echo PaddleOCR installation completed!
echo You can now use enhanced OCR with Russian language support.
echo.

pause