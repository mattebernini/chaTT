make

read -p "Compilazione eseguita. Premi invio per eseguire..."

gnome-terminal -t "chaTT server" --window -- sh -c "./serv 4242; exec bash"

for port in {5001..5003}
do
    gnome-terminal -t "chaTT porta:$port" --window -- sh -c "./dev $port; exec bash"
done