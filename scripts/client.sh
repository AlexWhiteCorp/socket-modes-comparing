gcc src/client.c -o build/client.exe

for i in {1..2}
do
   ./build/client.exe -t unix -b true -e true &
done