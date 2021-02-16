echo Building vidReader...
mkdir -p ./bin/lin
g++ -I. bayer.cpp VidHeader.cpp -lz read-frame.cpp main.cpp showErrMsgBox.cpp -o ./bin/lin/vidReader

echo Done
