:: This demo launches 3 servers, and one listener that is listening at server 3
:: One client will send a message in server 1 and the message should appear in client_1 (echo) and client_2 (broadcasted) console once.

:: Run 3 servers
start Server.exe --id server_1 --port 9000 --peers ../peers.txt --type 1
start Server.exe --id server_2 --port 9002 --peers ../peers.txt --type 2
start Server.exe --id server_3 --port 9004 --peers ../peers.txt --type 0

::Add thread delay to make sure peers are connectecd to each other before we start the clients
echo "Launching Clients in 5 seconds to make sure servers are already connected"
TIMEOUT 5

:: Run Listener clients
start Client.exe --id client_2 --host localhost --port 9004 --listener

:: make sure listener established its connections first
TIMEOUT 2

:: Run clients connected to each servers
start Client.exe --id client_1 --host localhost --port 9000 --frequency 1 --delay 0 --message "Hello from client 1"


