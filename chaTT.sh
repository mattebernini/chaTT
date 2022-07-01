# make
gcc server.c "lib/chatlib.c" -o serv 
gcc device.c "lib/chatlib.c" -o dev 

read -p "Compilazione eseguita. Premi invio per eseguire..."

gnome-terminal -t "chaTT server" --window -x sh -c "./serv 4242; exec bash"

for port in {5001..5002}
do
    gnome-terminal -t "chaTT" --window -x sh -c "./dev $port; exec bash"
#    gnome-terminal --window -x sh -c "./dev $port; exec bash"
done