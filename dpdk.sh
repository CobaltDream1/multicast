sudo ifconfig enp130s0f0 down
sudo dpdk-devbind.py -b=vfio-pci 0000:82:00.0
