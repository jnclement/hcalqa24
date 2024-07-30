One time after cloning:
1. do ```mkdir -p output/evt```
2. do ```mkdir -p output/images```
3. do ```mkdir -p output/out```
4. do ```mkdir -p output/err```
5. do ```mkdir -p output/img```
6. do ```chmod +x earlydata.sh```
7. do ```chmod +x quickroot.sh```

To compile:

1. Build your own copy of jetbase from coresoftware and modify it to ignore the lack of a global vertex. See ```/sphenix/user/jocl/projects/coresoftware/offline/packages/jetbase/TowerJetInput.cc``` for the way I did it. This washes out any z info in your jet, but this module doesn't use that anyway.
2. Build your own copy of CaloReco from coresoftware and modify it to set BadChi2 tower energies to 0. See ```/sphenix/user/jocl/projects/coresoftware/offline/packages/CaloReco/CaloTowerCalib.cc``` for the way I did it.
3. cd to your build directory and do ```[path_to_autogen]/autogen.sh --prefix=[your_install_directory]```
4. In your build directory, run ```make install```

Everything should compile nicely.


To create trees:

1. cd to ```[this_repo]/run```
2. In this directory, run ```grep jocl ./*``` and modify each file you see as appropriate to you
3. do ```bash getlist.sh [run number] [software zero suppressed]``` where run number is the run you want to look at, and the other argument is either 1 or 0 for software zero suppression on or off, respectively
4. Alternatively, do ```bash getlists.sh``` to get all of the runs for which DSTs are produced in the early data set
5. do ```bash run_runs.sh [tag]``` with some tag you want on all of your files to run all of the runs for which you have list files, or ```run_everything.sh [tag] [number of files from list] [run number] [software zero suppression]``` to run just one run

The tree files are located in ```output/evt```


To create images of your maybe jetty events:
1. do ```bash get_image_lists.sh``` to get a some list files for images that will concatenate runs into a TChain
2. do ```bash run_images_for_all.sh``` to run over all files in your ```output/evt``` directory, or ```bash run_images.sh [filename]``` with the base name (no pathing) of the file you want for a single file

The images are located in ```output/img```

If something goes wrong, error logs are in ```output/err```, and log files are in ```output/out```.
