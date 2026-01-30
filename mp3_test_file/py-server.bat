@echo off
echo ==============================
echo  Python HTTP 서버를 시작합니다.
echo.
echo  로컬 주소:
echo  http://localhost:8000
echo.
echo  종료하려면 Ctrl + C 를 누르세요.
echo ==============================
python -m http.server 8000
exit