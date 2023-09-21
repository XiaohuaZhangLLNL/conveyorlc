# Spack

Build the container:

```bash
$ docker build -t clc .
```

And shell inside 🐚️:

```bash
$ docker run -it clc bash
```

Activate the spack environment with all the important stuff.

```bash
cd /opt/spack-environment
. /opt/spack-environment/spack/share/spack/setup-env.sh
spack env activate .
```

Let's start a flux mini cluster 🌀️: 

```bash
flux start --test-size=4
```

As a sanity check, you should have spack on your path, and the view provided by the environment.
You should be able to find these executables:

```bash
# ambertools
$ which sander 

# conveyorlc
$ which CDT1Receptor
```
And of course we already started flux (also installed in the spack environment)
Next, let's run the example! Go to `/code/examples`, the root
where you want to run them from. 

```bash
cd /code/examples
```
and then export some paths.

```bash
export CONVEYORLCHOME=$(spack find --format "{prefix}" conveyorlc)
export LBindData=$CONVEYORLCHOME/data
export AMBERHOME=/opt/conda
```

If the spack find doesn't work (once for me it did not) just manually find the path to
conveyorlc first. E.g.,:

```bash
export CONVEYORLCHOME=/opt/software/linux-ubuntu20.04-x86_64/gcc-9.4.0/conveyorlc-pancakes-yoyyryn6raxcptak5imtcj7cd3fjsgu4/
```

And now we are ready to run with flux! The `-n 2` for processes and option for mpi is important - it won't work without them.
Also note that if you have a "scratch" in your examples folder you should remove it entirely first.

```bash
cd /code/examples
rm -rf ./scratch
flux mini run -n 2 -ompi=openmpi@5 CDT1Receptor --input  pdb.list --output out --version 16 --spacing 1.4 --minimize on --forceRedo on
```

If all is well, you should see the start of the run look like this:

```bash
WARNING: atom HNAI from HID will be treated as hydrogen
WARNING: atom HNAI from HID will be treated as hydrogen
WARNING: atom HAHA from HID will be treated as hydrogen
WARNING: atom HAHA from HID will be treated as hydrogen
WARNING: atom HAHA from HID will be treated as hydrogen
mkdir -p /code/examples/scratch/rec/sarinXtalnAChE_A return normally exit code 0
cp /code/examples/pdb/sarinXtalnAChE_A.pdb /code/examples/scratch/rec/sarinXtalnAChE_A return normally exit code 0
reduce -Quiet -Trim  rec_AForm.pdb > rec_noh.pdb  return normally exit code 0
reduce -Quiet -BUILD rec_noh.pdb -DB "/opt/software/linux-ubuntu20.04-x86_64/gcc-9.4.0/conveyorlc-master-sl5hzatz6k3qsqcjxsmyktsudou7tges/data/amber16_reduce_wwPDB_het_dict.txt" > rec_rd.pdb return normally exit code 1
 CYX 66
 CYX 93
 CYX 254
 CYX 269
 CYX 406
 CYX 526
 CYS 66
 CYS 93
 CYS 254
 CYS 269
 CYS 406
 CYS 526
...
```

On a Linux machine it should take 45-60 minutes. On a Mac, well come back tomorrow! 🌓️
Note that the other examples in the directory haven't been modified to run here, nor have they
been tested.
