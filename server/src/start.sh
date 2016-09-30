export LD_LIBRARY_PATH=/local/data/DeepBlue/server/third_party/luajit-2.0/src/
(nohup ./server -A 0.0.0.0 -P 56573 -D deepblue_1_0_0_pre_2  -R 8 -T 8 -M localhost:27027 2>&1; echo \"PID $! exited with status $?\") >> deepblue.log
