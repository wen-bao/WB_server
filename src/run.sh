echo "make WB_server ====>"
$(make clean)
$(make)
echo "solve daemon ====>"
$(gcc WBrun.c -o WBrun)
echo "running ====>"
$(./WBrun)

