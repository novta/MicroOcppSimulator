# Create  mo_simulator service

## Step 1: Create a Service File
```
sudo nano /etc/systemd/system/mo_simulator.service
```
## Step 2: Add following configuration to the Service File
```
[Unit]
Description=Mo Simulator Service
After=network.target

[Service]
Type=simple
ExecStart=/home/azadmin/MicroOcppSimulator/build/mo_simulator 	# Change to appropriate full executable path
WorkingDirectory=/home/azadmin/MicroOcppSimulator				# Change to appropriate full working directory path
Restart=always
RestartSec=5
User=azadmin			# Change to appropriate user
StandardOutput=syslog
StandardError=syslog

[Install]
WantedBy=multi-user.target
```
## Step 3: Enable and start Service
```
sudo systemctl daemon-reload
sudo systemctl enable mo_simulator
sudo systemctl start mo_simulator
```
## Step 4: Managing Your Service
```
# Start the service
sudo systemctl start mo_simulator

# Stop the service
sudo systemctl stop mo_simulator

# Check status
sudo systemctl status mo_simulator

# View logs
journalctl -u mo_simulator -f
```

# Important Considerations

## Make sure your executable has proper permissions:
```
chmod +x /home/azadmin/MicroOcppSimulator/build/mo_simulator
```
## Test the service thoroughly after setup:
```
sudo systemctl restart mo_simulator
sleep 5
sudo systemctl status mo_simulator
```
## Monitor logs regularly for issues:
```
journalctl -u mo_simulator -n 100 --no-pager
```