# DocLynk
Blockchain document and certification transfering platform.

# Development
Currently verified changes in development are pushed to the master branch. Once finalized versions
are made the master branch will hold the newest Doclynk version. Previous version branches will also be
made to track version history.

# Testing
Testing during development is done on a kubernetes cluster with 64 containers which is the maximum number possible
with current chord network architecture. Running the k8sBuildNet.sh in the ops directory will compile the code and replicate the binaries to be ran on the testing network containers.