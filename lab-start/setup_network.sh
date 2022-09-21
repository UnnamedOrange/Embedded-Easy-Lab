# Setup IP.
ip address del 192.168.0.101/24 dev enx002427fe46a3
ip address add 192.168.0.101/24 dev enx002427fe46a3

# Start tftp.
service tftpd-hpa start

# Start NFS.
service nfs-kernel-server start
