#run:
#	g++ client-phase1.cpp -o client-phase1
#	g++ client-phase2.cpp -o client-phase2
#	g++ -Wall client-phase3.cpp -o client-phase3 -lcrypto
#	g++ client-phase4.cpp -o client-phase4
#	g++ -Wall client-phase5.cpp -o client-phase5 -lcrypto
run: client-phase1 client-phase2 client-phase3 client-phase4 client-phase5


client-phase1: client-phase1.cpp
	g++ client-phase1.cpp -o client-phase1

client-phase2: client-phase2.cpp
	g++ client-phase2.cpp -o client-phase2

client-phase3: client-phase3.cpp
	g++ -Wall client-phase3.cpp -o client-phase3 -lcrypto

client-phase4: client-phase4.cpp
	g++ client-phase4.cpp -o client-phase4

client-phase5: client-phase5.cpp
	g++ -Wall client-phase5.cpp -o client-phase5 -lcrypto



