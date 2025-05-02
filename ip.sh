sudo dpdk-devbind.py -b=ice 0000:82:00.0
sleep 3
sudo ip addr add 192.168.1.152/24 dev enp130s0f0
sudo ip route add 192.168.1.151 dev enp130s0f0